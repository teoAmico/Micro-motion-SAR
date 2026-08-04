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

/* Returns the reported frequency; also hands back the locally-normalised peak's
 * frequency and ratio, so the two policies can be scored on identical data. */
static double g_lp_freq, g_lp_ratio, g_ps_freq, g_am; static int g_ps_ok;

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
    g_lp_freq = 0.0; g_lp_ratio = 0.0;
    if (rs_spectrum_compute(&m, RS_SPEC_DISPLACEMENT, &spec) == RS_OK) {
        size_t w = 0; double p = 0.0;
        if (rs_spectrum_best_window(&spec, &w, &p, NULL) == RS_OK) out = spec.dominant_freq[w];
        rs_local_peak_t lp;
        if (rs_spectrum_local_window(&spec, &lp) == RS_OK) {
            g_lp_freq = lp.freq_hz; g_lp_ratio = lp.ratio;
        }
        rs_spectrum_ps_t ps;
        g_ps_ok = (rs_spectrum_ps_window(&spec, &ps) == RS_OK);
        g_ps_freq = g_ps_ok ? ps.freq_hz : 0.0;
        /* The AM check on the window and frequency actually reported. */
        g_am = 0.0;
        { rs_am_check_t ac;
          if (rs_spectrum_am_check(&m, w, spec.dominant_freq[w], &ac) == RS_OK)
              g_am = ac.am_ratio; }
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
    printf("%10s | %-25s | %-25s |\n", "", "PROMINENCE (best_window)", "LOCAL PEAK (item 47)");
    printf("%10s | %7s %7s %-8s | %7s %7s %-8s |\n", "lobe_frac", "slope","rms","verdict","slope","rms","verdict");
    printf("--------------------------------------------------------------------------------\n");
    for (int L = 0; L < 4; L++) {
        rs_sim_aspect_t asp = { .lobe_frac = lobes[L], .frac = 0.5,
                                .peak_gain = 16.0, .seed = 7u };
        double inj[5], rep[5], lrep[5]; int ok = 1;
        double lrmin = 1e300, lrmax = 0.0;
        double pinj[5], prep[5]; int np = 0;
        double amin_inj = 1e300, amax_inj = 0.0, amax_sta = 0.0, amin_sta = 1e300;
        for (int i = 0; i < 5; i++) {
            inj[i] = freqs[i]; rep[i] = run_one(freqs[i], &asp, 7u, &ok);
            lrep[i] = g_lp_freq;
            if (g_am > amax_inj) amax_inj = g_am;
            if (g_am < amin_inj) amin_inj = g_am;
            if (g_ps_ok) { prep[np] = g_ps_freq; pinj[np] = freqs[i]; np++; }
            if (g_lp_ratio < lrmin) lrmin = g_lp_ratio;
            if (g_lp_ratio > lrmax) lrmax = g_lp_ratio;
        }
        double slope = 0, rms = 0;
        rs_track_fit(inj, rep, 5, &slope, &rms);
        /* STATIC CONTROLS, three seeds. Item 25's most damning finding was
         * that two of twelve came back INSIDE the swept band on scenes where
         * nothing moves -- aspect dependence gives amplitude per-look structure
         * the prominence policy reads as a frequency. Swept band is 0.3-1.1. */
        double lslope=0, lrms=0;
        rs_track_fit(inj, lrep, 5, &lslope, &lrms);
        double sc[3], lsc[3], lscr[3], lir_min = 1e300; int in_band = 0, l_in_band = 0;
        int ps_ans = 0, ps_in_band = 0, rejected = 0;
        for (int s2 = 0; s2 < 3; s2++) {
            sc[s2] = run_one(0.0, &asp, 7u + (unsigned)s2, &ok);
            lsc[s2] = g_lp_freq; lscr[s2] = g_lp_ratio;
            if (lscr[s2] < lir_min) lir_min = lscr[s2];
            if (sc[s2] >= 0.3 && sc[s2] <= 1.1) in_band++;
            if (lsc[s2] >= 0.3 && lsc[s2] <= 1.1) l_in_band++;
            if (g_ps_ok) { ps_ans++; if (g_ps_freq >= 0.3 && g_ps_freq <= 1.1) ps_in_band++; }
            if (g_am > amax_sta) amax_sta = g_am;
            if (g_am < amin_sta) amin_sta = g_am;
            if (sc[s2] >= 0.3 && sc[s2] <= 1.1 && g_am > 8.0) rejected++;
        }
        printf("%10.2f | %7.4f %7.4f %-8s | %7.4f %7.4f %-8s |\n",
               lobes[L], slope, rms,
               (fabs(slope-1.0) < 0.1 && rms < 0.0417) ? "RECOVERS" : "fails",
               lslope, lrms,
               (fabs(lslope-1.0) < 0.1 && lrms < 0.0417) ? "RECOVERS" : "fails");
        double pslope=0, prms=0;
        if (np >= 3) rs_track_fit(pinj, prep, (size_t)np, &pslope, &prms);
        printf("%10s | static in band %d/3  | AM ratio: injected %6.1f-%-7.1f  static %6.1f-%-7.1f | "
               "in-band statics rejected %d/%d\n",
               "", in_band, amin_inj, amax_inj, amin_sta, amax_sta, rejected, in_band);
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
