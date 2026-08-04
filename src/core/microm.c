/* Micro-motion extraction by sub-pixel offset tracking across a sub-look stack.
 * See include/resonarsat/microm.h for the contract and the accuracy envelope. */

#include "resonarsat/microm.h"
#include "resonarsat/coreg.h"
#include "resonarsat/phaselink.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

/* Published working defaults. The patch size is a mid-range value from the
 * 51-131 pixel span reported in the validated literature, not a universal
 * optimum: too small a patch makes the tracker systematically underestimate
 * displacement, and the right size depends on the target's brightness and
 * apparent motion. Sweep it on real data. */
void rs_microm_params_default(rs_microm_params_t *params)
{
    if (!params) return;
    /* FIRST, not ADJACENT, despite ADJACENT correlating far better. Measured:
     * ADJACENT doubles per-window coherence (0.20 -> 0.50) and removes the
     * second-harmonic artefact, but accumulating its differentials integrates
     * tracking noise into a random walk whose low-frequency energy swamps the
     * signal -- on the single-target frequency sweep it takes recovery from
     * 5 of 6 to 0 of 6, with every window reporting the lowest spectral bin.
     * Until that accumulation is handled, FIRST is the better default. */
    params->estimator = RS_MICROM_EST_CORRELATION;
    params->reference = RS_MICROM_REF_FIRST;
    params->ref_lag = 1;
    params->win_az = 64;
    params->win_rg = 64;
    /* 1 keeps the single-pixel phase estimator every earlier measurement was
     * made on. See rs_microm_params_t.n_pixels. */
    params->n_pixels = 1;
    params->stride_az = 16;
    params->stride_rg = 16;
    params->upsample_az = 10;
    params->upsample_rg = 20;
    /* 0.4 is the literature value for distributed scenes. It is a real filter:
     * see rs_microm_track() for why sub-threshold windows are zeroed, and note
     * that synthetic point-target scenes score well below it. */
    params->coherence_min = 0.4;
    /* Off by default, on the same measured grounds as the reference mode above.
     * Common-mode removal is standard practice and sound where most of a scene
     * is genuinely static clutter, but the median it subtracts is only as good
     * as the windows it is taken over. On a scene dominated by EMPTY background
     * -- which every synthetic fixture here is -- most windows track noise, the
     * median is itself noisy, and subtracting it injects noise into the one
     * window that had signal: recovery falls from 5 of 6 to 3 of 6. Enable it
     * on real scenes with distributed clutter, where the premise holds. */
    params->remove_common_mode = 0;
    /* Optimised path by default.
     *
     * ASSIGNED EXPLICITLY, LIKE EVERY FIELD ABOVE, AND THAT IS LOAD-BEARING.
     * This function does not memset the struct, and its callers declare it
     * uninitialised on the stack -- so a field added to rs_microm_params_t and
     * not assigned here holds whatever was there. When this one was first added
     * and left out, the tracker read a garbage flag and took the exhaustive
     * correlation path on some runs and not others: test_nullmotion lost its
     * detections and test_tracking ran for over ten minutes, with nothing in
     * either output pointing at the cause. Any new field must be assigned here. */
    params->no_optimize = 0;
}

/* The trial carrier phase at look 'k', on a CENTRED and ORTHOGONALISED basis.
 *
 *     kc = k - (N-1)/2                  zero mean over the record
 *     q  = kc*kc - (N*N - 1)/12         zero mean, and sum(kc*q) = 0 by parity
 *     c  = kc*kc*kc - kc*(3*N*N - 7)/20 orthogonal to BOTH of the above
 *     phase = nu*kc + mu*q + kappa*c
 *
 * The cubic's constant is not decoration. c is odd and so is kc, so parity does
 * NOT make them orthogonal -- the coefficient is sum(kc^4)/sum(kc^2), which for
 * k = 0..N-1 is exactly (3*N*N - 7)/20. Verified to machine precision at N = 16,
 * 64, 128 and 512. Without it the cubic search would fail the same way the
 * uncentred quadratic did.
 *
 * WHY NOT SIMPLY nu*k + mu*k*k. Because k and k*k are strongly correlated over a
 * finite record, and the staged search below holds one parameter while scanning
 * the other. Adding mu*k*k shifts the record's MEAN RATE by about mu*N, which at
 * the curvature actually present on the Giza collect -- mu near 3.1e-4 at 128
 * looks -- is 0.04 rad per look against a coarse step of 2*pi/128 = 0.049. Nearly
 * a whole step. Scanning mu at the linear-only nu therefore made every trial
 * WORSE rather than better, because the phasors dephased linearly faster than the
 * curvature was removed, and the search returned mu = 0 every time.
 *
 * That is not a hypothetical. It is what the first version of this did: the
 * artefact it was written to remove fell from 12,060x to 10,697x, a factor of
 * 1.13 where fitting the same quadratic offline gave 2,000x. Centring makes the
 * two terms orthogonal over the record, so scanning one at a fixed other is a
 * valid stage.
 *
 * The constant offsets subtracted from each term do not matter to the result --
 * each look's phase is measured against the series' own mean phasor, which
 * absorbs any constant -- but they are what makes the basis orthogonal, so they
 * matter to the SEARCH. */
static double rs_carrier_phase(size_t k, size_t n_looks,
                               double nu, double mu, double kappa)
{
    const double nd = (double)n_looks;
    const double kc = (double)k - 0.5 * (nd - 1.0);
    const double q  = kc * kc - (nd * nd - 1.0) / 12.0;
    const double c  = kc * kc * kc - kc * (3.0 * nd * nd - 7.0) / 20.0;
    return nu * kc + mu * q + kappa * c;
}

/* Magnitude of the de-ramped phasor sum for one pixel at trial rates 'nu' and
 * 'mu', the linear and QUADRATIC parts of the carrier.
 *
 * |sum_k z[k] * exp(-i*(nu*k + mu*k*k))|, the objective the phase estimator
 * maximises to find the geometric carrier it must remove. See the phase branch
 * of rs_microm_track() for why that removal is the whole ballgame, and item 51
 * for why one term is not enough.
 *
 * THE QUADRATIC TERM IS NOT OPTIONAL AND WAS NOT THERE. A scatterer's range
 * history is quadratic in time, so its phase is, and fitting only a linear ramp
 * leaves the curvature behind. Measured on the real Giza collect with a bright
 * STATIC scatterer and zero injected motion, that residual reached 21,602 times
 * its own spectral neighbourhood at the band floor -- an artefact bigger than
 * any real signal below 0.125 mm, and the thing that set the sensitivity floor
 * of item 50. Fitting and removing a quadratic drops it to 10.4.
 *
 * Squared magnitude is returned rather than magnitude: the maximiser is the
 * same and the square root is not free at N evaluations per search step. */
static double rs_phasor_mag(const rs_subap_stack_t *stack, size_t pa, size_t pr,
                            size_t n_looks, double nu, double mu, double kappa)
{
    double sr = 0.0, si = 0.0;
    for (size_t k = 0; k < n_looks; k++) {
        const rs_slc_t *im = &stack->look[k];
        const float complex z = im->data[pa * im->n_rg + pr];
        const double ph = rs_carrier_phase(k, n_looks, nu, mu, kappa);
        const double c = cos(ph), s = sin(ph);
        sr += (double)crealf(z) * c + (double)cimagf(z) * s;
        si += (double)cimagf(z) * c - (double)crealf(z) * s;
    }
    return sr * sr + si * si;
}

/* Golden-section maximum of rs_phasor_mag() along ONE axis, holding the others.
 *
 * 'axis' selects which of nu, mu, kappa is varied; the three-element 'fixed'
 * supplies all of them and the varied one is overwritten per trial. The
 * objective is unimodal inside a bracket one coarse step wide, which is what
 * every caller passes. Forty iterations take the bracket far below the pi/N the
 * residual must stay under -- the argument the linear search already made,
 * applied to each axis in turn, which is only valid because the basis in
 * rs_carrier_phase() is orthogonal. */
static double rs_phasor_refine(const rs_subap_stack_t *stack, size_t pa, size_t pr,
                               size_t n_looks, double lo, double hi,
                               int axis, const double fixed[3])
{
    const double gr = 0.6180339887498949;
    double p[3] = { fixed[0], fixed[1], fixed[2] };
    double c1 = hi - gr * (hi - lo), c2 = lo + gr * (hi - lo);
    p[axis] = c1;
    double f1 = rs_phasor_mag(stack, pa, pr, n_looks, p[0], p[1], p[2]);
    p[axis] = c2;
    double f2 = rs_phasor_mag(stack, pa, pr, n_looks, p[0], p[1], p[2]);
    for (int it = 0; it < 40; it++) {
        if (f1 > f2) {
            hi = c2; c2 = c1; f2 = f1;
            c1 = hi - gr * (hi - lo);
            p[axis] = c1;
            f1 = rs_phasor_mag(stack, pa, pr, n_looks, p[0], p[1], p[2]);
        } else {
            lo = c1; c1 = c2; f1 = f2;
            c2 = lo + gr * (hi - lo);
            p[axis] = c2;
            f2 = rs_phasor_mag(stack, pa, pr, n_looks, p[0], p[1], p[2]);
        }
    }
    return 0.5 * (lo + hi);
}

static int rs_cmp_dbl(const void *a, const void *b);   /* defined below */

/* Two-sample Kolmogorov-Smirnov statistic between two amplitude samples.
 *
 * D = max |F1(x) - F2(x)| over the pooled order statistics. This is SqueeSAR's
 * test for whether two pixels are drawn from the same scattering population --
 * STATISTICALLY HOMOGENEOUS PIXELS, the set that may legitimately be combined.
 *
 * Item 64 measured what happens without it. Taking the K BRIGHTEST pixels
 * instead made the artefact grow faster than the signal, 70.7 to 182 at K = 4,
 * because the second and third brightest pixels in a window are usually
 * DIFFERENT scatterers, each with its own sub-pixel offset and so its own
 * carrier residual. Homogeneity is a statement about the amplitude
 * DISTRIBUTION across looks; brightness is not.
 *
 * Both arrays are sorted in place, so callers pass scratch. */
static double rs_ks_stat(double *a, size_t na, double *b, size_t nb)
{
    if (na == 0 || nb == 0) return 1.0;
    qsort(a, na, sizeof *a, rs_cmp_dbl);
    qsort(b, nb, sizeof *b, rs_cmp_dbl);
    size_t i = 0, j = 0;
    double d = 0.0;
    while (i < na && j < nb) {
        const double x = (a[i] <= b[j]) ? a[i] : b[j];
        while (i < na && a[i] <= x) i++;
        while (j < nb && b[j] <= x) j++;
        const double gap = fabs((double)i / (double)na - (double)j / (double)nb);
        if (gap > d) d = gap;
    }
    return d;
}

/* Median of a scratch array, which this reorders. */
static int rs_cmp_dbl(const void *a, const void *b)
{
    const double x = *(const double *)a, y = *(const double *)b;
    return (x < y) ? -1 : (x > y) ? 1 : 0;
}

/* Median of 'v', which this REORDERS in place. Callers must pass scratch they
 * do not need preserved. Returns 0 for an empty array. */
static double rs_median(double *v, size_t n)
{
    if (n == 0) return 0.0;
    qsort(v, n, sizeof *v, rs_cmp_dbl);
    return (n % 2) ? v[n / 2] : 0.5 * (v[n / 2 - 1] + v[n / 2]);
}

/* Subtract the scene-median shift from every window, look by look.
 *
 * See rs_microm_params_t.remove_common_mode for why this is not cosmetic. The
 * line-of-sight velocities are recomputed from the corrected shifts rather than
 * corrected separately, so the two cannot drift apart. */
static void rs_microm_remove_common_mode(rs_microm_t *m, double az_spacing,
                                         double v_plat, double r_slant,
                                         int can_velocity)
{
    double *scratch = malloc(m->n_win * sizeof *scratch);
    if (!scratch) return;

    for (size_t k = 0; k < m->n_looks; k++) {
        for (size_t w = 0; w < m->n_win; w++) scratch[w] = m->disp_az[w * m->n_looks + k];
        const double med_az = rs_median(scratch, m->n_win);

        for (size_t w = 0; w < m->n_win; w++) scratch[w] = m->disp_rg[w * m->n_looks + k];
        const double med_rg = rs_median(scratch, m->n_win);

        for (size_t w = 0; w < m->n_win; w++) {
            const size_t idx = w * m->n_looks + k;
            m->disp_az[idx] -= med_az;
            m->disp_rg[idx] -= med_rg;
            if (can_velocity) {
                m->vel_los[idx] = m->disp_az[idx] * az_spacing * v_plat / r_slant;
            }
        }
    }
    free(scratch);
}

/* Release every array a micro-motion result owns. */
void rs_microm_free(rs_microm_t *m)
{
    if (!m) return;
    free(m->disp_az);
    free(m->disp_rg);
    free(m->vel_los);
    free(m->disp_los);
    free(m->phase);
    free(m->amp);
    free(m->quality);
    free(m->snr);
    free(m->sigma_px);
    free(m->d_a);
    memset(m, 0, sizeof *m);
}

/* Track all windows across the stack. */
resonarsat_status_t rs_microm_track(const rs_subap_stack_t *stack,
                                    const rs_microm_params_t *params,
                                    rs_microm_t *out)
{
    if (!out) return RS_ERR_ARG;

    /* Zeroed before any validation can fail, so that "check the status, then
     * free the struct" is safe on every path rather than on most of them.
     * rs_cphd_read() carried the non-uniform version of this and aborted the
     * first time a corpus made it fail on real data. */
    memset(out, 0, sizeof *out);

    if (!stack || !params || !out || !stack->look) return RS_ERR_ARG;
    if (stack->n_looks < 2) {
        rs_set_error("microm: need at least 2 sub-looks, have %zu", stack->n_looks);
        return RS_ERR_ARG;
    }

    if (params->reference == RS_MICROM_REF_PAIR) {
        if (!stack->slave) {
            rs_set_error("microm: reference 'pair' needs a stack built with "
                         "rs_subap_params_t.pair (the slave bands are absent)");
            return RS_ERR_ARG;
        }
        if (params->estimator != RS_MICROM_EST_CORRELATION) {
            rs_set_error("microm: reference 'pair' measures a master-slave "
                         "offset, so it requires the correlation estimator");
            return RS_ERR_ARG;
        }
    }

    const rs_slc_t *ref_img = &stack->look[0];
    const size_t win_az = params->win_az, win_rg = params->win_rg;
    if (win_az == 0 || win_rg == 0 ||
        win_az > ref_img->n_az || win_rg > ref_img->n_rg) {
        rs_set_error("microm: window %zux%zu does not fit image %zux%zu",
                     win_az, win_rg, ref_img->n_az, ref_img->n_rg);
        return RS_ERR_ARG;
    }

    /* Reject an oversized exhaustive surface HERE, not in the tracking loop.
     *
     * The correlator's size ceiling depends only on the window and the upsample
     * factors, so if it is going to be exceeded it is exceeded for every window
     * identically. The loop body treats a failed correlation as "this window did
     * not track" and moves on, which is right for a degenerate patch and wrong
     * for this: it would return a full, well-formed result in which every single
     * window is zero, with no indication why. That is the project's signature
     * failure mode -- a plausible empty product -- so the condition is turned into
     * a refusal before anything is allocated. */
    if (params->no_optimize && params->estimator != RS_MICROM_EST_PHASE) {
        const resonarsat_status_t size_st =
            rs_coreg_surface_check(win_az, win_rg,
                                   params->upsample_az, params->upsample_rg);
        if (size_st != RS_OK) return size_st;
    }

    const size_t stride_az = params->stride_az ? params->stride_az : win_az;
    const size_t stride_rg = params->stride_rg ? params->stride_rg : win_rg;

    const size_t n_win_az = (ref_img->n_az - win_az) / stride_az + 1;
    const size_t n_win_rg = (ref_img->n_rg - win_rg) / stride_rg + 1;
    const size_t n_win = n_win_az * n_win_rg;
    const size_t n_looks = stack->n_looks;

    out->disp_az  = calloc(n_win * n_looks, sizeof *out->disp_az);
    out->disp_rg  = calloc(n_win * n_looks, sizeof *out->disp_rg);
    out->vel_los  = calloc(n_win * n_looks, sizeof *out->vel_los);
    out->disp_los = calloc(n_win * n_looks, sizeof *out->disp_los);
    out->phase    = calloc(n_win * n_looks, sizeof *out->phase);
    out->amp      = calloc(n_win * n_looks, sizeof *out->amp);
    out->quality  = calloc(n_win, sizeof *out->quality);
    out->snr      = calloc(n_win, sizeof *out->snr);
    out->sigma_px = calloc(n_win, sizeof *out->sigma_px);
    out->d_a      = calloc(n_win, sizeof *out->d_a);
    if (!out->disp_az || !out->disp_rg || !out->vel_los || !out->disp_los ||
        !out->phase || !out->quality || !out->snr || !out->sigma_px ||
        !out->d_a) {
        rs_microm_free(out);
        rs_set_error("microm: cannot allocate the tracking result for %zu windows "
                     "x %zu looks", n_win, n_looks);
        return RS_ERR_ALLOC;
    }

    out->n_win_az = n_win_az;
    out->n_win_rg = n_win_rg;
    out->n_win = n_win;
    out->n_looks = n_looks;
    out->win_az = win_az;
    out->win_rg = win_rg;
    out->stride_az = stride_az;
    out->stride_rg = stride_rg;
    out->dt = stack->dt;
    out->f_max = stack->f_max;
    out->az_spacing_m = ref_img->az_spacing_m;
    out->rg_spacing_m = ref_img->rg_spacing_m;
    /* The correlator locates its peak on a grid of 1/upsample_az pixels, so
     * that is the finest excursion it can report and the floor below which a
     * series is rounding rather than motion. The phase estimator does not use
     * the correlation surface at all, so the concept does not apply and the
     * field stays zero -- see rs_microm_t.quant_px. */
    out->quant_px = (params->estimator == RS_MICROM_EST_PHASE) ? 0.0
                  : (params->estimator == RS_MICROM_EST_ARGMAX) ? 1.0
                  : (params->upsample_az == 0) ? 0.0
                  : 1.0 / (double)params->upsample_az;

    /* What this window size scores on noise alone, for the SNR the correlation
     * branch fills in below. Left at zero for the estimators that form no
     * correlation surface, which is how a consumer tells "no signal here" from
     * "this statistic does not exist for this estimator" -- a gate keyed on a
     * null of zero would pass every window rather than none. */
    out->snr_null = (params->estimator == RS_MICROM_EST_CORRELATION)
                  ? rs_coreg_snr_null(win_az, win_rg) : 0.0;

    const double lambda = (ref_img->lambda > 0.0) ? ref_img->lambda : 0.031;

    /* Geometry for converting a tracked azimuth shift into a line-of-sight
     * velocity: v_r = dx * V / R. All three come from what the focuser measured
     * off the recorded pulse positions. If any is missing the conversion is
     * skipped rather than guessed -- a plausible-looking velocity computed from
     * an assumed platform speed would be worse than none. */
    const double az_spacing = ref_img->az_spacing_m;
    const double v_plat     = ref_img->v_platform;
    const double r_slant    = ref_img->r_scene_m;
    const int    can_velocity = (az_spacing > 0.0 && v_plat > 0.0 && r_slant > 0.0);

    volatile resonarsat_status_t shared_st = RS_OK;

    /* Which extent the correlator searches. The exhaustive baseline is the half
     * of --no-optimize that can actually move a number; see
     * rs_microm_params_t.no_optimize. */
    const rs_coreg_refine_t refine = params->no_optimize
                                   ? RS_COREG_REFINE_EXHAUSTIVE
                                   : RS_COREG_REFINE_LOCAL;

    /* Windows are independent, so this is the pipeline's most naturally
     * parallel loop. Each thread allocates its own patch buffers; sharing them
     * would serialise the whole thing.
     *
     * Under --no-optimize the region is suppressed by the 'if' clause rather than
     * by a second copy of the loop, for the reason given in
     * rs_focus_backproject_opts(): one body means the arithmetic cannot drift
     * between the two modes. Note that 'dynamic' scheduling below already makes
     * the ORDER in which windows are visited unpredictable between runs -- and
     * that this is harmless, because each window writes only its own slots and
     * reads nothing another window wrote. The serial mode makes that visitation
     * order fixed, which is what allows a debugger or a profiler to be pointed at
     * a particular window. */
#ifdef _OPENMP
#pragma omp parallel if (!params->no_optimize)
#endif
    {
        float complex *pref = malloc(win_az * win_rg * sizeof *pref);
        float complex *pcur = malloc(win_az * win_rg * sizeof *pcur);
        /* Multi-pixel scratch: one series being built, one accumulating the
         * weighted mean, and a taken-mask for the partial selection. */
        double *pxacc = malloc(n_looks * sizeof *pxacc);
        double *pxsum = malloc(n_looks * sizeof *pxsum);
        unsigned char *pxtaken = malloc(win_az * win_rg);
        /* KS-test scratch, and the de-ramped stack handed to the phase linker:
         * n_looks signals of up to RS_MICROM_MAX_PIXELS samples. */
        double *ksref = malloc(n_looks * sizeof *ksref);
        double *kscan = malloc(n_looks * sizeof *kscan);
        double *kstmp = malloc(n_looks * sizeof *kstmp);
        float complex *plsig = malloc(n_looks * RS_MICROM_MAX_PIXELS * sizeof *plsig);
        double *plph = malloc(n_looks * sizeof *plph);
        /* Split-band estimation needs every look's patch simultaneously. */
        const int want_stack = (params->estimator == RS_MICROM_EST_SPLITBAND);
        float complex *pstack = want_stack
            ? malloc(n_looks * win_az * win_rg * sizeof *pstack) : NULL;
        double *pshift = want_stack ? malloc(n_looks * sizeof *pshift) : NULL;

        if (!pref || !pcur || !pxacc || !pxsum || !pxtaken || !ksref || !kscan || !kstmp || !plsig || !plph || (want_stack && (!pstack || !pshift))) {
            shared_st = RS_ERR_ALLOC;
        } else {
#ifdef _OPENMP
#pragma omp for schedule(dynamic)
#endif
            for (long w = 0; w < (long)n_win; w++) {
                if (shared_st != RS_OK) continue;

                const size_t wa = (size_t)w / n_win_rg;
                const size_t wr = (size_t)w % n_win_rg;
                const size_t az0 = wa * stride_az;
                const size_t rg0 = wr * stride_rg;

                if (rs_coreg_extract(ref_img, az0, rg0, win_az, win_rg, pref) != RS_OK)
                    continue;

                /* AMPLITUDE DISPERSION, for every estimator, because it says
                 * whether the SCENE supports the phase route rather than
                 * whether this tracker happened to work. See rs_microm_t.d_a.
                 *
                 * Computed before the estimator branch so that a correlation run
                 * also reports it: the most useful thing it can tell a caller is
                 * "switch estimator", and a statistic only produced by the route
                 * it recommends cannot say that. The brightest-pixel search is
                 * repeated inside the phase branch rather than shared, which
                 * costs one pass over the window and keeps this independent of
                 * that branch's own reference choices. */
                {
                    size_t bi = 0;
                    double ba = -1.0;
                    for (size_t i = 0; i < win_az * win_rg; i++) {
                        const double a = (double)cabsf(pref[i]);
                        if (a > ba) { ba = a; bi = i; }
                    }
                    const size_t da_a = az0 + bi / win_rg;
                    const size_t da_r = rg0 + bi % win_rg;

                    double sum = 0.0, sq = 0.0;
                    size_t cnt = 0;
                    for (size_t k = 0; k < n_looks; k++) {
                        const rs_slc_t *im = &stack->look[k];
                        if (da_a >= im->n_az || da_r >= im->n_rg) break;
                        const double a =
                            (double)cabsf(im->data[da_a * im->n_rg + da_r]);
                        sum += a;
                        sq  += a * a;
                        cnt++;
                    }

                    double da = RS_DA_MAX;
                    if (cnt > 0) {
                        const double mu = sum / (double)cnt;
                        if (mu > 0.0) {
                            const double var = sq / (double)cnt - mu * mu;
                            da = (var > 0.0) ? sqrt(var) / mu : 0.0;
                            if (da > RS_DA_MAX) da = RS_DA_MAX;
                        }
                    }
                    out->d_a[(size_t)w] = da;
                }

                if (params->estimator == RS_MICROM_EST_ARGMAX) {
                    /* Where the brightest sample sits in each sub-look.
                     *
                     * The whole estimator. A vibrating scatterer's paired echoes
                     * walk along azimuth as the aperture sweeps, so the azimuth
                     * index of the window's peak IS the displacement series --
                     * no correlation surface, no phase, and no reference look,
                     * which is why rs_microm_ref_t does not apply here.
                     *
                     * Integer, deliberately. See rs_microm_estimator_t: the
                     * published validation and this project's own item 6
                     * measurement are both for the plain argmax, and quant_px is
                     * 1.0 so the quantisation floor tests that honestly rather
                     * than a sub-cell refinement nothing has evidence for.
                     *
                     * The RANGE index is tracked too but not reported as a
                     * shift: range walk over a dwell is dominated by the
                     * platform's own geometry rather than by target motion, and
                     * the observable this project measures is the azimuth one.
                     * disp_rg stays zero rather than carrying something that
                     * looks like a measurement and is not. */
                    double q_sum = 0.0;
                    size_t az0_peak = 0;
                    int ok = 1;

                    for (size_t k = 0; k < n_looks && ok; k++) {
                        const rs_slc_t *im = &stack->look[k];
                        if (az0 + win_az > im->n_az || rg0 + win_rg > im->n_rg) {
                            ok = 0; break;
                        }
                        double best_a = -1.0, sum_a = 0.0;
                        size_t best_ia = 0;
                        for (size_t i = 0; i < win_az; i++) {
                            const size_t row = (az0 + i) * im->n_rg + rg0;
                            for (size_t j = 0; j < win_rg; j++) {
                                const double a = (double)cabsf(im->data[row + j]);
                                sum_a += a;
                                if (a > best_a) { best_a = a; best_ia = i; }
                            }
                        }
                        if (k == 0) az0_peak = best_ia;

                        const size_t idx = (size_t)w * n_looks + k;
                        /* Displacement from the first look, in cells, matching
                         * what every other estimator puts in disp_az. */
                        out->disp_az[idx] = (double)best_ia - (double)az0_peak;
                        out->disp_rg[idx] = 0.0;
                        if (can_velocity) {
                            out->vel_los[idx] = out->disp_az[idx] * az_spacing
                                              * v_plat / r_slant;
                        }

                        /* How distinguished the peak is, which is this
                         * estimator's precondition. One minus the mean-to-peak
                         * amplitude ratio: a lone bright scatterer drives the
                         * mean far below the peak and scores near one, a flat
                         * window scores near zero. Bounded in [0,1] by
                         * construction, and NOT the amplitude stability the
                         * phase branch reports -- see rs_microm_t.quality. */
                        const double mean_a = sum_a / (double)(win_az * win_rg);
                        q_sum += (best_a > 0.0) ? (1.0 - mean_a / best_a) : 0.0;
                    }

                    if (!ok) goto next_window;

                    out->quality[w] = q_sum / (double)n_looks;

                    if (params->coherence_min > 0.0 &&
                        out->quality[w] < params->coherence_min) {
                        for (size_t k = 0; k < n_looks; k++) {
                            const size_t idx = (size_t)w * n_looks + k;
                            out->disp_az[idx] = out->disp_rg[idx] = 0.0;
                            out->vel_los[idx] = out->disp_los[idx] = 0.0;
                        }
                    }
                    goto next_window;
                }

                if (params->estimator == RS_MICROM_EST_PHASE) {
                    /* The dominant scatterer's phase, read from each sub-look.
                     *
                     * A single pixel rather than the window's coherent sum: the
                     * sum averages scatterers that may be moving differently,
                     * which is the very thing being measured. The brightest
                     * pixel of the reference look is used throughout, so the
                     * series follows one scatterer rather than wandering to
                     * whichever happens to be brightest in each look. */
                    const size_t n_px = params->n_pixels ? params->n_pixels : 1;
                    double best_a = -1.0, ref_mean = 0.0;
                    for (size_t i = 0; i < win_az * win_rg; i++) {
                        const double a = (double)cabsf(pref[i]);
                        ref_mean += a;
                        if (a > best_a) best_a = a;
                    }
                    ref_mean /= (double)(win_az * win_rg);

                    /* The pixels this window's estimate is built from. */
                    size_t sel[RS_MICROM_MAX_PIXELS];
                    size_t n_sel = 0;
                    {
                        /* THE BRIGHTEST, PLUS ITS STATISTICALLY HOMOGENEOUS
                         * NEIGHBOURS -- SqueeSAR's construction. Item 64
                         * measured why the obvious alternative fails: the K
                         * BRIGHTEST pixels make the artefact grow faster than
                         * the signal, because the second and third brightest
                         * are usually DIFFERENT scatterers carrying their own
                         * carriers and residuals. Homogeneity is a statement
                         * about the amplitude DISTRIBUTION across looks, not
                         * about brightness.
                         *
                         * The threshold is the standard two-sample KS critical
                         * value c(alpha)*sqrt((n+m)/(n*m)), c = 1.36 at
                         * alpha = 0.05, computed from the look count rather
                         * than chosen. */
                        size_t bi = 0; double bv = -1.0;
                        for (size_t i = 0; i < win_az * win_rg; i++) {
                            const double a = (double)cabsf(pref[i]);
                            if (a > bv) { bv = a; bi = i; }
                        }
                        sel[n_sel++] = bi;
                        const size_t want = (n_px < RS_MICROM_MAX_PIXELS)
                                          ? n_px : RS_MICROM_MAX_PIXELS;
                        if (want > 1) {
                            const double nd = (double)n_looks;
                            const double dcrit = 1.36 * sqrt(2.0 / nd);
                            const size_t ra = bi / win_rg, rr = bi % win_rg;
                            for (size_t k = 0; k < n_looks; k++) {
                                const rs_slc_t *im = &stack->look[k];
                                const size_t a_ = az0 + ra, r_ = rg0 + rr;
                                ksref[k] = (a_ < im->n_az && r_ < im->n_rg)
                                    ? (double)cabsf(im->data[a_ * im->n_rg + r_]) : 0.0;
                            }
                            for (size_t i = 0; i < win_az * win_rg && n_sel < want; i++) {
                                if (i == bi) continue;
                                const size_t ca = i / win_rg, cr = i % win_rg;
                                int inb = 1;
                                for (size_t k = 0; k < n_looks; k++) {
                                    const rs_slc_t *im = &stack->look[k];
                                    const size_t a_ = az0 + ca, r_ = rg0 + cr;
                                    if (a_ >= im->n_az || r_ >= im->n_rg) { inb = 0; break; }
                                    kscan[k] = (double)cabsf(im->data[a_ * im->n_rg + r_]);
                                }
                                if (!inb) continue;
                                memcpy(kstmp, ksref, n_looks * sizeof *kstmp);
                                if (rs_ks_stat(kstmp, n_looks, kscan, n_looks) < dcrit)
                                    sel[n_sel++] = i;
                            }
                        }
                    }

                    /* SPATIAL dominance of that pixel over its own window, which
                     * is this window's quality. See below on why it is not the
                     * temporal amplitude stability it used to be. */
                    const double dominance = (best_a > 0.0)
                                           ? 1.0 - ref_mean / best_a : 0.0;

                    /* PHASE AGAINST THE MEAN PHASOR, NOT ACCUMULATED FORWARD.
                     *
                     * This used to unwrap temporally: fold each step into
                     * (-pi, pi] and add it to a running total. That is the
                     * textbook way to extend the unambiguous range beyond
                     * lambda/4, and on a decorrelating aperture it is unusable,
                     * because the fold's error is added to the total and never
                     * heals.
                     *
                     * Measured on a real X-band spotlight collect: sub-look
                     * coherence is the fraction of pulses two looks share, so
                     * adjacent looks at 95 percent overlap sit at gamma = 0.85
                     * and their phase difference has a spread of 0.65 rad. Over
                     * 1548 looks the accumulated random walk is sigma*sqrt(N),
                     * about 25 radians -- four cycles of pure noise on a
                     * quantity whose whole unambiguous range is pi/2. The
                     * recovered series was a random walk in every window, on
                     * distributed clutter and on a scatterer 74x above its
                     * surroundings alike. See rs_microm_estimator_t, which
                     * carries the coherence-versus-lag measurements.
                     *
                     * Raising the overlap does not rescue it and makes it
                     * worse: per-step noise falls as sqrt(1-gamma) while the
                     * step count rises as 1/(1-gamma), and gamma levels off
                     * near 0.9 rather than reaching 1, so sigma stops falling
                     * while N keeps growing.
                     *
                     * So the accumulation is removed rather than tuned. Each
                     * look's phase is measured against the series' own mean
                     * phasor, which makes every sample independent: noise stays
                     * at sigma instead of growing as sigma*sqrt(N), and a
                     * periodic signal still averages down across the
                     * periodogram. The cost is the ambiguity the unwrap was
                     * there to buy -- motion beyond +/-lambda/4 in TOTAL now
                     * folds, where before it folded only beyond lambda/4
                     * BETWEEN looks. That is the right trade here: 1.7 mm of
                     * per-sample noise against an 8 mm range, versus a range
                     * that was unbounded in principle and swamped in practice.
                     * Use the correlation estimator for motion larger than
                     * that; it has no ambiguity at all. */
                    /* Each selected pixel checks its own bounds inside the
                     * per-pixel loop below, since they are different pixels. */

                    /* THE CARRIER MUST COME OFF BEFORE THE PHASE IS WRAPPED, AND
                     * NOT DOING SO WAS THIS ESTIMATOR'S WHOLE FAILURE.
                     *
                     * A scatterer sitting anywhere but exactly at its pixel's
                     * centre has a range to the platform that changes linearly
                     * as the aperture sweeps, so its phase in sub-look k is
                     * linear in k. The rate is (4*pi/lambda) * dX * dx / R --
                     * platform travel per look times the target's offset from
                     * the pixel centre, over the slant range -- and it has
                     * nothing to do with the target moving. Measured on the
                     * isolated-point fixture at 128 looks it is 1.1 to 1.9
                     * radians PER LOOK, which is 23 to 39 full cycles across the
                     * stack.
                     *
                     * Wrapping that ramp into (-pi, pi] turns it into a
                     * SAWTOOTH, and a sawtooth has a strong line at its own
                     * repetition rate. That line is set by the target's
                     * sub-pixel offset and by the geometry, so it does not move
                     * when the scene does: the chain reported one fixed
                     * frequency for every injection AND for a scene with no
                     * motion in it, at a prominence higher than any of the
                     * moving cases. FOLLOW-UPS.md item 11 records it as the
                     * common-mode artefact that proved the consensus gate blind;
                     * this is where it came from.
                     *
                     * Detrending the DISPLACEMENT series cannot undo it, because
                     * by then the wrap has already happened and the sawtooth is
                     * not a trend. The removal has to happen on the phasors.
                     *
                     * The rate is the one that maximises the de-ramped phasor
                     * sum,
                     *
                     *     nu = argmax_v | sum_k z[k] * exp(-i*v*k) |
                     *
                     * which is the maximum-likelihood frequency of a phasor in
                     * noise and needs no unwrapping to compute. For a phase
                     * MODULATED by a zero-mean vibration it returns the carrier
                     * and leaves the modulation, which is exactly the split
                     * wanted: the carrier is the geometry, the modulation is the
                     * target.
                     *
                     * THE OBVIOUS CHEAPER ESTIMATOR IS NOT ACCURATE ENOUGH, and
                     * this was measured rather than assumed. The mean lag-one
                     * product, nu = arg(sum_k z[k+1]*conj(z[k])), is the
                     * textbook one-line answer and it carries a bias of order
                     * beta^2 from the modulation itself: on the isolated-point
                     * fixture it returned -0.694 rad/look against a true -0.760
                     * at 64 looks, and -1.879 against -1.909 at 128. Those look
                     * like small errors and they are not, because the residual
                     * ramp is the error times the LOOK COUNT: 4.2 radians over
                     * 64 looks, which wraps, which puts the sawtooth straight
                     * back. The requirement is error << pi/N, and only a proper
                     * peak search meets it.
                     *
                     * Coarse search over the N Fourier bins, then a
                     * golden-section refinement inside the winning bin. The
                     * coarse pass is O(N^2) per window, which is affordable
                     * because this estimator reads ONE PIXEL per window where
                     * the correlator does two-dimensional transforms over the
                     * whole patch, and because rs_microm_estimator_t advises the
                     * fewest looks that sample the frequency of interest.
                     *
                     * IT ALIASES IF THE RAMP EXCEEDS pi RADIANS PER LOOK, which
                     * is a real limit and not a defect of the estimator: at that
                     * rate the sub-look series does not sample the target's own
                     * Doppler, and no processing downstream can recover what was
                     * not sampled. Fewer looks over a fixed dwell make it worse,
                     * since the platform travels further between them. */
                    /* ONE PIXEL AT A TIME, EACH WITH ITS OWN CARRIER. The
                     * carrier is (4*pi/lambda)*dX*dx/R with dx the offset from
                     * THAT pixel's centre, so two pixels a cell apart have
                     * measurably different rates and a shared de-ramp would
                     * leave a residual on all but one. */
                    memset(pxsum, 0, n_looks * sizeof *pxsum);
                    double wsum = 0.0;
                    int any_px = 0;

                    for (size_t si = 0; si < n_sel; si++) {
                    memset(pxacc, 0, n_looks * sizeof *pxacc);
                    const size_t pa = az0 + sel[si] / win_rg;
                    const size_t pr = rg0 + sel[si] % win_rg;

                    int inb = 1;
                    for (size_t k = 0; k < n_looks; k++) {
                        const rs_slc_t *im = &stack->look[k];
                        if (pa >= im->n_az || pr >= im->n_rg) { inb = 0; break; }
                    }
                    if (!inb) continue;

                    double nu = 0.0, mu = 0.0, kappa = 0.0;
                    {
                        /* STAGE 1: the linear rate, coarse over the N Fourier
                         * bins then refined. The right starting point: the
                         * curvature is small beside the ramp, so the linear
                         * maximum is close to the joint one. */
                        double par[3] = { 0.0, 0.0, 0.0 };
                        double best_m = -1.0, nu_c = 0.0;
                        const double step = 2.0 * M_PI / (double)n_looks;
                        for (size_t b = 0; b < n_looks; b++) {
                            /* Signed bin, so the ramp may run either way. */
                            const double v = step * ((b < n_looks / 2)
                                                   ? (double)b
                                                   : (double)b - (double)n_looks);
                            const double mag = rs_phasor_mag(stack, pa, pr,
                                                             n_looks, v, 0.0, 0.0);
                            if (mag > best_m) { best_m = mag; nu_c = v; }
                        }
                        nu = rs_phasor_refine(stack, pa, pr, n_looks,
                                              nu_c - step, nu_c + step, 0, par);
                        par[0] = nu;

                        /* STAGE 2: the QUADRATIC rate, holding the rest.
                         *
                         * THE RANGE IS DERIVED. The instantaneous rate is
                         * nu + 2*mu*kc, so the quadratic sweeps it by mu*N
                         * across the record. A sweep wider than the unambiguous
                         * +-pi per look is not something this observable could
                         * have sampled, so |mu| <= pi/(2N) covers everything
                         * reachable and nothing that is not.
                         *
                         * THE STEP IS DERIVED TOO. An error d leaves a phase
                         * error of order d*N^2 at the ends of the record, and
                         * the requirement is the linear search's, error << pi.
                         * N steps across the range meets it and keeps the whole
                         * search O(N^2) per window rather than O(N^3) -- a
                         * three-dimensional grid would have been O(N^4), which
                         * is why this is staged rather than joint. */
                        const double mu_max = M_PI / (2.0 * (double)n_looks);
                        const double mu_step = 2.0 * mu_max / (double)n_looks;
                        double best_q = rs_phasor_mag(stack, pa, pr, n_looks,
                                                      nu, 0.0, 0.0);
                        double mu_c = 0.0;
                        for (size_t b = 0; b <= n_looks; b++) {
                            const double m = -mu_max + mu_step * (double)b;
                            const double mag = rs_phasor_mag(stack, pa, pr,
                                                             n_looks, nu, m, 0.0);
                            if (mag > best_q) { best_q = mag; mu_c = m; }
                        }
                        mu = rs_phasor_refine(stack, pa, pr, n_looks,
                                              mu_c - mu_step, mu_c + mu_step,
                                              1, par);
                        par[1] = mu;

                        /* STAGE 3: the CUBIC rate, holding the rest. Same
                         * derivation: the cubic contributes 3*kappa*kc^2 to the
                         * instantaneous rate, largest at the record's ends where
                         * kc = N/2, so bounding that by pi gives
                         * |kappa| <= 4*pi/(3*N^2).
                         *
                         * Item 51 measured a further 3.6x of artefact under a
                         * cubic fit after the quadratic, which is what this is
                         * for. */
                        const double ka_max = 4.0 * M_PI
                                            / (3.0 * (double)n_looks * (double)n_looks);
                        const double ka_step = 2.0 * ka_max / (double)n_looks;
                        double best_c = rs_phasor_mag(stack, pa, pr, n_looks,
                                                      nu, mu, 0.0);
                        double ka_c = 0.0;
                        for (size_t b = 0; b <= n_looks; b++) {
                            const double kk = -ka_max + ka_step * (double)b;
                            const double mag = rs_phasor_mag(stack, pa, pr,
                                                             n_looks, nu, mu, kk);
                            if (mag > best_c) { best_c = mag; ka_c = kk; }
                        }
                        kappa = rs_phasor_refine(stack, pa, pr, n_looks,
                                                 ka_c - ka_step, ka_c + ka_step,
                                                 2, par);
                        par[2] = kappa;

                        /* STAGE 4: nu and mu again, now that the higher terms
                         * are known. The basis is orthogonal over a UNIFORM
                         * record, but rs_phasor_mag() weights by amplitude and
                         * the amplitudes are not uniform, so the terms are only
                         * nearly independent. One re-refinement of each absorbs
                         * that; the brackets are a coarse step wide and the
                         * shifts are far smaller. */
                        nu = rs_phasor_refine(stack, pa, pr, n_looks,
                                              nu - step, nu + step, 0, par);
                        par[0] = nu;
                        mu = rs_phasor_refine(stack, pa, pr, n_looks,
                                              mu - mu_step, mu + mu_step, 1, par);
                    }

                    /* De-ramped phasors, and the reference they are measured
                     * against. Amplitude-weighted as before, so looks in which
                     * the scatterer faded count for less. */
                    double ref_re = 0.0, ref_im = 0.0;
                    for (size_t k = 0; k < n_looks; k++) {
                        const rs_slc_t *im = &stack->look[k];
                        const float complex z = im->data[pa * im->n_rg + pr];
                        const double ph_ = rs_carrier_phase(k, n_looks, nu, mu, kappa);
                        const double c_ = cos(ph_), s_ = sin(ph_);
                        ref_re += (double)crealf(z) * c_ + (double)cimagf(z) * s_;
                        ref_im += (double)cimagf(z) * c_ - (double)crealf(z) * s_;
                    }

                    for (size_t k = 0; k < n_looks; k++) {
                        const rs_slc_t *im = &stack->look[k];
                        const float complex z = im->data[pa * im->n_rg + pr];
                        const double ph_ = rs_carrier_phase(k, n_looks, nu, mu, kappa);
                        const double c_ = cos(ph_), s_ = sin(ph_);
                        const double dr_ = (double)crealf(z) * c_ + (double)cimagf(z) * s_;
                        const double di_ = (double)cimagf(z) * c_ - (double)crealf(z) * s_;
                        /* arg(deramped * conj(reference)): the phase relative to
                         * the mean, already inside (-pi, pi] with no folding. */
                        const double pr_re = dr_ * ref_re + di_ * ref_im;
                        const double pr_im = di_ * ref_re - dr_ * ref_im;
                        const double psi = (pr_re != 0.0 || pr_im != 0.0)
                                         ? atan2(pr_im, pr_re) : 0.0;

                        /* THE DE-RAMPED SAMPLE ITSELF goes to the phase linker,
                         * not a phase estimated from it. rs_phase_link() forms
                         * the sample covariance across sub-looks and solves for
                         * the maximum-likelihood phase vector using every pair,
                         * which is what item 64's averaging discarded.
                         *
                         * De-ramped BEFORE the covariance because each pixel has
                         * its own carrier -- averaging cross-terms over pixels
                         * with different carriers decoheres them. */
                        plsig[k * n_sel + si] = (float complex)(dr_ + I * di_);
                        pxacc[k] += -psi * lambda / (4.0 * M_PI);

                        const size_t idx = (size_t)w * n_looks + k;
                        /* The BRIGHTEST pixel's amplitude is what
                         * rs_spectrum_am_check() reads, so only si == 0 writes
                         * it -- a mean over pixels would not be any scatterer's
                         * brightness. */
                        if (si == 0 && out->amp) out->amp[idx] = (double)cabsf(z);
                    }

                    {
                        /* This pixel's series, scaled by its weight and folded
                         * into the window accumulator. */
                        double amean = 0.0;
                        for (size_t k = 0; k < n_looks; k++) {
                            const rs_slc_t *im = &stack->look[k];
                            amean += (double)cabsf(im->data[pa * im->n_rg + pr]);
                        }
                        amean /= (double)n_looks;
                        const double wgt = amean * amean;
                        for (size_t k = 0; k < n_looks; k++) pxsum[k] += wgt * pxacc[k];
                        wsum += wgt;
                        any_px = 1;
                    }
                    }   /* end per-pixel loop */

                    if (!any_px || !(wsum > 0.0)) goto next_window;

                    if (n_sel > 1 &&
                        rs_phase_link(plsig, n_looks, n_sel, plph) == RS_OK) {
                        /* Phase linking: the maximum-likelihood phase over the
                         * whole covariance, referenced to look 0. */
                        for (size_t k = 0; k < n_looks; k++) {
                            const size_t idx = (size_t)w * n_looks + k;
                            out->phase[idx] = plph[k];
                            out->disp_los[idx] = -plph[k] * lambda / (4.0 * M_PI);
                            out->disp_az[idx] = out->disp_rg[idx] = 0.0;
                        }
                    } else {
                        for (size_t k = 0; k < n_looks; k++) {
                            const size_t idx = (size_t)w * n_looks + k;
                            const double d = pxsum[k] / wsum;
                            out->disp_los[idx] = d;
                            out->phase[idx] = -d * 4.0 * M_PI / lambda;
                            out->disp_az[idx] = out->disp_rg[idx] = 0.0;
                        }
                    }

                    /* Velocity by differencing, so that the spectrum sees the
                     * same observable it does for the other estimators. */
                    for (size_t k = 0; k < n_looks; k++) {
                        const size_t idx = (size_t)w * n_looks + k;
                        if (k == 0 || stack->dt <= 0.0) { out->vel_los[idx] = 0.0; continue; }
                        out->vel_los[idx] = (out->disp_los[idx] - out->disp_los[idx - 1])
                                          / stack->dt;
                    }

                    /* QUALITY IS SPATIAL DOMINANCE, NOT TEMPORAL STABILITY.
                     *
                     * Scoring a phase window by how constant its PHASE is would
                     * reward exactly the windows where nothing moves, and the
                     * gate would then select static ground. That argument is
                     * still right and it is why this is not phase stability.
                     *
                     * It used to be amplitude stability, 1 - sigma_A/mu_A over
                     * the looks -- the complement of d_a on the same pixel. That
                     * has the same defect one level down, and item 45 measured
                     * it on ICEYE Houston: a scatterer vibrating at 2 mm is NOT
                     * amplitude-stable across sub-looks, because that is what
                     * the motion does to it. Six windows carried the injected
                     * frequency at the highest prominence in the scene and every
                     * one of them failed the gate at quality 0.21-0.24 against a
                     * reported artefact's 0.54, so the tool reported a trend
                     * instead. The gate preferred targets that did not move.
                     *
                     * The precondition this is meant to be a proxy for is item
                     * 15's: ONE DOMINANT SCATTERER PER SUB-LOOK RESOLUTION CELL.
                     * That is a statement about space, not time -- and a
                     * vibrating dominant is still dominant. So the measure is
                     * the peak's contrast against its own window,
                     * 1 - mean/peak, which is what RS_MICROM_EST_ARGMAX already
                     * uses and which the motion does not degrade.
                     *
                     * Taken on the REFERENCE look only, not averaged over the
                     * stack. That is the look the pixel was chosen from, and it
                     * keeps the estimator reading one pixel per look rather than
                     * a whole patch -- the cost argument in rs_phasor_mag()'s
                     * comment depends on it.
                     *
                     * d_a is unchanged and still reports amplitude dispersion,
                     * so the two are no longer complements: quality now answers
                     * "is there a dominant scatterer here" and d_a answers "is
                     * it a persistent one". Those were one number and are two. */
                    out->quality[w] = dominance;

                    if (params->coherence_min > 0.0 && dominance < params->coherence_min) {
                        for (size_t k = 0; k < n_looks; k++) {
                            const size_t idx = (size_t)w * n_looks + k;
                            out->disp_los[idx] = out->vel_los[idx] = 0.0;
                            out->phase[idx] = 0.0;
                        }
                    }
                    goto next_window;
                }

                if (params->estimator == RS_MICROM_EST_SPLITBAND) {
                    /* Gather every look's patch, then estimate all shifts at
                     * once across the stack. The estimator is inherently
                     * multi-image: it cannot be decomposed into per-look calls
                     * without discarding the information that makes it better
                     * than correlation. */
                    for (size_t k = 0; k < n_looks; k++) {
                        if (rs_coreg_extract(&stack->look[k], az0, rg0, win_az, win_rg,
                                             pstack + k * win_az * win_rg) != RS_OK) {
                            goto next_window;
                        }
                    }

                    double coh = 0.0;
                    if (rs_splitband_shift(pstack, n_looks, win_az, win_rg,
                                           pshift, &coh) != RS_OK) {
                        goto next_window;
                    }

                    for (size_t k = 0; k < n_looks; k++) {
                        const size_t idx = (size_t)w * n_looks + k;
                        out->disp_az[idx] = pshift[k];
                        out->disp_rg[idx] = 0.0;
                        if (can_velocity) {
                            out->vel_los[idx] = pshift[k] * az_spacing * v_plat / r_slant;
                        }
                    }
                    out->quality[w] = coh;

                    if (params->coherence_min > 0.0 && coh < params->coherence_min) {
                        for (size_t k = 0; k < n_looks; k++) {
                            const size_t idx = (size_t)w * n_looks + k;
                            out->disp_az[idx] = out->disp_rg[idx] = 0.0;
                            out->vel_los[idx] = out->disp_los[idx] = 0.0;
                        }
                    }
                    goto next_window;
                }

                /* Summarise correlation across looks by the mean, not the
                 * minimum. With many looks the first and last share few or no
                 * pulses and correlate poorly by construction; taking the worst
                 * pair would then report near-zero quality for a window that
                 * tracks perfectly well over most of the aperture. */
                double qsum = 0.0;
                size_t qn = 0;

                /* Ampcor-style surface statistics, accumulated over the same
                 * looks 'qsum' is: the SNR as a plain mean, the azimuth sigma
                 * as a sum of variances so the aggregate is an rms. See
                 * rs_microm_t.snr and .sigma_px. */
                double snr_sum = 0.0, var_sum = 0.0;

                /* Running absolute shift, used when accumulating differentials
                 * from adjacent looks. */
                double acc_az = 0.0, acc_rg = 0.0;

                for (size_t k = 0; k < n_looks; k++) {
                    const size_t idx = (size_t)w * n_looks + k;

                    /* PAIR measures slave(k) against master(k), so every k
                     * carries a sample -- there is no zero reference look. LAG
                     * has no sample until the lag is available. */
                    const size_t lag = (params->reference == RS_MICROM_REF_LAG)
                                     ? ((params->ref_lag > 0) ? params->ref_lag : 1)
                                     : 0;
                    if (params->reference == RS_MICROM_REF_LAG ? (k < lag)
                        : (k == 0 && params->reference != RS_MICROM_REF_PAIR)) {
                        out->disp_az[idx] = out->disp_rg[idx] = out->disp_los[idx] = 0.0;
                        continue;
                    }

                    if (params->reference == RS_MICROM_REF_LAG) {
                        /* The reference moves with the look, so it is re-extracted
                         * each time rather than held from look 0. That is the whole
                         * point of the mode: coherence is set by the lag alone. */
                        if (rs_coreg_extract(&stack->look[k - lag], az0, rg0,
                                             win_az, win_rg, pref) != RS_OK)
                            continue;
                        if (rs_coreg_extract(&stack->look[k], az0, rg0,
                                             win_az, win_rg, pcur) != RS_OK)
                            continue;
                    } else if (params->reference == RS_MICROM_REF_PAIR) {
                        if (rs_coreg_extract(&stack->look[k], az0, rg0,
                                             win_az, win_rg, pref) != RS_OK)
                            continue;
                        if (rs_coreg_extract(&stack->slave[k], az0, rg0,
                                             win_az, win_rg, pcur) != RS_OK)
                            continue;
                    } else if (rs_coreg_extract(&stack->look[k], az0, rg0,
                                                win_az, win_rg, pcur) != RS_OK) {
                        continue;
                    }

                    double sa = 0.0, sr = 0.0;
                    rs_coreg_quality_t cq;
                    if (rs_coreg_shift_q(pref, pcur, win_az, win_rg,
                                         params->upsample_az, params->upsample_rg,
                                         refine, &sa, &sr, &cq) != RS_OK) {
                        continue;
                    }
                    const double pk = cq.peak;

                    if (params->reference == RS_MICROM_REF_ADJACENT) {
                        /* 'sa' is the step since the previous look; the absolute
                         * shift is its running sum. The previous look becomes
                         * the reference for the next comparison. */
                        acc_az += sa;
                        acc_rg += sr;
                        sa = acc_az;
                        sr = acc_rg;
                        float complex *swap = pref; pref = pcur; pcur = swap;
                    }

                    out->disp_az[idx] = sa;
                    out->disp_rg[idx] = sr;
                    qsum += pk; qn++;
                    snr_sum += cq.snr;
                    var_sum += cq.sigma_az_px * cq.sigma_az_px;

                    /* Azimuth shift to line-of-sight velocity. This observable
                     * does not wrap, which is why it is the primary one. */
                    if (can_velocity) {
                        out->vel_los[idx] = sa * az_spacing * v_plat / r_slant;
                    }

                    /* Phase refinement. The window-averaged interferometric
                     * phase between this look and the reference converts to a
                     * line-of-sight displacement, which is far finer than the
                     * tracked shift but ambiguous modulo lambda/2. Tracking
                     * above resolves the ambiguity; this supplies precision
                     * within it.
                     *
                     * THE REFERENCE HERE MUST BE THE ONE THE SHIFT WAS TAKEN
                     * AGAINST. It was stack->look[0] for every mode except PAIR,
                     * which is right for FIRST, defensible for ADJACENT -- whose
                     * shift accumulates to an absolute displacement, so an
                     * absolute phase matches it -- and WRONG FOR LAG. Lag exists
                     * precisely to be a differencing observable with no
                     * accumulation: it compares look k against look k-lag
                     * because that is the only pair that stays coherent. Reading
                     * its phase against look 0 gave a --shifts dump whose
                     * disp_az column differenced over one interval and whose
                     * phase column differenced over another, so the two
                     * described different measurements and only the header said
                     * which. Found by giving the mode its first test; see
                     * docs/CODE-REVIEW.md. */
                    const size_t ref_k =
                        (params->reference == RS_MICROM_REF_LAG) ? (k - lag) : 0;
                    double acc_re = 0.0, acc_im = 0.0;
                    const float complex *a =
                        (params->reference == RS_MICROM_REF_PAIR)
                            ? stack->look[k].data : stack->look[ref_k].data;
                    const float complex *b =
                        (params->reference == RS_MICROM_REF_PAIR)
                            ? stack->slave[k].data : stack->look[k].data;
                    for (size_t i = 0; i < win_az; i++) {
                        const size_t row = (az0 + i) * ref_img->n_rg + rg0;
                        for (size_t j = 0; j < win_rg; j++) {
                            const float complex prod = b[row + j] * conjf(a[row + j]);
                            acc_re += (double)crealf(prod);
                            acc_im += (double)cimagf(prod);
                        }
                    }
                    out->phase[idx] = atan2(acc_im, acc_re);
                }

                /* Phase against the series' own mean phasor, for the same
                 * reason the phase estimator above no longer accumulates: a
                 * temporal unwrap adds each fold's error to a running total on
                 * an aperture whose sub-looks decorrelate, and the total is a
                 * random walk of sigma*sqrt(N) that swamps the signal. This is
                 * the interferometric phase between look and reference, which
                 * decorrelates the same way the single-pixel phase does.
                 *
                 * Averaging the phasor rather than the angle is what makes this
                 * well defined: angles near +/-pi average to nonsense, phasors
                 * do not. */
                {
                    double ref_re = 0.0, ref_im = 0.0;
                    for (size_t k = 0; k < n_looks; k++) {
                        const double p = out->phase[(size_t)w * n_looks + k];
                        ref_re += cos(p);
                        ref_im += sin(p);
                    }
                    for (size_t k = 0; k < n_looks; k++) {
                        const size_t idx = (size_t)w * n_looks + k;
                        const double c = cos(out->phase[idx]), s = sin(out->phase[idx]);
                        const double d_re = c * ref_re + s * ref_im;
                        const double d_im = s * ref_re - c * ref_im;
                        const double psi = (d_re != 0.0 || d_im != 0.0)
                                         ? atan2(d_im, d_re) : 0.0;
                        out->disp_los[idx] = -lambda / (4.0 * M_PI) * psi;
                    }
                }

                const double qmean = qn ? qsum / (double)qn : 0.0;
                out->quality[w] = qmean;
                out->snr[w] = qn ? snr_sum / (double)qn : 0.0;
                /* A window that produced no correlation at all gets the ceiling
                 * rather than zero. Zero would read as a perfectly determined
                 * offset, which is the opposite of what "nothing tracked" means,
                 * and would carry such a window through a cull that exists to
                 * remove it. */
                out->sigma_px[w] = qn ? sqrt(var_sum / (double)qn)
                                      : RS_COREG_SIGMA_MAX;

                /* Enforce the coherence mask here rather than leaving it to
                 * consumers. A window whose sub-looks do not correlate has no
                 * displacement measurement in it, only tracking noise -- and
                 * noise put through a periodogram yields a confident-looking
                 * spectral peak indistinguishable in appearance from a real
                 * structural mode. Zeroing the series makes such a window
                 * contribute a flat spectrum and no dominant frequency, while
                 * quality[] preserves why it was rejected.
                 *
                 * Note that isolated point targets on an otherwise empty scene
                 * score low here even when tracking perfectly, because most of
                 * the correlation window is background. Distributed clutter --
                 * which is what real structures sit in -- scores far higher.
                 * Set coherence_min to 0 to inspect an unmasked result. */
                if (params->coherence_min > 0.0 && qmean < params->coherence_min) {
                    for (size_t k = 0; k < n_looks; k++) {
                        const size_t idx = (size_t)w * n_looks + k;
                        out->disp_az[idx] = out->disp_rg[idx] = 0.0;
                        out->vel_los[idx] = out->disp_los[idx] = 0.0;
                    }
                }
            next_window: ;
            }
        }

        free(pref);
        free(pcur);
        free(pxacc);
        free(pxsum);
        free(pxtaken);
        free(ksref); free(kscan); free(kstmp); free(plsig); free(plph);
        free(pstack);
        free(pshift);
    }

    if (shared_st != RS_OK) {
        rs_microm_free(out);
        rs_set_error("microm: allocation failed while tracking");
        return shared_st;
    }

    if (params->remove_common_mode && out->n_win > 2) {
        rs_microm_remove_common_mode(out, az_spacing, v_plat, r_slant, can_velocity);
    }

    return RS_OK;
}


/* Sub-look count implied by the phase-ambiguity condition. */
size_t rs_microm_recommend_looks(double vib_freq, double amp_los,
                                 double t_dwell, double lambda, double overlap)
{
    if (!(vib_freq > 0.0) || !(amp_los > 0.0) || !(t_dwell > 0.0) ||
        !(lambda > 0.0) || !(overlap >= 0.0 && overlap < 1.0)) {
        return 0;
    }

    const double m_need = (4.0 * M_PI / 0.75) * vib_freq * amp_los * t_dwell / lambda;
    const double n = 1.0 + (m_need - 1.0) / (1.0 - overlap);
    if (!(n > 1.0)) return 2;
    return (size_t)ceil(n);
}
