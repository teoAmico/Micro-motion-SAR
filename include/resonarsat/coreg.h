/* Cross-correlation primitives used by the micro-motion stage. */

#ifndef RESONARSAT_COREG_H
#define RESONARSAT_COREG_H

#include <complex.h>
#include <stddef.h>

#include "resonarsat/resonarsat.h"
#include "resonarsat/slc.h"

/* How the sub-pixel peak is located on the correlation surface.
 *
 * The two modes search the SAME lattice -- 1/upsample_az by 1/upsample_rg
 * pixels -- and differ only in extent. That is what makes them comparable: a
 * disagreement is a disagreement about where the peak is, not about how finely
 * the surface was sampled.
 *
 * RS_COREG_REFINE_LOCAL is the production path. It takes the integer peak of the
 * full surface and then evaluates the fine lattice within one pixel of it.
 *
 * RS_COREG_REFINE_EXHAUSTIVE is the audit baseline reached by --no-optimize. It
 * evaluates every point of the fine lattice over the WHOLE surface, by
 * zero-padding the cross-spectrum to (n_az*upsample_az) by (n_rg*upsample_rg)
 * and inverse transforming, then taking the global maximum. This is the
 * textbook, uninteresting way to do it and it costs the memory to match:
 * upsample factors of 10 by 20 on a 24 by 24 patch build a 240 by 480 surface
 * per call per thread.
 *
 * WHAT THE COMPARISON CAN AND CANNOT SHOW. The local path's integer peak is
 * already the GLOBAL maximum of the sampled surface -- it is found by a full
 * inverse transform and a scan over every bin (see rs_coreg_shift()), not by a
 * local search. So the exhaustive mode cannot detect a missed distant lobe in
 * the sampled surface; it can only detect the case where the true continuous
 * peak lies more than one pixel from the strongest SAMPLE. That happens when the
 * surface is not adequately sampled: two comparable lobes a few pixels apart,
 * where interpolation between the samples crests higher near the weaker one.
 * On decorrelated sub-looks with several competing scatterers this is a real
 * possibility, and before this mode existed there was no way to measure how
 * often it occurred. Expect the two modes to agree on almost every window; the
 * windows where they do not are the interesting output. */
typedef enum {
    RS_COREG_REFINE_LOCAL = 0,  /* one pixel about the integer peak (default) */
    RS_COREG_REFINE_EXHAUSTIVE  /* whole zero-padded surface, global maximum */
} rs_coreg_refine_t;

/* What the correlation surface says about its own reliability.
 *
 * WHY THIS EXISTS. The pipeline has always carried one number per correlation --
 * the normalised peak value -- and used it as the mask. That number answers "how
 * alike are these two patches", which is not the same question as "how well is
 * this offset determined". A pair of patches can be highly correlated and still
 * place the peak badly, if the surface is broad or if a second lobe of nearly
 * equal height sits a few pixels away; and a pair with modest correlation can
 * place a sharp peak precisely. The offset-tracking correlators in the InSAR
 * community -- ISCE's `ampcor`, GMTSAR's `xcorr` -- have carried the other two
 * numbers since the beginning and cull on them, and the culling is the part of
 * those implementations worth having. This is that.
 *
 * 'peak' is the normalised correlation coefficient, unchanged and still the
 * mask. The two new quantities are:
 *
 * 'snr' -- the peak's power divided by the mean power of the correlation surface
 * away from it. This is a SHAPE statistic and is nearly independent of 'peak':
 * it asks whether the surface has one distinguished maximum or a field of
 * comparable bumps. Two lobes of similar height give a low SNR at a high peak
 * value, which is precisely the configuration coreg.h warns about under
 * RS_COREG_REFINE_EXHAUSTIVE and which the peak value alone cannot report.
 *
 * The value it takes on a surface with no signal in it is calculable rather than
 * empirical, which is what makes it usable as a gate. For N independent bins
 * whose powers are exponentially distributed -- the null the surface takes when
 * the patches share nothing -- the expected maximum is the mean times the N-th
 * harmonic number, so
 *
 *     E[snr | no signal]  =  H_N  =  sum_{k=1..N} 1/k  ~  ln N + 0.5772
 *
 * which for a 32 by 32 window is about 7.5. rs_coreg_snr_null() computes it, and
 * the selection stage gates well above it. THE BINS ARE NOT INDEPENDENT -- the
 * surface is the transform of a band-limited cross-spectrum, so neighbouring
 * bins are correlated and the effective N is smaller than the literal one. That
 * makes H_N an OVERESTIMATE of the effective count and therefore of the null,
 * which errs towards a stricter gate; the direction is stated because it is the
 * safe one, not because the size of the error is known.
 *
 * 'sigma_az_px' and 'sigma_rg_px' -- the one-sigma uncertainty of the located
 * offset along each axis, in pixels, from the curvature of the peak. Model the
 * surface near its maximum as C(x) = C_pk * (1 - kappa*x^2/2), measure kappa by
 * second difference on the same fine lattice the refinement searched, and take
 *
 *     sigma^2  =  (1 - gamma^2) / (2 * gamma^2 * kappa)
 *
 * with gamma the normalised peak. The shape is the standard curvature-over-noise
 * result: a sharp peak (large kappa) or a coherent one (gamma near 1) is well
 * located, and the uncertainty diverges as either fails. It is NOT CALIBRATED IN
 * AN ABSOLUTE SENSE and must not be read as an error bar. The omitted factor is
 * the number of independent looks in the patch, which is constant across every
 * window of a run and therefore cancels wherever this is used -- for ranking and
 * culling windows against each other, which is the only thing it is used for.
 *
 * A peak that lands on the EDGE of the refinement lattice has no second
 * difference and yields RS_COREG_SIGMA_MAX rather than an invented number. That
 * is the correct outcome and not a degradation: a maximum at the edge of the
 * searched neighbourhood means the surface was not peaked where it was looked
 * at, and a cull should drop the window for exactly that reason.
 *
 * Zeroed, with sigma at RS_COREG_SIGMA_MAX, on a degenerate patch. */
typedef struct {
    double peak;                     /* normalised correlation coefficient */
    double snr;                      /* peak power / mean off-peak power */
    double sigma_az_px, sigma_rg_px; /* 1-sigma offset uncertainty, pixels */
} rs_coreg_quality_t;

/* Ceiling on a reported sigma, in pixels.
 *
 * The estimator diverges by construction -- at zero curvature or zero coherence
 * the uncertainty is genuinely unbounded -- and an infinity propagates into
 * every average a caller forms over looks, turning one unusable correlation into
 * an unusable window with no way to tell how many were involved. Clamping keeps
 * the aggregate finite and monotone: a window of hopeless correlations reaches
 * the ceiling, a window with one bad look among many is pulled up in proportion.
 * A thousand pixels is far outside anything a patch a few tens of pixels across
 * could mean, so no clamped value can be mistaken for a measurement. */
#define RS_COREG_SIGMA_MAX 1.0e3

/* Baseline, in PIXELS, over which the peak's curvature is differenced.
 *
 * NOT one lattice step, and the reason is numerical. The refinement lattice is
 * 1/upsample pixels, and upsample reaches 200 in documented configurations. A
 * parabola's dip over a half-baseline h is kappa*h^2/2 RELATIVE to the peak, so
 * at h = 1/200 px the three samples differ in their sixth significant figure --
 * against correlation magnitudes accumulated from single-precision patches,
 * which carry about seven. The second difference would then be one or two
 * significant digits of a quantity the reported sigma depends on inversely.
 *
 * A tenth of a pixel puts the dip near 1e-3 of the peak, which is resolved with
 * room to spare, and is still well inside a main lobe about a pixel across so
 * the parabolic model still holds. Fixing it in PIXELS rather than in lattice
 * steps has a second benefit worth as much as the first: the sigma from a run at
 * --upsample 10 and one at --upsample 200 are then the same measurement of the
 * same surface, where a one-step baseline would make them incomparable while
 * looking identical.
 *
 * Rounded to at least one lattice step, so the default 1/10-pixel refinement
 * differences its immediate neighbours as it always did. */
#define RS_COREG_CURV_BASELINE_PX 0.1

/* Radius, in PIXELS, of the zone around the peak excluded from the SNR's
 * background mean.
 *
 * The peak is not one bin wide; it occupies its own main lobe, and including
 * that lobe in the background would divide the peak by a mean it dominates,
 * compressing every surface towards the same uninformative ratio. Two pixels
 * spans the main lobe of a correlation between patches whose spectra fill the
 * band, which is the case here because rs_coreg_extract() removes the mean and
 * nothing else shapes the patch.
 *
 * It is defined in pixels rather than in bins so that the two refinement modes
 * exclude the same PHYSICAL neighbourhood: the exhaustive path's surface is
 * sampled 'upsample' times more finely, so it converts this to that many more
 * bins. coreg.h's contract is that the two modes report comparable numbers, and
 * an exclusion fixed in bins would quietly break it for the SNR. */
#define RS_COREG_SNR_EXCLUDE_PX 2.0

/* The SNR a surface of 'n_az' by 'n_rg' bins reaches on noise alone.
 *
 * The N-th harmonic number, as derived on rs_coreg_quality_t: the expected
 * peak-to-mean power ratio of a correlation surface holding no signal. Computed
 * by summation below 64 bins and by the log-plus-Euler-gamma asymptotic above,
 * where the two agree to better than a part in 10^4.
 *
 * Exposed so that a caller gating on SNR states a threshold in units of "times
 * what noise alone produces" rather than as a bare constant whose provenance is
 * a memory of a default in someone else's program. Returns zero on an empty
 * patch. */
double rs_coreg_snr_null(size_t n_az, size_t n_rg);

/* Estimate the shift between two equally sized complex patches.
 *
 * 'ref' and 'img' are 'n_az' by 'n_rg' row-major patches with their means
 * already removed (rs_coreg_extract() does that). The shift that best aligns
 * 'img' onto 'ref' is written to '*shift_az' and '*shift_rg' in pixels, and the
 * normalised correlation coefficient at that shift to '*peak', in [0, 1].
 *
 * The estimate is refined below one pixel by evaluating the correlation surface
 * on a grid of 1/upsample_az by 1/upsample_rg spacing spanning one pixel either
 * side of the integer peak, following Guizar-Sicairos et al. (2008): only the
 * neighbourhood of the peak is upsampled, never the whole surface. Published
 * working values for this pipeline's data are 10 in azimuth and 20 in range.
 *
 * Patches with no variance yield a zero shift and a zero peak rather than a
 * division by zero, so a blank region of a scene masks itself out.
 *
 * Equivalent to rs_coreg_shift_ex() with RS_COREG_REFINE_LOCAL. */
resonarsat_status_t rs_coreg_shift(const float complex *ref, const float complex *img,
                                   size_t n_az, size_t n_rg,
                                   size_t upsample_az, size_t upsample_rg,
                                   double *shift_az, double *shift_rg, double *peak);

/* As rs_coreg_shift(), with the peak-search extent selectable.
 *
 * 'refine' picks the search strategy; see rs_coreg_refine_t for what the choice
 * does and does not buy. Both modes report the shift on the same 1/upsample
 * lattice and normalise '*peak' identically, so a caller may switch between them
 * and compare the numbers directly.
 *
 * RS_COREG_REFINE_EXHAUSTIVE returns RS_ERR_RANGE if the padded surface would
 * exceed RS_COREG_MAX_SURFACE elements, rather than attempting the allocation --
 * the product of four caller-supplied sizes is easy to make enormous by
 * accident. */
resonarsat_status_t rs_coreg_shift_ex(const float complex *ref, const float complex *img,
                                      size_t n_az, size_t n_rg,
                                      size_t upsample_az, size_t upsample_rg,
                                      rs_coreg_refine_t refine,
                                      double *shift_az, double *shift_rg, double *peak);

/* As rs_coreg_shift_ex(), also reporting what the surface says about itself.
 *
 * '*q' receives the peak value the other two entry points return through
 * '*peak', plus the SNR and the per-axis offset uncertainty described on
 * rs_coreg_quality_t. Both refinement modes fill all four fields, from the same
 * definitions, so a caller may compare them across modes as it already can for
 * the peak.
 *
 * 'q' may be NULL, in which case this is exactly rs_coreg_shift_ex() and none of
 * the extra work is done -- the local path skips an allocation and a second scan
 * of the surface. That matters: this is called once per window per look.
 *
 * Returns what rs_coreg_shift_ex() returns, for the same reasons. */
resonarsat_status_t rs_coreg_shift_q(const float complex *ref, const float complex *img,
                                     size_t n_az, size_t n_rg,
                                     size_t upsample_az, size_t upsample_rg,
                                     rs_coreg_refine_t refine,
                                     double *shift_az, double *shift_rg,
                                     rs_coreg_quality_t *q);

/* Report whether an exhaustive search of this size is permitted.
 *
 * Returns RS_OK if the zero-padded surface for 'n_az' by 'n_rg' patches at
 * 'upsample_az' by 'upsample_rg' fits within RS_COREG_MAX_SURFACE, and
 * RS_ERR_RANGE with a description naming the offending sizes otherwise.
 *
 * Exposed so a caller can refuse an impossible configuration up front instead of
 * discovering it once per window inside a tracking loop. That distinction
 * matters: a tracker treats a failed correlation as "this window did not track",
 * so a size error raised per window returns a complete result in which
 * everything is zero and nothing says why. rs_microm_track() calls this before
 * it allocates. */
resonarsat_status_t rs_coreg_surface_check(size_t n_az, size_t n_rg,
                                          size_t upsample_az, size_t upsample_rg);

/* Ceiling on the zero-padded correlation surface, in complex samples.
 *
 * 2^24 elements is 128 MB at single precision, per call and per thread. Well
 * above anything the pipeline asks for (10 by 20 upsampling on a 24 by 24 patch
 * needs 115200) and low enough that a mistyped upsample factor fails with a
 * message instead of exhausting memory. */
#define RS_COREG_MAX_SURFACE ((size_t)1 << 24)

/* Copy the 'n_az' by 'n_rg' patch whose top-left corner is at (az0, rg0) out of
 * an image into 'patch', subtracting the patch mean as it goes.
 *
 * Mean removal turns the subsequent correlation into a covariance, so that a
 * brightness offset between two sub-looks does not bias the peak. Returns
 * RS_ERR_ARG, with a description naming the offending extent, if any part of
 * the patch would fall outside the image. */
resonarsat_status_t rs_coreg_extract(const rs_slc_t *img, size_t az0, size_t rg0,
                                     size_t n_az, size_t n_rg, float complex *patch);

#endif /* RESONARSAT_COREG_H */
