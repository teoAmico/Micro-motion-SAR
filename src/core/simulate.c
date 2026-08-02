/* Static-scene phase history over a real collect's geometry. See simulate.h. */

#include "resonarsat/simulate.h"

#include <complex.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef RS_C_LIGHT
#define RS_C_LIGHT 299792458.0
#endif

/* A small deterministic generator, so a seed reproduces a realisation exactly
 * on any platform.
 *
 * rand() is unsuitable here for a reason that matters to the result rather than
 * to taste: its sequence differs between C libraries, so a null distribution
 * quoted in the documentation would not reproduce on another machine. This is
 * xorshift64*, which is short enough to audit and has no structure at the
 * lag lengths a speckle field cares about. */
static unsigned long long rs_sim_rand(unsigned long long *state)
{
    unsigned long long x = *state;
    x ^= x >> 12;
    x ^= x << 25;
    x ^= x >> 27;
    *state = x;
    return x * 2685821657736338717ULL;
}

/* Uniform in [0,1). */
static double rs_sim_uniform(unsigned long long *state)
{
    return (double)(rs_sim_rand(state) >> 11) * (1.0 / 9007199254740992.0);
}

/* Synthesise a static scene with the reference's geometry. See simulate.h. */
resonarsat_status_t rs_simulate_static_like(const rs_cphd_t *ref, unsigned seed,
                                            size_t n_target, const double centre[2],
                                            double extent_m,
                                            size_t n_rbin, rs_cphd_t *out)
{
    if (!ref || !out || !centre) return RS_ERR_ARG;
    if (ref->n_pulse == 0 || !ref->t || !ref->pos) {
        rs_set_error("simulate: the reference collect carries no pulse geometry");
        return RS_ERR_ARG;
    }
    if (n_target == 0) n_target = 400;
    if (!(extent_m > 0.0)) extent_m = 300.0;
    if (n_rbin == 0 || n_rbin > ref->n_rbin) n_rbin = ref->n_rbin;
    if (n_rbin < 16) n_rbin = 16;

    const size_t n_pulse = ref->n_pulse;
    resonarsat_status_t st = rs_cphd_alloc(out, n_pulse, n_rbin);
    if (st != RS_OK) return st;

    out->fc     = ref->fc;
    out->lambda = (ref->fc > 0.0) ? RS_C_LIGHT / ref->fc : ref->lambda;
    out->prf    = ref->prf;
    out->dr     = ref->dr;
    out->plane  = ref->plane;
    out->phase_ref_srp = 1;
    snprintf(out->source, sizeof out->source, "simulated-static/%zux%zu",
             n_pulse, n_rbin);

    /* Geometry copied verbatim. The whole point is that the aperture, the dwell
     * and the Doppler history are the real ones: a null built on an idealised
     * straight track would not inherit whatever the real orbit does to the
     * sub-look overlap. */
    memcpy(out->t, ref->t, n_pulse * sizeof *out->t);
    memcpy(out->pos, ref->pos, 3 * n_pulse * sizeof *out->pos);

    /* Scatterers, static by construction: there is no time argument anywhere in
     * the position below, which is the property the whole test rests on. */
    double *tx = malloc(n_target * sizeof *tx);
    double *ty = malloc(n_target * sizeof *ty);
    double *ta = malloc(n_target * sizeof *ta);
    double *tp = malloc(n_target * sizeof *tp);
    if (!tx || !ty || !ta || !tp) {
        free(tx); free(ty); free(ta); free(tp);
        rs_cphd_free(out);
        return RS_ERR_ALLOC;
    }

    unsigned long long rng = (unsigned long long)seed * 6364136223846793005ULL + 1442695040888963407ULL;
    if (rng == 0) rng = 88172645463325252ULL;

    for (size_t g = 0; g < n_target; g++) {
        tx[g] = centre[0] + (rs_sim_uniform(&rng) * 2.0 - 1.0) * extent_m;
        ty[g] = centre[1] + (rs_sim_uniform(&rng) * 2.0 - 1.0) * extent_m;
        /* Rayleigh amplitude gives the focused image the speckle statistics of
         * distributed clutter. A field of equal-amplitude points would focus
         * unnaturally cleanly and understate the tracker's noise. */
        const double u = rs_sim_uniform(&rng);
        ta[g] = sqrt(-2.0 * log(u > 1e-12 ? u : 1e-12));
        tp[g] = rs_sim_uniform(&rng) * 2.0 * M_PI;
    }

    /* Recentre the swath on the grid origin, so the scene sits mid-range
     * whatever range extent the caller asked for. */
    const double mx = ref->pos[3 * (n_pulse / 2) + 0] - centre[0];
    const double my = ref->pos[3 * (n_pulse / 2) + 1] - centre[1];
    const double mz = ref->pos[3 * (n_pulse / 2) + 2];
    const double r0 = sqrt(mx * mx + my * my + mz * mz);
    out->r_near = r0 - 0.5 * (double)n_rbin * out->dr;

    const double k_phase = 4.0 * M_PI / out->lambda;
    const double sigma = 2.0 * out->dr / 2.355;   /* FWHM of about two bins */
    const double inv_2s2 = 1.0 / (2.0 * sigma * sigma);

#ifdef _OPENMP
#pragma omp parallel for schedule(static)
#endif
    for (size_t i = 0; i < n_pulse; i++) {
        const double px = out->pos[3 * i + 0];
        const double py = out->pos[3 * i + 1];
        const double pz = out->pos[3 * i + 2];

        /* The receive window follows the CENTRE OF THE TARGET FIELD, which is
         * the processing grid's origin and not the scene's. Using the scene
         * origin here while placing the swath around the grid put every target
         * about a kilometre outside a 378 m swath, so none was deposited and the
         * focused image came back empty. The two must be the same point. */
        const double cx = px - centre[0], cy = py - centre[1];
        const double rref = sqrt(cx * cx + cy * cy + pz * pz);
        out->r_ref[i] = rref;

        float complex *row = out->signal + i * n_rbin;

        for (size_t g = 0; g < n_target; g++) {
            const double dx = px - tx[g];
            const double dy = py - ty[g];
            const double R = sqrt(dx * dx + dy * dy + pz * pz);


            const double fbin = (R - rref) / out->dr + 0.5 * (double)n_rbin;
            if (fbin < 0.0 || fbin >= (double)n_rbin) continue;

            /* SRP-REFERENCED, TO MATCH WHAT phase_ref_srp SAYS BELOW AND WHAT
             * A REAL COLLECT CARRIES. rs_focus_backproject() undoes
             * k*(R - r_ref) when that flag is set and k*R when it is not, so
             * writing the absolute phase here while setting the flag left
             * exp(-i k r_ref[p]) behind -- a per-pulse phase of many cycles,
             * common to every pixel, which destroys the coherent sum. Measured:
             * the simulated scene focused to a peak-to-mean of 3.6, i.e. noise,
             * where the same scatterers focus to 93.7 once the conventions
             * agree.
             *
             * The flag stays at 1 rather than the phase being made absolute,
             * because a real CPHD sets 1 and the whole value of this null is
             * that it travels the SAME arithmetic as the data it stands in for.
             * Matching the convention costs nothing; taking the other branch
             * would mean the control no longer inherits the behaviour it exists
             * to measure. */
            const double phase = -k_phase * (R - rref) + tp[g];
            const double complex amp = ta[g] * (cos(phase) + I * sin(phase));

            /* Deposit the compressed response over the bins the envelope
             * actually reaches, rather than over the whole row. */
            const long lo = (long)(fbin - 4.0), hi = (long)(fbin + 4.0);
            for (long b = (lo < 0 ? 0 : lo); b <= hi && b < (long)n_rbin; b++) {
                const double d = (double)b - fbin;
                row[b] += (float complex)(amp * exp(-d * d * inv_2s2));
            }
        }
    }

    free(tx); free(ty); free(ta); free(tp);
    return RS_OK;
}

/* Median magnitude of a sample of the phase history, as the brightness scale an
 * injected scatterer is measured against.
 *
 * Sampled rather than exhaustive: a real collect holds billions of samples and
 * the median only has to set a scale. The stride is forced odd so the sample is
 * not confined to one column of each pulse.
 *
 * OVER THE NON-ZERO SAMPLES ONLY. A synthetic scene occupies a few dozen of its
 * range bins and leaves the rest exactly zero, so the median over ALL samples is
 * zero and the scale collapses -- which is how this was first written and what
 * the injected-vibrator test caught. On a real collect, where every bin carries
 * clutter or noise, the two agree. Returns 0 only if the phase history is
 * entirely zero or non-finite, which the caller reports rather than dividing by.
 *
 * Probing stops after a bounded number of positions so an almost-empty container
 * cannot make this walk the whole array for a scale it will not find. */
static double rs_signal_median_mag(const rs_cphd_t *cphd, size_t want)
{
    const size_t total = cphd->n_pulse * cphd->n_rbin;
    if (total == 0) return 0.0;
    if (want == 0) want = 1u << 16;
    if (want > total) want = total;

    double *v = malloc(want * sizeof *v);
    if (!v) return 0.0;

    const size_t stride = ((total / want) | 1u);
    size_t n = 0, probed = 0;
    const size_t probe_max = 16 * want;
    for (size_t i = 0; n < want && probed < probe_max && i < total;
         i += stride, probed++) {
        const double a = (double)cabsf(cphd->signal[i]);
        if (isfinite(a) && a > 0.0) v[n++] = a;
    }
    if (n == 0) { free(v); return 0.0; }

    /* Insertion sort: n is bounded by 'want' and this runs once per injection. */
    for (size_t i = 1; i < n; i++) {
        const double k = v[i];
        size_t j = i;
        while (j > 0 && v[j - 1] > k) { v[j] = v[j - 1]; j--; }
        v[j] = k;
    }
    const double m = v[n / 2];
    free(v);
    return m;
}

/* Add a vibrating point scatterer to a real collect. See simulate.h. */
resonarsat_status_t rs_simulate_inject_vibrator(rs_cphd_t *cphd,
                                                const double centre[2],
                                                double freq_hz, double amp_m,
                                                double rel_amp,
                                                rs_inject_report_t *report)
{
    if (report) {
        report->n_pulse = 0; report->n_deposited = 0;
        report->fbin_min = NAN; report->fbin_max = NAN;
        report->scale_ref = 0.0; report->amp = 0.0;
    }
    if (!cphd || !centre) return RS_ERR_ARG;
    if (cphd->n_pulse == 0 || !cphd->t || !cphd->pos || !cphd->r_ref ||
        !cphd->signal) {
        rs_set_error("inject: the collect carries no pulse geometry");
        return RS_ERR_ARG;
    }
    if (!isfinite(freq_hz) || freq_hz <= 0.0) {
        rs_set_error("inject: frequency must be positive and finite (got %g Hz)",
                     freq_hz);
        return RS_ERR_ARG;
    }
    if (!isfinite(amp_m) || !isfinite(rel_amp)) {
        rs_set_error("inject: amplitude %g m and relative brightness %g must be "
                     "finite", amp_m, rel_amp);
        return RS_ERR_ARG;
    }
    if (!(cphd->dr > 0.0)) {
        rs_set_error("inject: the collect has no range bin spacing");
        return RS_ERR_ARG;
    }
    const double lambda = (cphd->lambda > 0.0) ? cphd->lambda
                        : ((cphd->fc > 0.0) ? RS_C_LIGHT / cphd->fc : 0.0);
    if (!(lambda > 0.0)) {
        rs_set_error("inject: the collect has no carrier wavelength");
        return RS_ERR_ARG;
    }

    if (rel_amp <= 0.0) rel_amp = 1.0;
    const double scale_ref = rs_signal_median_mag(cphd, 0);
    if (!(scale_ref > 0.0)) {
        rs_set_error("inject: the collect's samples have no finite magnitude to "
                     "scale against");
        return RS_ERR_ARG;
    }
    const double amp = rel_amp * scale_ref;

    if (report) {
        report->n_pulse = cphd->n_pulse;
        report->scale_ref = scale_ref;
        report->amp = amp;
    }

    const size_t n_rbin = cphd->n_rbin;
    const double k_phase = 4.0 * M_PI / lambda;
    const double sigma = 2.0 * cphd->dr / 2.355;   /* FWHM of about two bins */
    const double inv_2s2 = 1.0 / (2.0 * sigma * sigma);

    /* Accounted rather than assumed: see rs_inject_report_t. */
    size_t deposited = 0;
    double fb_lo = INFINITY, fb_hi = -INFINITY;

#ifdef _OPENMP
#pragma omp parallel for schedule(static) \
        reduction(+:deposited) reduction(min:fb_lo) reduction(max:fb_hi)
#endif
    for (size_t i = 0; i < cphd->n_pulse; i++) {
        const double px = cphd->pos[3 * i + 0];
        const double py = cphd->pos[3 * i + 1];
        const double pz = cphd->pos[3 * i + 2];

        /* Vertical displacement; the collect's own geometry projects it onto the
         * line of sight, which is why the observable is smaller than amp_m. */
        const double dz = amp_m * sin(2.0 * M_PI * freq_hz * cphd->t[i]);

        const double dx = px - centre[0];
        const double dy = py - centre[1];
        const double dzz = pz - dz;
        const double R = sqrt(dx * dx + dy * dy + dzz * dzz);

        const double fbin = (R - cphd->r_ref[i]) / cphd->dr + 0.5 * (double)n_rbin;
        if (!isfinite(fbin) || fbin < 0.0 || fbin >= (double)n_rbin) continue;
        deposited++;
        if (fbin < fb_lo) fb_lo = fbin;
        if (fbin > fb_hi) fb_hi = fbin;

        /* Whichever reference the data were recorded on -- see item 27. */
        const double phase =
            -k_phase * (cphd->phase_ref_srp ? (R - cphd->r_ref[i]) : R);
        const double complex a = amp * (cos(phase) + I * sin(phase));

        float complex *row = cphd->signal + i * n_rbin;
        const long lo = (long)(fbin - 4.0), hi = (long)(fbin + 4.0);
        for (long b = (lo < 0 ? 0 : lo); b <= hi && b < (long)n_rbin; b++) {
            const double d = (double)b - fbin;
            row[b] += (float complex)(a * exp(-d * d * inv_2s2));
        }
    }

    if (report) {
        report->n_deposited = deposited;
        if (deposited > 0) { report->fbin_min = fb_lo; report->fbin_max = fb_hi; }
    }
    if (deposited == 0) {
        rs_set_error("inject: the target at (%.1f, %.1f) falls outside all %zu "
                     "pulses' %zu-bin range windows, so nothing was injected; "
                     "widen --rbins or move the grid origin",
                     centre[0], centre[1], cphd->n_pulse, n_rbin);
        return RS_ERR_RANGE;
    }
    return RS_OK;
}
