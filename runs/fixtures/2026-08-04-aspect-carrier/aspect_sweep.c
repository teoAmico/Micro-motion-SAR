/* Item 25's sweep, re-run with the carrier removal of items 52-53.
 * Item 14's phase recovery against the ASPECT fixture, scored by slope and rms. */
#include "resonarsat/microm.h"
#include "resonarsat/subaperture.h"
#include "resonarsat/focus.h"
#include "resonarsat/simulate.h"
#include "rs_sim.h"
#include "rs_test.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

static double run_one(double freq, const rs_sim_aspect_t *asp, unsigned seed, int *ok)
{
    const double amp_phase = (freq > 0.0) ? 0.002442 : 0.0;
    rs_sim_tgt_t tg[400];
    unsigned sd = seed * 2654435761u + 1u;
    for (size_t i = 0; i < 400; i++) {
        const double u1 = rs_sim_u01(&sd), u2 = rs_sim_u01(&sd), u3 = rs_sim_u01(&sd);
        tg[i].x = (u1 - 0.5) * 32.0; tg[i].y = (u2 - 0.5) * 32.0; tg[i].z = 0.0;
        tg[i].rcs = 0.3 * (-log(u3));
        tg[i].vib_freq = freq; tg[i].vib_amp = amp_phase; tg[i].vib_phase = 0.0;
    }
    rs_cphd_t c;
    if (rs_sim_scene_aspect(&c, tg, 400, 12.0, 400.0, 256, 0.5, asp) != RS_OK) { *ok=0; return 0; }
    rs_grid_t g = { .origin={0,0,0}, .n_x=64, .n_y=64, .dx=0.5, .dy=0.5, .height=0.0 };
    rs_subap_params_t sp; rs_subap_params_default(&sp);
    sp.n_looks = 64; sp.overlap = 0.0;
    rs_subap_stack_t st;
    if (rs_subaperture_from_cphd(&c, &g, &sp, &st) != RS_OK) { rs_cphd_free(&c); *ok=0; return 0; }
    rs_microm_params_t mp; rs_microm_params_default(&mp);
    mp.estimator = RS_MICROM_EST_PHASE;
    mp.win_az = mp.win_rg = 16; mp.stride_az = mp.stride_rg = 8; mp.coherence_min = 0.0;
    rs_microm_t m;
    if (rs_microm_track(&st, &mp, &m) != RS_OK) { rs_subap_stack_free(&st); rs_cphd_free(&c); *ok=0; return 0; }
    rs_spectrum_t spec;
    double out = 0.0;
    if (rs_spectrum_compute(&m, RS_SPEC_DISPLACEMENT, &spec) == RS_OK) {
        size_t w = 0; double p = 0.0;
        if (rs_spectrum_best_window(&spec, &w, &p, NULL) == RS_OK) out = spec.dominant_freq[w];
        rs_spectrum_free(&spec);
    }
    rs_microm_free(&m); rs_subap_stack_free(&st); rs_cphd_free(&c);
    return out;
}

int main(void)
{
    const double freqs[5] = { 0.3, 0.5, 0.7, 0.9, 1.1 };
    const double lobes[4] = { 1.00, 0.50, 0.25, 0.12 };
    printf("Item 25's sweep, carrier removal now linear+quadratic+cubic.\n");
    printf("Aspect fixture, rs_spectrum_best_window(), 64 looks, df = %.4f Hz,\n", 1.0/12.0);
    printf("half-bin bound 0.0417 Hz.\n\n");
    printf("%10s | %8s %8s | %-8s | static controls (3 seeds)\n", "lobe_frac", "slope", "rms Hz", "verdict");
    printf("---------------------------------------------------------------------------\n");
    for (int L = 0; L < 4; L++) {
        rs_sim_aspect_t asp = { .lobe_frac = lobes[L], .frac = 0.5,
                                .peak_gain = 16.0, .seed = 7u };
        double inj[5], rep[5]; int ok = 1;
        for (int i = 0; i < 5; i++) { inj[i] = freqs[i]; rep[i] = run_one(freqs[i], &asp, 7u, &ok); }
        double slope = 0, rms = 0;
        rs_track_fit(inj, rep, 5, &slope, &rms);
        /* STATIC CONTROLS, three seeds. Item 25's most damning finding was
         * that two of twelve came back INSIDE the swept band on scenes where
         * nothing moves -- aspect dependence gives amplitude per-look structure
         * the prominence policy reads as a frequency. Swept band is 0.3-1.1. */
        double sc[3]; int in_band = 0;
        for (int s2 = 0; s2 < 3; s2++) {
            sc[s2] = run_one(0.0, &asp, 7u + (unsigned)s2, &ok);
            if (sc[s2] >= 0.3 && sc[s2] <= 1.1) in_band++;
        }
        printf("%10.2f | %8.4f %8.4f | %-8s | %5.2f %5.2f %5.2f  %s\n",
               lobes[L], slope, rms,
               (fabs(slope-1.0) < 0.1 && rms < 0.0417) ? "RECOVERS" : "fails",
               sc[0], sc[1], sc[2], in_band ? "IN BAND" : "outside");
    }
    printf("\nISOTROPIC CONTROL (asp = NULL), same targets and processing:\n");
    double inj[5], rep[5]; int ok = 1;
    for (int i = 0; i < 5; i++) { inj[i] = freqs[i]; rep[i] = run_one(freqs[i], NULL, 7u, &ok); }
    double slope = 0, rms = 0;
    rs_track_fit(inj, rep, 5, &slope, &rms);
    printf("%10s | %8.4f %8.4f | %s\n", "isotropic", slope, rms,
           (fabs(slope-1.0) < 0.1 && rms < 0.0417) ? "RECOVERS" : "fails");
    return 0;
}
