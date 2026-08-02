/* Shared synthetic-scene helper for the test suite.
 *
 * Builds phase history for a straight-line spotlight collect over point
 * targets, some vibrating, so that several test binaries can share one
 * definition of ground truth rather than each rolling its own and drifting. */

#ifndef RS_SIM_H
#define RS_SIM_H

#include "resonarsat/focus.h"
#include "resonarsat/geom.h"

#include <math.h>
#include <stdlib.h>

/* A simulated point target.
 *
 * Aspect dependence is deliberately NOT a field here. Callers build target
 * lists in uninitialised stack arrays and assign field by field, so a new
 * member would be read as garbage by every existing fixture. It lives in
 * rs_sim_aspect_t instead, passed alongside. */
typedef struct {
    double x, y, z, rcs, vib_freq, vib_amp, vib_phase;
} rs_sim_tgt_t;

/* Uniform draw in (0,1] from a stateful LCG, so a scene is reproducible from its
 * seed alone and two seeds are independent realisations of one experiment. */
static inline double rs_sim_u01(unsigned *state)
{
    *state = *state * 1103515245u + 12345u;
    const double u = (double)(*state >> 8) / 16777216.0;
    return (u > 1e-9) ? u : 1e-9;
}

/* ASPECT-DEPENDENT SCATTERING: how a target's brightness varies with the
 * along-track viewing angle over the dwell.
 *
 * WHY THIS EXISTS. Every scatterer in rs_sim_scene() is isotropic -- its
 * amplitude is a constant and only its phase moves -- so a simulated dominant
 * is dominant in EVERY sub-look by construction. Real scenes do not behave that
 * way, and two independent measurements say so. FOLLOW-UPS.md item 12f: sub-look
 * coherence on these fixtures is set by look separation alone and is invariant
 * to scene content, so the --coherence gate cannot be tested by any target list.
 * Item 23: amplitude dispersion on real data rises steeply with window size
 * (median 0.915, 1.865, 2.590 across win 16/32/64) and on these fixtures it does
 * not move at all (0.452 to 0.477), which reads as the brightest real return
 * also being the most angle-selective one. Both point at the propagation model
 * rather than at the target list. This is the model change they ask for.
 *
 * THE MODEL is a uniformly illuminated flat facet, whose two-way backscatter
 * pattern is a sinc in the along-track direction cosine u:
 *
 *     A(u) = sinc( 2 L (u - u0) / lambda ),   sinc(x) = sin(pi x) / (pi x)
 *
 * for a facet of along-track extent L pointed at u0. Its first null sits at
 * |u - u0| = lambda / (2L), so the null-to-null lobe width is lambda / L.
 * Parameterising by that width as a fraction of the aperture the collect
 * actually spans, du_ap = v * t_dwell / r, the facet length drops out:
 *
 *     A(u) = sinc( 2 (u - u0) / (lobe_frac * du_ap) )
 *
 * and lobe_frac is directly the quantity the experiment cares about -- what
 * fraction of the aperture, hence of the sub-looks, a scatterer is bright over.
 * The equivalent facet is L = lambda / (lobe_frac * du_ap), which at the default
 * geometry and a 25-look dwell is a few metres: a building face, not a mountain.
 *
 * A SINC RATHER THAN A GAUSSIAN, deliberately. It is the textbook result rather
 * than a chosen curve, and its nulls and sign changes are the point: a facet does
 * not fade out of the aperture, it switches off and comes back in antiphase.
 * That is what "bright in some sub-looks and gone in others" means physically,
 * and a smooth bump would not reproduce it.
 *
 * Amplitude is signed. A sidelobe is a pi phase flip, which is real.
 *
 * WHAT THIS IS NOT. It is not calibrated against any measured scene -- no
 * collect available to this project has per-scatterer ground truth to calibrate
 * against. It adds a mechanism the model lacked; whether it reproduces the real
 * numbers is the experiment, not the premise. */
typedef struct {
    double lobe_frac;  /* null-to-null lobe width as a fraction of the full
                          aperture. <= 0 leaves every target isotropic, which is
                          what rs_sim_scene() passes. */
    double frac;       /* fraction of targets that are aspect-selective; the
                          rest stay isotropic, since a real scene mixes
                          specular facets with diffuse surfaces. */
    double peak_gain;  /* amplitude multiplier at a selective target's lobe
                          PEAK, over the rcs it was given. <= 0 reads as 1.
                          A facet is not merely a modulated diffuse scatterer:
                          a flat face or dihedral returns far more at its
                          specular angle than clutter does at any angle, and
                          with a gain of 1 the lobe only ever makes a target
                          DIMMER, so the brightest-pixel statistics preferentially
                          avoid exactly the targets this models. Bounded and
                          explicit, unlike the unbounded gain FOLLOW-UPS.md item
                          21 had to retract a conclusion over. */
    unsigned seed;     /* selects which targets are selective and where each
                          lobe points, so a scene is reproducible from it. */
} rs_sim_aspect_t;

/* Generate range-compressed phase history for a target list, with optional
 * aspect-dependent scattering.
 *
 * Geometry: platform along +x at 'v_platform', offset cross-track by
 * 'range_offset' and at height 'height', staring at the origin. The compressed
 * pulse is a Gaussian of width 'range_res' carrying the exact propagation
 * phase, which is all any stage downstream reads.
 *
 * 'asp' NULL, or with lobe_frac <= 0, reproduces rs_sim_scene() exactly --
 * every target isotropic, every existing fixture bit-for-bit unchanged.
 * Otherwise see rs_sim_aspect_t for the model.
 *
 * Returns RS_OK, an allocation failure from rs_cphd_alloc(), or RS_ERR_ALLOC if
 * the per-target lobe table cannot be sized. */
static resonarsat_status_t rs_sim_scene_aspect(rs_cphd_t *cphd,
                                               const rs_sim_tgt_t *tg,
                                               size_t n_tgt,
                                               double t_dwell, double prf,
                                               size_t n_rbin, double dr,
                                               const rs_sim_aspect_t *asp)
{
    const double fc = 9.6e9;
    const double v_platform = 7500.0;
    const double height = 500000.0;
    const double range_offset = 350000.0;
    const double range_res = 1.0;

    const size_t n_pulse = (size_t)(prf * t_dwell);
    resonarsat_status_t st = rs_cphd_alloc(cphd, n_pulse, n_rbin);
    if (st != RS_OK) return st;

    cphd->fc = fc;
    cphd->lambda = RS_C_LIGHT / fc;
    cphd->prf = prf;
    cphd->dr = dr;

    const double r_centre = sqrt(height * height + range_offset * range_offset);
    cphd->r_near = r_centre - 0.5 * (double)n_rbin * dr;

    const double k_phase = 4.0 * M_PI / cphd->lambda;
    const double sigma = range_res / 2.355;

    /* Per-target lobe centres, in along-track direction cosine. NAN marks an
     * isotropic target, which is every target when 'asp' is off. Drawn here
     * rather than per pulse so a target's lobe is a property of the scene. */
    const int use_aspect = (asp != NULL && asp->lobe_frac > 0.0 && n_tgt > 0);
    double *u0 = NULL;
    double lobe_u = 0.0;
    double peak_gain = 1.0;
    if (use_aspect) {
        if (asp->peak_gain > 0.0) peak_gain = asp->peak_gain;
        u0 = (double *)malloc(n_tgt * sizeof *u0);
        if (!u0) {
            rs_cphd_free(cphd);
            return RS_ERR_ALLOC;
        }
        /* The aperture's angular span, as a direction-cosine width. */
        const double du_ap = v_platform * t_dwell / r_centre;
        lobe_u = asp->lobe_frac * du_ap;

        unsigned st_a = asp->seed * 2654435761u + 101u;
        for (size_t g = 0; g < n_tgt; g++) {
            if (rs_sim_u01(&st_a) > asp->frac) {
                u0[g] = NAN;                       /* stays isotropic */
                continue;
            }
            /* Point the lobe at the aspect this target is seen from at one
             * pulse drawn uniformly over the dwell, computed from the same
             * geometry the main loop uses so the two cannot drift apart. */
            const double tq = rs_sim_u01(&st_a) * t_dwell;
            const double xp = v_platform * (tq - 0.5 * t_dwell);
            const double ax = xp - tg[g].x;
            const double ay = range_offset - tg[g].y;
            const double az = height - tg[g].z;
            const double ar = sqrt(ax * ax + ay * ay + az * az);
            u0[g] = ax / ar;
        }
    }

    for (size_t i = 0; i < n_pulse; i++) {
        const double t = (double)i / prf;
        cphd->t[i] = t;
        cphd->pos[3 * i + 0] = v_platform * (t - 0.5 * t_dwell);
        cphd->pos[3 * i + 1] = range_offset;
        cphd->pos[3 * i + 2] = height;

        /* Motion compensation: the receive window follows the scene reference
         * point (the origin), so bin n_rbin/2 always sits on it. Without this
         * a long dwell would walk the target clean out of a narrow swath. */
        cphd->r_ref[i] = sqrt(cphd->pos[3 * i + 0] * cphd->pos[3 * i + 0]
                            + cphd->pos[3 * i + 1] * cphd->pos[3 * i + 1]
                            + cphd->pos[3 * i + 2] * cphd->pos[3 * i + 2]);

        float complex *row = cphd->signal + i * n_rbin;

        for (size_t g = 0; g < n_tgt; g++) {
            double dz = 0.0;
            if (tg[g].vib_freq > 0.0 && tg[g].vib_amp != 0.0) {
                dz = tg[g].vib_amp * sin(2.0 * M_PI * tg[g].vib_freq * t + tg[g].vib_phase);
            }
            const double dx = cphd->pos[3 * i + 0] - tg[g].x;
            const double dy = cphd->pos[3 * i + 1] - tg[g].y;
            const double dzz = cphd->pos[3 * i + 2] - (tg[g].z + dz);
            const double R = sqrt(dx * dx + dy * dy + dzz * dzz);

            const double fbin = (R - cphd->r_ref[i]) / dr + 0.5 * (double)n_rbin;
            if (fbin < 0.0 || fbin >= (double)n_rbin) continue;

            /* Aspect: the facet's sinc pattern in along-track direction cosine,
             * evaluated at this pulse's view of this target. Signed, so a
             * sidelobe enters in antiphase. */
            double rcs = tg[g].rcs;
            if (u0 && !isnan(u0[g])) {
                const double xarg = 2.0 * (dx / R - u0[g]) / lobe_u;
                rcs *= peak_gain * ((fabs(xarg) < 1e-12)
                           ? 1.0
                           : sin(M_PI * xarg) / (M_PI * xarg));
            }

            const long lo = (long)floor(fbin - 4.0 * sigma / dr);
            const long hi = (long)ceil(fbin + 4.0 * sigma / dr);
            const double ph = -k_phase * R;
            const double cr = cos(ph), ci = sin(ph);

            for (long b = lo; b <= hi; b++) {
                if (b < 0 || b >= (long)n_rbin) continue;
                const double d = ((double)b - fbin) * dr;
                const double env = rcs * exp(-0.5 * (d * d) / (sigma * sigma));
                row[b] += (float)(env * cr) + (float)(env * ci) * I;
            }
        }
    }
    free(u0);
    return RS_OK;
}

/* The isotropic scene every fixture predating aspect dependence was measured
 * on. Kept as its own entry point so those measurements stay reproducible by
 * the call they were made with. */
static resonarsat_status_t rs_sim_scene(rs_cphd_t *cphd,
                                        const rs_sim_tgt_t *tg, size_t n_tgt,
                                        double t_dwell, double prf,
                                        size_t n_rbin, double dr)
{
    return rs_sim_scene_aspect(cphd, tg, n_tgt, t_dwell, prf, n_rbin, dr, NULL);
}

/* THE SECOND FIXTURE FAMILY: persistent dominant scatterers on a diffuse
 * background.
 *
 * WHY IT EXISTS. Every result in `FOLLOW-UPS.md` items 12a to 12e rests on one
 * scene: equal-brightness Rayleigh scatterers, which is fully developed speckle
 * and nothing else. That scene cannot reach the coherence the pipeline is built
 * to work at. Measured, at 96 scatterers over 24 m, the tracking coherence tops
 * out at 0.323 even at 95 percent sub-look overlap, against a `coherence_min`
 * default of 0.4 and the 0.85 measured between 95-percent-overlapped looks on
 * the real Giza collect. So the coherence gate has never been exercised by
 * anything, and no sweep of it on that fixture could mean more than a
 * measurement of the fixture's own ceiling.
 *
 * WHY A DOMINANT SCATTERER IS THE RIGHT FIX, and not simply more scatterers.
 * Sub-look decorrelation is the changing interference of comparable scatterers
 * within one resolution cell as the aspect angle sweeps. Adding scatterers of
 * equal brightness does not slow that down -- it is fully developed either way.
 * What slows it down is one scatterer carrying most of the cell's power: the
 * cell's phase is then that scatterer's phase, which is a smooth function of
 * aspect, and the diffuse remainder perturbs it rather than setting it. For a
 * dominant of power A^2 against diffuse power S^2 in the same cell the coherent
 * fraction is
 *
 *     gamma  ~  A^2 / (A^2 + S^2)
 *
 * so gamma = 0.85 wants a dominance of about 5.7, under 8 dB. This is not a
 * modelling convenience: it is what a built structure presents. Corner
 * reflectors, dihedrals formed by a wall against the ground, and building edges
 * all return far more than the surface around them and hold their phase across
 * aspect. A bridge pier is not speckle.
 *
 * WHAT IT DOES NOT MODEL. There is still no sub-resolution scatterer model here
 * -- the diffuse background is a finite set of ideal points, not a continuum --
 * so the diffuse part decorrelates faster than real clutter of the same
 * brightness would. That makes this fixture OPTIMISTIC about the dominant's
 * contribution and pessimistic about the background's, and the two do not
 * cancel. It reaches the coherence regime; it is not a physical model of one.
 *
 * THE LATTICE SPACING MUST EXCEED THE SUB-LOOK RESOLUTION CELL, and getting that
 * wrong is what made this fixture's first use useless. 'extent_m'/'n_side' is the
 * spacing; the sub-look azimuth resolution is what a resolution cell spans. At
 * 8x8 over 24 m the spacing is 3 m against an 8.26 m sub-look cell, so every cell
 * held 2.75 dominants OF EQUAL BRIGHTNESS -- and a cell containing three equal
 * scatterers has no dominant one, whatever the 'dominance' parameter says about
 * the diffuse background. The parameter controls dominant-against-diffuse; it
 * says nothing about dominant-against-dominant, and only the spacing does.
 *
 * Measured, phase estimator, 128 diffuse, seed 7, sub-look resolution 8.26 m:
 *
 *   n_side  spacing  dominants/cell   recovered?
 *      2     12.0 m       0.69        yes, all three selection policies
 *      3      8.0 m       1.03        yes, all three
 *      4      6.0 m       1.38        yes, all three
 *      8      3.0 m       2.75        NO -- every policy, every seed
 *
 * Choose 'n_side' from the sub-look resolution the stack will have, not from how
 * many scatterers seem tidy.
 *
 * LAYOUT. 'n_side' by 'n_side' dominant scatterers on a lattice spanning
 * 'extent_m', each jittered within its own cell so the set does not become a
 * periodic array with grating lobes of its own. 'n_diffuse' Rayleigh scatterers
 * are spread over the same patch with their TOTAL power set so that the share
 * falling in one lattice cell is 1/'dominance' of a single dominant's power.
 * Every scatterer, dominant and diffuse alike, carries the same vibration, so
 * the patch translates as a whole -- see rs_microm_estimator_t on why a lone
 * mover inside a static background measures something else entirely.
 *
 * Writes at most 'cap' targets and returns how many it wrote, so a caller sizing
 * an array wrongly loses scatterers rather than the stack beyond them. */
static inline size_t rs_sim_dominant_patch(rs_sim_tgt_t *tg, size_t cap,
                                    size_t n_side, size_t n_diffuse,
                                    double extent_m, double dominance,
                                    unsigned seed, double freq, double amp)
{
    if (!tg || cap == 0 || n_side == 0) return 0;

    unsigned st = seed * 2654435761u + 1u;
    const double cell = extent_m / (double)n_side;
    const size_t n_cell = n_side * n_side;
    size_t n = 0;

    /* Dominants first, so that a caller whose 'cap' is too small keeps the part
     * of the scene that defines it rather than a random half of the background. */
    for (size_t iy = 0; iy < n_side && n < cap; iy++) {
        for (size_t ix = 0; ix < n_side && n < cap; ix++) {
            const double jx = (rs_sim_u01(&st) - 0.5) * cell;
            const double jy = (rs_sim_u01(&st) - 0.5) * cell;
            tg[n].x = -0.5 * extent_m + ((double)ix + 0.5) * cell + jx;
            tg[n].y = -0.5 * extent_m + ((double)iy + 0.5) * cell + jy;
            tg[n].z = 0.0;
            tg[n].rcs = 1.0;                  /* the unit the dominance is in */
            tg[n].vib_freq = freq;
            tg[n].vib_amp = amp;
            tg[n].vib_phase = 0.0;
            n++;
        }
    }

    /* Mean diffuse POWER per scatterer, from the per-cell budget. One dominant
     * has power 1, one cell should hold 1/dominance of diffuse power, and the
     * n_diffuse scatterers are spread over n_cell cells. */
    double p_mean = 0.0;
    if (n_diffuse > 0 && dominance > 0.0) {
        p_mean = (double)n_cell / ((double)n_diffuse * dominance);
    }

    for (size_t i = 0; i < n_diffuse && n < cap; i++) {
        tg[n].x = (rs_sim_u01(&st) - 0.5) * extent_m;
        tg[n].y = (rs_sim_u01(&st) - 0.5) * extent_m;
        tg[n].z = 0.0;
        /* Exponential in power is Rayleigh in amplitude, which is what fully
         * developed speckle has; the mean of -ln(u) is one, so scaling by
         * p_mean gives that mean power exactly. */
        tg[n].rcs = sqrt(p_mean * -log(rs_sim_u01(&st)));
        tg[n].vib_freq = freq;
        tg[n].vib_amp = amp;
        tg[n].vib_phase = 0.0;
        n++;
    }
    return n;
}

#endif /* RS_SIM_H */
