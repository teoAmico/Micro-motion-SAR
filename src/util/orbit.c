/* Image allocation, metadata derivation, validation and cropping. */

#include "resonarsat/slc.h"
#include "resonarsat/geom.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

/* Allocate and zero an image's sample buffer. */
resonarsat_status_t rs_slc_alloc(rs_slc_t *img, size_t n_az, size_t n_rg)
{
    if (!img) return RS_ERR_ARG;
    if (n_az == 0 || n_rg == 0) {
        rs_set_error("slc: dimensions must be positive (got %zux%zu)", n_az, n_rg);
        return RS_ERR_ARG;
    }
    /* Guard the multiplication before it wraps: a corrupt header claiming
     * absurd dimensions must be rejected, not turned into a short buffer. */
    if (n_az > SIZE_MAX / n_rg || n_az * n_rg > SIZE_MAX / sizeof(float complex)) {
        rs_set_error("slc: dimensions %zux%zu overflow addressable memory", n_az, n_rg);
        return RS_ERR_RANGE;
    }

    float complex *buf = calloc(n_az * n_rg, sizeof *buf);
    if (!buf) {
        rs_set_error("slc: cannot allocate %zux%zu complex samples", n_az, n_rg);
        return RS_ERR_ALLOC;
    }

    img->data = buf;
    img->n_az = n_az;
    img->n_rg = n_rg;
    return RS_OK;
}

/* Release the sample buffer and leave the struct reusable. */
void rs_slc_free(rs_slc_t *img)
{
    if (!img) return;
    free(img->data);
    img->data = NULL;
    img->n_az = img->n_rg = 0;
}

/* Derive the fields that must not be parsed independently of their sources. */
resonarsat_status_t rs_slc_finalise_metadata(rs_slc_t *img)
{
    if (!img) return RS_ERR_ARG;

    if (img->azimuth_time_interval <= 0.0) {
        rs_set_error("slc: azimuth_time_interval unset; "
                     "it must come from the product's line timing, not its PRF");
        return RS_ERR_MISSING_META;
    }
    if (img->fc <= 0.0) {
        rs_set_error("slc: carrier frequency unset");
        return RS_ERR_MISSING_META;
    }

    img->fs_az  = 1.0 / img->azimuth_time_interval;
    img->lambda = RS_C_LIGHT / img->fc;
    return RS_OK;
}

/* Bounds-check metadata and cross-check the azimuth sampling rate.
 *
 * The cross-check is the substantive one. Azimuth spacing, platform speed and
 * line rate are related by spacing = v_ground / fs_az, so a stored fs_az that
 * disagrees with the other two by a large factor is evidence that a transmit
 * PRF was stored where a line rate belongs. The tolerance is deliberately loose
 * (a factor of 1.5) because ground speed is not platform speed and the ratio
 * varies with geometry; the error being guarded against is a factor of three. */
resonarsat_status_t rs_slc_validate(const rs_slc_t *img)
{
    if (!img) return RS_ERR_ARG;

    if (!(img->fs_az >= 1.0 && img->fs_az <= 100000.0)) {
        rs_set_error("slc: azimuth sampling rate %g Hz outside [1, 1e5]", img->fs_az);
        return RS_ERR_RANGE;
    }
    if (!(img->lambda >= 0.001 && img->lambda <= 1.0)) {
        rs_set_error("slc: wavelength %g m outside [1 mm, 1 m]", img->lambda);
        return RS_ERR_RANGE;
    }
    if (img->az_spacing_m != 0.0 &&
        !(fabs(img->az_spacing_m) >= 0.01 && fabs(img->az_spacing_m) <= 1000.0)) {
        rs_set_error("slc: azimuth spacing %g m outside [0.01, 1000]", img->az_spacing_m);
        return RS_ERR_RANGE;
    }
    if (img->rg_spacing_m != 0.0 &&
        !(fabs(img->rg_spacing_m) >= 0.01 && fabs(img->rg_spacing_m) <= 1000.0)) {
        rs_set_error("slc: range spacing %g m outside [0.01, 1000]", img->rg_spacing_m);
        return RS_ERR_RANGE;
    }
    if (img->incidence != 0.0 && !(img->incidence > 0.0 && img->incidence < M_PI / 2.0)) {
        rs_set_error("slc: incidence angle %g rad outside (0, pi/2)", img->incidence);
        return RS_ERR_RANGE;
    }

    if (img->v_platform > 0.0 && img->az_spacing_m > 0.0 && img->fs_az > 0.0) {
        const double implied_fs = img->v_platform / img->az_spacing_m;
        const double ratio = implied_fs / img->fs_az;
        if (ratio > 1.5 || ratio < 1.0 / 1.5) {
            rs_set_error("slc: azimuth sampling rate %g Hz disagrees with "
                         "v_platform/az_spacing = %g Hz (ratio %.2f); a transmit PRF "
                         "may have been stored where a line rate belongs",
                         img->fs_az, implied_fs, ratio);
            return RS_ERR_RANGE;
        }
    }

    return RS_OK;
}

/* Copy a rectangular region of one image into another the caller must free.
 *
 * Bounds are clamped to the source rather than rejected, so a patch requested
 * near an edge yields the available part instead of an error -- callers are
 * cutting a patch around a structure and an edge is not a mistake.
 *
 * THE GEOMETRY UPDATE IS THE WHOLE REASON THIS IS IN THE LIBRARY. It lived as a
 * static helper in tools/crop_slc.c and moved 'r_scene_m' by rg0 * rg_spacing_m,
 * which is right for a first-sample range and wrong for the scene-centre range
 * that field actually holds -- on a stripmap swath, wrong by kilometres. See
 * FOLLOW-UPS.md item 5. A geometry transformation nothing can test is how that
 * survived, so it is here and tests/test_readers.c pins it.
 *
 * What moves and what does not:
 *
 *   r_scene_m  by the change in CENTRE bin, (rg0 + n_rg/2) - (src->n_rg/2)
 *              range spacings -- NOT by rg0, and the two agree only when the
 *              crop happens to be centred on the source
 *   t0         by az0 azimuth line intervals, which IS a first-line quantity
 *              and so does offset from zero
 *   t_dwell    recomputed from the retained line count
 *
 * Everything else is copied unchanged: spacings, carrier, platform speed and
 * the ground plane all describe the collect rather than the window onto it.
 * 'plane' in particular stays valid, because it is defined against the scene
 * reference point and cropping does not move that point -- only which samples
 * are kept. Returns RS_ERR_ARG if the origin lies outside the source. */
resonarsat_status_t rs_slc_crop(const rs_slc_t *src, size_t az0, size_t rg0,
                                size_t n_az, size_t n_rg, rs_slc_t *dst)
{
    if (!src || !dst || !src->data) return RS_ERR_ARG;
    if (az0 >= src->n_az || rg0 >= src->n_rg) {
        rs_set_error("slc: crop origin (%zu,%zu) is outside image %zux%zu",
                     az0, rg0, src->n_az, src->n_rg);
        return RS_ERR_ARG;
    }
    if (n_az == 0 || n_rg == 0) {
        rs_set_error("slc: crop size %zux%zu is empty", n_az, n_rg);
        return RS_ERR_ARG;
    }
    if (az0 + n_az > src->n_az) n_az = src->n_az - az0;
    if (rg0 + n_rg > src->n_rg) n_rg = src->n_rg - rg0;

    resonarsat_status_t st = rs_slc_alloc(dst, n_az, n_rg);
    if (st != RS_OK) return st;

    for (size_t a = 0; a < n_az; a++) {
        memcpy(dst->data + a * n_rg,
               src->data + (az0 + a) * src->n_rg + rg0,
               n_rg * sizeof *dst->data);
    }

    /* Carry the metadata across, then adjust only what the window changes. The
     * buffer and dimensions are saved and restored around the struct copy so
     * that adding a field to rs_slc_t carries it here by default -- the failure
     * mode of listing fields explicitly is that a new one is silently zero. */
    float complex *keep = dst->data;
    *dst = *src;
    dst->data = keep;
    dst->n_az = n_az;
    dst->n_rg = n_rg;

    const double centre_shift_bins = ((double)rg0 + 0.5 * (double)n_rg)
                                   - 0.5 * (double)src->n_rg;
    dst->r_scene_m = (src->r_scene_m > 0.0)
                   ? src->r_scene_m + centre_shift_bins * src->rg_spacing_m
                   : 0.0;
    dst->t0 = src->t0 + (double)az0 * src->azimuth_time_interval;
    dst->t_dwell = (double)n_az * src->azimuth_time_interval;
    return RS_OK;
}
