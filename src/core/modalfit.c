/* Joint transient-and-mode estimation on a short record.
 *
 * See microm.h on rs_transient_fit() for why a Hann window is the wrong thing to
 * do to a burst, and what the fitted model is. This file is the arithmetic. */

#include "resonarsat/microm.h"
#include "resonarsat/fft.h"

#include <complex.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

/* Ascending comparator over doubles, for the medians reported per scene. */
static int rs_tf_cmp_double(const void *a, const void *b)
{
    const double x = *(const double *)a, y = *(const double *)b;
    return (x > y) - (x < y);
}

/* Median of an array the caller allows us to reorder; 0 for an empty one. */
static double rs_tf_median(double *v, size_t n)
{
    if (n == 0) return 0.0;
    qsort(v, n, sizeof *v, rs_tf_cmp_double);
    return (n % 2) ? v[n / 2] : 0.5 * (v[n / 2 - 1] + v[n / 2]);
}

/* Remove a least-squares line from 'y' in place, so the carrier residual items
 * 51-53 leave behind does not become the first "mode" the search finds.
 * Returns the variance about the fitted line, which is what every var_frac
 * below is a fraction of. */
static double rs_tf_detrend(double *y, size_t n)
{
    double sx = 0.0, sxx = 0.0, sy = 0.0, sxy = 0.0;
    for (size_t i = 0; i < n; i++) {
        const double x = (double)i;
        sx += x; sxx += x * x; sy += y[i]; sxy += x * y[i];
    }
    const double det = (double)n * sxx - sx * sx;
    double a = sy / (double)n, b = 0.0;
    if (det != 0.0) {
        b = ((double)n * sxy - sx * sy) / det;
        a = (sy - b * sx) / (double)n;
    }
    double ss = 0.0;
    for (size_t i = 0; i < n; i++) {
        y[i] -= a + b * (double)i;
        ss += y[i] * y[i];
    }
    return ss;
}

/* The energy a damped sinusoid of frequency 'f' and decay 'alpha' starting at
 * sample 'i0' removes from 'y', with its two linear coefficients solved exactly.
 *
 * This is the variable-projection inner step: the model is linear in the
 * in-phase and quadrature coefficients, so for each nonlinear triple only a 2x2
 * normal-equation system is solved and the objective is the explained sum of
 * squares. 'c' and 's' hold the basis for onset zero, of length n. An onset of
 * i0 uses the FIRST m = n - i0 basis samples against y[i0..n-1], so the
 * basis-only sums are PREFIX sums of the basis and cost nothing per onset:
 * 'scc', 'sss' and 'scs' are indexed by that count m, never by i0.
 *
 * Writes the coefficients to 'out_a' and 'out_b' and returns the explained
 * energy, or 0 when the system is singular -- which happens legitimately at
 * f = 0 and at Nyquist, where the quadrature basis vanishes. */
static double rs_tf_explained(const double *y, size_t n, size_t i0,
                              const double *c, const double *s,
                              const double *scc, const double *sss,
                              const double *scs,
                              double *out_a, double *out_b)
{
    const size_t m = n - i0;
    double syc = 0.0, sys = 0.0;
    for (size_t j = 0; j < m; j++) {
        syc += y[i0 + j] * c[j];
        sys += y[i0 + j] * s[j];
    }
    const double Scc = scc[m], Sss = sss[m], Scs = scs[m];
    const double det = Scc * Sss - Scs * Scs;
    /* Scaled against the diagonal, so the test means "these two basis vectors
     * are collinear" rather than "the numbers are small". */
    if (!(det > 1e-12 * Scc * Sss) || !(Scc > 0.0) || !(Sss > 0.0)) {
        *out_a = *out_b = 0.0;
        return 0.0;
    }
    const double a = (syc * Sss - sys * Scs) / det;
    const double b = (sys * Scc - syc * Scs) / det;
    *out_a = a; *out_b = b;
    const double ex = a * syc + b * sys;
    return (ex > 0.0) ? ex : 0.0;
}

/* Fit the transient and the modes together. See microm.h. */
resonarsat_status_t rs_transient_fit(const double *series, size_t n, double fs,
                                     double f_min, double f_max,
                                     size_t n_mode_max,
                                     rs_transient_fit_t *out)
{
    if (!out) {
        rs_set_error("transient fit: no output structure");
        return RS_ERR_ARG;
    }
    memset(out, 0, sizeof *out);
    if (!series || n < 16 || !(fs > 0.0)) {
        rs_set_error("transient fit: need at least 16 samples at a positive "
                     "rate, got %zu at %g Hz", n, fs);
        return RS_ERR_ARG;
    }
    if (n_mode_max > RS_TFIT_MAX_MODES) n_mode_max = RS_TFIT_MAX_MODES;
    if (n_mode_max == 0) n_mode_max = 1;

    const double dt = 1.0 / fs;
    const double T  = (double)n * dt;
    const double df = fs / (double)n;
    const double nyq = 0.5 * fs;
    double lo = (f_min > 0.0) ? f_min : (double)RS_SPECTRUM_LEAKAGE_BINS * df;
    double hi = (f_max > 0.0 && f_max < nyq) ? f_max : nyq;
    /* The first RS_SPECTRUM_LEAKAGE_BINS bins are not reportable anywhere in
     * this project and that is not a tunable, so the fit does not search them
     * either -- a "mode" at one cycle per dwell is a trend. */
    const double floor_hz = (double)RS_SPECTRUM_LEAKAGE_BINS * df;
    if (lo < floor_hz) lo = floor_hz;
    if (!(hi > lo)) {
        rs_set_error("transient fit: band %g-%g Hz holds no grid point at a bin "
                     "spacing of %g Hz", lo, hi, df);
        return RS_ERR_RANGE;
    }

    const double f_step = df / (double)RS_TFIT_FREQ_OVERSAMPLE;
    const size_t n_f = (size_t)floor((hi - lo) / f_step) + 1;
    /* Onsets on a sixteenth of the record, capped where too few samples remain
     * to determine a mode at all. */
    const size_t onset_step = (n / 16 > 0) ? n / 16 : 1;
    const size_t i0_max = (size_t)((double)n * RS_TFIT_ONSET_MAX_FRAC);

    double *y   = malloc(n * sizeof *y);
    double *c   = malloc(n * sizeof *c);
    double *s   = malloc(n * sizeof *s);
    double *scc = malloc((n + 1) * sizeof *scc);
    double *sss = malloc((n + 1) * sizeof *sss);
    double *scs = malloc((n + 1) * sizeof *scs);
    if (!y || !c || !s || !scc || !sss || !scs) {
        free(y); free(c); free(s); free(scc); free(sss); free(scs);
        rs_set_error("transient fit: out of memory");
        return RS_ERR_ALLOC;
    }

    for (size_t i = 0; i < n; i++) y[i] = series[i];
    const double var_total = rs_tf_detrend(y, n);
    out->var_total = var_total / (double)n;
    if (!(var_total > 0.0)) {
        free(y); free(c); free(s); free(scc); free(sss); free(scs);
        out->resid_frac = 1.0;
        return RS_OK;                       /* a flat series: no modes, no error */
    }

    double resid = var_total;
    for (size_t k = 0; k < n_mode_max; k++) {
        double best_ex = 0.0, best_f = 0.0, best_alpha = 0.0;
        double best_a = 0.0, best_b = 0.0;
        size_t best_i0 = 0;

        for (size_t fi = 0; fi < n_f; fi++) {
            const double f = lo + (double)fi * f_step;
            const double w = 2.0 * M_PI * f;
            for (size_t di = 0; di < RS_TFIT_N_DECAY; di++) {
                /* alpha*T from 0 (sustained) to RS_TFIT_DECAY_MAX, so the grid
                 * means the same thing whatever the dwell length is. */
                const double decay = RS_TFIT_DECAY_MAX * (double)di
                                   / (double)(RS_TFIT_N_DECAY - 1);
                const double alpha = decay / T;

                for (size_t j = 0; j < n; j++) {
                    const double tau = (double)j * dt;
                    const double e = exp(-alpha * tau);
                    c[j] = e * cos(w * tau);
                    s[j] = e * sin(w * tau);
                }
                /* Prefix sums over the basis, indexed by how many samples an
                 * onset leaves, so every onset reuses this one pass. */
                scc[0] = sss[0] = scs[0] = 0.0;
                for (size_t j = 0; j < n; j++) {
                    scc[j + 1] = scc[j] + c[j] * c[j];
                    sss[j + 1] = sss[j] + s[j] * s[j];
                    scs[j + 1] = scs[j] + c[j] * s[j];
                }
                for (size_t i0 = 0; i0 <= i0_max; i0 += onset_step) {
                    double a, b;
                    const double ex = rs_tf_explained(y, n, i0, c, s,
                                                      scc, sss, scs, &a, &b);
                    if (ex > best_ex) {
                        best_ex = ex; best_f = f; best_alpha = alpha;
                        best_i0 = i0; best_a = a; best_b = b;
                    }
                }
            }
        }

        if (!(best_ex > 0.0)) break;

        /* Subtract it and record it. */
        const double w = 2.0 * M_PI * best_f;
        for (size_t i = best_i0; i < n; i++) {
            const double tau = (double)(i - best_i0) * dt;
            const double e = exp(-best_alpha * tau);
            y[i] -= best_a * e * cos(w * tau) + best_b * e * sin(w * tau);
        }
        double ss = 0.0;
        for (size_t i = 0; i < n; i++) ss += y[i] * y[i];

        rs_transient_mode_t *md = &out->mode[out->n_mode++];
        md->freq_hz  = best_f;
        md->zeta     = (best_f > 0.0) ? best_alpha / (2.0 * M_PI * best_f) : 0.0;
        md->onset_s  = (double)best_i0 * dt;
        md->amp      = hypot(best_a, best_b);
        md->phase_rad = atan2(-best_b, best_a);
        md->var_frac = (resid - ss) / var_total;
        if (md->var_frac < 0.0) md->var_frac = 0.0;
        resid = ss;
    }

    out->resid_frac = (var_total > 0.0) ? resid / var_total : 1.0;
    free(y); free(c); free(s); free(scc); free(sss); free(scs);
    return RS_OK;
}

/* Fit every window and express the answer as a spectrum. See microm.h for why
 * the spectrum this writes is the fit's rather than the data's. */
resonarsat_status_t rs_transient_fit_windows(rs_spectrum_t *spec,
                                             const rs_microm_t *m,
                                             rs_spectrum_source_t source,
                                             size_t n_mode_max,
                                             double f_min,
                                             rs_transient_stats_t *out_stats)
{
    if (out_stats) memset(out_stats, 0, sizeof *out_stats);
    if (!spec || !m || !spec->psd || !m->disp_los || !m->vel_los) {
        rs_set_error("transient fit: NULL spectrum or tracking result");
        return RS_ERR_ARG;
    }
    const size_t n = m->n_looks, n_freq = spec->n_freq;
    if (n < 16 || n_freq == 0) {
        rs_set_error("transient fit: %zu looks is too few to fit", n);
        return RS_ERR_ARG;
    }
    const double *all = (source == RS_SPEC_DISPLACEMENT) ? m->disp_los : m->vel_los;
    const double fs = (spec->df > 0.0) ? spec->df * (double)n : 1.0;

    double *resid = malloc(n * sizeof *resid);
    float complex *buf = malloc(n * sizeof *buf);
    double *zetas  = malloc(spec->n_win * RS_TFIT_MAX_MODES * sizeof *zetas);
    double *onsets = malloc(spec->n_win * RS_TFIT_MAX_MODES * sizeof *onsets);
    double *resids = malloc(spec->n_win * sizeof *resids);
    rs_fft_plan *plan = NULL;
    if (!resid || !buf || !zetas || !onsets || !resids ||
        rs_fft_plan_create(n, &plan) != RS_OK) {
        free(resid); free(buf); free(zetas); free(onsets); free(resids);
        rs_set_error("transient fit: out of memory");
        return RS_ERR_ALLOC;
    }
    size_t n_stat = 0, n_fitted = 0;

    for (size_t w = 0; w < spec->n_win; w++) {
        double *psd = spec->psd + w * n_freq;
        for (size_t k = 0; k < n_freq; k++) psd[k] = 0.0;
        spec->dominant_freq[w] = 0.0;
        spec->amplitude[w] = 0.0;
        spec->prominence[w] = 0.0;

        rs_transient_fit_t fit;
        if (rs_transient_fit(all + w * n, n, fs, f_min, 0.0, n_mode_max,
                             &fit) != RS_OK)
            continue;
        resids[n_fitted++] = fit.resid_frac;

        /* The residual's periodogram is the floor every local-background
         * statistic downstream needs; without it a line spectrum has no
         * neighbourhood to stand above. Unwindowed, to stay consistent with the
         * fit that produced it. */
        for (size_t i = 0; i < n; i++) resid[i] = all[w * n + i];
        rs_tf_detrend(resid, n);
        for (size_t k = 0; k < fit.n_mode; k++) {
            const rs_transient_mode_t *md = &fit.mode[k];
            const double om = 2.0 * M_PI * md->freq_hz;
            const double alpha = md->zeta * om;
            const size_t i0 = (size_t)(md->onset_s * fs + 0.5);
            for (size_t i = i0; i < n; i++) {
                const double tau = (double)(i - i0) / fs;
                resid[i] -= md->amp * exp(-alpha * tau)
                          * cos(om * tau + md->phase_rad);
            }
        }
        for (size_t i = 0; i < n; i++) buf[i] = (float complex)resid[i];
        if (rs_fft_forward(plan, buf) == RS_OK) {
            for (size_t k = 0; k < n_freq; k++) {
                const double mag = (double)cabsf(buf[k]);
                const double scale =
                    (k == 0 || (n % 2 == 0 && k == n_freq - 1)) ? 1.0 : 2.0;
                psd[k] = scale * mag * mag / ((double)n * fs);
            }
        }

        /* Each fitted mode's power at its OWN bin. The line is narrow because
         * the damping is a parameter here, not a smear left in the data. */
        for (size_t k = 0; k < fit.n_mode; k++) {
            const rs_transient_mode_t *md = &fit.mode[k];
            long bin = (spec->df > 0.0) ? lrint(md->freq_hz / spec->df) : 0;
            if (bin < 0) bin = 0;
            if ((size_t)bin >= n_freq) bin = (long)n_freq - 1;
            /* Mean-square of the mode over the record, per bin width, so it is
             * commensurate with the residual psd beside it. */
            const double om = 2.0 * M_PI * md->freq_hz;
            const double alpha = md->zeta * om;
            const double dur = (double)n / fs - md->onset_s;
            double ms;
            if (alpha * dur > 1e-6)
                ms = 0.5 * md->amp * md->amp * (1.0 - exp(-2.0 * alpha * dur))
                   / (2.0 * alpha * ((double)n / fs));
            else
                ms = 0.5 * md->amp * md->amp * dur / ((double)n / fs);
            psd[(size_t)bin] += ms / (spec->df > 0.0 ? spec->df : 1.0);

            if (n_stat < spec->n_win * RS_TFIT_MAX_MODES) {
                zetas[n_stat] = md->zeta;
                onsets[n_stat] = md->onset_s;
                n_stat++;
            }
        }

        /* Read back exactly as rs_spectrum_maxhold() does, so every downstream
         * policy sees the same contract. */
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

    if (out_stats) {
        out_stats->n_fitted = n_fitted;
        out_stats->n_mode_total = n_stat;
        out_stats->median_zeta = rs_tf_median(zetas, n_stat);
        out_stats->median_onset_s = rs_tf_median(onsets, n_stat);
        out_stats->median_resid = rs_tf_median(resids, n_fitted);
    }

    rs_fft_plan_destroy(plan);
    free(resid); free(buf); free(zetas); free(onsets); free(resids);
    return RS_OK;
}
