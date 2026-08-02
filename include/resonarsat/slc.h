/* The complex image type every stage of the pipeline flows through, plus the
 * orbit and Doppler metadata that travels with it. */

#ifndef RESONARSAT_SLC_H
#define RESONARSAT_SLC_H

#include <complex.h>
#include <stddef.h>
#include <stdint.h>

#include "resonarsat/geocode.h"
#include "resonarsat/resonarsat.h"

#define RS_DOPP_POLY_MAX 6   /* highest polynomial order we retain */
#define RS_ORBIT_MAX     64  /* state vectors kept per product */

/* A Doppler centroid or FM-rate polynomial in slant-range time.
 *
 * Evaluated as sum(coeff[i] * (tau - tau_ref)^i) for i in [0, n_coeff), with
 * 'tau' the two-way slant-range time in seconds. Products that carry only a
 * constant Doppler centroid set n_coeff = 1. */
typedef struct {
    double coeff[RS_DOPP_POLY_MAX];
    size_t n_coeff;
    double tau_ref;   /* reference slant-range time, s */
} rs_dopp_poly_t;

/* One orbit state vector: time in seconds since the product epoch, position
 * and velocity in metres and metres per second in an earth-fixed frame. */
typedef struct {
    double t;
    double pos[3];
    double vel[3];
} rs_state_vector_t;

/* Platform trajectory as a short table of state vectors, interpolatable. */
typedef struct {
    rs_state_vector_t sv[RS_ORBIT_MAX];
    size_t            n;
} rs_orbit_t;

/* A focused single-look complex image and everything needed to process it.
 *
 * Azimuth sampling deserves the comment it gets below. For Sentinel-1 IW the
 * transmit PRF (about 1451.6 Hz) is shared across the three IW sub-swaths and
 * is roughly three times the per-swath azimuth line rate (about 486 Hz for
 * IW2). Storing "the PRF" as the azimuth sampling rate scales every Doppler
 * axis in the pipeline by that factor, and the error hides itself: a Doppler
 * bandwidth measured against an axis built from the same wrong constant still
 * lands on its expected value. So the line period is the stored quantity, the
 * sampling rate is derived from it, and the transmit PRF is carried separately
 * where no signal-processing code will reach for it by accident. */
typedef struct {
    float complex *data;      /* row-major [azimuth][range], owned */
    size_t n_az, n_rg;

    double azimuth_time_interval;  /* s per azimuth line */
    double fs_az;                  /* Hz, == 1/azimuth_time_interval (derived) */
    double pulse_prf;              /* Hz, transmit PRF -- diagnostics only */

    double fc;                     /* radar centre frequency, Hz */
    double lambda;                 /* radar wavelength, m (derived from fc) */
    double rg_spacing_m;           /* slant-range sample spacing, m */
    double az_spacing_m;           /* azimuth sample spacing, m */

    /* SLANT RANGE TO THE SCENE REFERENCE POINT AT MID-DWELL, m. Zero when the
     * product does not support it; every consumer treats non-positive as absent.
     *
     * THIS FIELD WAS CALLED 'r0' AND DOCUMENTED AS THE FIRST RANGE SAMPLE'S
     * RANGE, WHICH NO PRODUCER EVER WROTE. The rename is the fix for
     * FOLLOW-UPS.md item 5 and is deliberate: renaming rather than
     * re-documenting is what stops a caller keeping its old assumption silently,
     * because a stale `img->r0` now fails to compile instead of reading a
     * quantity that moved underneath it.
     *
     * The four producers, all of which already agreed with each other and not
     * with the old comment:
     *
     *   focus.c        r_ref[p_mid] -- the CPHD's own reference range at the
     *                  middle of the dwell, i.e. to the SRP
     *   subaperture.c  the same, per sub-look
     *   sicd.c         SCPCOA/SlantRange, the range to the scene centre point
     *   uavsar.c       (altitude - terrain) / cos(look) at the scene's average
     *                  look angle
     *
     * WHY SCENE CENTRE AND NOT FIRST SAMPLE. It is what the products actually
     * guarantee -- SICD states the SCP range as a calibrated quantity, and a
     * CPHD carries r_ref per pulse -- and it is what every consumer wants:
     * rs_azimuth_resolution() and the shift-to-velocity conversion in
     * rs_microm_track() both need the range to the patch being measured, and
     * the incidence derivation acos(|pz|/R) is only meaningful about the point
     * the geometry is referenced to. Deriving a first-sample range and then
     * having four call sites add half a swath back would be arithmetic in
     * service of a convention nothing needed.
     *
     * WHAT THIS COSTS. On a wide swath the scene centre and the first sample
     * differ by half a swath -- tens of metres on a spotlight product,
     * kilometres on a stripmap one -- so code that indexes range bins against
     * this value must offset from the CENTRE bin, not from bin zero. That is one
     * caller today: rs_crop() in tools/crop_slc.c, which moves the value by the
     * change in centre bin. It previously added rg0 * rg_spacing_m, correct for
     * the documented meaning and wrong for the real one. */
    double r_scene_m;
    double t0;                     /* azimuth time of first line, s */
    double t_dwell;                /* target illumination time, s */
    double incidence;              /* incidence angle at scene centre, rad */
    double v_platform;             /* platform speed, m/s */

    rs_dopp_poly_t doppler;
    rs_orbit_t     orbit;

    /* Where this image sits on the earth, when the product says. Zeroed and
     * marked invalid for products that carry no plane definition. */
    rs_geo_plane_t plane;

    char source[64];               /* e.g. "UAVSAR", "SICD", for provenance */
} rs_slc_t;

/* Allocate an image of 'n_az' by 'n_rg' complex samples and zero it.
 *
 * Only the sample buffer and the dimensions are set; every metadata field is
 * left zeroed for the caller or reader to populate. Returns RS_ERR_ALLOC if the
 * buffer cannot be obtained, and RS_ERR_ARG if either dimension is zero or the
 * product would overflow. On failure 'img' is left untouched. */
resonarsat_status_t rs_slc_alloc(rs_slc_t *img, size_t n_az, size_t n_rg);

/* Release an image's sample buffer and zero its dimensions. Accepts an image
 * whose data pointer is already NULL, so it is safe on any error path, and
 * leaves the struct reusable for a subsequent rs_slc_alloc(). */
void rs_slc_free(rs_slc_t *img);

/* Set the derived fields that must never be parsed independently of the values
 * they come from: 'fs_az' from 'azimuth_time_interval', and 'lambda' from 'fc'.
 *
 * Readers call this once after populating the primary metadata. Keeping the
 * derivation in one place is what stops a reader from filling 'fs_az' straight
 * out of a product's "PRF" field, which is the specific mistake described in
 * the struct comment above.
 *
 * Returns RS_ERR_MISSING_META if either source field is still zero, since an
 * image with no azimuth timing or no carrier frequency cannot be processed and
 * failing here is far cheaper than discovering it three stages downstream. */
resonarsat_status_t rs_slc_finalise_metadata(rs_slc_t *img);

/* Copy a rectangular region of 'src' into 'dst', which the caller must free.
 *
 * Bounds are clamped to the source rather than rejected. The metadata is carried
 * across with only what the window changes adjusted -- and 'r_scene_m' moves by
 * the change in CENTRE bin, not by rg0, because it is a scene-centre range. See
 * the implementation for the full list of what moves and what does not, and the
 * field's contract above for why that distinction is not cosmetic.
 *
 * Returns RS_ERR_ARG for an origin outside the source or an empty size. */
resonarsat_status_t rs_slc_crop(const rs_slc_t *src, size_t az0, size_t rg0,
                                size_t n_az, size_t n_rg, rs_slc_t *dst);

/* Check that an image's metadata is self-consistent and physically plausible,
 * writing a description of the first problem found via rs_set_error().
 *
 * Applies bounds a real spaceborne or airborne SAR product must satisfy: the
 * azimuth sampling rate lies in [1, 100000] Hz, the wavelength in [0.001, 1] m,
 * sample spacings in [0.01, 1000] m, and the incidence angle in (0, pi/2). It
 * additionally cross-checks the azimuth sampling rate against platform speed
 * and azimuth spacing when both are known, since those three cannot vary
 * independently, and a threefold disagreement there is the signature of the
 * PRF-for-line-rate substitution.
 *
 * This is advisory: it is called by the CLI's info path and by readers after
 * parsing, and it catches transcription errors rather than proving a product
 * good. Returns RS_OK when every check passes. */
resonarsat_status_t rs_slc_validate(const rs_slc_t *img);

/* THE 'doppler' AND 'orbit' FIELDS ABOVE ARE CARRIED, NOT USED.
 *
 * No reader populates either: neither the CPHD, SICD nor UAVSAR path writes a
 * Doppler coefficient or a state vector, so both structures are zero on every
 * product this software has ever read. rs_subaperture_split() copies them into
 * each sub-look's metadata, which propagates zero to zero.
 *
 * A Lagrange orbit interpolator and a Horner Doppler evaluator lived here to
 * consume them. Both were deleted in the 2026-08-02 review (docs/CODE-REVIEW.md
 * finding 5): they were the only readers of the only fields nothing writes, so
 * the whole chain was unreachable and untested, and this project has twice found
 * that a documented, unexercised function does not do what its comment claims
 * (FOLLOW-UPS.md items 27 and 28). Restore them from git when a reader actually
 * fills the fields, and give them a fixture at the same time.
 *
 * The fields stay because the parse that would fill them is a real gap rather
 * than a rejected idea, and because rs_slc_t is what a future reader writes into.
 * Read them as reserved, not as available metadata. */

/* Return a pointer to the first sample of azimuth line 'az'. No bounds check;
 * callers iterate over known dimensions. */
static inline float complex *rs_slc_row(rs_slc_t *img, size_t az)
{
    return img->data + az * img->n_rg;
}

#endif /* RESONARSAT_SLC_H */
