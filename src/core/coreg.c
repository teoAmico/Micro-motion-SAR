/* Two-dimensional normalised cross-correlation with sub-pixel refinement.
 *
 * This is the measurement primitive of the whole pipeline: everything the
 * micro-motion stage reports rests on locating a correlation peak to a small
 * fraction of a pixel. */

#include "resonarsat/coreg.h"
#include "resonarsat/fft.h"

#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* Copy a patch out of an image, subtracting its mean.
 *
 * Mean removal is what makes the subsequent correlation a covariance rather
 * than a raw inner product, so that a uniform brightness offset between two
 * looks does not bias the peak. Patches partly outside the image are rejected
 * by the caller, so no bounds handling is needed here. */
static void rs_extract_patch(const rs_slc_t *img, size_t az0, size_t rg0,
                             size_t n_az, size_t n_rg, float complex *patch)
{
    float complex sum = 0.0f;
    for (size_t a = 0; a < n_az; a++) {
        for (size_t r = 0; r < n_rg; r++) {
            const float complex v = img->data[(az0 + a) * img->n_rg + (rg0 + r)];
            patch[a * n_rg + r] = v;
            sum += v;
        }
    }
    const float complex mean = sum / (float)(n_az * n_rg);
    for (size_t i = 0; i < n_az * n_rg; i++) patch[i] -= mean;
}

/* Fill 'out[k] = exp(i * tau * f_k)', with f_k the SIGNED frequency index of
 * bin k -- bins above Nyquist represent negative frequencies.
 *
 * This is half of the separation described on rs_correlation_at(). */
static void rs_phase_ramp(double tau, size_t n, double complex *out)
{
    for (size_t k = 0; k < n; k++) {
        const double f = (k < n / 2) ? (double)k : (double)k - (double)n;
        const double p = tau * f;
        out[k] = cos(p) + I * sin(p);
    }
}

/* Evaluate the cross-correlation surface at one fractional offset, by direct
 * summation of the cross-spectrum against a complex exponential.
 *
 * This is the Guizar-Sicairos refinement in its essential form: instead of
 * upsampling the entire correlation surface by a large factor, evaluate the
 * inverse transform only at the handful of fractional offsets near the coarse
 * peak. The cost is O(N) per evaluated point rather than O(N log N) for a whole
 * upsampled surface, and only a few dozen points are ever needed.
 *
 * THE PHASE IS SEPARABLE, WHICH IS WHY NO TRIGONOMETRY HAPPENS HERE. The
 * exponent is tau_az*f_a + tau_rg*f_r, linear in each index independently, so
 *
 *     exp(i*(tau_az*f_a + tau_rg*f_r)) = exp(i*tau_az*f_a) * exp(i*tau_rg*f_r)
 *
 * and the double loop is an outer product of two one-dimensional ramps. Written
 * directly it evaluated cos and sin at all n_az*n_rg inner points -- 576 for a
 * 24x24 patch -- where n_az + n_rg = 48 suffice. That mattered: a sample(1)
 * profile of the test suite attributed 88 percent of the whole process to
 * __sincos_stret underneath this function, which is the pipeline's innermost
 * loop and runs (2*upsample_az+1)*(2*upsample_rg+1) times per patch per look.
 *
 * The identity is exact, so this is not an approximation and no result changes
 * beyond floating-point reassociation. The caller supplies the two ramps, which
 * lets it hoist them out of the refinement grid entirely -- see
 * rs_coreg_shift(), where each is built once per grid LINE rather than once per
 * grid POINT.
 *
 * 'cross' is the cross-spectrum F(a) * conj(F(b)) in unshifted FFT order; 'ea'
 * and 'er' are the azimuth and range ramps from rs_phase_ramp(), of n_az and
 * n_rg elements. */
static double rs_correlation_at(const float complex *cross, size_t n_az, size_t n_rg,
                                const double complex *ea, const double complex *er)
{
    double complex acc = 0.0;

    for (size_t a = 0; a < n_az; a++) {
        const float complex *row = cross + a * n_rg;
        double complex s = 0.0;
        for (size_t r = 0; r < n_rg; r++) {
            s += (double complex)row[r] * er[r];
        }
        acc += s * ea[a];
    }
    return cabs(acc);
}

/* The peak-to-mean power ratio a correlation surface reaches on noise alone.
 * See coreg.h for the derivation. */
double rs_coreg_snr_null(size_t n_az, size_t n_rg)
{
    const size_t n = n_az * n_rg;
    if (n == 0) return 0.0;
    if (n < 64) {
        double h = 0.0;
        for (size_t k = 1; k <= n; k++) h += 1.0 / (double)k;
        return h;
    }
    /* H_n = ln n + gamma + 1/(2n) - ..., truncated after the term that matters
     * at n >= 64; the omitted 1/(12n^2) is below 3e-6 there. */
    return log((double)n) + 0.5772156649015329 + 0.5 / (double)n;
}

/* Peak power over the mean power of the surface outside an exclusion zone.
 *
 * 'surf' is a correlation surface of 'n_a' by 'n_r' bins in unshifted FFT order,
 * 'peak' its maximum bin index, and 'ex_a'/'ex_r' the exclusion half-widths in
 * BINS -- the caller converts RS_COREG_SNR_EXCLUDE_PX to bins according to how
 * finely its own surface is sampled.
 *
 * Taken as the complex surface rather than as precomputed magnitudes so that
 * both refinement paths can pass what they already hold. The exhaustive path's
 * surface reaches RS_COREG_MAX_SURFACE elements, and a magnitude copy of it
 * would be a second allocation larger than the surface itself.
 *
 * Distances wrap, because the surface does: a peak near bin zero has its main
 * lobe split across both ends of the axis, and measuring the exclusion without
 * wrapping would leave half that lobe in the background mean. That case is not
 * rare, it is the ordinary one -- a small shift puts the peak at or beside the
 * origin.
 *
 * Returns zero when the exclusion swallows the whole surface or the background
 * has no power, rather than dividing by zero; a zero SNR fails every gate, which
 * is the right reading of a surface that could not be characterised. */
static double rs_surface_snr(const float complex *surf, size_t n_a, size_t n_r,
                             size_t peak, size_t ex_a, size_t ex_r)
{
    const size_t pa = peak / n_r, pr = peak % n_r;
    const double peak_mag = (double)cabsf(surf[peak]);
    const double peak_pow = peak_mag * peak_mag;

    double sum = 0.0;
    size_t count = 0;
    for (size_t a = 0; a < n_a; a++) {
        size_t da = (a > pa) ? a - pa : pa - a;
        if (da > n_a - da) da = n_a - da;
        if (da <= ex_a) continue;
        for (size_t r = 0; r < n_r; r++) {
            size_t dr = (r > pr) ? r - pr : pr - r;
            if (dr > n_r - dr) dr = n_r - dr;
            if (dr <= ex_r) continue;
            const double m = (double)cabsf(surf[a * n_r + r]);
            sum += m * m;
            count++;
        }
    }
    if (count == 0 || !(sum > 0.0)) return 0.0;
    return peak_pow / (sum / (double)count);
}

/* The curvature baseline, in lattice steps, for a given upsample factor.
 *
 * RS_COREG_CURV_BASELINE_PX converted to steps and floored at one, so that a
 * lattice too coarse to hold the baseline still differences its immediate
 * neighbours rather than degenerating to a zero offset. See that constant. */
static size_t rs_curv_baseline(size_t upsample)
{
    const double h = RS_COREG_CURV_BASELINE_PX * (double)upsample;
    return (h >= 1.0) ? (size_t)(h + 0.5) : 1;
}

/* One-sigma offset uncertainty along one axis, from three samples of the peak.
 *
 * 'c_lo', 'c_pk' and 'c_hi' are the surface magnitudes at the peak and its two
 * neighbours along the axis, 'step' their spacing in pixels, and 'gamma' the
 * normalised correlation coefficient. See rs_coreg_quality_t for the model and
 * for why the result ranks windows rather than bounding an error.
 *
 * Returns RS_COREG_SIGMA_MAX whenever the model does not apply: a non-negative
 * second difference (the sample is not a maximum along this axis), a
 * non-positive peak, or a coherence at or above one, which would claim a
 * noiseless estimate. The caller supplies RS_COREG_SIGMA_MAX for the remaining
 * case, a peak on the lattice edge, where there is no 'c_lo' or 'c_hi' to pass. */
static double rs_peak_sigma(double c_lo, double c_pk, double c_hi,
                            double step, double gamma)
{
    if (!(c_pk > 0.0) || !(step > 0.0)) return RS_COREG_SIGMA_MAX;

    /* kappa = -C''/C_pk, in 1/px^2. Positive at a maximum. */
    const double d2 = (c_lo - 2.0 * c_pk + c_hi) / (step * step);
    const double kappa = -d2 / c_pk;
    if (!(kappa > 0.0)) return RS_COREG_SIGMA_MAX;

    const double g2 = gamma * gamma;
    if (!(g2 > 0.0) || g2 >= 1.0) {
        return (g2 >= 1.0) ? 0.0 : RS_COREG_SIGMA_MAX;
    }

    const double var = (1.0 - g2) / (2.0 * g2 * kappa);
    const double sigma = sqrt(var);
    return (sigma < RS_COREG_SIGMA_MAX) ? sigma : RS_COREG_SIGMA_MAX;
}

/* Scatter one axis of an unshifted spectrum into a longer zero-padded axis.
 *
 * Returns the destination index of source bin 'k' of an axis of length 'n' being
 * padded to length 'n_pad'. Positive frequencies keep their index; negative ones
 * move to the far end, so the zeros land in the middle of the spectrum where the
 * frequencies the original never carried belong. Padding at the end instead --
 * the obvious wrong version -- reinterprets every negative frequency as a high
 * positive one and produces a surface that is not an interpolation of anything.
 *
 * The split point matches rs_phase_ramp() exactly: bins below n/2 are positive,
 * the rest negative, which for even 'n' puts the Nyquist bin on the negative
 * side. Both refinement modes must agree about that or they would be sampling
 * two different surfaces and their disagreement would mean nothing. */
static size_t rs_pad_index(size_t k, size_t n, size_t n_pad)
{
    return (k < n / 2) ? k : n_pad + k - n;
}

/* Locate the peak by evaluating the fine lattice over the entire surface.
 *
 * Zero-pads the cross-spectrum to 'n_pad_az' by 'n_pad_rg', inverse transforms,
 * and takes the global maximum. That is exactly the correlation surface sampled
 * at 1/upsample-pixel spacing everywhere, with no assumption that the true peak
 * lies near the strongest integer sample -- which is the assumption the local
 * path makes and the one this exists to audit.
 *
 * Deliberately written the flat, obvious way: one pass to scatter, one transform,
 * one linear scan. Nothing here is blocked for cache or hoisted out of a loop,
 * because the value of a baseline is that its correctness is apparent by reading
 * it.
 *
 * It is also not nearly as slow as that suggests -- measured at 1.7x to 3.2x the
 * optimised path per call, because rs_correlation_at() costs O(win_az*win_rg) at
 * every one of its refinement points and there are hundreds of them. See
 * rs_microm_params_t.no_optimize for the numbers.
 *
 * 'ref_mag' receives the UNNORMALISED sum magnitude at the peak, on the same
 * scale rs_correlation_at() returns, so the caller's normalisation applies
 * unchanged to either mode. The 1/(n_pad_az*n_pad_rg) that rs_fft2() applies in
 * the inverse direction is undone here.
 *
 * Ties are broken by strict '>' over an ascending row-major scan, keeping the
 * first maximum, matching the local path's documented rule.
 *
 * 'snr' and 'sig_az'/'sig_rg' are optional. When supplied they are measured on
 * this same padded surface: the SNR by rs_surface_snr() with the exclusion zone
 * converted from pixels to this surface's finer bins, and the curvature from the
 * peak's immediate neighbours, one bin apart, which is 1/upsample of a pixel --
 * the same spacing the local path's refinement lattice uses. The two modes
 * therefore report the two statistics from the same definitions at the same
 * scale, which is the contract coreg.h states for the peak value. */
static resonarsat_status_t rs_coreg_exhaustive(const float complex *cross,
                                               size_t n_az, size_t n_rg,
                                               size_t n_pad_az, size_t n_pad_rg,
                                               size_t upsample_az, size_t upsample_rg,
                                               double *shift_az, double *shift_rg,
                                               double *ref_mag,
                                               double *snr, double *sig_az, double *sig_rg,
                                               double norm)
{
    const size_t n_pad = n_pad_az * n_pad_rg;

    float complex *surf = calloc(n_pad, sizeof *surf);
    if (!surf) {
        rs_set_error("coreg: cannot allocate a %zux%zu exhaustive search surface",
                     n_pad_az, n_pad_rg);
        return RS_ERR_ALLOC;
    }

    for (size_t a = 0; a < n_az; a++) {
        const size_t pa = rs_pad_index(a, n_az, n_pad_az);
        for (size_t r = 0; r < n_rg; r++) {
            surf[pa * n_pad_rg + rs_pad_index(r, n_rg, n_pad_rg)] = cross[a * n_rg + r];
        }
    }

    const resonarsat_status_t st = rs_fft2(surf, n_pad_az, n_pad_rg, 1);
    if (st != RS_OK) { free(surf); return st; }

    size_t best = 0;
    double best_mag = -1.0;
    for (size_t i = 0; i < n_pad; i++) {
        const double m = (double)cabsf(surf[i]);
        if (m > best_mag) { best_mag = m; best = i; }
    }

    if (snr) {
        /* The exclusion is stated in pixels; this surface has 'upsample' bins
         * per pixel along each axis. */
        *snr = rs_surface_snr(surf, n_pad_az, n_pad_rg, best,
                              (size_t)(RS_COREG_SNR_EXCLUDE_PX * (double)upsample_az),
                              (size_t)(RS_COREG_SNR_EXCLUDE_PX * (double)upsample_rg));
    }
    if (sig_az || sig_rg) {
        /* Curvature from the peak's neighbours on the padded lattice. The
         * surface wraps, so a peak at bin zero has neighbours at the far end
         * and there is no edge case to exclude -- unlike the local path, whose
         * lattice is a bounded window and genuinely has edges. */
        const double gamma_pk = (norm > 0.0)
            ? ((best_mag * (double)n_pad / norm > 1.0) ? 1.0
                                                       : best_mag * (double)n_pad / norm)
            : 0.0;
        const size_t pa = best / n_pad_rg, pr = best % n_pad_rg;
        const double c_pk = best_mag;
        const size_t hb_a = rs_curv_baseline(upsample_az);
        const size_t hb_r = rs_curv_baseline(upsample_rg);
        if (sig_az && 2 * hb_a < n_pad_az) {
            const size_t lo = (pa + n_pad_az - hb_a) % n_pad_az;
            const size_t hi = (pa + hb_a) % n_pad_az;
            *sig_az = rs_peak_sigma((double)cabsf(surf[lo * n_pad_rg + pr]), c_pk,
                                    (double)cabsf(surf[hi * n_pad_rg + pr]),
                                    (double)hb_a / (double)upsample_az, gamma_pk);
        }
        if (sig_rg && 2 * hb_r < n_pad_rg) {
            const size_t lo = (pr + n_pad_rg - hb_r) % n_pad_rg;
            const size_t hi = (pr + hb_r) % n_pad_rg;
            *sig_rg = rs_peak_sigma((double)cabsf(surf[pa * n_pad_rg + lo]), c_pk,
                                    (double)cabsf(surf[pa * n_pad_rg + hi]),
                                    (double)hb_r / (double)upsample_rg, gamma_pk);
        }
    }
    free(surf);

    /* Unwrap the padded index into a signed shift. Sample i along an axis sits
     * at i/upsample pixels, and the upper half of the axis is negative. */
    double ia = (double)(best / n_pad_rg);
    double ir = (double)(best % n_pad_rg);
    if (ia > (double)n_pad_az / 2.0) ia -= (double)n_pad_az;
    if (ir > (double)n_pad_rg / 2.0) ir -= (double)n_pad_rg;

    *shift_az = ia / (double)upsample_az;
    *shift_rg = ir / (double)upsample_rg;
    *ref_mag  = best_mag * (double)n_pad;
    return RS_OK;
}

/* Bound the exhaustive surface before anything allocates it.
 *
 * Each of the four factors is separately plausible and their product is not:
 * 40x20 upsampling on a 256x256 patch asks for 52 billion samples. The
 * comparisons are written as divisions so the multiplications cannot wrap --
 * a wrapped size would pass the ceiling test and then be allocated. */
resonarsat_status_t rs_coreg_surface_check(size_t n_az, size_t n_rg,
                                          size_t upsample_az, size_t upsample_rg)
{
    if (n_az == 0 || n_rg == 0) return RS_ERR_ARG;
    if (upsample_az == 0) upsample_az = 1;
    if (upsample_rg == 0) upsample_rg = 1;

    if (upsample_az > SIZE_MAX / n_az || upsample_rg > SIZE_MAX / n_rg) {
        rs_set_error("coreg: exhaustive search extent overflows");
        return RS_ERR_RANGE;
    }
    const size_t n_pad_az = n_az * upsample_az;
    const size_t n_pad_rg = n_rg * upsample_rg;
    if (n_pad_rg > RS_COREG_MAX_SURFACE / n_pad_az) {
        rs_set_error("coreg: exhaustive search over %zux%zu patches at %zux%zu "
                     "upsampling needs a %zux%zu surface, above the %zu-sample "
                     "ceiling; reduce --upsample or the window size",
                     n_az, n_rg, upsample_az, upsample_rg,
                     n_pad_az, n_pad_rg, RS_COREG_MAX_SURFACE);
        return RS_ERR_RANGE;
    }
    return RS_OK;
}

/* Locate the shift between two equally sized patches, on the local refinement
 * path. See rs_coreg_shift_ex() for the full contract. */
resonarsat_status_t rs_coreg_shift(const float complex *ref, const float complex *img,
                                   size_t n_az, size_t n_rg,
                                   size_t upsample_az, size_t upsample_rg,
                                   double *shift_az, double *shift_rg, double *peak)
{
    return rs_coreg_shift_ex(ref, img, n_az, n_rg, upsample_az, upsample_rg,
                             RS_COREG_REFINE_LOCAL, shift_az, shift_rg, peak);
}

/* Locate the shift between two equally sized patches.
 *
 * Forms the cross-spectrum, inverse transforms it to get the correlation
 * surface, finds the integer peak, then refines to sub-pixel precision by
 * evaluating the surface on a fine grid around that peak. The refinement grid
 * is +/- 1 pixel at 1/upsample resolution, which is where the true peak must
 * lie once the integer peak is known.
 *
 * Under RS_COREG_REFINE_EXHAUSTIVE the last step is replaced by a global search
 * over the whole zero-padded surface, dropping that "must lie" assumption. The
 * cross-spectrum, its normalisation and the reported quantities are identical
 * either way; only the extent of the peak search changes.
 *
 * The returned 'peak' is the normalised correlation coefficient at the refined
 * location, in [0, 1]; it is the quality metric that masks unreliable windows.
 *
 * Returns RS_ERR_ALLOC on memory failure and, in exhaustive mode, RS_ERR_RANGE
 * if the padded surface would exceed RS_COREG_MAX_SURFACE. Degenerate patches
 * (either one having no variance) yield a peak of zero and a shift of zero
 * rather than a division by zero.
 *
 * 'q' is optional. When NULL nothing beyond the peak is measured and neither the
 * background scan nor the refinement lattice's storage is paid for; when
 * supplied its SNR and sigma fields are filled as rs_coreg_quality_t describes.
 * Its 'peak' field is not written here -- the caller owns '*peak' and copies. */
static resonarsat_status_t rs_coreg_shift_impl(const float complex *ref,
                                               const float complex *img,
                                               size_t n_az, size_t n_rg,
                                               size_t upsample_az, size_t upsample_rg,
                                               rs_coreg_refine_t refine,
                                               double *shift_az, double *shift_rg,
                                               double *peak, rs_coreg_quality_t *q)
{
    if (!ref || !img || !shift_az || !shift_rg || !peak) return RS_ERR_ARG;
    if (q) {
        q->snr = 0.0;
        q->sigma_az_px = q->sigma_rg_px = RS_COREG_SIGMA_MAX;
    }
    if (n_az == 0 || n_rg == 0) return RS_ERR_ARG;
    if (upsample_az == 0) upsample_az = 1;
    if (upsample_rg == 0) upsample_rg = 1;

    /* Bound the padded surface before any allocation, and before the caller has
     * spent two forward transforms on a request that cannot be served. */
    size_t n_pad_az = 0, n_pad_rg = 0;
    if (refine == RS_COREG_REFINE_EXHAUSTIVE) {
        const resonarsat_status_t size_st =
            rs_coreg_surface_check(n_az, n_rg, upsample_az, upsample_rg);
        if (size_st != RS_OK) return size_st;
        n_pad_az = n_az * upsample_az;
        n_pad_rg = n_rg * upsample_rg;
    }

    const size_t n = n_az * n_rg;
    float complex *fa = malloc(n * sizeof *fa);
    float complex *fb = malloc(n * sizeof *fb);
    float complex *cross = malloc(n * sizeof *cross);
    /* Declared here, before the first goto, so the single cleanup path can free
     * them unconditionally. */
    double complex *ea_tab = NULL, *er_tab = NULL;
    /* Magnitudes of the refinement lattice, kept only when the caller asked for
     * the curvature. Allocated with the ramps below so the single cleanup path
     * covers it. */
    double *grid = NULL;
    if (!fa || !fb || !cross) { free(fa); free(fb); free(cross); return RS_ERR_ALLOC; }

    memcpy(fa, ref, n * sizeof *fa);
    memcpy(fb, img, n * sizeof *fb);

    resonarsat_status_t st;
    if ((st = rs_fft2(fa, n_az, n_rg, 0)) != RS_OK) goto done;
    if ((st = rs_fft2(fb, n_az, n_rg, 0)) != RS_OK) goto done;

    /* Energy for normalisation, computed in the frequency domain via
     * Parseval so no extra pass over the patches is needed.
     *
     * The order of the cross-spectrum sets the sign of the reported shift and
     * is easy to get backwards. If img(x) = ref(x - s), then F_img = F_ref *
     * exp(-j*2*pi*f*s), so conj(F_ref) * F_img = |F|^2 * exp(-j*2*pi*f*s),
     * whose inverse transform peaks at +s. Taking the factors the other way
     * round yields a peak at -s: a tracker that reports every displacement
     * with the wrong sign, which still produces plausible-looking vibration
     * spectra because a sign flip is invisible in a power spectrum. */
    double ea = 0.0, eb = 0.0;
    for (size_t i = 0; i < n; i++) {
        const double ma = (double)cabsf(fa[i]), mb = (double)cabsf(fb[i]);
        ea += ma * ma;
        eb += mb * mb;
        cross[i] = conjf(fa[i]) * fb[i];
    }

    const double norm = sqrt(ea * eb);
    if (!(norm > 0.0)) {
        *shift_az = *shift_rg = 0.0;
        *peak = 0.0;
        st = RS_OK;
        goto done;
    }

    /* The audit baseline searches the whole surface and is done here. It needs
     * no coarse peak, so it skips the third transform below rather than
     * computing a starting point it would not use. */
    if (refine == RS_COREG_REFINE_EXHAUSTIVE) {
        double mag = 0.0;
        st = rs_coreg_exhaustive(cross, n_az, n_rg, n_pad_az, n_pad_rg,
                                 upsample_az, upsample_rg, shift_az, shift_rg, &mag,
                                 q ? &q->snr : NULL,
                                 q ? &q->sigma_az_px : NULL,
                                 q ? &q->sigma_rg_px : NULL,
                                 norm);
        if (st == RS_OK) {
            *peak = mag / norm;
            if (*peak > 1.0) *peak = 1.0;
        }
        goto done;
    }

    /* Coarse peak from the full correlation surface. */
    memcpy(fa, cross, n * sizeof *fa);
    if ((st = rs_fft2(fa, n_az, n_rg, 1)) != RS_OK) goto done;

    size_t best = 0;
    double best_mag = -1.0;
    for (size_t i = 0; i < n; i++) {
        const double m = (double)cabsf(fa[i]);
        if (m > best_mag) { best_mag = m; best = i; }
    }

    /* The SNR is measured on the INTEGER surface, whose bins are one pixel
     * apart, so the exclusion converts directly. Measuring it here rather than
     * on the refinement lattice is deliberate: the statistic is about the whole
     * surface -- whether a competing lobe exists somewhere else entirely -- and
     * the refinement lattice spans one pixel and can see no such thing. */
    if (q) {
        q->snr = rs_surface_snr(fa, n_az, n_rg, best,
                                (size_t)RS_COREG_SNR_EXCLUDE_PX,
                                (size_t)RS_COREG_SNR_EXCLUDE_PX);
    }

    /* Unwrap the integer peak index into a signed shift. A peak in the upper
     * half of either axis means a negative shift. */
    double coarse_az = (double)(best / n_rg);
    double coarse_rg = (double)(best % n_rg);
    if (coarse_az > (double)n_az / 2.0) coarse_az -= (double)n_az;
    if (coarse_rg > (double)n_rg / 2.0) coarse_rg -= (double)n_rg;

    /* Refine on a fine grid spanning +/- 1 pixel about the coarse peak. */
    double ref_az = coarse_az, ref_rg = coarse_rg, ref_mag = best_mag;
    const double step_az = 1.0 / (double)upsample_az;
    const double step_rg = 1.0 / (double)upsample_rg;

    /* Build every phase ramp the grid needs, once.
     *
     * The azimuth ramp depends only on the row of the search grid and the range
     * ramp only on its column, so the grid needs n_ia + n_ir ramps rather than
     * n_ia * n_ir of them. Combined with the separation inside
     * rs_correlation_at(), the trigonometry for a 24x24 patch at 40x20
     * upsampling falls from about 1.9 million evaluations to under three
     * thousand, and what remains in the inner loop is multiply-accumulate.
     *
     * The tables are small -- (n_ia*n_az + n_ir*n_rg) complex doubles, tens of
     * kilobytes at the sizes this is called with -- and are allocated per call
     * because rs_coreg_shift() is already called once per patch per look. */
    const size_t n_ia = 2 * upsample_az + 1;
    const size_t n_ir = 2 * upsample_rg + 1;
    ea_tab = malloc(n_ia * n_az * sizeof *ea_tab);
    er_tab = malloc(n_ir * n_rg * sizeof *er_tab);
    if (!ea_tab || !er_tab) { st = RS_ERR_ALLOC; goto done; }
    if (q) {
        grid = malloc(n_ia * n_ir * sizeof *grid);
        if (!grid) { st = RS_ERR_ALLOC; goto done; }
    }

    for (size_t i = 0; i < n_ia; i++) {
        const double ta = coarse_az + ((double)i - (double)upsample_az) * step_az;
        rs_phase_ramp(2.0 * M_PI * ta / (double)n_az, n_az, ea_tab + i * n_az);
    }
    for (size_t j = 0; j < n_ir; j++) {
        const double tr = coarse_rg + ((double)j - (double)upsample_rg) * step_rg;
        rs_phase_ramp(2.0 * M_PI * tr / (double)n_rg, n_rg, er_tab + j * n_rg);
    }

    /* Ascending in both axes, matching the original loop over
     * ia = -upsample_az .. +upsample_az. The order is not incidental: ties are
     * resolved by strict '>' and therefore keep the FIRST maximum, so visiting
     * the grid in another order would move the reported shift on a flat peak. */
    size_t peak_i = n_ia, peak_j = n_ir;
    for (size_t i = 0; i < n_ia; i++) {
        const double ta = coarse_az + ((double)i - (double)upsample_az) * step_az;
        for (size_t j = 0; j < n_ir; j++) {
            const double tr = coarse_rg + ((double)j - (double)upsample_rg) * step_rg;
            const double m = rs_correlation_at(cross, n_az, n_rg,
                                               ea_tab + i * n_az, er_tab + j * n_rg);
            if (grid) grid[i * n_ir + j] = m;
            if (m > ref_mag) { ref_mag = m; ref_az = ta; ref_rg = tr; peak_i = i; peak_j = j; }
        }
    }

    *shift_az = ref_az;
    *shift_rg = ref_rg;
    /* The inverse transform carries a 1/n normalisation the direct evaluation
     * does not, so scale the direct result before comparing with the energy. */
    *peak = (ref_mag / (double)n) / norm * (double)n;
    if (*peak > 1.0) *peak = 1.0;

    /* Curvature, once the normalised peak it needs is known.
     *
     * The lattice is bounded, unlike the padded surface the exhaustive path
     * measures, so a maximum on its edge has no second difference. That is left
     * at RS_COREG_SIGMA_MAX rather than estimated one-sidedly: an edge maximum
     * means the true peak is outside the searched neighbourhood, which makes the
     * OFFSET suspect and not merely its uncertainty. 'peak_i' stays out of range
     * only if no lattice point beat the coarse magnitude, which cannot happen --
     * the lattice contains the coarse point itself -- and is handled anyway
     * because the alternative is an out-of-bounds read if it ever does. */
    if (grid && peak_i < n_ia && peak_j < n_ir) {
        const double c_pk = grid[peak_i * n_ir + peak_j];
        const size_t hb_a = rs_curv_baseline(upsample_az);
        const size_t hb_r = rs_curv_baseline(upsample_rg);
        if (peak_i >= hb_a && peak_i + hb_a < n_ia) {
            q->sigma_az_px = rs_peak_sigma(grid[(peak_i - hb_a) * n_ir + peak_j], c_pk,
                                           grid[(peak_i + hb_a) * n_ir + peak_j],
                                           (double)hb_a * step_az, *peak);
        }
        if (peak_j >= hb_r && peak_j + hb_r < n_ir) {
            q->sigma_rg_px = rs_peak_sigma(grid[peak_i * n_ir + peak_j - hb_r], c_pk,
                                           grid[peak_i * n_ir + peak_j + hb_r],
                                           (double)hb_r * step_rg, *peak);
        }
    }
    st = RS_OK;

done:
    free(fa); free(fb); free(cross); free(ea_tab); free(er_tab); free(grid);
    return st;
}

/* Locate the shift between two patches, reporting only the peak value.
 * See the contract above. */
resonarsat_status_t rs_coreg_shift_ex(const float complex *ref, const float complex *img,
                                      size_t n_az, size_t n_rg,
                                      size_t upsample_az, size_t upsample_rg,
                                      rs_coreg_refine_t refine,
                                      double *shift_az, double *shift_rg, double *peak)
{
    return rs_coreg_shift_impl(ref, img, n_az, n_rg, upsample_az, upsample_rg,
                               refine, shift_az, shift_rg, peak, NULL);
}

/* Locate the shift and report what the surface says about its own reliability.
 * See coreg.h and rs_coreg_quality_t. */
resonarsat_status_t rs_coreg_shift_q(const float complex *ref, const float complex *img,
                                     size_t n_az, size_t n_rg,
                                     size_t upsample_az, size_t upsample_rg,
                                     rs_coreg_refine_t refine,
                                     double *shift_az, double *shift_rg,
                                     rs_coreg_quality_t *q)
{
    if (!q) return RS_ERR_ARG;
    return rs_coreg_shift_impl(ref, img, n_az, n_rg, upsample_az, upsample_rg,
                               refine, shift_az, shift_rg, &q->peak, q);
}

/* Extract a patch from an image with the mean removed. Thin wrapper so callers
 * outside this file need not know the layout convention. */
resonarsat_status_t rs_coreg_extract(const rs_slc_t *img, size_t az0, size_t rg0,
                                     size_t n_az, size_t n_rg, float complex *patch)
{
    if (!img || !patch || !img->data) return RS_ERR_ARG;
    if (az0 + n_az > img->n_az || rg0 + n_rg > img->n_rg) {
        rs_set_error("coreg: patch at (%zu,%zu) size %zux%zu exceeds image %zux%zu",
                     az0, rg0, n_az, n_rg, img->n_az, img->n_rg);
        return RS_ERR_ARG;
    }
    rs_extract_patch(img, az0, rg0, n_az, n_rg, patch);
    return RS_OK;
}
