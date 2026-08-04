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

    const size_t k_lo = RS_SPECTRUM_LEAKAGE_BINS;
    const size_t lo = (k > k_lo + RS_LOCAL_HALF_BINS) ? k - RS_LOCAL_HALF_BINS : k_lo;
    const size_t hi = (k + RS_LOCAL_HALF_BINS + 1 < n_freq)
                    ? k + RS_LOCAL_HALF_BINS + 1 : n_freq;
    size_t n_ref = 0;
    for (size_t j = lo; j < hi; j++) {
        const size_t d = (j > k) ? j - k : k - j;
        if (d <= RS_LOCAL_GUARD_BINS) continue;
        ref[n_ref++] = P[j];
    }
    if (n_ref >= 4) {
        const double med = rs_median_inplace(ref, n_ref);
        if (med > 0.0) {
            out->am_ratio = P[k] / med;
            out->ref_median = med;
        }
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

    double *ref = malloc((2 * RS_LOCAL_HALF_BINS + 1) * sizeof *ref);
    if (!ref) {
        rs_set_error("local peak: out of memory");
        return RS_ERR_ALLOC;
    }

    double best = -1.0;
    size_t n_searched = 0;

    for (size_t w = 0; w < spec->n_win; w++) {
        const double *P = spec->psd + w * n_freq;
        for (size_t k = k_lo; k < n_freq; k++) {
            /* The neighbourhood is clipped to the admissible band, so a
             * candidate near either end is scored against fewer bins rather
             * than against bins that do not exist or must not be read. */
            const size_t lo = (k > k_lo + RS_LOCAL_HALF_BINS)
                            ? k - RS_LOCAL_HALF_BINS : k_lo;
            const size_t hi = (k + RS_LOCAL_HALF_BINS + 1 < n_freq)
                            ? k + RS_LOCAL_HALF_BINS + 1 : n_freq;
            size_t n_ref = 0;
            for (size_t j = lo; j < hi; j++) {
                const size_t d = (j > k) ? j - k : k - j;
                if (d <= RS_LOCAL_GUARD_BINS) continue;
                ref[n_ref++] = P[j];
            }
            /* Fewer than four reference bins is a background estimated from
             * noise, not an estimate of noise. */
            if (n_ref < 4) continue;
            const double med = rs_median_inplace(ref, n_ref);
            if (!(med > 0.0)) continue;

            n_searched++;
            const double ratio = P[k] / med;
            if (ratio > best) {
                best = ratio;
                out->window = w;
                out->bin = k;
                out->freq_hz = spec->freq[k];
                out->ratio = ratio;
                out->ref_median = med;
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
