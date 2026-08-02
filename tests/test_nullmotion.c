/* Null test for the micro-motion stage.
 *
 * Phase 5 has a null test -- run the tomography over featureless ground and
 * confirm no structure is reported. Phase 3 had none, and this is it: run the
 * tracking over a scene where nothing moves, and measure how strong a "detection"
 * it reports anyway.
 *
 * That number is the false-positive floor. Any genuine detection must clear it
 * to mean anything, and the comparison is the honest measure of whether the
 * micro-motion stage discriminates at all. Both scenes below use identical
 * geometry, identical processing parameters and identical scatterer brightness,
 * so the only difference is whether one target moves.
 *
 * The test asserts that the floor is measured and finite, and PRINTS the
 * comparison. It deliberately does not assert that detections clear the floor,
 * because at present they do not -- see STATUS.md. Asserting it would either
 * fail permanently or, if tuned to pass, encode the shortfall as acceptable. */

#include "resonarsat/focus.h"
#include "resonarsat/microm.h"
#include "resonarsat/simulate.h"
#include "resonarsat/subaperture.h"
#include "rs_sim.h"
#include "rs_test.h"

#include <math.h>
#include <stdlib.h>

/* Processing applied identically to every scene here, so that any difference in
 * the result is attributable to the scene rather than to the parameters. */
static resonarsat_status_t analyse(const rs_sim_tgt_t *tg, size_t n_tgt,
                                   double *max_prom_out,
                                   double *at_freq_prom_out, double want_freq,
                                   size_t *n_above_out, double above_thresh,
                                   size_t *n_win_out,
                                   double *peak_freq_out, double *df_out)
{
    rs_cphd_t c;
    resonarsat_status_t st = rs_sim_scene(&c, tg, n_tgt, 20.0, 400.0, 256, 0.5);
    if (st != RS_OK) return st;

    rs_grid_t g = { .origin = {0,0,0}, .n_x = 64, .n_y = 64,
                    .dx = 0.5, .dy = 0.5, .height = 0.0 };

    /* The operating point that works, and the reason it does.
     *
     * The sub-look count is set by the phase-ambiguity condition (see
     * rs_microm_recommend_looks): the peak azimuth shift must fall inside three
     * quarters of a sub-look resolution cell, and since the shift is fixed by
     * the geometry while the resolution cell widens as sub-looks shorten, that
     * demands MANY looks rather than few. Overlap works against the condition,
     * so it is zero here. At 32 looks with 0.4 overlap -- the settings this test
     * used previously -- no genuine detection cleared the false-positive floor. */
    rs_subap_params_t sp;
    rs_subap_params_default(&sp);
    sp.n_looks = 128;
    sp.overlap = 0.0;

    rs_subap_stack_t s;
    if ((st = rs_subaperture_from_cphd(&c, &g, &sp, &s)) != RS_OK) {
        rs_cphd_free(&c); return st;
    }

    rs_microm_params_t mp;
    rs_microm_params_default(&mp);
    mp.win_az = mp.win_rg = 32;
    mp.stride_az = mp.stride_rg = 16;
    mp.coherence_min = 0.0;

    rs_microm_t m;
    if ((st = rs_microm_track(&s, &mp, &m)) != RS_OK) {
        rs_subap_stack_free(&s); rs_cphd_free(&c); return st;
    }

    rs_spectrum_t spec;
    if ((st = rs_spectrum_compute(&m, RS_SPEC_VELOCITY, &spec)) != RS_OK) {
        rs_microm_free(&m); rs_subap_stack_free(&s); rs_cphd_free(&c); return st;
    }

    double max_prom = 0.0, at_freq = 0.0, peak_freq = 0.0;
    size_t n_above = 0;
    for (size_t w = 0; w < spec.n_win; w++) {
        if (spec.prominence[w] > max_prom) {
            max_prom = spec.prominence[w];
            /* The frequency the TOOL would report: the one belonging to the
             * most prominent window, which is what rs_spectrum_best_window()
             * selects and what mmotion prints. */
            peak_freq = spec.dominant_freq[w];
        }
        if (spec.prominence[w] > above_thresh) n_above++;
        if (want_freq > 0.0 &&
            fabs(spec.dominant_freq[w] - want_freq) < 2.0 * spec.df &&
            spec.prominence[w] > at_freq) {
            at_freq = spec.prominence[w];
        }
    }

    if (max_prom_out)     *max_prom_out = max_prom;
    if (at_freq_prom_out) *at_freq_prom_out = at_freq;
    if (n_above_out)      *n_above_out = n_above;
    if (n_win_out)        *n_win_out = spec.n_win;
    if (peak_freq_out)    *peak_freq_out = peak_freq;
    if (df_out)           *df_out = spec.df;

    rs_spectrum_free(&spec);
    rs_microm_free(&m);
    rs_subap_stack_free(&s);
    rs_cphd_free(&c);
    return RS_OK;
}

int main(void)
{
    /* ------------------------------------------------------------------
     * The floor: a scene containing bright scatterers, none of them moving.
     * Every peak reported here is an artefact.
     * ------------------------------------------------------------------ */
    RS_CASE("false-positive floor on a scene where nothing moves");
    double floor_prom = 0.0;
    size_t n_above = 0, n_win = 0;
    {
        const rs_sim_tgt_t stat[] = {
            { .x =   0.0, .y = 0.0, .z = 0.0, .rcs = 1.0 },
            { .x = -30.0, .y = 0.0, .z = 0.0, .rcs = 0.8 },
        };
        RS_CHECK_OK(analyse(stat, 2, &floor_prom, NULL, 0.0, &n_above, 8.0, &n_win,
                            NULL, NULL));

        printf("    highest prominence anywhere: %.1f\n", floor_prom);
        printf("    %zu of %zu windows exceed prominence 8\n", n_above, n_win);

        /* The floor must be a real number well above 1 (a flat spectrum), or
         * this test is not measuring what it claims. */
        RS_CHECK(floor_prom > 1.0);
        RS_CHECK(floor_prom == floor_prom);
    }

    /* ------------------------------------------------------------------
     * Genuine detections, same geometry and brightness, one target moving.
     * ------------------------------------------------------------------ */
    RS_CASE("genuine detections against that floor");
    {
        const double freqs[] = { 0.3, 0.5, 0.7, 0.9 };
        const size_t n_freq = sizeof freqs / sizeof freqs[0];

        printf("    %9s %13s %9s %8s %12s\n",
               "injected", "matched prom", "floor", "clears?", "TOOL reports");

        size_t n_clear = 0, n_correct = 0;
        double reported[sizeof freqs / sizeof freqs[0]];
        double last_df = 0.0;
        for (size_t i = 0; i < n_freq; i++) {
            const rs_sim_tgt_t vib[] = {
                { .x = 0.0, .y = 0.0, .z = 0.0, .rcs = 1.0,
                  .vib_freq = freqs[i], .vib_amp = 0.020 },
            };
            double tp = 0.0, peak_f = 0.0, df = 0.0;
            RS_CHECK_OK(analyse(vib, 1, NULL, &tp, freqs[i], NULL, 0.0, NULL,
                                &peak_f, &df));

            reported[i] = peak_f;
            last_df = df;
            const int clears = tp > floor_prom;
            const int correct = fabs(peak_f - freqs[i]) < 2.0 * df;
            if (clears) n_clear++;
            if (correct) n_correct++;
            printf("    %7.1f Hz %12.1f %9.1f %8s %8.3f Hz %s\n",
                   freqs[i], tp, floor_prom, clears ? "yes" : "NO",
                   peak_f, correct ? "ok" : "WRONG");
        }

        printf("\n    %zu of %zu clear the floor by the old criterion\n", n_clear, n_freq);
        printf("    %zu of %zu are the frequency the TOOL would report\n",
               n_correct, n_freq);

        /* THREE CRITERIA, AND ONLY THE THIRD IS HARD TO PASS BY ACCIDENT.
         *
         * The first asks whether SOME window reports approximately the injected
         * frequency with more prominence than a static scene manages anywhere.
         * That is weaker than it reads. A two-bin tolerance out of this
         * spectrum's bins is a few percent per window, and the chance that at
         * least one window of many lands inside it is not small: at the 9
         * windows here it is 0.44, and on the 361-window grids this project
         * runs over real data it is 1.000 to six decimal places. A criterion
         * that a static scene satisfies by chance cannot establish a detection
         * on its own; see runs/giza/2026-07-30-validated-spot-khufu/
         * POSITIVE-CONTROL.md, where seven configurations all "recovered" a
         * target under it, including ones that missed at every window actually
         * containing the target.
         *
         * The second asks what mmotion would PRINT: the dominant frequency of
         * the most prominent window, which is what rs_spectrum_best_window()
         * selects and what a user reads off. That cannot be produced by a lucky
         * window elsewhere in the scene, being one value per run.
         *
         * BUT THE SECOND IS STILL A PER-POINT MATCH, and that is not enough. A
         * chain that emits a FIXED spurious frequency passes it at every
         * injected frequency the fixed value happens to sit near, which at this
         * bin spacing is a wide range. Four conclusions were drawn and withdrawn
         * on 2026-07-31 for exactly that reason -- among them a configuration
         * reporting 1.569 Hz for every injection from 0.2 to 1.4 Hz, and one
         * reporting 0.314 Hz for six of seven, both scoring "recovered" wherever
         * the artefact fell within tolerance.
         *
         * The third fixes it. Sweeping the injection and fitting the reported
         * frequency against it separates a chain that follows the target from
         * one that does not: a working chain gives slope 1, a fixed artefact
         * gives slope 0, and no single point can tell them apart. The rms bound
         * is half a bin -- four times tighter than the per-point tolerance --
         * because a chain that genuinely tracks has no reason to be looser. */
        double slope = 0.0, rms = 0.0;
        RS_CHECK(rs_track_fit(freqs, reported, n_freq, &slope, &rms) == 1);
        printf("    tracking: slope %.3f (want 1), rms %.4f Hz (want < %.4f)\n",
               slope, rms, 0.5 * last_df);

        RS_CHECK(n_clear >= 3);
        RS_CHECK(n_correct >= 3);
        RS_CHECK_NEAR(slope, 1.0, 0.15);
        RS_CHECK(rms < 0.5 * last_df);

        /* The criterion has to be able to FAIL, or asserting it means nothing.
         * A chain reporting one fixed frequency regardless of the injection --
         * the exact defect this exists to catch -- must be rejected. */
        {
            const double fixed[] = { 1.569, 1.569, 1.569, 1.569 };
            double fs = 0.0, fr = 0.0;
            RS_CHECK(rs_track_fit(freqs, fixed, n_freq, &fs, &fr) == 1);
            RS_CHECK(fabs(fs - 1.0) > 0.15);      /* slope 0, not 1 */
            RS_CHECK(fr >= 0.5 * last_df);        /* and nowhere near */
            printf("    negative control: a fixed 1.569 Hz gives slope %.3f, "
                   "rms %.4f Hz -- rejected\n", fs, fr);
        }
    }

    RS_CASE("the static null focuses, rather than standing in as noise");
    {
        /* THE CONTROL HAS TO BE A SCENE, NOT NOISE. --null-static is the only
         * negative control this project has on real data, and everything it
         * claims rests on the simulated scene going through the identical chain
         * with only the motion absent. That fails silently if the phase history
         * does not focus: a defocused null still yields a complete spectrum and
         * a prominence distribution, just not one that means anything.
         *
         * It DID fail, until 2026-08-02. rs_simulate_static_like() wrote each
         * scatterer's phase as -k*R while setting phase_ref_srp = 1, which tells
         * rs_focus_backproject() to undo k*(R - r_ref) instead, leaving a
         * per-pulse phase of many cycles common to every pixel. The scene
         * focused to a peak-to-mean of 3.6 where it should reach 90-odd. No test
         * touched this function -- it is reachable only through the CLI's
         * --null-static -- which is why a broken control survived.
         *
         * Point scatterers must focus peakily. Noise cannot. */
        rs_sim_tgt_t one[1] = {{ .x = 0, .y = 0, .z = 0, .rcs = 1.0 }};
        rs_cphd_t ref;
        RS_CHECK_OK(rs_sim_scene(&ref, one, 1, 20.0, 400.0, 256, 0.5));

        const double centre[2] = { 0.0, 0.0 };
        rs_cphd_t sim;
        RS_CHECK_OK(rs_simulate_static_like(&ref, 7u, 200, centre, 24.0, 256, &sim));

        /* The flag it advertises is the flag the focuser must be able to act on;
         * a real CPHD sets 1, and the null is only a stand-in if it agrees. */
        RS_CHECK(sim.phase_ref_srp == 1);

        rs_grid_t g = { .origin = {0,0,0}, .n_x = 64, .n_y = 64,
                        .dx = 0.5, .dy = 0.5, .height = 0.0 };
        rs_slc_t img;
        RS_CHECK_OK(rs_slc_alloc(&img, g.n_y, g.n_x));
        RS_CHECK_OK(rs_focus_backproject(&sim, &g, 0, sim.n_pulse, &img));

        double pk = 0.0, sum = 0.0;
        const size_t n = img.n_az * img.n_rg;
        for (size_t i = 0; i < n; i++) {
            const double a = cabs(img.data[i]);
            if (a > pk) pk = a;
            sum += a;
        }
        const double mean = sum / (double)n;
        const double ratio = (mean > 0.0) ? pk / mean : 0.0;
        printf("      static null focuses to peak/mean %.1f\n", ratio);
        /* Measured 93.7 focused against 3.6 defocused; 20 separates them by a
         * margin neither seed nor grid choice comes near. */
        RS_CHECK(ratio > 20.0);

        rs_slc_free(&img);
        rs_cphd_free(&sim);
        rs_cphd_free(&ref);
    }

    RS_CASE("an injected vibrator is recovered from an otherwise static collect");
    {
        /* THE POSITIVE CONTROL. Every real collect this project has processed
         * returns a null, and a null cannot distinguish "nothing moved" from
         * "this chain cannot see motion in this data". rs_simulate_inject_vibrator()
         * settles that by adding a scatterer of known frequency to the phase
         * history BEFORE sub-aperture formation, so the whole chain -- decomposition,
         * tracking, spectrum -- runs over the real clutter and geometry.
         *
         * Here the stand-in for a real collect is a static scene, which is the
         * hardest honest test of the control: nothing else in it moves, so the
         * only thing that can produce the injected frequency is the injection.
         *
         * Both arms, because a positive control that fires without the injection
         * is not a control. */
        const double f_inj = 0.7;
        const double amp_z = 0.002442;   /* ~2 mm along the line of sight, inside
                                          * the lambda/4 wrap the phase route has */
        double got[2] = { -1.0, -1.0 };

        for (int arm = 0; arm < 2; arm++) {
            rs_sim_tgt_t one[1] = {{ .x = 0, .y = 0, .z = 0, .rcs = 1.0 }};
            rs_cphd_t ref;
            RS_CHECK_OK(rs_sim_scene(&ref, one, 1, 20.0, 400.0, 256, 0.5));

            const double centre[2] = { 0.0, 0.0 };
            rs_cphd_t c;
            RS_CHECK_OK(rs_simulate_static_like(&ref, 11u, 300, centre, 24.0,
                                                256, &c));
            rs_cphd_free(&ref);

            if (arm == 1) {
                rs_inject_report_t rep;
                RS_CHECK_OK(rs_simulate_inject_vibrator(&c, centre, f_inj,
                                                        amp_z, 20.0, &rep));
                /* The control's own control: an injection that lands nowhere
                 * produces a null indistinguishable from a chain that cannot see
                 * the motion, which is the failure this whole case exists to
                 * rule out. Every pulse must deposit on a scene built around
                 * this target. */
                printf("      injected into %zu of %zu pulses, bins %.1f-%.1f\n",
                       rep.n_deposited, rep.n_pulse, rep.fbin_min, rep.fbin_max);
                RS_CHECK(rep.n_deposited == rep.n_pulse);
            }

            rs_grid_t g = { .origin = {0,0,0}, .n_x = 64, .n_y = 64,
                            .dx = 0.5, .dy = 0.5, .height = 0.0 };
            rs_subap_params_t sp;
            rs_subap_params_default(&sp);
            sp.n_looks = 128;
            sp.overlap = 0.0;
            rs_subap_stack_t st;
            RS_CHECK_OK(rs_subaperture_from_cphd(&c, &g, &sp, &st));

            rs_microm_params_t mp;
            rs_microm_params_default(&mp);
            mp.estimator = RS_MICROM_EST_PHASE;
            mp.win_az = mp.win_rg = 32;
            mp.stride_az = mp.stride_rg = 16;
            mp.coherence_min = 0.0;
            rs_microm_t m;
            RS_CHECK_OK(rs_microm_track(&st, &mp, &m));

            rs_spectrum_t spec;
            RS_CHECK_OK(rs_spectrum_compute(&m, RS_SPEC_VELOCITY, &spec));

            /* Selected the way the source literature does: the injected
             * scatterer is a persistent one by construction, so this is the
             * policy that should find it. */
            rs_spectrum_ps_t ps;
            if (rs_spectrum_ps_window(&spec, &ps) == RS_OK) got[arm] = ps.freq_hz;
            printf("      %s: %.3f Hz (D_A %.3f, %zu candidates of %zu)\n",
                   arm ? "injected " : "control  ",
                   got[arm], ps.d_a, ps.n_candidate, ps.n_input);

            rs_spectrum_free(&spec);
            rs_microm_free(&m);
            rs_subap_stack_free(&st);
            rs_cphd_free(&c);
        }

        /* The injected arm must return the injected frequency. */
        RS_CHECK(got[1] >= 0.0);
        RS_CHECK_NEAR(got[1], f_inj, 0.05);
        /* And the control arm must not, or the chain is answering from
         * something other than the injection. */
        RS_CHECK(got[0] < 0.0 || fabs(got[0] - f_inj) > 0.05);
    }

    RS_TEST_END();
}
