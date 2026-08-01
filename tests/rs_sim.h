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

/* A simulated point target. */
typedef struct {
    double x, y, z, rcs, vib_freq, vib_amp, vib_phase;
} rs_sim_tgt_t;

/* Generate range-compressed phase history for a target list.
 *
 * Geometry: platform along +x at 'v_platform', offset cross-track by
 * 'range_offset' and at height 'height', staring at the origin. The compressed
 * pulse is a Gaussian of width 'range_res' carrying the exact propagation
 * phase, which is all any stage downstream reads.
 *
 * Returns RS_OK, or an allocation failure from rs_cphd_alloc(). */
static resonarsat_status_t rs_sim_scene(rs_cphd_t *cphd,
                                        const rs_sim_tgt_t *tg, size_t n_tgt,
                                        double t_dwell, double prf,
                                        size_t n_rbin, double dr)
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

            const long lo = (long)floor(fbin - 4.0 * sigma / dr);
            const long hi = (long)ceil(fbin + 4.0 * sigma / dr);
            const double ph = -k_phase * R;
            const double cr = cos(ph), ci = sin(ph);

            for (long b = lo; b <= hi; b++) {
                if (b < 0 || b >= (long)n_rbin) continue;
                const double d = ((double)b - fbin) * dr;
                const double env = tg[g].rcs * exp(-0.5 * (d * d) / (sigma * sigma));
                row[b] += (float)(env * cr) + (float)(env * ci) * I;
            }
        }
    }
    return RS_OK;
}

/* Uniform draw in (0,1] from a stateful LCG, so a scene is reproducible from its
 * seed alone and two seeds are independent realisations of one experiment. */
static double rs_sim_u01(unsigned *state)
{
    *state = *state * 1103515245u + 12345u;
    const double u = (double)(*state >> 8) / 16777216.0;
    return (u > 1e-9) ? u : 1e-9;
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
static size_t rs_sim_dominant_patch(rs_sim_tgt_t *tg, size_t cap,
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
