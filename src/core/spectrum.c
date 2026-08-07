/* Per-window vibration spectra from tracked displacement series. */

#include "resonarsat/microm.h"
#include "resonarsat/fft.h"

#include <float.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

/* Release every array a spectrum result owns. */
void rs_spectrum_free(rs_spectrum_t *s)
{
    if (!s) return;
    free(s->psd);
    free(s->freq);
    free(s->dominant_freq);
    free(s->amplitude);
    free(s->excursion_px);
    free(s->quality);
    free(s->prominence);
    free(s->snr);
    free(s->sigma_px);
    free(s->d_a);
    memset(s, 0, sizeof *s);
}

/* Hann-windowed periodogram of each window's line-of-sight displacement series.
 *
 * Three choices worth stating. A least-squares straight line is removed first,
 * not merely the mean. Removing the mean alone leaves any linear trend in the
 * record, and a trend is not a small effect here: temporal phase unwrapping
 * performs a random walk when the phase is noisy, which produces exactly such a
 * ramp and puts all the energy in the lowest bin. Every window would then report
 * the same spurious "dominant frequency" of one bin width, which looks like a
 * measurement and is not.
 * A Hann window is applied because the record is short and unwindowed leakage
 * from a strong low-frequency component would otherwise swamp weaker peaks. And
 * the zero bin is excluded when the dominant frequency is chosen, since a
 * residual trend is not a vibration mode.
 *
 * A plain periodogram is the right estimator at this record length. The source
 * papers invoke convex and atomic-norm methods that resolve closely spaced
 * peaks better, but with a few dozen samples those mostly offer new ways to
 * read structure into noise; a higher-resolution estimator belongs here only
 * once the record length justifies it. */
resonarsat_status_t rs_spectrum_compute(const rs_microm_t *m,
                                       rs_spectrum_source_t source,
                                       rs_spectrum_t *out)
{
    return rs_spectrum_compute_band(m, source, 0.0, out);
}

/* Spectra with a band floor. See the header. */
resonarsat_status_t rs_spectrum_compute_band(const rs_microm_t *m,
                                            rs_spectrum_source_t source,
                                            double f_min,
                                            rs_spectrum_t *out)
{
    return rs_spectrum_compute_opts(m, source, f_min, RS_DETREND_LINEAR, out);
}

/* Spectra with the detrend selectable. See the header on why it is. */
resonarsat_status_t rs_spectrum_compute_opts(const rs_microm_t *m,
                                             rs_spectrum_source_t source,
                                             double f_min,
                                             rs_detrend_t detrend,
                                             rs_spectrum_t *out)
{
    /* Zeroed before any validation can fail, so that "check the status, then
     * free the struct" is safe on every path rather than on most of them.
     * rs_cphd_read() carried the non-uniform version of this and aborted the
     * first time a corpus made it fail on real data. */
    memset(out, 0, sizeof *out);

    if (!m || !out || !m->disp_los || !m->vel_los) return RS_ERR_ARG;
    if (m->n_looks < 4) {
        rs_set_error("spectrum: %zu looks is too few to estimate a spectrum", m->n_looks);
        return RS_ERR_ARG;
    }
    if (!(m->dt > 0.0)) {
        rs_set_error("spectrum: sub-look sampling interval is unset");
        return RS_ERR_MISSING_META;
    }

    const size_t n = m->n_looks;
    const size_t n_freq = n / 2 + 1;      /* one-sided spectrum */
    const double fs = 1.0 / m->dt;

    out->psd           = calloc(m->n_win * n_freq, sizeof *out->psd);
    out->freq          = calloc(n_freq, sizeof *out->freq);
    out->dominant_freq = calloc(m->n_win, sizeof *out->dominant_freq);
    out->amplitude     = calloc(m->n_win, sizeof *out->amplitude);
    out->excursion_px  = calloc(m->n_win, sizeof *out->excursion_px);
    out->quality       = calloc(m->n_win, sizeof *out->quality);
    out->prominence    = calloc(m->n_win, sizeof *out->prominence);
    out->snr           = calloc(m->n_win, sizeof *out->snr);
    out->sigma_px      = calloc(m->n_win, sizeof *out->sigma_px);
    out->d_a           = calloc(m->n_win, sizeof *out->d_a);
    if (!out->psd || !out->freq || !out->dominant_freq || !out->amplitude ||
        !out->quality || !out->prominence || !out->excursion_px ||
        !out->snr || !out->sigma_px || !out->d_a) {
        rs_spectrum_free(out);
        rs_set_error("spectrum: cannot allocate %zu windows x %zu frequency bins",
                     m->n_win, n_freq);
        return RS_ERR_ALLOC;
    }

    out->n_win = m->n_win;
    out->n_win_az = m->n_win_az;
    out->n_win_rg = m->n_win_rg;
    out->n_freq = n_freq;
    out->df = fs / (double)n;
    /* Carried through so rs_spectrum_best_window() can evaluate the floor
     * without the caller having to pass the tracking parameters again. */
    out->quant_px = m->quant_px;
    /* And the correlation-surface null, which is what makes the SNRs below
     * interpretable. Zero for the estimators that produce none. */
    out->snr_null = m->snr_null;

    for (size_t k = 0; k < n_freq; k++) out->freq[k] = (double)k * out->df;

    /* Precompute the window and its power, so the periodogram is normalised
     * consistently regardless of record length. */
    double *win = malloc(n * sizeof *win);
    float complex *buf = malloc(n * sizeof *buf);
    rs_fft_plan *plan = NULL;
    resonarsat_status_t st = RS_OK;

    if (!win || !buf || (st = rs_fft_plan_create(n, &plan)) != RS_OK) {
        free(win); free(buf); rs_fft_plan_destroy(plan);
        rs_spectrum_free(out);
        return (st == RS_OK) ? RS_ERR_ALLOC : st;
    }

    double win_power = 0.0;
    for (size_t i = 0; i < n; i++) {
        win[i] = 0.5 * (1.0 - cos(2.0 * M_PI * (double)i / (double)(n - 1)));
        win_power += win[i] * win[i];
    }
    if (win_power <= 0.0) win_power = 1.0;

    const double *all = (source == RS_SPEC_DISPLACEMENT) ? m->disp_los : m->vel_los;

    /* Sums that depend only on the sample index, hoisted out of the window
     * loop: the abscissa is the same for every window. */
    const double sx  = 0.5 * (double)n * (double)(n - 1);
    const double sxx = (double)(n - 1) * (double)n * (2.0 * (double)(n - 1) + 1.0) / 6.0;
    const double det = (double)n * sxx - sx * sx;

    for (size_t w = 0; w < m->n_win; w++) {
        const double *series = all + w * n;

        /* Peak-to-peak of the TRACKED SHIFT, in pixels, before any detrending.
         * Not of 'series': that may be a velocity or a line-of-sight distance
         * depending on the observable, and the quantisation floor it is
         * compared against lives in pixels. Taken raw because detrending can
         * only shrink an excursion, and the question here is what the
         * correlator resolved, not what survives conditioning. */
        {
            double lo = m->disp_az[w * n], hi = lo;
            for (size_t i = 1; i < n; i++) {
                const double v = m->disp_az[w * n + i];
                if (v < lo) lo = v;
                if (v > hi) hi = v;
            }
            out->excursion_px[w] = hi - lo;
        }

        /* Fit y = a + b*i and subtract as much of it as the caller asked for.
         * RS_DETREND_NONE leaves a == b == 0, so the same expression below
         * covers every case without branching inside the sample loop. */
        double sy = 0.0, sxy = 0.0;
        for (size_t i = 0; i < n; i++) {
            sy  += series[i];
            sxy += (double)i * series[i];
        }
        double a = 0.0, b = 0.0;
        if (detrend == RS_DETREND_LINEAR && det != 0.0) {
            b = ((double)n * sxy - sx * sy) / det;
            a = (sy - b * sx) / (double)n;
        } else if (detrend == RS_DETREND_LINEAR || detrend == RS_DETREND_MEAN) {
            a = sy / (double)n;
        }

        for (size_t i = 0; i < n; i++) {
            buf[i] = (float)((series[i] - (a + b * (double)i)) * win[i]);
        }

        if (rs_fft_forward(plan, buf) != RS_OK) continue;

        double *psd = out->psd + w * n_freq;
        for (size_t k = 0; k < n_freq; k++) {
            const double mag = (double)cabsf(buf[k]);
            /* Scale to a one-sided density: double the interior bins, and
             * normalise by the window power and the sampling rate. */
            const double scale = (k == 0 || (n % 2 == 0 && k == n_freq - 1)) ? 1.0 : 2.0;
            psd[k] = scale * mag * mag / (win_power * fs);
        }

        /* Dominant peak, skipping the zero bin and anything below the band
         * floor. k_min is computed once per window rather than hoisted only for
         * clarity; it costs nothing beside the transform. */
        size_t k_min = RS_SPECTRUM_LEAKAGE_BINS;
        if (f_min > 0.0 && out->df > 0.0) {
            const double kf = ceil(f_min / out->df);
            if (kf > (double)k_min) k_min = (size_t)kf;
        }
        if (k_min >= n_freq) k_min = n_freq - 1;
        size_t best = k_min;
        for (size_t k = k_min + 1; k < n_freq; k++) if (psd[k] > psd[best]) best = k;

        out->dominant_freq[w] = out->freq[best];
        out->amplitude[w] = sqrt(psd[best]);   /* qualitative -- see the header */
        out->quality[w] = m->quality[w];
        /* Guarded because a caller may assemble an rs_microm_t by hand -- the
         * tests do -- and the surface statistics are optional in a way the
         * displacement series is not. Absent, they stay zero, which
         * rs_spectrum_ampcor_window() reads through 'snr_null' as "this
         * estimator has no surface" rather than as a window that failed. */
        if (m->snr)      out->snr[w] = m->snr[w];
        if (m->sigma_px) out->sigma_px[w] = m->sigma_px[w];
        out->d_a[w] = m->d_a ? m->d_a[w] : RS_DA_MAX;

        /* Prominence: peak power against the mean of the rest. A window holding
         * a vibrating target concentrates its energy in one bin; a window of
         * tracking noise spreads it evenly, giving a prominence near 1. */
        double sum = 0.0;
        for (size_t k = k_min; k < n_freq; k++) sum += psd[k];
        const double mean_power = (n_freq > k_min)
                                ? sum / (double)(n_freq - k_min) : 0.0;
        out->prominence[w] = (mean_power > 0.0) ? psd[best] / mean_power : 0.0;
    }

    rs_fft_plan_destroy(plan);
    free(win);
    free(buf);
    return RS_OK;
}


/* Window with the most prominent spectral peak, among those that actually
 * tracked something.
 *
 * The quality gate is not optional, and the failure it prevents is instructive.
 * On a scene where most windows contain empty background, those windows track
 * nothing; their displacement series is a slow drift, which concentrates almost
 * all its energy in the lowest frequency bin and therefore scores a HIGHER
 * prominence than a genuine vibrating target whose energy is spread by noise
 * across neighbouring bins. Ranking on prominence alone hands the answer to the
 * emptiest part of the scene, reporting a confident-looking sub-bin frequency
 * from a window with nothing in it.
 *
 * The gate is relative rather than absolute -- windows must reach half the best
 * tracking coherence present in this scene -- because absolute coherence depends
 * on scene content, and an isolated point target on empty background scores far
 * lower than distributed clutter however well it tracks. */
resonarsat_status_t rs_spectrum_best_window(const rs_spectrum_t *spec,
                                            size_t *out_window,
                                            double *out_prominence,
                                            size_t *out_n_candidates)
{
    if (out_n_candidates) *out_n_candidates = 0;
    if (!spec || !spec->prominence || !spec->quality || spec->n_win == 0)
        return RS_ERR_ARG;

    double q_max = 0.0;
    for (size_t w = 0; w < spec->n_win; w++) {
        if (spec->quality[w] > q_max) q_max = spec->quality[w];
    }
    const double q_min = 0.5 * q_max;

    /* Three sigma of the quantisation noise, in pixels. See the header for the
     * derivation; zero disables the floor for estimators it does not describe. */
    const double floor_px = (spec->quant_px > 0.0) ? 2.449 * spec->quant_px : 0.0;

    size_t best = spec->n_win;
    size_t n_cand = 0;
    for (size_t w = 0; w < spec->n_win; w++) {
        if (spec->quality[w] < q_min) continue;
        /* The coherence gate above is necessary and not sufficient: a window
         * that never moved correlates with itself perfectly and clears it at
         * the top. This is the gate that removes it. */
        if (floor_px > 0.0 && spec->excursion_px &&
            spec->excursion_px[w] < floor_px) continue;
        n_cand++;
        if (best == spec->n_win || spec->prominence[w] > spec->prominence[best]) best = w;
    }
    if (out_n_candidates) *out_n_candidates = n_cand;

    if (best == spec->n_win) {
        /* Nothing resolved motion above the tracker's own resolution. Reported
         * rather than papered over: the previous behaviour fell back to window
         * zero, which handed the caller a frequency and a prominence computed
         * from rounding noise and no way to tell. An absent measurement and a
         * wrong one are not the same answer. */
        rs_set_error("spectrum: no window resolved motion above the %g px "
                     "quantisation floor (3 sigma of 1/%g px); the scene may "
                     "hold no detectable motion, or the sub-pixel refinement "
                     "may be too coarse for the excursion present",
                     floor_px, (spec->quant_px > 0.0) ? 1.0 / spec->quant_px : 0.0);
        return RS_ERR_RANGE;
    }

    if (out_window)     *out_window = best;
    if (out_prominence) *out_prominence = spec->prominence[best];
    return RS_OK;
}
/* The frequency the most windows agree on. See microm.h. */
resonarsat_status_t rs_spectrum_consensus(const rs_spectrum_t *spec,
                                          double *out_freq,
                                          size_t *out_n_agree,
                                          size_t *out_n_distinct,
                                          size_t *out_n_voting,
                                          size_t *out_n_contiguous)
{
    if (out_freq)         *out_freq = 0.0;
    if (out_n_agree)      *out_n_agree = 0;
    if (out_n_distinct)   *out_n_distinct = 0;
    if (out_n_voting)     *out_n_voting = 0;
    if (out_n_contiguous) *out_n_contiguous = 0;
    if (!spec || !spec->prominence || !spec->quality || !spec->dominant_freq ||
        spec->n_win == 0)
        return RS_ERR_ARG;

    /* The same two gates rs_spectrum_best_window() applies, so that the counts
     * the two functions return describe one population. Duplicated rather than
     * factored out because the gates are the contract here, not an
     * implementation detail: a change to one that silently changed the other
     * would make the counts incomparable without anything saying so. */
    double q_max = 0.0;
    for (size_t w = 0; w < spec->n_win; w++) {
        if (spec->quality[w] > q_max) q_max = spec->quality[w];
    }
    const double q_min = 0.5 * q_max;
    const double floor_px = (spec->quant_px > 0.0) ? 2.449 * spec->quant_px : 0.0;

    /* Half a bin, so two windows agree exactly when they picked the same bin.
     * Falls back to an absolute tolerance if the axis carries no spacing. */
    const double tol = (spec->df > 0.0) ? 0.5 * spec->df : 1e-9;

    size_t n_vote = 0, best_count = 0, n_distinct = 0;
    double best_freq = 0.0;

    for (size_t w = 0; w < spec->n_win; w++) {
        if (spec->quality[w] < q_min) continue;
        if (floor_px > 0.0 && spec->excursion_px &&
            spec->excursion_px[w] < floor_px) continue;
        n_vote++;

        /* Count how many gated windows share this one's bin. O(n_win^2), which
         * is nothing beside the tracking that produced the spectrum: n_win is
         * tens to low thousands and each comparison is a subtraction. */
        size_t count = 0;
        int seen_earlier = 0;
        for (size_t v = 0; v < spec->n_win; v++) {
            if (spec->quality[v] < q_min) continue;
            if (floor_px > 0.0 && spec->excursion_px &&
                spec->excursion_px[v] < floor_px) continue;
            if (fabs(spec->dominant_freq[v] - spec->dominant_freq[w]) <= tol) {
                count++;
                /* Distinct values are counted once, at their first occurrence. */
                if (v < w) seen_earlier = 1;
            }
        }
        if (!seen_earlier) n_distinct++;

        /* Ties go to the lower frequency, which is arbitrary but fixed: an
         * unstable choice would make the count meaningful and the frequency
         * beside it not. */
        if (count > best_count ||
            (count == best_count && best_count > 0 &&
             spec->dominant_freq[w] < best_freq)) {
            best_count = count;
            best_freq  = spec->dominant_freq[w];
        }
    }

    if (out_n_voting)   *out_n_voting = n_vote;
    if (out_n_distinct) *out_n_distinct = n_distinct;

    if (n_vote == 0) {
        rs_set_error("spectrum: no window passed the coherence gate and the "
                     "quantisation floor, so there is nothing to take a "
                     "consensus over");
        return RS_ERR_RANGE;
    }

    if (out_freq)    *out_freq = best_freq;
    if (out_n_agree) *out_n_agree = best_count;

    /* Largest 4-connected block of agreeing windows on the window grid.
     *
     * Iterative flood fill with an explicit stack rather than recursion: n_win
     * runs to thousands and a scattered agreement set would otherwise recurse
     * as deep as it is large. Allocation failure leaves the count at zero and
     * the rest of the answer intact, since contiguity refines the result rather
     * than constituting it. */
    if (out_n_contiguous && spec->n_win_az > 0 && spec->n_win_rg > 0 &&
        spec->n_win_az * spec->n_win_rg == spec->n_win) {
        unsigned char *mark = calloc(spec->n_win, 1);
        size_t *stack = malloc(spec->n_win * sizeof *stack);
        if (mark && stack) {
            for (size_t w = 0; w < spec->n_win; w++) {
                if (spec->quality[w] < q_min) continue;
                if (floor_px > 0.0 && spec->excursion_px &&
                    spec->excursion_px[w] < floor_px) continue;
                if (fabs(spec->dominant_freq[w] - best_freq) <= tol) mark[w] = 1;
            }
            size_t largest = 0;
            for (size_t seed = 0; seed < spec->n_win; seed++) {
                if (mark[seed] != 1) continue;
                size_t top = 0, size = 0;
                stack[top++] = seed; mark[seed] = 2;
                while (top > 0) {
                    const size_t c = stack[--top];
                    size++;
                    const size_t ia = c / spec->n_win_rg;
                    const size_t ir = c % spec->n_win_rg;
                    /* four neighbours on the window lattice */
                    const long da[4] = { -1, 1, 0, 0 };
                    const long dr[4] = { 0, 0, -1, 1 };
                    for (int k = 0; k < 4; k++) {
                        const long na = (long)ia + da[k];
                        const long nr = (long)ir + dr[k];
                        if (na < 0 || nr < 0 ||
                            (size_t)na >= spec->n_win_az ||
                            (size_t)nr >= spec->n_win_rg) continue;
                        const size_t nb = (size_t)na * spec->n_win_rg + (size_t)nr;
                        if (mark[nb] == 1) { mark[nb] = 2; stack[top++] = nb; }
                    }
                }
                if (size > largest) largest = size;
            }
            *out_n_contiguous = largest;
        }
        free(mark); free(stack);
    }
    return RS_OK;
}


/* Windows agreeing with one nominated frequency, and their largest block.
 *
 * See microm.h. Shares rs_spectrum_consensus()'s gates and flood fill; what
 * differs is that the frequency is given rather than won by plurality. */
resonarsat_status_t rs_spectrum_block_at(const rs_spectrum_t *spec, double freq_hz,
                                         size_t *out_n_agree, size_t *out_n_block)
{
    if (out_n_agree) *out_n_agree = 0;
    if (out_n_block) *out_n_block = 0;
    if (!spec || !spec->dominant_freq || !spec->quality || spec->n_win == 0)
        return RS_ERR_ARG;

    double q_max = 0.0;
    for (size_t w = 0; w < spec->n_win; w++)
        if (spec->quality[w] > q_max) q_max = spec->quality[w];
    const double q_min = 0.5 * q_max;
    const double floor_px = (spec->quant_px > 0.0) ? 2.449 * spec->quant_px : 0.0;
    const double tol = (spec->df > 0.0) ? 0.5 * spec->df : 1e-9;

    unsigned char *mark = calloc(spec->n_win, 1);
    if (!mark) {
        rs_set_error("spectrum: cannot allocate a %zu-window agreement mask",
                     spec->n_win);
        return RS_ERR_ALLOC;
    }
    size_t n_agree = 0;
    for (size_t w = 0; w < spec->n_win; w++) {
        if (spec->quality[w] < q_min) continue;
        if (floor_px > 0.0 && spec->excursion_px &&
            spec->excursion_px[w] < floor_px) continue;
        if (fabs(spec->dominant_freq[w] - freq_hz) <= tol) { mark[w] = 1; n_agree++; }
    }
    if (out_n_agree) *out_n_agree = n_agree;

    if (out_n_block && spec->n_win_az > 0 && spec->n_win_rg > 0 &&
        spec->n_win_az * spec->n_win_rg == spec->n_win) {
        size_t *stk = malloc(spec->n_win * sizeof *stk);
        if (stk) {
            size_t largest = 0;
            for (size_t seed = 0; seed < spec->n_win; seed++) {
                if (mark[seed] != 1) continue;
                size_t top = 0, size = 0;
                stk[top++] = seed; mark[seed] = 2;
                while (top > 0) {
                    const size_t c = stk[--top];
                    size++;
                    const size_t ia = c / spec->n_win_rg, ir = c % spec->n_win_rg;
                    const long da[4] = { -1, 1, 0, 0 }, dr[4] = { 0, 0, -1, 1 };
                    for (int k = 0; k < 4; k++) {
                        const long na = (long)ia + da[k], nr = (long)ir + dr[k];
                        if (na < 0 || nr < 0 || (size_t)na >= spec->n_win_az ||
                            (size_t)nr >= spec->n_win_rg) continue;
                        const size_t nb = (size_t)na * spec->n_win_rg + (size_t)nr;
                        if (mark[nb] == 1) { mark[nb] = 2; stk[top++] = nb; }
                    }
                }
                if (size > largest) largest = size;
            }
            *out_n_block = largest;
            free(stk);
        }
    }
    free(mark);
    return RS_OK;
}

/* Multiple of the noise-alone SNR a window must reach to enter the cull.
 *
 * The null itself is derived (rs_coreg_snr_null); this factor is not. It was
 * originally chosen on the argument that a surface failing to stand at twice
 * what an empty surface produces has not distinguished itself from one, which
 * was a plausible sentence and not evidence. It has since been SWEPT, over one
 * set of spectra so that the threshold is the only thing varying --
 * tests/test_cullsweep.c, and FOLLOW-UPS.md item 12d:
 *
 *   factor  gate   clutter: answers/correct/distinct   static answers
 *    0.00    0.0            8 / 7 / 4                   1   DISQUALIFIED
 *    1.00    7.5            7 / 6 / 4                   1   DISQUALIFIED
 *    1.25    9.4            6 / 5 / 3                   0
 *    1.50   11.3            6 / 5 / 3                   0
 *    1.75   13.1            5 / 5 / 2                   0
 *    2.00   15.0            5 / 5 / 2                   0
 *    2.50   18.8            5 / 5 / 2                   0
 *    3.00   22.5            5 / 5 / 2                   0
 *
 * TWO THINGS THE SWEEP ESTABLISHED, neither of which was known before it.
 *
 * The gate is LOAD-BEARING. Disabled, and set exactly at the null, the cull
 * answers on a scene where nothing moves. That false positive is the failure the
 * whole policy exists to avoid, and it appears the moment the factor reaches the
 * null and not before. So the boundary is measured, and it falls where the
 * derivation said it should.
 *
 * And 2.0 sits on a PLATEAU rather than at an edge. Every factor from 1.75 to
 * 3.0 gives identical counts, so the choice among them costs nothing and buys
 * nothing -- which is the only honest reason to leave a tuned constant where it
 * is. Below 1.75 recall improves by one answer and one distinct injection at the
 * price of a WRONG one, taking the rms from 0.0035 Hz to 0.85; that is a real
 * trade and it is available to anyone who wants it through
 * rs_spectrum_ampcor_window_opts(), which exists so this constant can be
 * measured against rather than argued about.
 *
 * The gate actually applied is reported in rs_spectrum_cull_t.snr_gate, so no
 * result depends on a reader knowing this number. */
#define RS_CULL_SNR_FACTOR 2.0

/* How many times the scene's median offset uncertainty a window may carry.
 *
 * THIS GATE WAS ABSOLUTE AND THAT WAS WRONG. It read "excursion >= 3*sigma",
 * comparing a peak-to-peak excursion in real pixels against sigma -- and
 * rs_coreg_quality_t states in terms that could not be plainer that sigma is NOT
 * CALIBRATED IN AN ABSOLUTE SENSE, because the estimator omits the patch's
 * independent-sample count. Using an explicitly uncalibrated quantity in a
 * calibrated comparison is exactly the dimensional error this codebase's comment
 * rule exists to prevent, and it was measured: on an isolated point target with
 * surface SNRs near 80 -- ten times the noise-alone value, so gate 1 culled
 * nothing at all -- sigma came out at 130 to 200 PIXELS on 32-pixel patches
 * against excursions of 10 to 18 px, and the gate removed 100% of windows at
 * every frequency of a sweep. See FOLLOW-UPS.md item 12c.
 *
 * The relative form is what the quantity actually supports, and it is also what
 * the ampcor family does: those cullers reject offsets deviating from a local
 * median rather than from a fixed bound. A window is culled when its offset
 * uncertainty is more than twice the median of the windows entering the cull.
 *
 * The median rather than the mean, because one hopeless window returning
 * RS_COREG_SIGMA_MAX would drag a mean past every real one and cull the scene.
 * The factor of two is tuned, like the coherence gate's one half and for the
 * same reason -- an absolute scale does not exist here -- and it is reported in
 * rs_spectrum_cull_t.sigma_gate so that no result depends on knowing it.
 *
 * The excursion is not left unguarded by the change: rs_spectrum_best_window()'s
 * quantisation floor is applied before this and is the test that a window moved
 * further than the tracker's own grid step. That one IS calibrated. */
#define RS_CULL_SIGMA_FACTOR 2.0

/* Agreeing four-neighbours a window needs to survive the consistency cull.
 *
 * Geometric, not tuned: each cell of a 2x2 block has exactly two of its four
 * lattice neighbours inside the block, and a 2x2 block is the smallest footprint
 * a resolvable target can occupy given that windows overlap at the tracking
 * stride. See rs_spectrum_ampcor_window() in microm.h.
 *
 * SWEPT, AND THE DERIVED VALUE IS EXACTLY THE BOUNDARY. Over one set of spectra
 * -- tests/test_cullsweep.c, FOLLOW-UPS.md item 12e:
 *
 *   min nbrs   clutter ans/correct/distinct   isolated   static answers
 *      0             18 / 12 / 6                6 / 3      3   DISQUALIFIED
 *      1             15 / 11 / 6                4 / 2      1   DISQUALIFIED
 *      2              5 /  5 / 2                2 / 2      0
 *      3              0 /  0 / 0                0 / 0      0
 *      4              0 /  0 / 0                0 / 0      0
 *
 * Below two, a scene with nothing moving gets an answer -- at zero it gets one
 * at every seed. Above two, nothing gets an answer at all. So two is not merely
 * a defensible choice on a plateau, the way gate 1's factor is: it is the only
 * value that both refuses every static control and answers anything, and the
 * geometric argument predicted that boundary before it was measured.
 *
 * THE PRICE IS RECALL AND IT IS SEVERE. This gate is what holds the cull at 5
 * answers of 18 -- disabled it answers all 18, and 12 of those are correct
 * across all six distinct injections. The coverage a fit needs therefore EXISTS
 * in the tracking, and it exists only together with static false positives.
 * That is a statement about the operating point rather than about this constant:
 * at these coherences the chain cannot both answer across the band and refuse a
 * motionless scene, and no setting of this threshold changes that. */
#define RS_CULL_MIN_NEIGHBOURS 2

/* Select by culling on what the correlator knew, at the default factors.
 * See microm.h. */
resonarsat_status_t rs_spectrum_ampcor_window(const rs_spectrum_t *spec,
                                              unsigned char *out_state,
                                              rs_spectrum_cull_t *out)
{
    return rs_spectrum_ampcor_window_opts(spec, RS_CULL_SNR_FACTOR,
                                          RS_CULL_SIGMA_FACTOR,
                                          RS_CULL_MIN_NEIGHBOURS, out_state, out);
}

/* Select by culling, with both tuned factors supplied. See microm.h. */
resonarsat_status_t rs_spectrum_ampcor_window_opts(const rs_spectrum_t *spec,
                                                   double snr_factor,
                                                   double sigma_factor,
                                                   size_t min_neighbours,
                                                   unsigned char *out_state,
                                                   rs_spectrum_cull_t *out)
{
    if (!out) return RS_ERR_ARG;
    memset(out, 0, sizeof *out);
    if (!spec || !spec->dominant_freq || !spec->quality || spec->n_win == 0)
        return RS_ERR_ARG;
    /* Cleared before any early return, so a caller that allocated the buffer
     * sees "nothing entered" rather than whatever was on its stack. */
    if (out_state) memset(out_state, 0, spec->n_win);
    out->window = spec->n_win;

    /* The shared gates, in the same form rs_spectrum_best_window() and
     * rs_spectrum_consensus() apply them. Duplicated for the reason stated on
     * the consensus: the gates are the contract that makes the three functions'
     * counts describe one population, so a change to one that silently changed
     * the others would make them incomparable with nothing saying so. */
    double q_max = 0.0;
    for (size_t w = 0; w < spec->n_win; w++) {
        if (spec->quality[w] > q_max) q_max = spec->quality[w];
    }
    const double q_min = 0.5 * q_max;
    const double floor_px = (spec->quant_px > 0.0) ? 2.449 * spec->quant_px : 0.0;

    const int have_surface = (spec->snr_null > 0.0 && spec->snr && spec->sigma_px);
    out->gates_applied = have_surface;
    out->snr_gate = (have_surface && snr_factor > 0.0)
                  ? snr_factor * spec->snr_null : 0.0;

    const double tol = (spec->df > 0.0) ? 0.5 * spec->df : 1e-9;

    /* 0 = did not enter, 1 = entered but culled by gate 1 or 2, 2 = candidate
     * for gate 3, 3 = survived all three. Separate passes are needed because
     * gate 3 reads its neighbourhood, so no window can be decided until every
     * window's membership is known. */
    unsigned char *state = calloc(spec->n_win, 1);
    if (!state) {
        rs_set_error("spectrum: cannot allocate the cull state for %zu windows",
                     spec->n_win);
        return RS_ERR_ALLOC;
    }

    /* Which windows enter, and the median offset uncertainty among them.
     *
     * The median has to be taken over the ENTRANTS rather than over every
     * window, or windows already excluded by the coherence gate -- which are
     * the ones with the worst-determined offsets -- would set the scale that
     * the surviving windows are then judged against, and a scene that is mostly
     * empty background would gate on the background. Two passes for that
     * reason; the alternative is a threshold that moves with how much of the
     * scene was masked. */
    double *sig = NULL;
    size_t n_sig = 0;
    if (have_surface) {
        sig = malloc(spec->n_win * sizeof *sig);
        if (!sig) {
            free(state);
            rs_set_error("spectrum: cannot allocate the offset-uncertainty scratch "
                         "for %zu windows", spec->n_win);
            return RS_ERR_ALLOC;
        }
    }
    for (size_t w = 0; w < spec->n_win; w++) {
        if (spec->quality[w] < q_min) continue;
        if (floor_px > 0.0 && spec->excursion_px &&
            spec->excursion_px[w] < floor_px) continue;
        out->n_input++;
        if (sig) sig[n_sig++] = spec->sigma_px[w];
    }

    if (sig && n_sig) {
        /* Insertion sort: n_sig is the window count, tens to low thousands, and
         * this runs once per call against a tracking stage that has already
         * spent orders of magnitude more. */
        for (size_t i = 1; i < n_sig; i++) {
            const double v = sig[i];
            size_t j = i;
            while (j > 0 && sig[j - 1] > v) { sig[j] = sig[j - 1]; j--; }
            sig[j] = v;
        }
        out->sigma_gate = (sigma_factor > 0.0)
                        ? sigma_factor * sig[n_sig / 2] : 0.0;
    }
    free(sig);

    for (size_t w = 0; w < spec->n_win; w++) {
        if (spec->quality[w] < q_min) continue;
        if (floor_px > 0.0 && spec->excursion_px &&
            spec->excursion_px[w] < floor_px) continue;

        state[w] = 1;                       /* entered; may still vote below */

        if (have_surface) {
            if (out->snr_gate > 0.0 && spec->snr[w] < out->snr_gate) {
                out->n_snr_cull++;
                continue;
            }
            /* Offset determination against the scene's own typical window. A
             * gate of zero means every entrant reported a zero sigma, which the
             * estimator only does at a coherence of one; nothing is culled
             * then, rather than everything. */
            if (out->sigma_gate > 0.0 && spec->sigma_px[w] > out->sigma_gate) {
                out->n_sigma_cull++;
                continue;
            }
        }
        state[w] = 2;
    }

    /* Gate 3, on the window lattice. Skipped when the lattice is not rectangular
     * or its extents do not describe n_win, since neighbours are then undefined;
     * every survivor of the first two gates carries through in that case rather
     * than being culled by a test that could not be run. */
    const int have_lattice = (spec->n_win_az > 0 && spec->n_win_rg > 0 &&
                              spec->n_win_az * spec->n_win_rg == spec->n_win);
    for (size_t w = 0; w < spec->n_win; w++) {
        if (state[w] != 2) continue;
        if (!have_lattice || min_neighbours == 0) {
            state[w] = 3; out->n_survivor++; continue;
        }

        const size_t ia = w / spec->n_win_rg, ir = w % spec->n_win_rg;
        const long da[4] = { -1, 1, 0, 0 };
        const long dr[4] = { 0, 0, -1, 1 };
        size_t agree = 0;
        for (int k = 0; k < 4; k++) {
            const long na = (long)ia + da[k], nr = (long)ir + dr[k];
            if (na < 0 || nr < 0 || (size_t)na >= spec->n_win_az ||
                (size_t)nr >= spec->n_win_rg) continue;
            const size_t nb = (size_t)na * spec->n_win_rg + (size_t)nr;
            /* ANY WINDOW THAT ENTERED THE CULL MAY VOTE, including one gates 1
             * and 2 removed. This gate asks whether a FREQUENCY is spatially
             * supported, which is a property of the answers and not of the
             * neighbours' own reliability -- the window under test has already
             * been judged on that. Restricting the vote to gate-1-and-2
             * survivors was the first version and it was wrong for a measurable
             * reason (FOLLOW-UPS.md item 12c): this threshold is derived from a
             * target's FOOTPRINT -- each cell of a 2x2 block has exactly two
             * in-block neighbours -- and that derivation describes the whole
             * block. After gate 1 has removed a third to two thirds of the
             * population, the survivors are too sparse to form blocks, and the
             * geometric bound was being applied to a population it was never
             * about. It removed every window of every run of a sweep.
             *
             * It also makes the vote consistent with rs_spectrum_consensus(),
             * which counts every shared-gate survivor as an equal voter. */
            if (state[nb] == 0) continue;
            if (fabs(spec->dominant_freq[nb] - spec->dominant_freq[w]) <= tol) agree++;
        }
        if (agree >= min_neighbours) { state[w] = 3; out->n_survivor++; }
        else                         { out->n_neigh_cull++; }
    }

    if (out_state) memcpy(out_state, state, spec->n_win);

    if (out->n_survivor == 0) {
        free(state);
        rs_set_error("spectrum: the correlation cull removed every window -- "
                     "%zu entered, %zu failed the SNR gate at %.3g, %zu failed "
                     "the offset-uncertainty gate at %.3g px, %zu had too few "
                     "agreeing neighbours",
                     out->n_input, out->n_snr_cull, out->snr_gate,
                     out->n_sigma_cull, out->sigma_gate, out->n_neigh_cull);
        return RS_ERR_RANGE;
    }

    /* The answer is the frequency the survivors agree on, not the single best
     * survivor's. The cull has already decided which windows are believed; the
     * remaining question is what they say, and the mode of a believed population
     * is a better answer than the argmax of any statistic over it -- which is
     * the whole complaint FOLLOW-UPS items 7-9 record against ranking by
     * prominence. Ties go to the lower frequency, matching
     * rs_spectrum_consensus() so the two cannot disagree by convention alone. */
    size_t best_count = 0;
    double best_freq = 0.0;
    for (size_t w = 0; w < spec->n_win; w++) {
        if (state[w] != 3) continue;
        size_t count = 0;
        for (size_t v = 0; v < spec->n_win; v++) {
            if (state[v] != 3) continue;
            if (fabs(spec->dominant_freq[v] - spec->dominant_freq[w]) <= tol) count++;
        }
        if (count > best_count ||
            (count == best_count && best_count > 0 &&
             spec->dominant_freq[w] < best_freq)) {
            best_count = count;
            best_freq  = spec->dominant_freq[w];
        }
    }

    /* Which window to name. Among the survivors at the reported frequency, the
     * one whose correlation was best determined -- highest SNR, and lowest sigma
     * to break a tie. Without surface statistics that ordering does not exist,
     * so the first such window is named and the caller learns from
     * 'gates_applied' that the choice carries no weight. */
    size_t pick = spec->n_win;
    for (size_t w = 0; w < spec->n_win; w++) {
        if (state[w] != 3) continue;
        if (fabs(spec->dominant_freq[w] - best_freq) > tol) continue;
        if (pick == spec->n_win) { pick = w; continue; }
        if (!have_surface) continue;
        if (spec->snr[w] > spec->snr[pick] ||
            (spec->snr[w] == spec->snr[pick] &&
             spec->sigma_px[w] < spec->sigma_px[pick])) {
            pick = w;
        }
    }

    out->window   = pick;
    out->freq_hz  = best_freq;
    out->n_agree  = best_count;
    out->snr      = (spec->snr && pick < spec->n_win) ? spec->snr[pick] : 0.0;
    out->sigma_px = (spec->sigma_px && pick < spec->n_win) ? spec->sigma_px[pick] : 0.0;

    free(state);
    return RS_OK;
}


/* Select the persistent scatterers and report what they agree on. See microm.h. */
resonarsat_status_t rs_spectrum_ps_window(const rs_spectrum_t *spec,
                                          rs_spectrum_ps_t *out)
{
    return rs_spectrum_ps_window_opts(spec, RS_PS_DA_MAX, out);
}

/* Select on amplitude dispersion, with the criterion supplied. See microm.h. */
resonarsat_status_t rs_spectrum_ps_window_opts(const rs_spectrum_t *spec,
                                               double da_max,
                                               rs_spectrum_ps_t *out)
{
    if (!out) return RS_ERR_ARG;
    memset(out, 0, sizeof *out);
    if (!spec || !spec->dominant_freq || !spec->quality || !spec->d_a ||
        spec->n_win == 0)
        return RS_ERR_ARG;
    out->window = spec->n_win;
    out->da_gate = da_max;

    /* The same shared gates the other three policies apply, so the counts
     * describe one population. Duplicated for the reason stated on
     * rs_spectrum_consensus(): the gates are the contract, not an
     * implementation detail. */
    double q_max = 0.0;
    for (size_t w = 0; w < spec->n_win; w++)
        if (spec->quality[w] > q_max) q_max = spec->quality[w];
    const double q_min = 0.5 * q_max;
    const double floor_px = (spec->quant_px > 0.0) ? 2.449 * spec->quant_px : 0.0;
    const double tol = (spec->df > 0.0) ? 0.5 * spec->df : 1e-9;

    unsigned char *cand = calloc(spec->n_win, 1);
    if (!cand) {
        rs_set_error("spectrum: cannot allocate the persistent-scatterer candidate "
                     "mask for %zu windows", spec->n_win);
        return RS_ERR_ALLOC;
    }

    for (size_t w = 0; w < spec->n_win; w++) {
        if (spec->quality[w] < q_min) continue;
        if (floor_px > 0.0 && spec->excursion_px &&
            spec->excursion_px[w] < floor_px) continue;
        out->n_input++;
        /* A non-positive criterion selects everything that entered, which is
         * the control a caller needs to see what the selection is worth. */
        if (da_max > 0.0 && spec->d_a[w] > da_max) continue;
        cand[w] = 1;
        out->n_candidate++;
    }

    if (out->n_candidate == 0) {
        free(cand);
        rs_set_error("spectrum: no window meets the amplitude-dispersion "
                     "criterion D_A <= %.3g, so the scene holds no persistent "
                     "scatterer for the phase route to read; %zu windows passed "
                     "the shared gates", da_max, out->n_input);
        return RS_ERR_RANGE;
    }

    /* What the candidates agree on. The mode rather than the argmax of any
     * statistic over them: the selection has already decided which windows are
     * believed, and the remaining question is what they say. Ties to the lower
     * frequency, matching the other policies so they cannot disagree by
     * convention alone. */
    size_t best_count = 0;
    double best_freq = 0.0;
    for (size_t w = 0; w < spec->n_win; w++) {
        if (!cand[w]) continue;
        size_t count = 0;
        for (size_t v = 0; v < spec->n_win; v++) {
            if (!cand[v]) continue;
            if (fabs(spec->dominant_freq[v] - spec->dominant_freq[w]) <= tol) count++;
        }
        if (count > best_count ||
            (count == best_count && best_count > 0 &&
             spec->dominant_freq[w] < best_freq)) {
            best_count = count;
            best_freq = spec->dominant_freq[w];
        }
    }

    /* Name the most persistent scatterer among those at the reported frequency:
     * lowest dispersion is the one the criterion likes best, and is the window a
     * reader should look at first. */
    size_t pick = spec->n_win;
    for (size_t w = 0; w < spec->n_win; w++) {
        if (!cand[w]) continue;
        if (fabs(spec->dominant_freq[w] - best_freq) > tol) continue;
        if (pick == spec->n_win || spec->d_a[w] < spec->d_a[pick]) pick = w;
    }

    out->window  = pick;
    out->freq_hz = best_freq;
    out->n_agree = best_count;
    out->d_a     = (pick < spec->n_win) ? spec->d_a[pick] : RS_DA_MAX;
    free(cand);
    return RS_OK;
}

/* Defined below, beside the scene-derived null that also needs it. */
static double rs_median_inplace(double *v, size_t n);

/* The local background ratio for bin k: its power over the median of its own
 * neighbourhood, with the Hann main lobe guarded out. Returns -1 when there are
 * too few reference bins to call it an estimate of noise. 'ref' is scratch of at
 * least RS_LOCAL_REF_BINS doubles, supplied by the caller so the inner loops
 * allocate nothing.
 *
 * EVERY BIN IS SCORED AGAINST THE SAME NUMBER OF REFERENCES, TAKEN AS THE
 * NEAREST AVAILABLE ONES OUTSIDE THE GUARD, AND THE COUNT IS WHAT MATTERS
 * (item 110). The earlier form took a fixed +-RS_LOCAL_HALF_BINS interval and
 * CLIPPED it at the band edges, so the first admissible bin was scored against
 * 10 references where a mid-band bin got 20. The median of 10 draws is about
 * twice as variable as the median of 20; this statistic is then MAXIMISED over
 * the band, so the noisier denominators won the maximum out of proportion to
 * their content. Measured on scenes containing NOTHING -- 200 realisations of
 * flat unit-mean noise, the argmax bin recorded each time:
 *
 *     starved bins   24 of 62 (39% of the band)   took 72% of the maxima
 *     per-bin rate   2.98% starved  against  0.75% full   -- 4.0x
 *
 * and the two frequencies item 109's report actually named, its first
 * admissible bin and bin 61, are both in that starved zone.
 *
 * THE FIX IS A NARROWER NEIGHBOURHOOD, NOT A WIDER ONE, and getting that
 * backwards is how the first attempt failed. Growing outward until every bin has
 * TWENTY references equalises the count and FAILS test_tracking's red-floor
 * case, because on a floor rolling off as sinc^2 the extension reaches past the
 * first null into the deep tail, depresses the median and inflates the very low
 * bins it was meant to demote. Requiring the count every bin can supply --
 * RS_LOCAL_REF_BINS, which is what the band floor itself has room for -- is a
 * strict NARROWING: the span never exceeds RS_LOCAL_HALF_BINS anywhere, and
 * mid-band it shrinks to about +-7. A narrower neighbourhood is also what this
 * header has said a steep floor needs since item 47.
 *
 * Reference bins are taken only from at or above 'k_lo': below it lies the Hann
 * skirt of any residual trend (RS_SPECTRUM_LEAKAGE_BINS), which is not an
 * estimate of this band's noise and would inflate every background near the
 * floor. The neighbourhood at the floor is therefore one-sided -- one-sided and
 * the same length as everywhere else, which is the property that matters.
 *
 * 'out_n_ref' receives how many references were found, for a caller that reports
 * it; it may be NULL. It is set even on the refusal paths, because "too few
 * reference bins" is the thing a caller wants to say. */
static double rs_local_ratio(const double *P, size_t k, size_t k_lo,
                             size_t n_freq, double *ref, size_t *out_n_ref)
{
    size_t n_ref = 0;
    for (size_t d = RS_LOCAL_GUARD_BINS + 1;
         d <= RS_LOCAL_HALF_BINS && n_ref < RS_LOCAL_REF_BINS; d++) {
        if (k >= k_lo + d)                          ref[n_ref++] = P[k - d];
        if (n_ref < RS_LOCAL_REF_BINS && k + d < n_freq) ref[n_ref++] = P[k + d];
    }
    if (out_n_ref) *out_n_ref = n_ref;
    if (n_ref < 4) return -1.0;
    const double med = rs_median_inplace(ref, n_ref);
    if (!(med > 0.0)) return -1.0;
    return P[k] / med;
}

/* Does a displacement peak also appear in the amplitude spectrum? See microm.h,
 * which records that this does NOT separate the two on the fixture it was built
 * for, and why. Reported as a diagnostic; there is no threshold. */
resonarsat_status_t rs_spectrum_am_check(const rs_microm_t *m,
                                         size_t window, double freq_hz,
                                         rs_am_check_t *out)
{
    if (!out) {
        rs_set_error("am check: no output structure");
        return RS_ERR_ARG;
    }
    memset(out, 0, sizeof *out);
    if (!m || !m->amp) {
        rs_set_error("am check: this estimator keeps no per-look amplitude, so "
                     "there is nothing to test a peak against");
        return RS_ERR_ARG;
    }
    if (window >= m->n_win) {
        rs_set_error("am check: window %zu of %zu", window, m->n_win);
        return RS_ERR_ARG;
    }
    const size_t n = m->n_looks;
    if (n < 8 || !(m->dt > 0.0) || !(freq_hz >= 0.0)) {
        rs_set_error("am check: %zu looks at dt %g cannot be asked about %g Hz",
                     n, m->dt, freq_hz);
        return RS_ERR_ARG;
    }

    const size_t n_freq = n / 2 + 1;
    const double df = 1.0 / ((double)n * m->dt);
    size_t k = (size_t)(freq_hz / df + 0.5);
    if (k >= n_freq) k = n_freq - 1;

    double *win = malloc(n * sizeof *win);
    float complex *buf = malloc(n * sizeof *buf);
    double *P = malloc(n_freq * sizeof *P);
    double *ref = malloc(n_freq * sizeof *ref);
    rs_fft_plan *plan = NULL;
    resonarsat_status_t st = RS_OK;
    if (!win || !buf || !P || !ref ||
        (st = rs_fft_plan_create(n, &plan)) != RS_OK) {
        free(win); free(buf); free(P); free(ref); rs_fft_plan_destroy(plan);
        rs_set_error("am check: out of memory for %zu looks", n);
        return (st == RS_OK) ? RS_ERR_ALLOC : st;
    }

    /* Same Hann window the displacement spectrum uses, so the two are
     * comparable bin for bin, and the MEAN removed first -- a bright window
     * would otherwise put all its power at DC and flatten every ratio. */
    double wp = 0.0, mean = 0.0;
    for (size_t i = 0; i < n; i++) {
        win[i] = 0.5 * (1.0 - cos(2.0 * M_PI * (double)i / (double)(n - 1)));
        wp += win[i] * win[i];
        mean += m->amp[window * n + i];
    }
    if (wp <= 0.0) wp = 1.0;
    mean /= (double)n;

    for (size_t i = 0; i < n; i++)
        buf[i] = (float complex)((m->amp[window * n + i] - mean) * win[i]);
    rs_fft_forward(plan, buf);
    for (size_t j = 0; j < n_freq; j++) {
        const double mag = (double)cabsf(buf[j]);
        P[j] = mag * mag / wp;
    }

    /* The same statistic rs_spectrum_local_window() and rs_spectrum_modal_set()
     * use, so a ratio quoted here is comparable with one quoted there. */
    const size_t k_lo = RS_SPECTRUM_LEAKAGE_BINS;
    size_t n_ref = 0;
    const double r = rs_local_ratio(P, k, k_lo, n_freq, ref, &n_ref);
    if (r > 0.0) {
        out->am_ratio = r;
        out->ref_median = P[k] / r;
    }
    out->bin = k;
    out->n_ref = n_ref;

    free(win); free(buf); free(P); free(ref); rs_fft_plan_destroy(plan);
    if (n_ref < 4) {
        rs_set_error("am check: only %zu reference bins around bin %zu", n_ref, k);
        return RS_ERR_RANGE;
    }
    return RS_OK;
}

/* Short-time max-hold spectrum. See microm.h for why a maximum and not a mean,
 * and for the resolution this trades away to get it. */
resonarsat_status_t rs_spectrum_maxhold(rs_spectrum_t *spec,
                                        const rs_microm_t *m,
                                        rs_spectrum_source_t source,
                                        size_t seg_len,
                                        double f_min,
                                        rs_detrend_t detrend)
{
    if (!spec || !m || !spec->psd || !m->disp_los || !m->vel_los) {
        rs_set_error("maxhold: NULL spectrum or tracking result");
        return RS_ERR_ARG;
    }
    const size_t n = m->n_looks, n_freq = spec->n_freq;
    if (seg_len < 4 || seg_len > n) {
        rs_set_error("maxhold: segment of %zu looks is outside 4..%zu",
                     seg_len, n);
        return RS_ERR_ARG;
    }
    const double *all = (source == RS_SPEC_DISPLACEMENT) ? m->disp_los : m->vel_los;

    /* Half-segment hop: enough overlap that a burst falling on a segment
     * boundary is still whole in the neighbouring segment, without the strongly
     * correlated segments a finer hop would produce. */
    const size_t hop = seg_len / 2 > 0 ? seg_len / 2 : 1;
    const size_t n_seg = (n >= seg_len) ? 1 + (n - seg_len) / hop : 0;
    if (n_seg == 0) {
        rs_set_error("maxhold: no whole segment of %zu fits in %zu looks",
                     seg_len, n);
        return RS_ERR_RANGE;
    }

    double *win = malloc(seg_len * sizeof *win);
    /* Zero-padded to the full length, so bin spacing and every downstream index
     * are the ones rs_spectrum_compute_opts() produced. */
    float complex *buf = malloc(n * sizeof *buf);
    double *hold = malloc(n_freq * sizeof *hold);
    rs_fft_plan *plan = NULL;
    if (!win || !buf || !hold || rs_fft_plan_create(n, &plan) != RS_OK) {
        free(win); free(buf); free(hold);
        rs_set_error("maxhold: out of memory");
        return RS_ERR_ALLOC;
    }
    double win_power = 0.0;
    for (size_t i = 0; i < seg_len; i++) {
        win[i] = 0.5 * (1.0 - cos(2.0 * M_PI * (double)i / (double)(seg_len - 1)));
        win_power += win[i] * win[i];
    }
    if (win_power <= 0.0) win_power = 1.0;
    const double fs = (spec->df > 0.0) ? spec->df * (double)n : 1.0;

    for (size_t w = 0; w < spec->n_win; w++) {
        for (size_t k = 0; k < n_freq; k++) hold[k] = 0.0;
        const double *series = all + w * n;

        for (size_t sgi = 0; sgi < n_seg; sgi++) {
            const double *seg = series + sgi * hop;

            /* Detrended per SEGMENT, not once over the record. A trend removed
             * globally leaves a local slope inside each segment, which is the
             * same leakage the band floor exists to keep out. */
            double sx = 0.0, sxx = 0.0, sy = 0.0, sxy = 0.0;
            for (size_t i = 0; i < seg_len; i++) {
                sx += (double)i; sxx += (double)i * (double)i;
                sy += seg[i];    sxy += (double)i * seg[i];
            }
            const double det = (double)seg_len * sxx - sx * sx;
            double a = 0.0, b = 0.0;
            if (detrend == RS_DETREND_LINEAR && det != 0.0) {
                b = ((double)seg_len * sxy - sx * sy) / det;
                a = (sy - b * sx) / (double)seg_len;
            } else if (detrend == RS_DETREND_LINEAR || detrend == RS_DETREND_MEAN) {
                a = sy / (double)seg_len;
            }

            for (size_t i = 0; i < n; i++) buf[i] = 0.0f;
            for (size_t i = 0; i < seg_len; i++)
                buf[i] = (float complex)((seg[i] - (a + b * (double)i)) * win[i]);

            if (rs_fft_forward(plan, buf) != RS_OK) continue;
            for (size_t k = 0; k < n_freq; k++) {
                const double mag = (double)cabsf(buf[k]);
                const double scale =
                    (k == 0 || (n % 2 == 0 && k == n_freq - 1)) ? 1.0 : 2.0;
                const double p = scale * mag * mag / (win_power * fs);
                if (p > hold[k]) hold[k] = p;
            }
        }

        double *psd = spec->psd + w * n_freq;
        for (size_t k = 0; k < n_freq; k++) psd[k] = hold[k];

        /* The band floor and the prominence definition are
         * rs_spectrum_compute_opts()'s, unchanged, so the two spectra are read
         * by every downstream policy in exactly the same way. */
        size_t k_min = RS_SPECTRUM_LEAKAGE_BINS;
        if (f_min > 0.0 && spec->df > 0.0) {
            const double kf = ceil(f_min / spec->df);
            if (kf > (double)k_min) k_min = (size_t)kf;
        }
        if (k_min >= n_freq) k_min = n_freq - 1;
        size_t best = k_min;
        for (size_t k = k_min + 1; k < n_freq; k++) if (psd[k] > psd[best]) best = k;

        spec->dominant_freq[w] = spec->freq[best];
        spec->amplitude[w] = sqrt(psd[best]);
        double sum = 0.0;
        for (size_t k = k_min; k < n_freq; k++) sum += psd[k];
        const double mean_power = (n_freq > k_min)
                                ? sum / (double)(n_freq - k_min) : 0.0;
        spec->prominence[w] = (mean_power > 0.0) ? psd[best] / mean_power : 0.0;
    }

    rs_fft_plan_destroy(plan);
    free(win); free(buf); free(hold);
    return RS_OK;
}

/* log(n!) by lgamma, for the binomial tail below. */
static double rs_log_fact(size_t n)
{
    return lgamma((double)n + 1.0);
}

/* P(S >= s) for S ~ Binomial(n, p), summed from the top so the small tail this
 * threshold lives in does not lose its significant digits to cancellation. */
static double rs_binom_tail(size_t n, double p, size_t s)
{
    if (s == 0) return 1.0;
    if (s > n) return 0.0;
    if (!(p > 0.0)) return 0.0;
    if (p >= 1.0) return 1.0;
    const double lp = log(p), lq = log1p(-p);
    double sum = 0.0;
    for (size_t i = n; i + 1 > s; i--) {
        const double lt = rs_log_fact(n) - rs_log_fact(i) - rs_log_fact(n - i)
                        + (double)i * lp + (double)(n - i) * lq;
        sum += exp(lt);
        if (i == 0) break;
    }
    return sum > 1.0 ? 1.0 : sum;
}

/* The largest 4-connected block of set windows in 'v', by flood fill over the
 * naz x nrg window grid. This is the mode SHAPE measure: a structure occupies
 * contiguous ground, a noise line does not. 'seen' and 'stack' are scratch of
 * n_win each, supplied by the caller because the null below calls this tens of
 * thousands of times and must allocate nothing inside its loop. */
static size_t rs_largest_block(const unsigned char *v, size_t n_win,
                               size_t naz, size_t nrg,
                               unsigned char *seen, size_t *stack)
{
    memset(seen, 0, n_win * sizeof *seen);
    size_t largest = 0;
    for (size_t start = 0; start < n_win; start++) {
        if (!v[start] || seen[start]) continue;
        size_t top = 0, size = 0;
        stack[top++] = start; seen[start] = 1;
        while (top > 0) {
            const size_t cur = stack[--top];
            size++;
            const size_t ia = (naz > 0) ? cur % naz : cur;
            const size_t ir = (naz > 0) ? cur / naz : 0;
            const long da[4] = { -1, 1, 0, 0 }, dr[4] = { 0, 0, -1, 1 };
            for (int d = 0; d < 4; d++) {
                const long a2 = (long)ia + da[d], r2 = (long)ir + dr[d];
                if (a2 < 0 || r2 < 0 || (size_t)a2 >= naz || (size_t)r2 >= nrg)
                    continue;
                const size_t nb = (size_t)r2 * naz + (size_t)a2;
                if (nb >= n_win || seen[nb] || !v[nb]) continue;
                seen[nb] = 1; stack[top++] = nb;
            }
        }
        if (size > largest) largest = size;
    }
    return largest;
}

/* xorshift64*, so a p-value quoted in the documentation reproduces on another
 * machine. rand() does not: its sequence is a property of the C library. */
static unsigned long long rs_modal_rand(unsigned long long *state)
{
    unsigned long long x = *state;
    x ^= x >> 12; x ^= x << 25; x ^= x >> 27;
    *state = x;
    return x * 2685821657736338717ULL;
}

/* THE CHANCE MODEL FOR THE BLOCK. See microm.h: the 2x2 geometric floor is
 * contingent on the look count, because the look count sets the bin count and
 * the bin count sets how often windows agree by accident.
 *
 * Draws 'n_trial' realisations in which every voting window re-nominates
 * RS_MODAL_PER_WINDOW bins uniformly over [k_lo, n_freq) under the same leakage
 * separation the real nomination uses, and records each trial's LARGEST block
 * among bins clearing 'support_min' -- a max statistic, so the look-elsewhere
 * cost is inside it. Writes the counts into 'hist', indexed by block size and
 * of length n_win + 1; the caller turns that into a tail probability.
 *
 * 'voting' is the real mask of which windows nominated anything, not a count:
 * the windows that fall silent are a property of the scene, and drawing the
 * null over a different set of grid positions would compare the candidate's
 * shape against a differently-shaped lattice.
 *
 * Returns RS_ERR_ALLOC if its scratch cannot be had, in which case 'hist' is
 * untouched and the caller must not read a p-value from it. */
static resonarsat_status_t rs_modal_null(const unsigned char *voting, size_t n_win,
                                         size_t naz, size_t nrg,
                                         size_t k_lo, size_t n_freq,
                                         size_t support_min, size_t n_trial,
                                         unsigned long long seed, size_t *hist)
{
    const size_t n_bin = n_freq - k_lo;
    size_t *support      = calloc(n_freq, sizeof *support);
    unsigned char *voted = calloc(n_freq * n_win, sizeof *voted);
    unsigned char *seen  = malloc(n_win * sizeof *seen);
    size_t *stack        = malloc(n_win * sizeof *stack);
    size_t *pick         = malloc(RS_MODAL_PER_WINDOW * sizeof *pick);
    unsigned char *col   = calloc(n_win, sizeof *col);
    if (!support || !voted || !seen || !stack || !pick || !col) {
        free(support); free(voted); free(seen); free(stack); free(pick); free(col);
        rs_set_error("modal null: out of memory");
        return RS_ERR_ALLOC;
    }

    unsigned long long st = seed ? seed : 1ULL;
    for (size_t t = 0; t < n_trial; t++) {
        memset(support, 0, n_freq * sizeof *support);
        memset(voted, 0, n_freq * n_win * sizeof *voted);
        for (size_t w = 0; w < n_win; w++) {
            if (!voting[w]) continue;
            size_t n_pick = 0;
            /* Rejection sampling against the leakage separation. The attempt cap
             * matters: with a narrow band and six picks the draw can genuinely
             * run out of room, and a window that nominates fewer bins is the
             * honest outcome rather than a hang. */
            for (size_t attempt = 0; attempt < 64 && n_pick < RS_MODAL_PER_WINDOW;
                 attempt++) {
                const size_t k = k_lo + (size_t)(rs_modal_rand(&st) % (unsigned long long)n_bin);
                int blocked = 0;
                for (size_t i = 0; i < n_pick; i++) {
                    const size_t d = (k > pick[i]) ? k - pick[i] : pick[i] - k;
                    if (d < RS_SPECTRUM_LEAKAGE_BINS) { blocked = 1; break; }
                }
                if (blocked) continue;
                pick[n_pick++] = k;
                support[k]++;
                voted[k * n_win + w] = 1;
            }
        }
        size_t largest = 0;
        for (size_t k = k_lo; k < n_freq; k++) {
            if (support[k] < support_min) continue;
            const unsigned char *v = voted + k * n_win;
            for (size_t i = 0; i < n_win; i++) col[i] = v[i];
            const size_t b = rs_largest_block(col, n_win, naz, nrg, seen, stack);
            if (b > largest) largest = b;
        }
        hist[largest]++;
    }

    free(support); free(voted); free(seen); free(stack); free(pick); free(col);
    return RS_OK;
}

/* The sub-bin position of a peak at bin k, by parabolic interpolation of the
 * log-power across it -- the standard estimator for a windowed line, exact for a
 * Gaussian and close for a Hann main lobe. Returns the offset in BINS, clamped
 * to +-0.5 because a larger excursion means the peak is not at k, and 0 when the
 * three points do not describe a maximum or sit at the band edge. */
static double rs_parabolic_offset(const double *P, size_t k, size_t k_lo,
                                  size_t n_freq)
{
    if (k <= k_lo || k + 1 >= n_freq) return 0.0;
    if (!(P[k - 1] > 0.0) || !(P[k] > 0.0) || !(P[k + 1] > 0.0)) return 0.0;
    const double a = log(P[k - 1]), b = log(P[k]), c = log(P[k + 1]);
    const double den = a - 2.0 * b + c;
    if (!(den < 0.0)) return 0.0;          /* not a maximum */
    double d = 0.5 * (a - c) / den;
    if (d > 0.5) d = 0.5;
    if (d < -0.5) d = -0.5;
    return d;
}

/* The ordering statistic for an admitted mode: its contiguous block times the
 * log of how far it stood above its own background. See rs_mode_t.evidence. */
static double rs_mode_evidence(const rs_mode_t *m)
{
    if (!(m->median_ratio > 1.0)) return 0.0;
    return (double)m->n_contiguous * log(m->median_ratio);
}

/* Sort key for ranking the modes that have ALREADY been admitted: accumulated
 * evidence first, then block, then support. See rs_mode_t.evidence in microm.h
 * for the derivation and for the two measurements that forced it. */
static int rs_cmp_mode(const void *a, const void *b)
{
    const rs_mode_t *x = a, *y = b;
    const double ex = rs_mode_evidence(x), ey = rs_mode_evidence(y);
    if (ex != ey) return ex > ey ? -1 : 1;
    if (x->n_contiguous != y->n_contiguous)
        return x->n_contiguous > y->n_contiguous ? -1 : 1;
    if (x->n_support != y->n_support)
        return x->n_support > y->n_support ? -1 : 1;
    return x->bin < y->bin ? -1 : (x->bin > y->bin);
}

/* Which frequencies recur across the windows. See microm.h for why a structure
 * needs a set reported rather than a winner. */
resonarsat_status_t rs_spectrum_modal_set(const rs_spectrum_t *spec,
                                          rs_modal_set_t *out)
{
    if (!out) {
        rs_set_error("modal set: no output structure");
        return RS_ERR_ARG;
    }
    memset(out, 0, sizeof *out);
    if (!spec || !spec->psd || spec->n_win == 0 || spec->n_freq == 0) {
        rs_set_error("modal set: empty spectrum");
        return RS_ERR_ARG;
    }

    const size_t n_freq = spec->n_freq;
    const size_t k_lo = RS_SPECTRUM_LEAKAGE_BINS;
    if (n_freq <= k_lo + RS_LOCAL_GUARD_BINS + 1) {
        rs_set_error("modal set: %zu bins is too few to estimate a local "
                     "background above bin %zu", n_freq, k_lo);
        return RS_ERR_RANGE;
    }
    const size_t n_bin = n_freq - k_lo;

    double *ref     = malloc(RS_LOCAL_REF_BINS * sizeof *ref);
    double *ratio   = malloc(n_freq * sizeof *ratio);
    size_t *support = calloc(n_freq, sizeof *support);
    /* Every window's ratio at every bin, so a mode's median can be taken over
     * the windows that actually nominated it rather than over all of them. */
    double *acc     = malloc(spec->n_win * sizeof *acc);
    size_t *pick    = malloc(RS_MODAL_PER_WINDOW * sizeof *pick);
    double **nom    = calloc(n_freq, sizeof *nom);
    /* [n_freq][n_win] which windows nominated each bin, so a candidate's
     * SHAPE on the window grid can be measured and not just its count. */
    unsigned char *voted = calloc(n_freq * spec->n_win, sizeof *voted);
    unsigned char *seen  = malloc(spec->n_win * sizeof *seen);
    size_t *stack   = malloc(spec->n_win * sizeof *stack);
    /* Which windows nominated anything, so the chance model below draws over
     * the same grid positions rather than over an equally-sized different set. */
    unsigned char *voting = calloc(spec->n_win, sizeof *voting);
    if (!ref || !ratio || !support || !acc || !pick || !nom || !voted ||
        !seen || !stack || !voting) {
        free(ref); free(ratio); free(support); free(acc); free(pick); free(nom);
        free(voted); free(seen); free(stack); free(voting);
        rs_set_error("modal set: out of memory");
        return RS_ERR_ALLOC;
    }
    for (size_t k = 0; k < n_freq; k++) {
        nom[k] = malloc(spec->n_win * sizeof **nom);
        if (!nom[k]) {
            for (size_t j = 0; j < k; j++) free(nom[j]);
            free(ref); free(ratio); free(support); free(acc); free(pick); free(nom);
            free(voted); free(seen); free(stack); free(voting);
            rs_set_error("modal set: out of memory");
            return RS_ERR_ALLOC;
        }
    }

    size_t n_voting = 0;
    for (size_t w = 0; w < spec->n_win; w++) {
        const double *P = spec->psd + w * n_freq;
        int any = 0;
        for (size_t k = k_lo; k < n_freq; k++) {
            ratio[k] = rs_local_ratio(P, k, k_lo, n_freq, ref, NULL);
            if (ratio[k] > 0.0) any = 1;
        }
        if (!any) continue;

        /* Greedy nomination: take the best remaining bin, then forbid its Hann
         * skirt so one mode cannot be nominated twice. */
        size_t n_pick = 0;
        while (n_pick < RS_MODAL_PER_WINDOW) {
            size_t best = n_freq;
            double best_r = 0.0;
            for (size_t k = k_lo; k < n_freq; k++) {
                if (!(ratio[k] > best_r)) continue;
                int blocked = 0;
                for (size_t i = 0; i < n_pick; i++) {
                    const size_t d = (k > pick[i]) ? k - pick[i] : pick[i] - k;
                    if (d < RS_SPECTRUM_LEAKAGE_BINS) { blocked = 1; break; }
                }
                if (blocked) continue;
                best = k; best_r = ratio[k];
            }
            if (best == n_freq) break;
            nom[best][support[best]] = best_r;
            support[best]++;
            voted[best * spec->n_win + w] = 1;
            pick[n_pick++] = best;
        }
        if (n_pick > 0) { voting[w] = 1; n_voting++; }
    }

    /* The binomial threshold: the smallest support at which fewer than half a
     * bin is expected to clear it by chance over the whole band. It is REPORTED
     * and no longer gates (item 110) -- see rs_modal_set_t.support_min for the
     * measurement. Admission is RS_MODAL_BLOCK_MIN, which is the block floor
     * restated, and the family-wise work is done by the block's own null. */
    const double p = (double)RS_MODAL_PER_WINDOW / (double)n_bin;
    size_t support_min = n_voting + 1;
    double expected = 0.0;
    for (size_t s = 1; s <= n_voting; s++) {
        const double e = (double)n_bin * rs_binom_tail(n_voting, p, s);
        if (e < 0.5) { support_min = s; expected = e; break; }
    }
    /* A single window agreeing with itself is not agreement, whatever the
     * arithmetic says about a band this narrow. */
    if (support_min < 2) { support_min = 2; expected = (double)n_bin * rs_binom_tail(n_voting, p, 2); }

    out->n_voting     = n_voting;
    out->n_bin        = n_bin;
    out->n_per_window = RS_MODAL_PER_WINDOW;
    out->support_min  = support_min;
    out->expected_false = expected;
    out->admit_min    = RS_MODAL_BLOCK_MIN;

    /* What chance alone reaches at THIS configuration, which is the number the
     * fixed floor could not know (items 77-78). It is drawn under the SAME
     * admission rule the candidates face -- RS_MODAL_BLOCK_MIN, not the
     * binomial 'support_min' -- so relaxing that rule raises these blocks to
     * match instead of quietly loosening the test (item 110). */
    size_t *hist = calloc(spec->n_win + 1, sizeof *hist);
    if (!hist) {
        for (size_t k = 0; k < n_freq; k++) free(nom[k]);
        free(ref); free(ratio); free(support); free(acc); free(pick); free(nom);
        free(voted); free(seen); free(stack); free(voting);
        rs_set_error("modal set: out of memory");
        return RS_ERR_ALLOC;
    }
    const resonarsat_status_t st_null =
        rs_modal_null(voting, spec->n_win, spec->n_win_az, spec->n_win_rg,
                      k_lo, n_freq, RS_MODAL_BLOCK_MIN, RS_MODAL_NULL_TRIALS,
                      0x9E3779B97F4A7C15ULL, hist);
    if (st_null != RS_OK) {
        free(hist);
        for (size_t k = 0; k < n_freq; k++) free(nom[k]);
        free(ref); free(ratio); free(support); free(acc); free(pick); free(nom);
        free(voted); free(seen); free(stack); free(voting);
        return st_null;                     /* rs_modal_null set the message */
    }
    out->n_trial = RS_MODAL_NULL_TRIALS;
    /* Upper tail of the null: tail[b] trials reached a block of b or more. */
    size_t *tail = calloc(spec->n_win + 2, sizeof *tail);
    if (!tail) {
        free(hist);
        for (size_t k = 0; k < n_freq; k++) free(nom[k]);
        free(ref); free(ratio); free(support); free(acc); free(pick); free(nom);
        free(voted); free(seen); free(stack); free(voting);
        rs_set_error("modal set: out of memory");
        return RS_ERR_ALLOC;
    }
    for (size_t b = spec->n_win + 1; b > 0; b--) tail[b - 1] = tail[b] + hist[b - 1];
    out->null_block_crit = spec->n_win + 1;
    for (size_t b = 0; b <= spec->n_win; b++) {
        if (hist[b] > 0) out->null_block_max = b;
        if (out->null_block_crit > spec->n_win &&
            (double)(1 + tail[b]) / (double)(1 + RS_MODAL_NULL_TRIALS) <= RS_MODAL_P_MAX)
            out->null_block_crit = b;
    }
    free(hist);

    for (size_t k = k_lo; k < n_freq; k++) {
        if (out->n_mode >= RS_MODAL_MAX_MODES) break;
        /* Track the best refusal even when the support gate is what refused it,
         * so the caller can say WHICH gate spoke. The floor is the geometric
         * one: fewer nominating windows than RS_MODAL_BLOCK_MIN cannot form the
         * block the next gate requires, so this refuses only what that gate
         * would refuse anyway. */
        if (support[k] < RS_MODAL_BLOCK_MIN) {
            if (!out->near_miss_had_support &&
                support[k] > out->near_miss.n_support) {
                out->near_miss.bin = k;
                out->near_miss.freq_hz = spec->freq[k];
                out->near_miss.n_support = support[k];
                out->near_miss.n_contiguous = 0;
            }
            continue;
        }

        /* The largest 4-connected block of nominating windows. This is the mode
         * SHAPE test: a structure occupies contiguous ground, a noise line does
         * not. */
        const unsigned char *v = voted + k * spec->n_win;
        const size_t largest = rs_largest_block(v, spec->n_win, spec->n_win_az,
                                                spec->n_win_rg, seen, stack);
        const double p_chance = (double)(1 + tail[largest])
                              / (double)(1 + RS_MODAL_NULL_TRIALS);

        /* Two gates, and they are different in kind. The 2x2 geometric floor
         * says a block this small cannot DESCRIBE a spatially resolved mode
         * whatever else is true; the chance model says a block this large is
         * what this configuration produces from nothing. Item 77: the geometric
         * bound is a floor, never a separator, so both apply. */
        if (largest < RS_MODAL_BLOCK_MIN || p_chance > RS_MODAL_P_MAX) {
            if (largest > out->near_miss.n_contiguous ||
                !out->near_miss_had_support) {
                out->near_miss.bin = k;
                out->near_miss.freq_hz = spec->freq[k];
                out->near_miss.n_support = support[k];
                out->near_miss.n_contiguous = largest;
                out->near_miss.p_chance = p_chance;
                out->near_miss_had_support = 1;
            }
            continue;
        }

        for (size_t i = 0; i < support[k]; i++) acc[i] = nom[k][i];
        rs_mode_t *m = &out->mode[out->n_mode++];
        m->bin = k;
        m->freq_hz = spec->freq[k];
        m->n_support = support[k];
        m->n_contiguous = largest;
        m->median_ratio = rs_median_inplace(acc, support[k]);
        m->p_chance = p_chance;
        m->evidence = rs_mode_evidence(m);

        /* Sub-bin frequency across the windows that nominated it. The mean is
         * the estimate, the spread is the honest uncertainty, and freq_se
         * discounts for the overlap that makes a 2x2 block one look. */
        const double bin_hz = (n_freq > 1) ? spec->freq[1] - spec->freq[0] : 0.0;
        double sum = 0.0, sum2 = 0.0;
        size_t n_est = 0;
        for (size_t w = 0; w < spec->n_win; w++) {
            if (!v[w]) continue;
            const double d = rs_parabolic_offset(spec->psd + w * n_freq, k,
                                                 k_lo, n_freq);
            const double f = spec->freq[k] + d * bin_hz;
            sum += f; sum2 += f * f; n_est++;
        }
        if (n_est > 0) {
            m->freq_mean = sum / (double)n_est;
            if (n_est > 1) {
                double var = (sum2 - sum * sum / (double)n_est) / (double)(n_est - 1);
                if (var < 0.0) var = 0.0;
                m->freq_sd = sqrt(var);
                const double n_eff = (double)n_est / 4.0;
                m->freq_se = m->freq_sd / sqrt(n_eff > 1.0 ? n_eff : 1.0);
            }
        } else {
            m->freq_mean = spec->freq[k];
        }
    }
    free(tail);
    qsort(out->mode, out->n_mode, sizeof out->mode[0], rs_cmp_mode);

    const size_t n_mode = out->n_mode;
    for (size_t k = 0; k < n_freq; k++) free(nom[k]);
    free(ref); free(ratio); free(support); free(acc); free(pick); free(nom);
    free(voted); free(seen); free(stack); free(voting);

    if (n_mode == 0) {
        rs_set_error("modal set: no bin was nominated by %u of %zu windows in a "
                     "block of %zu or more, so nothing recurs with a shape "
                     "chance does not reach here",
                     RS_MODAL_BLOCK_MIN, n_voting,
                     out->null_block_crit > RS_MODAL_BLOCK_MIN
                         ? out->null_block_crit : RS_MODAL_BLOCK_MIN);
        return RS_ERR_RANGE;
    }
    return RS_OK;
}

/* Strongest peak against its own neighbourhood. See microm.h on why the plain
 * prominence is biased toward low frequencies on a red noise floor. */
resonarsat_status_t rs_spectrum_local_window(const rs_spectrum_t *spec,
                                             rs_local_peak_t *out)
{
    if (!out) {
        rs_set_error("local peak: no output structure");
        return RS_ERR_ARG;
    }
    memset(out, 0, sizeof *out);
    if (!spec || !spec->psd || !spec->freq || spec->n_win == 0 ||
        spec->n_freq == 0) {
        rs_set_error("local peak: spectrum has no power density");
        return RS_ERR_ARG;
    }
    out->bin = spec->n_freq;

    const size_t n_freq = spec->n_freq;
    const size_t k_lo = RS_SPECTRUM_LEAKAGE_BINS;
    if (n_freq <= k_lo + RS_LOCAL_GUARD_BINS + 1) {
        rs_set_error("local peak: %zu bins is too few to estimate a local "
                     "background above bin %zu", n_freq, k_lo);
        return RS_ERR_RANGE;
    }

    double *ref = malloc(RS_LOCAL_REF_BINS * sizeof *ref);
    if (!ref) {
        rs_set_error("local peak: out of memory");
        return RS_ERR_ALLOC;
    }

    double best = -1.0;
    size_t n_searched = 0;

    for (size_t w = 0; w < spec->n_win; w++) {
        const double *P = spec->psd + w * n_freq;
        for (size_t k = k_lo; k < n_freq; k++) {
            /* One shared implementation, so this and the modal set's nomination
             * cannot drift apart -- they were separate copies until item 110
             * changed the neighbourhood in one place. Fewer than four reference
             * bins is a background estimated from noise rather than an estimate
             * of noise, and rs_local_ratio() refuses it. */
            size_t n_ref = 0;
            const double ratio = rs_local_ratio(P, k, k_lo, n_freq, ref, &n_ref);
            if (!(ratio > 0.0)) continue;

            n_searched++;
            if (ratio > best) {
                best = ratio;
                out->window = w;
                out->bin = k;
                out->freq_hz = spec->freq[k];
                out->ratio = ratio;
                out->ref_median = P[k] / ratio;
                out->n_ref = n_ref;
            }
        }
    }
    free(ref);

    if (n_searched == 0) {
        memset(out, 0, sizeof *out);
        out->bin = spec->n_freq;
        rs_set_error("local peak: no bin had a usable neighbourhood");
        return RS_ERR_RANGE;
    }
    out->n_searched = n_searched;
    return RS_OK;
}

/* Centre of mass of the windows agreeing with a seed. See microm.h on why a
 * single window index cannot be the answer at 50 percent overlap. */
resonarsat_status_t rs_spectrum_centroid(const rs_spectrum_t *spec,
                                         size_t seed,
                                         size_t stride_az, size_t stride_rg,
                                         size_t win_az, size_t win_rg,
                                         rs_centroid_t *out)
{
    if (!out) {
        rs_set_error("centroid: no output structure");
        return RS_ERR_ARG;
    }
    memset(out, 0, sizeof *out);
    if (!spec || !spec->prominence || !spec->dominant_freq || spec->n_win == 0) {
        rs_set_error("centroid: spectrum has no per-window prominence");
        return RS_ERR_ARG;
    }
    if (seed >= spec->n_win) {
        rs_set_error("centroid: seed window %zu of %zu", seed, spec->n_win);
        return RS_ERR_ARG;
    }
    if (spec->n_win_az == 0 || spec->n_win_rg == 0 ||
        spec->n_win_az * spec->n_win_rg != spec->n_win) {
        rs_set_error("centroid: window grid %zux%zu does not describe %zu windows",
                     spec->n_win_az, spec->n_win_rg, spec->n_win);
        return RS_ERR_ARG;
    }
    if (!(spec->df > 0.0)) {
        rs_set_error("centroid: spectrum has no bin width");
        return RS_ERR_ARG;
    }

    const size_t n = spec->n_win, nrg = spec->n_win_rg;
    const double f0 = spec->dominant_freq[seed];
    const double tol = 0.5 * spec->df;

    /* The scene median prominence, which the weight is taken above. Copied
     * because rs_median_inplace() sorts what it is given. */
    double *tmp = malloc(n * sizeof *tmp);
    unsigned char *in = calloc(n, 1);
    size_t *stack = malloc(n * sizeof *stack);
    if (!tmp || !in || !stack) {
        free(tmp); free(in); free(stack);
        rs_set_error("centroid: out of memory for %zu windows", n);
        return RS_ERR_ALLOC;
    }
    for (size_t w = 0; w < n; w++) tmp[w] = spec->prominence[w];
    const double med = rs_median_inplace(tmp, n);
    free(tmp);

    /* Flood-fill the 4-connected block agreeing with the seed's frequency --
     * the same notion of agreement rs_spectrum_block_at() uses, so membership
     * comes from the measurement and not from a tolerance parameter. */
    size_t top = 0;
    stack[top++] = seed;
    in[seed] = 1;
    size_t n_cluster = 0;
    double sw = 0.0, saz = 0.0, srg = 0.0;

    while (top > 0) {
        const size_t w = stack[--top];
        const size_t a = w / nrg, r = w % nrg;
        n_cluster++;

        const double excess = spec->prominence[w] - med;
        if (excess > 0.0) {
            sw  += excess;
            saz += excess * (double)a;
            srg += excess * (double)r;
        }

        const long da[4] = { 1, -1, 0, 0 }, dr[4] = { 0, 0, 1, -1 };
        for (int k = 0; k < 4; k++) {
            const long na = (long)a + da[k], nr = (long)r + dr[k];
            if (na < 0 || nr < 0 ||
                na >= (long)spec->n_win_az || nr >= (long)nrg) continue;
            const size_t nv = (size_t)na * nrg + (size_t)nr;
            if (in[nv]) continue;
            if (fabs(spec->dominant_freq[nv] - f0) > tol) continue;
            in[nv] = 1;
            stack[top++] = nv;
        }
    }
    free(in); free(stack);

    /* Every member at or below the scene median leaves no mass to average. The
     * seed alone then answers, which is argmax's behaviour and is reported as
     * a one-window cluster rather than dressed up. */
    if (!(sw > 0.0)) {
        out->c_az = (double)(seed / nrg);
        out->c_rg = (double)(seed % nrg);
    } else {
        out->c_az = saz / sw;
        out->c_rg = srg / sw;
    }
    out->seed = seed;
    out->freq_hz = f0;
    out->n_cluster = n_cluster;
    /* Truncation is about where the ANSWER sits, not where the cluster reaches.
     * A centroid a full window inside the boundary has the target's whole
     * footprint on the grid even when the agreeing block runs to the edge --
     * measured, four of five accurate placements had cluster AND weight
     * touching the edge and were still right to 0.01 windows. What actually
     * biases the answer is the target itself being at the boundary, so half its
     * footprint is off the grid. That is this test. */
    out->clipped = (out->c_az < 1.0 || out->c_rg < 1.0 ||
                    out->c_az > (double)spec->n_win_az - 2.0 ||
                    out->c_rg > (double)nrg - 2.0);

    /* Window w spans pixels [w*stride, w*stride + win), so its centre is
     * w*stride + (win-1)/2 -- rs_microm_track()'s convention. */
    if (stride_az > 0 && win_az > 0)
        out->az_px = out->c_az * (double)stride_az + 0.5 * (double)(win_az - 1);
    if (stride_rg > 0 && win_rg > 0)
        out->rg_px = out->c_rg * (double)stride_rg + 0.5 * (double)(win_rg - 1);
    return RS_OK;
}

/* Prominence at a nominated frequency for one window. See microm.h on why this
 * is a different question from the dominant peak, and on item 38. */
resonarsat_status_t rs_spectrum_prominence_at(const rs_spectrum_t *spec,
                                              size_t window,
                                              double freq_hz,
                                              size_t *out_bin,
                                              double *out_psd,
                                              double *out_prom)
{
    if (out_bin) *out_bin = 0;
    if (out_psd) *out_psd = 0.0;
    if (out_prom) *out_prom = 0.0;

    if (!spec || !spec->psd || !spec->freq || spec->n_freq == 0) {
        rs_set_error("prominence at frequency: spectrum has no power density");
        return RS_ERR_ARG;
    }
    if (window >= spec->n_win) {
        rs_set_error("prominence at frequency: window %zu of %zu",
                     window, spec->n_win);
        return RS_ERR_ARG;
    }
    if (!(freq_hz >= 0.0) || !(spec->df > 0.0)) {
        rs_set_error("prominence at frequency: %.6g Hz is not a frequency this "
                     "spectrum can be asked about", freq_hz);
        return RS_ERR_ARG;
    }

    /* Snap to the nearest bin, clamped to the band the transform covers. A
     * caller naming a frequency above Nyquist has made an error worth
     * reporting rather than silently answering about the top bin. */
    if (freq_hz > spec->freq[spec->n_freq - 1] + 0.5 * spec->df) {
        rs_set_error("prominence at frequency: %.6g Hz is above this spectrum's "
                     "%.6g Hz limit", freq_hz, spec->freq[spec->n_freq - 1]);
        return RS_ERR_RANGE;
    }
    size_t k = (size_t)(freq_hz / spec->df + 0.5);
    if (k >= spec->n_freq) k = spec->n_freq - 1;

    const double *psd = spec->psd + window * spec->n_freq;
    /* Same definition as the dominant-peak column: this bin's power against the
     * mean of every other bin, so the two numbers can be subtracted and
     * compared. n_freq == 1 has no "rest" to compare against. */
    double sum = 0.0;
    for (size_t i = 0; i < spec->n_freq; i++) sum += psd[i];
    const double rest = sum - psd[k];
    const double mean_rest = (spec->n_freq > 1)
                           ? rest / (double)(spec->n_freq - 1) : 0.0;

    if (out_bin) *out_bin = k;
    if (out_psd) *out_psd = psd[k];
    if (out_prom) *out_prom = (mean_rest > 0.0) ? psd[k] / mean_rest : 0.0;
    return RS_OK;
}

/* Ascending comparator over doubles, for the median and MAD below. */
static int rs_cmp_double(const void *a, const void *b)
{
    const double x = *(const double *)a, y = *(const double *)b;
    return (x > y) - (x < y);
}

/* Median of an array the caller allows us to reorder. Returns 0 for n == 0 so
 * the callers below can treat an empty reference set as "no background". */
static double rs_median_inplace(double *v, size_t n)
{
    if (n == 0) return 0.0;
    qsort(v, n, sizeof *v, rs_cmp_double);
    return (n % 2) ? v[n / 2] : 0.5 * (v[n / 2 - 1] + v[n / 2]);
}

/* Predicted detectable-amplitude floor of one window. See microm.h for the
 * derivation, and for the three different floors this is one of. */
resonarsat_status_t rs_microm_floor(const rs_microm_t *m, double lambda_m,
                                    size_t window, double *out_floor_m,
                                    double *out_phase_sd)
{
    if (out_floor_m)  *out_floor_m = 0.0;
    if (out_phase_sd) *out_phase_sd = 0.0;
    if (!m || !m->phase) {
        rs_set_error("window floor: no phase series; this is the phase route only");
        return RS_ERR_ARG;
    }
    if (window >= m->n_win || m->n_looks < 2 || !(lambda_m > 0.0)) {
        rs_set_error("window floor: window %zu of %zu, %zu looks, lambda %g m",
                     window, m->n_win, m->n_looks, lambda_m);
        return RS_ERR_ARG;
    }
    /* Circular mean resultant: a wrapped series has no linear standard
     * deviation, and treating it as if it had is what item 101 corrected. */
    double sx = 0.0, sy = 0.0;
    for (size_t k = 0; k < m->n_looks; k++) {
        const double p = m->phase[window * m->n_looks + k];
        sx += cos(p); sy += sin(p);
    }
    double R = sqrt(sx * sx + sy * sy) / (double)m->n_looks;
    if (R < 1e-12) R = 1e-12;          /* uniform phase: sd saturates, not diverges */
    if (R > 1.0)   R = 1.0;
    const double sd = sqrt(-2.0 * log(R));
    if (out_phase_sd) *out_phase_sd = sd;
    if (out_floor_m)
        *out_floor_m = sd * lambda_m / (4.0 * M_PI)
                     * sqrt(2.0 / (double)m->n_looks);
    return RS_OK;
}

/* Two-sample change statistic for a paired run. See microm.h for the model, the
 * derivation and the two-degrees-of-freedom ceiling this carries. */
resonarsat_status_t rs_twin_llr(double p_injected, double p_twin,
                                double *out_llr, double *out_p)
{
    if (out_llr) *out_llr = 0.0;
    if (out_p)   *out_p   = 1.0;
    if (!(p_injected > 0.0) || !(p_twin > 0.0)) {
        rs_set_error("twin llr: powers must be positive, got %g and %g",
                     p_injected, p_twin);
        return RS_ERR_ARG;
    }
    const double r = p_injected / p_twin;
    /* One-sided: below r = 1 the injected run has LESS power, which is the H0
     * boundary, so the statistic is zero rather than negative. */
    if (out_llr)
        *out_llr = (r <= 1.0) ? 0.0
                 : 2.0 * log1p(r) - 2.0 * M_LN2 - log(r);
    /* Exact under H0: the ratio of two i.i.d. exponentials is F(2,2). */
    if (out_p) *out_p = (r <= 1.0) ? 1.0 : 1.0 / (1.0 + r);
    return RS_OK;
}

/* One (index, key) pair, so reference windows can be ranked by how close their
 * amplitude dispersion is to the candidate's without disturbing the spectrum. */
typedef struct { double key; size_t idx; } rs_keyed_t;

/* Ascending comparator over rs_keyed_t.key. Ties keep qsort's order, which is
 * unspecified but harmless: tied keys are equally good matches by definition. */
static int rs_cmp_keyed(const void *a, const void *b)
{
    const double x = ((const rs_keyed_t *)a)->key, y = ((const rs_keyed_t *)b)->key;
    return (x > y) - (x < y);
}

/* Score every window against the rest of its own scene. See microm.h for why
 * this is spatial rather than a time-domain surrogate, and for what it cannot
 * test. */
resonarsat_status_t rs_spectrum_scene_null(const rs_spectrum_t *spec,
                                           size_t guard,
                                           rs_scene_null_t *out)
{
    if (!out) {
        rs_set_error("scene null: no output structure");
        return RS_ERR_ARG;
    }
    memset(out, 0, sizeof *out);
    if (!spec || !spec->prominence || !spec->dominant_freq || spec->n_win == 0) {
        rs_set_error("scene null: spectrum has no per-window prominence");
        return RS_ERR_ARG;
    }
    if (spec->n_win_az == 0 || spec->n_win_rg == 0 ||
        spec->n_win_az * spec->n_win_rg != spec->n_win) {
        rs_set_error("scene null: window grid %zux%zu does not describe %zu windows",
                     spec->n_win_az, spec->n_win_rg, spec->n_win);
        return RS_ERR_ARG;
    }
    out->window = spec->n_win;              /* "none qualified" until one does */

    const size_t n = spec->n_win;
    /* Three scratch arrays sized once: the candidate's matched keys, the
     * retained reference prominences, and the deviations the MAD is taken of. */
    rs_keyed_t *keys = malloc(n * sizeof *keys);
    double *ref = malloc(n * sizeof *ref);
    double *dev = malloc(n * sizeof *dev);
    if (!keys || !ref || !dev) {
        free(keys); free(ref); free(dev);
        rs_set_error("scene null: out of memory for %zu windows", n);
        return RS_ERR_ALLOC;
    }

    /* Matching needs a D_A that actually varies. rs_spectrum_compute() fills
     * every entry with RS_DA_MAX when the tracker left none, and a constant key
     * ranks nothing -- so detect that once rather than silently "matching" on a
     * constant. */
    int have_da = 0;
    if (spec->d_a) {
        for (size_t w = 1; w < n && !have_da; w++)
            if (spec->d_a[w] != spec->d_a[0]) have_da = 1;
    }
    out->matched = have_da;

    double best_z = -DBL_MAX, second_z = -DBL_MAX;
    size_t n_searched = 0;

    for (size_t w = 0; w < n; w++) {
        const double p_w = spec->prominence[w];
        if (!(p_w > 0.0)) continue;         /* masked or empty window */
        const size_t caz = w / spec->n_win_rg, crg = w % spec->n_win_rg;

        /* Everything outside the guard ring is a candidate reference. The ring
         * is Chebyshev because window overlap is separable in the two axes. */
        size_t n_keys = 0;
        for (size_t v = 0; v < n; v++) {
            if (!(spec->prominence[v] > 0.0)) continue;
            const size_t az = v / spec->n_win_rg, rg = v % spec->n_win_rg;
            const size_t daz = (az > caz) ? az - caz : caz - az;
            const size_t drg = (rg > crg) ? rg - crg : crg - rg;
            if (daz <= guard && drg <= guard) continue;
            keys[n_keys].idx = v;
            keys[n_keys].key = have_da ? fabs(spec->d_a[v] - spec->d_a[w]) : 0.0;
            n_keys++;
        }
        if (n_keys < RS_SCENE_NULL_MIN_REF) continue;

        /* Keep the closest matches in D_A. Without D_A every key is zero, the
         * sort is a no-op and this becomes a plain unmatched neighbourhood --
         * which is what 'matched' reports to the caller. */
        size_t n_ref = n_keys;
        if (have_da && n_keys > RS_SCENE_NULL_MATCH) {
            qsort(keys, n_keys, sizeof *keys, rs_cmp_keyed);
            n_ref = RS_SCENE_NULL_MATCH;
        }
        for (size_t i = 0; i < n_ref; i++) ref[i] = spec->prominence[keys[i].idx];

        const double med = rs_median_inplace(ref, n_ref);
        for (size_t i = 0; i < n_ref; i++) dev[i] = fabs(ref[i] - med);
        const double mad = rs_median_inplace(dev, n_ref);
        /* A degenerate scale means every reference window scored identically,
         * which is a background with no spread rather than an infinitely
         * significant candidate. Skipping is the honest reading. */
        const double scale = 1.4826 * mad;
        if (!(scale > 0.0)) continue;

        const double z = (p_w - med) / scale;
        n_searched++;
        if (z > best_z) {
            second_z = best_z;
            best_z = z;
            out->window = w;
            out->freq_hz = spec->dominant_freq[w];
            out->prominence = p_w;
            out->z = z;
            out->ref_median = med;
            out->ref_scale = scale;
            out->n_ref = n_ref;
        } else if (z > second_z) {
            second_z = z;
        }
    }

    free(keys); free(ref); free(dev);

    if (n_searched == 0) {
        memset(out, 0, sizeof *out);
        out->window = spec->n_win;
        rs_set_error("scene null: no window had %u reference windows outside a "
                     "guard of %zu with a non-degenerate spread",
                     RS_SCENE_NULL_MIN_REF, guard);
        return RS_ERR_RANGE;
    }
    out->n_searched = n_searched;
    out->z_runner_up = (second_z == -DBL_MAX) ? 0.0 : second_z;
    return RS_OK;
}

/* Observation ratio for a measured frequency. See the header on why this is a
 * diagnostic quantity and not a validity threshold. */
double rs_spectrum_observation_ratio(double t_sap, double freq)
{
    if (!(freq > 0.0)) return 0.0;
    return t_sap * freq;
}

/* Sinc response of the sub-aperture averaging window. See microm.h. */
double rs_spectrum_subaperture_response(double t_sap, double freq)
{
    const double x = M_PI * freq * t_sap;
    if (!(t_sap > 0.0) || !(freq > 0.0)) return 1.0;
    /* The small-argument branch avoids 0/0; sinc(x) -> 1 - x^2/6 there, and the
     * cutoff is where the series and the quotient agree to double precision. */
    if (fabs(x) < 1.0e-8) return 1.0;
    return fabs(sin(x) / x);
}
