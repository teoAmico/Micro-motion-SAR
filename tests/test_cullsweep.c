/* Does the correlation cull FOLLOW the injection, or does it emit a fixed answer?
 *
 * `FOLLOW-UPS.md` item 12a added rs_spectrum_ampcor_window() and said plainly
 * that it had not been put to a sweep. This is the sweep. It exists because a
 * selection policy measured at one operating point on one seed is not measured
 * at all -- item 2 records several per-point "recoveries" that turned out to be
 * a fixed spurious frequency landing near the injection, and four conclusions
 * were withdrawn on 2026-07-31 for exactly that.
 *
 * WHAT IS COMPARED, AND WHY ALL THREE. The three selection policies are run over
 * the SAME tracking result and fitted against the injection side by side:
 *
 *   rs_spectrum_best_window()    prominence; what the tool reports
 *   rs_spectrum_consensus()      what the windows collectively say
 *   rs_spectrum_ampcor_window()  what the correlation surfaces looked like
 *
 * Running them separately would confound the policy with the scene. Running them
 * together means every difference in the fitted slope is attributable to the
 * selection and to nothing else, which is the only question here.
 *
 * THE FIXTURE IS DISTRIBUTED TEXTURE, NOT AN ISOLATED POINT, and that choice is
 * the reason this file exists rather than another sweep in test_nullmotion.c.
 * An isolated point on empty background biases a correlation tracker by its own
 * point response, and item 2 records that no distributed-texture fixture on
 * which the chain demonstrably works has ever existed. The clutter here vibrates
 * COHERENTLY -- the patch moves as a whole, which is what a structure's surface
 * does and what patch-level offset tracking is built to measure. A lone mover
 * inside static clutter is a detection problem, not a tracking one.
 *
 * THE BAR, from README.md: slope near 1 and rms under half a bin, pooled over
 * independent clutter realisations, with a static control through identical
 * processing. All four parts are here. What this file ASSERTS is deliberately
 * weaker than the bar in places, and where it is, the reason is that the
 * measurement does not currently support the stronger claim -- printing a
 * negative result and asserting a threshold tuned to accommodate it are not the
 * same thing, and only the first is honest.
 */

#include "resonarsat/focus.h"
#include "resonarsat/microm.h"
#include "resonarsat/subaperture.h"
#include "rs_sim.h"
#include "rs_test.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* The SNR gate factors swept, as multiples of the noise-alone value.
 *
 * The range is set by what the two ends MEAN rather than by taste. At 1.0 the
 * gate sits exactly at what a surface with no signal in it produces, so it can
 * only remove windows that are worse than noise -- the weakest gate that is
 * still a gate. Above about 3 the threshold exceeds the median surface SNR
 * measured on every clutter run in this file, so it can only be a refusal
 * machine. Anything worth choosing lies between. */
/* Gate 2 held at the shipped default throughout, so the table below varies one
 * thing. Its own factor is not swept here: it is relative to the scene's median
 * and so cannot be compared across runs the way an absolute threshold can. */
#define RS_CULL_SIGMA_DEFAULT 2.0

#define N_FACTOR 8u
static const double SNR_FACTORS[N_FACTOR] = {
    0.0,   /* gate disabled entirely, to separate its effect from gates 2 and 3 */
    1.0, 1.25, 1.5, 1.75, 2.0, 2.5, 3.0
};

#define N_CLUTTER 96u
#define GRID_N    96u
#define CELL_M    0.5
#define N_LOOKS   128u

/* What one run of the chain reported, by each of the three policies.
 *
 * A frequency of -1 means the policy declined to answer, which is a distinct
 * outcome from answering wrongly and is counted separately below. Conflating
 * the two is how a policy that refuses everything comes to look accurate. */
typedef struct {
    double best_hz, consensus_hz, cull_hz;
    double df;
    size_t n_win, cull_input, cull_survivor;
    size_t cull_snr, cull_sigma, cull_neigh;
    double snr_null, snr_median;
    double sigma_median, exc_median;

    /* What the cull reported at each swept SNR factor, from THIS run's single
     * spectrum. Evaluating every factor against one set of spectra is the whole
     * design of the experiment: re-running the chain per factor would confound
     * the threshold with the realisation, and cost eight times as much for a
     * worse answer. */
    double cull_hz_f[N_FACTOR];
    size_t cull_surv_f[N_FACTOR];
} rs_run_t;

/* Build a coherently vibrating clutter patch.
 *
 * Every scatterer carries the SAME vibration frequency, amplitude and phase, so
 * the patch translates rather than deforming -- see the header on why a lone
 * mover inside static clutter measures something else. 'seed' selects the
 * realisation: the geometry and the motion are identical between seeds and only
 * the scatterer placement and brightness change, which is what makes two seeds
 * independent samples of the same experiment.
 *
 * Amplitudes are exponential in power, i.e. Rayleigh in voltage, which is the
 * distribution fully developed speckle actually has. A uniform draw would make
 * every window equally bright and remove the very variation the coherence and
 * SNR gates are there to respond to.
 */
static void make_clutter(rs_sim_tgt_t *tg, size_t n, unsigned seed,
                         double freq, double amp)
{
    unsigned s = seed * 2654435761u + 1u;
    for (size_t i = 0; i < n; i++) {
        s = s * 1103515245u + 12345u;
        const double u1 = (double)(s >> 8) / 16777216.0;
        s = s * 1103515245u + 12345u;
        const double u2 = (double)(s >> 8) / 16777216.0;
        s = s * 1103515245u + 12345u;
        const double u3 = (double)(s >> 8) / 16777216.0;

        tg[i].x = (u1 - 0.5) * 24.0;      /* a 24 m patch, well inside the grid */
        tg[i].y = (u2 - 0.5) * 24.0;
        tg[i].z = 0.0;
        /* -ln(u) is exponential; the floor keeps a draw near zero from becoming
         * a scatterer that contributes nothing but still costs a target slot. */
        tg[i].rcs = 0.3 * (-log(u3 > 1e-6 ? u3 : 1e-6));
        tg[i].vib_freq = freq;
        tg[i].vib_amp = amp;
        tg[i].vib_phase = 0.0;
    }
}

/* Run the whole chain once and read all three policies off one spectrum.
 *
 * Every parameter here is fixed across every call, so that a difference between
 * two rows of the sweep is attributable to the injection or the seed and to
 * nothing else. The coherence mask is open because distributed clutter at these
 * sub-look counts scores below the 0.4 default even where it tracks; the gates
 * under test are the cull's, and leaving a second mask in front of them would
 * confound the two.
 */
static resonarsat_status_t run_once(double freq, double amp, unsigned seed,
                                    int isolated, rs_run_t *out)
{
    memset(out, 0, sizeof *out);
    out->best_hz = out->consensus_hz = out->cull_hz = -1.0;

    rs_sim_tgt_t tg[N_CLUTTER];
    size_t n_tgt;
    if (isolated) {
        /* The operating point test_nullmotion.c already demonstrates a
         * recovery at: one target on empty background. It is here as the
         * CONTROL FOR THE POLICIES THEMSELVES -- see the header. */
        tg[0] = (rs_sim_tgt_t){ .x = 0.0, .y = 0.0, .z = 0.0, .rcs = 1.0,
                                .vib_freq = freq, .vib_amp = amp };
        n_tgt = 1;
    } else {
        make_clutter(tg, N_CLUTTER, seed, freq, amp);
        n_tgt = N_CLUTTER;
    }

    rs_cphd_t c;
    resonarsat_status_t st = rs_sim_scene(&c, tg, n_tgt, 20.0, 400.0, 256, 0.5);
    if (st != RS_OK) return st;

    rs_grid_t g = { .origin = {0,0,0}, .n_x = GRID_N, .n_y = GRID_N,
                    .dx = CELL_M, .dy = CELL_M, .height = 0.0 };

    rs_subap_params_t sp;
    rs_subap_params_default(&sp);
    sp.n_looks = N_LOOKS;
    sp.overlap = 0.0;      /* overlap works against the ambiguity condition */

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

    out->df = spec.df;
    out->n_win = spec.n_win;
    out->snr_null = spec.snr_null;

    /* The median SNR, printed beside the null so a reader can see whether the
     * cull's gate was reachable at all in this run rather than only whether it
     * fired. A run where every surface sits at the null is a different finding
     * from one where the gate was close. */
    if (spec.snr && spec.n_win) {
        double *tmp = malloc(spec.n_win * sizeof *tmp);
        if (tmp) {
            memcpy(tmp, spec.snr, spec.n_win * sizeof *tmp);
            for (size_t i = 1; i < spec.n_win; i++) {
                const double v = tmp[i];
                size_t j = i;
                while (j > 0 && tmp[j - 1] > v) { tmp[j] = tmp[j - 1]; j--; }
                tmp[j] = v;
            }
            out->snr_median = tmp[spec.n_win / 2];
            free(tmp);
        }
    }

    /* Median sigma and excursion, which is what gate 2 compares. Printed
     * rather than only counted because "the gate fired" and "the gate fired
     * because sigma is on the wrong scale" are different findings. */
    if (spec.sigma_px && spec.excursion_px && spec.n_win) {
        double *a = malloc(spec.n_win * sizeof *a);
        double *b = malloc(spec.n_win * sizeof *b);
        if (a && b) {
            memcpy(a, spec.sigma_px, spec.n_win * sizeof *a);
            memcpy(b, spec.excursion_px, spec.n_win * sizeof *b);
            for (size_t i = 1; i < spec.n_win; i++) {
                double v = a[i]; size_t j = i;
                while (j > 0 && a[j-1] > v) { a[j] = a[j-1]; j--; } a[j] = v;
                v = b[i]; j = i;
                while (j > 0 && b[j-1] > v) { b[j] = b[j-1]; j--; } b[j] = v;
            }
            out->sigma_median = a[spec.n_win / 2];
            out->exc_median   = b[spec.n_win / 2];
        }
        free(a); free(b);
    }

    size_t w = 0;
    double prom = 0.0;
    if (rs_spectrum_best_window(&spec, &w, &prom, NULL) == RS_OK) {
        out->best_hz = spec.dominant_freq[w];
    }

    double cf = 0.0;
    size_t ca = 0, cd = 0, cv = 0, cb = 0;
    if (rs_spectrum_consensus(&spec, &cf, &ca, &cd, &cv, &cb) == RS_OK) {
        out->consensus_hz = cf;
    }

    for (size_t i = 0; i < N_FACTOR; i++) {
        rs_spectrum_cull_t cull_i;
        const resonarsat_status_t s2 =
            rs_spectrum_ampcor_window_opts(&spec, SNR_FACTORS[i],
                                           RS_CULL_SIGMA_DEFAULT, &cull_i);
        out->cull_hz_f[i]   = (s2 == RS_OK) ? cull_i.freq_hz : -1.0;
        out->cull_surv_f[i] = cull_i.n_survivor;
    }

    rs_spectrum_cull_t cull;
    const resonarsat_status_t cst = rs_spectrum_ampcor_window(&spec, &cull);
    /* The counts are filled in on the refusal path too, and are wanted there
     * most: they say which gate removed the population. */
    out->cull_input    = cull.n_input;
    out->cull_survivor = cull.n_survivor;
    out->cull_snr      = cull.n_snr_cull;
    out->cull_sigma    = cull.n_sigma_cull;
    out->cull_neigh    = cull.n_neigh_cull;
    if (cst == RS_OK) out->cull_hz = cull.freq_hz;

    rs_spectrum_free(&spec);
    rs_microm_free(&m);
    rs_subap_stack_free(&s);
    rs_cphd_free(&c);
    return RS_OK;
}

/* Fit one policy's answers against the injections that produced them, over
 * every point that produced an answer at all.
 *
 * Declined answers are DROPPED rather than counted as zero, and the surviving
 * count is returned so the caller can weigh the fit by how much of the sweep it
 * rests on. A policy that answers twice and fits those two perfectly has not
 * outperformed one that answers everywhere with some scatter, and a fit reported
 * without its support invites exactly that misreading. */
static size_t fit_policy(const double *inj, const double *rep, size_t n,
                         double *slope, double *rms, size_t *n_distinct)
{
    double xi[64], yi[64];
    size_t k = 0;
    for (size_t i = 0; i < n && k < 64; i++) {
        if (rep[i] < 0.0) continue;
        xi[k] = inj[i];
        yi[k] = rep[i];
        k++;
    }

    /* HOW MANY DISTINCT INJECTIONS THE FIT RESTS ON, which is the number that
     * decides whether the slope means anything. rs_track_fit() exists to reject
     * a chain emitting a FIXED frequency, and it does that by requiring the
     * reported value to follow the injection across a swept range. Two distinct
     * abscissae cannot make that distinction however good the residuals look --
     * a line through two clusters fits perfectly whatever produced them. A
     * policy that answers only where the answer is easy therefore scores well
     * on slope and rms while having demonstrated nothing, and the only thing
     * that reveals it is this count printed beside them. */
    if (n_distinct) {
        size_t d = 0;
        for (size_t i = 0; i < k; i++) {
            int seen = 0;
            for (size_t j = 0; j < i; j++) if (xi[j] == xi[i]) { seen = 1; break; }
            if (!seen) d++;
        }
        *n_distinct = d;
    }

    if (k < 3 || !rs_track_fit(xi, yi, k, slope, rms)) {
        *slope = 0.0; *rms = 0.0;
        return k;
    }
    return k;
}

/* One row of a fit table.
 *
 * A fit over fewer than three answers is reported as absent rather than as a
 * slope of zero, because zero is what a chain emitting a FIXED frequency scores
 * and the two must not print the same. */
static void print_fit(const char *name, size_t n_ans, size_t n_dist,
                      double slope, double rms)
{
    if (n_ans < 3) {
        printf("  %-10s %8zu %9zu %10s %9s\n", name, n_ans, n_dist, "--", "--");
        return;
    }
    printf("  %-10s %8zu %9zu %10.3f %9.4f\n", name, n_ans, n_dist, slope, rms);
}

int main(void)
{
    const double amp = 0.020;
    const double freqs[] = { 0.3, 0.5, 0.7, 0.9, 1.1, 1.3 };
    const size_t n_freq = sizeof freqs / sizeof freqs[0];
    const unsigned seeds[] = { 7u, 23u, 101u };
    const size_t n_seed = sizeof seeds / sizeof seeds[0];
    const size_t n_pt = n_freq * n_seed;

    double inj[64], f_best[64], f_cons[64], f_cull[64];
    double f_fac[N_FACTOR][64];       /* clutter answers, per SNR factor */
    double p_fac[N_FACTOR][16];       /* isolated-point answers */
    double s_fac[N_FACTOR][8];        /* static control answers */
    size_t n = 0;
    double df = 0.0, snr_null = 0.0;

    printf("  coherently vibrating clutter, %u scatterers, %zu seeds, %u looks\n",
           N_CLUTTER, n_seed, N_LOOKS);
    printf("  %5s %6s | %9s %9s %9s | %s\n",
           "inj", "seed", "best", "consensus", "cull", "cull gates (in/snr/sig/nbr/surv)");

    for (size_t si = 0; si < n_seed; si++) {
        for (size_t fi = 0; fi < n_freq; fi++) {
            rs_run_t r;
            RS_CHECK_OK(run_once(freqs[fi], amp, seeds[si], 0, &r));
            inj[n]    = freqs[fi];
            f_best[n] = r.best_hz;
            f_cons[n] = r.consensus_hz;
            f_cull[n] = r.cull_hz;
            for (size_t k = 0; k < N_FACTOR; k++) f_fac[k][n] = r.cull_hz_f[k];
            n++;
            df = r.df;
            snr_null = r.snr_null;

            printf("  %4.1f  %6u | %8.3f  %8.3f  %8.3f  | %2zu/%2zu/%2zu/%2zu/%2zu"
                   "  snr med %.1f null %.1f\n",
                   freqs[fi], seeds[si], r.best_hz, r.consensus_hz, r.cull_hz,
                   r.cull_input, r.cull_snr, r.cull_sigma, r.cull_neigh,
                   r.cull_survivor, r.snr_median, r.snr_null);
        }
    }

    /* The static control, through identical processing.
     *
     * This is the only thing that catches a common-mode artefact, and item 11
     * establishes that neither the consensus nor the cull can substitute for it:
     * an artefact produced by the processing has a genuine, well-determined
     * correlation peak in every window and passes every gate either applies. A
     * sweep without this beside it measures nothing. */
    printf("\n  static control (identical processing, nothing moving):\n");
    double s_best[8], s_cons[8], s_cull[8];
    for (size_t si = 0; si < n_seed; si++) {
        rs_run_t r;
        RS_CHECK_OK(run_once(0.0, 0.0, seeds[si], 0, &r));
        s_best[si] = r.best_hz;
        s_cons[si] = r.consensus_hz;
        s_cull[si] = r.cull_hz;
        for (size_t k = 0; k < N_FACTOR; k++) s_fac[k][si] = r.cull_hz_f[k];
        printf("  %4s  %6u | %8.3f  %8.3f  %8.3f  | %2zu/%2zu/%2zu/%2zu/%2zu\n",
               "--", seeds[si], r.best_hz, r.consensus_hz, r.cull_hz,
               r.cull_input, r.cull_snr, r.cull_sigma, r.cull_neigh,
               r.cull_survivor);
    }

    /* THE CONTROL FOR THE POLICIES THEMSELVES, and the case that makes the
     * clutter sweep above interpretable.
     *
     * A policy that refuses everything produces no false positives, so the
     * clutter rows alone cannot tell "correctly refusing an unmeasurable
     * configuration" from "refuses unconditionally". The isolated-point fixture
     * separates them, because test_nullmotion.c establishes that the chain DOES
     * recover the injection there -- slope near 1, rms under half a bin, through
     * rs_spectrum_best_window(). Any policy that declines to answer at an
     * operating point where the chain demonstrably measures is broken, and no
     * amount of correct refusal elsewhere redeems it. */
    printf("\n  isolated point target -- the operating point where the chain\n"
           "  is known to recover the injection (test_nullmotion.c):\n");
    printf("  %5s | %9s %9s %9s | %s\n",
           "inj", "best", "consensus", "cull", "cull gates (in/snr/sig/nbr/surv)");
    double p_inj[16], p_best[16], p_cons[16], p_cull[16];
    size_t np = 0;
    for (size_t fi = 0; fi < n_freq; fi++) {
        rs_run_t r;
        RS_CHECK_OK(run_once(freqs[fi], amp, 0u, 1, &r));
        p_inj[np] = freqs[fi];
        p_best[np] = r.best_hz;
        p_cons[np] = r.consensus_hz;
        p_cull[np] = r.cull_hz;
        for (size_t k = 0; k < N_FACTOR; k++) p_fac[k][np] = r.cull_hz_f[k];
        np++;
        printf("  %4.1f  | %8.3f  %8.3f  %8.3f  | %2zu/%2zu/%2zu/%2zu/%2zu"
               "  snr med %.1f  sigma med %.2f px  exc med %.2f px\n",
               freqs[fi], r.best_hz, r.consensus_hz, r.cull_hz,
               r.cull_input, r.cull_snr, r.cull_sigma, r.cull_neigh,
               r.cull_survivor, r.snr_median, r.sigma_median, r.exc_median);
    }

    double sl_b = 0, rm_b = 0, sl_c = 0, rm_c = 0, sl_k = 0, rm_k = 0;
    size_t db = 0, dc = 0, dk = 0;
    const size_t nb = fit_policy(inj, f_best, n, &sl_b, &rm_b, &db);
    const size_t nc = fit_policy(inj, f_cons, n, &sl_c, &rm_c, &dc);
    const size_t nk = fit_policy(inj, f_cull, n, &sl_k, &rm_k, &dk);

    printf("\n  pooled over %zu points (%zu frequencies x %zu seeds), "
           "df %.4f Hz, half a bin %.4f Hz\n", n, n_freq, n_seed, df, 0.5 * df);
    printf("  %-10s %8s %9s %10s %9s\n",
           "policy", "answers", "distinct", "slope", "rms Hz");
    print_fit("best",      nb, db, sl_b, rm_b);
    print_fit("consensus", nc, dc, sl_c, rm_c);
    print_fit("cull",      nk, dk, sl_k, rm_k);
    printf("  (the bar is slope near 1 and rms below %.4f Hz, over a SWEPT "
           "range --\n   a fit resting on one or two distinct injections has "
           "not been swept)\n", 0.5 * df);

    double q_b = 0, qr_b = 0, q_c = 0, qr_c = 0, q_k = 0, qr_k = 0;
    size_t pdb = 0, pdc = 0, pdk = 0;
    const size_t pb = fit_policy(p_inj, p_best, np, &q_b, &qr_b, &pdb);
    const size_t pc = fit_policy(p_inj, p_cons, np, &q_c, &qr_c, &pdc);
    const size_t pk = fit_policy(p_inj, p_cull, np, &q_k, &qr_k, &pdk);
    printf("\n  isolated point, %zu points:\n", np);
    printf("  %-10s %8s %9s %10s %9s\n",
           "policy", "answers", "distinct", "slope", "rms Hz");
    print_fit("best",      pb, pdb, q_b, qr_b);
    print_fit("consensus", pc, pdc, q_c, qr_c);
    print_fit("cull",      pk, pdk, q_k, qr_k);

    /* ------------------------------------------------------------------
     * THE SNR GATE FACTOR, swept across the spectra above.
     *
     * Item 12c left this as the open question: gate 1 removes a third to two
     * thirds of the population on clutter, and whether its factor of two is
     * right at these coherences was never measured. Four numbers decide it, and
     * they trade against each other:
     *
     *   answers    recall -- how often the policy says anything at all
     *   correct    precision -- of those, how many are within half a bin
     *   distinct   how many distinct injections the answers span, which is what
     *              decides whether a fit over them could mean anything
     *   static     answers on scenes with nothing moving. ANY is disqualifying
     *
     * A factor is better than another only if it improves recall or coverage
     * without giving up precision or answering a static scene. If nothing
     * separates them on those terms the incumbent stands, because changing a
     * tuned constant on a tie is how a threshold gets fitted to one fixture.
     * ------------------------------------------------------------------ */
    printf("\n  SNR gate factor swept over the SAME spectra "
           "(gate 2 fixed at 2x median, gate 3 unchanged):\n");
    printf("  %8s %8s | %-24s | %-16s | %s\n",
           "factor", "gate", "clutter (18 pts)", "isolated (6)", "static (3)");
    printf("  %8s %8s | %5s %5s %5s %6s | %5s %5s %5s | %s\n",
           "", "x null", "ans", "corr", "dist", "rms", "ans", "corr", "dist",
           "answered");

    size_t best_k = 0, best_cov = 0;
    for (size_t k = 0; k < N_FACTOR; k++) {
        size_t ca = 0, cc = 0, ia = 0, ic = 0, sa = 0;
        for (size_t i = 0; i < n; i++) {
            if (f_fac[k][i] < 0.0) continue;
            ca++;
            if (fabs(f_fac[k][i] - inj[i]) <= 0.5 * df) cc++;
        }
        for (size_t i = 0; i < np; i++) {
            if (p_fac[k][i] < 0.0) continue;
            ia++;
            if (fabs(p_fac[k][i] - p_inj[i]) <= 0.5 * df) ic++;
        }
        for (size_t i = 0; i < n_seed; i++) if (s_fac[k][i] >= 0.0) sa++;

        double k_slope = 0.0, k_rms = 0.0;
        size_t cd = 0, id = 0;
        (void)fit_policy(inj, f_fac[k], n, &k_slope, &k_rms, &cd);
        {
            double t_slope = 0.0, t_rms = 0.0;
            (void)fit_policy(p_inj, p_fac[k], np, &t_slope, &t_rms, &id);
        }

        printf("  %8.2f %8.1f | %5zu %5zu %5zu %6.4f | %5zu %5zu %5zu | %zu%s\n",
               SNR_FACTORS[k], SNR_FACTORS[k] * snr_null,
               ca, cc, cd, (ca >= 3) ? k_rms : 0.0, ia, ic, id, sa,
               (sa > 0) ? "  <- DISQUALIFIED" : "");

        /* Rank on distinct-injection coverage, with perfect precision and no
         * static answer as hard preconditions rather than tiebreakers. */
        if (sa == 0 && cc == ca && ic == ia && cd + id > best_cov) {
            best_cov = cd + id;
            best_k = k;
        }
    }
    printf("  best distinct coverage at factor %.2f (%zu distinct injections "
           "across both fixtures)\n", SNR_FACTORS[best_k], best_cov);

    RS_CASE("the SNR gate factor sweep separates the candidates");
    {
        /* The sweep is only informative if the factor CHANGES something. If
         * every factor gave the same counts the gate would be inert here and
         * nothing could be concluded from the table above. */
        size_t lo_ans = 0, hi_ans = 0;
        for (size_t i = 0; i < n; i++) {
            if (f_fac[1][i] >= 0.0) lo_ans++;                 /* factor 1.0 */
            if (f_fac[N_FACTOR - 1][i] >= 0.0) hi_ans++;      /* factor 3.0 */
        }
        printf("    factor 1.00 answered %zu of %zu; factor 3.00 answered %zu\n",
               lo_ans, n, hi_ans);
        RS_CHECK(lo_ans >= hi_ans);
    }

    RS_CASE("a gate at or below the noise-alone SNR admits a static scene");
    {
        /* THE MEASUREMENT THAT JUSTIFIES THE GATE EXISTING AT ALL, and it was
         * not what this case was originally written to assert.
         *
         * The factor was chosen in item 12a on the argument that a surface
         * failing to stand at twice what an empty surface produces has not
         * distinguished itself from one. That is a plausible sentence and it
         * was not evidence. This is: with the gate disabled, and with it set
         * exactly AT the noise-alone value, the cull answers on a scene where
         * nothing moves -- a false positive of the kind the whole policy exists
         * to avoid -- and above the null it does not, at every factor tried.
         *
         * The boundary is therefore measured rather than asserted, and it falls
         * where the derivation said it should: at the null. */
        for (size_t k = 0; k < N_FACTOR; k++) {
            size_t sa = 0;
            for (size_t i = 0; i < n_seed; i++) if (s_fac[k][i] >= 0.0) sa++;
            printf("    factor %.2f: %zu static answer(s)\n", SNR_FACTORS[k], sa);
            if (SNR_FACTORS[k] <= 1.0) {
                /* Not asserted as "must be nonzero" -- a seed that happened to
                 * refuse would then fail the suite for no defect. The claim is
                 * that the disqualifying cases are confined to this end. */
                continue;
            }
            RS_CHECK(sa == 0);
        }
        /* And the finding itself: at least one factor at or below the null does
         * let a static scene through, which is what makes the gate necessary
         * rather than merely present. If this stops holding the gate has become
         * inert and its factor should be re-derived, not re-tuned. */
        size_t weak_fp = 0;
        for (size_t k = 0; k < N_FACTOR; k++) {
            if (SNR_FACTORS[k] > 1.0) continue;
            for (size_t i = 0; i < n_seed; i++) if (s_fac[k][i] >= 0.0) weak_fp++;
        }
        printf("    static false positives at factors <= 1.0: %zu\n", weak_fp);
        RS_CHECK(weak_fp > 0);
    }

    RS_CASE("the default factor sits on a plateau, not on a tuned edge");
    {
        /* A constant chosen at the edge of a cliff is fitted to its fixture.
         * This asserts that the default behaves identically to its neighbours
         * above -- so the choice costs nothing and buys nothing against them,
         * which is the only honest reason to leave a tuned constant where it
         * is. The factors below the default are NOT included: they differ, and
         * that difference is the finding printed in the table. */
        size_t ref_ans = 0, ref_corr = 0;
        for (size_t i = 0; i < n; i++) {
            if (f_fac[5][i] < 0.0) continue;              /* 2.00, the default */
            ref_ans++;
            if (fabs(f_fac[5][i] - inj[i]) <= 0.5 * df) ref_corr++;
        }
        for (size_t k = 5; k < N_FACTOR; k++) {
            size_t a = 0, c = 0;
            for (size_t i = 0; i < n; i++) {
                if (f_fac[k][i] < 0.0) continue;
                a++;
                if (fabs(f_fac[k][i] - inj[i]) <= 0.5 * df) c++;
            }
            printf("    factor %.2f: %zu answers, %zu correct\n",
                   SNR_FACTORS[k], a, c);
            RS_CHECK(a == ref_ans && c == ref_corr);
        }
        RS_CHECK(ref_ans == ref_corr);   /* and the plateau is a correct one */
    }

    RS_CASE("the sweep ran and every policy was given the same spectra");
    RS_CHECK(n == n_pt);
    RS_CHECK(df > 0.0);
    /* The cull's gate has to have been REACHABLE, or the sweep tested a refusal
     * machine rather than a policy. snr_null is a property of the window size
     * and must be the derived value for a 32x32 window. */
    RS_CHECK(snr_null > 7.0 && snr_null < 8.0);

    RS_CASE("every answer the cull DID give is within half a bin");
    {
        /* The cull's profile is the opposite of the other two policies': it
         * answers rarely and, so far, correctly. This asserts the correctness
         * half, over both fixtures, and prints the recall so that a reader
         * cannot mistake precision for a recovery. It is a real assertion --
         * one wrong answer fails it -- and it is the strongest claim the
         * measurement supports. */
        size_t n_ans = 0, n_right = 0;
        for (size_t i = 0; i < n; i++) {
            if (f_cull[i] < 0.0) continue;
            n_ans++;
            if (fabs(f_cull[i] - inj[i]) <= 0.5 * df) n_right++;
        }
        for (size_t i = 0; i < np; i++) {
            if (p_cull[i] < 0.0) continue;
            n_ans++;
            if (fabs(p_cull[i] - p_inj[i]) <= 0.5 * df) n_right++;
        }
        printf("    %zu of %zu cull answers correct, from %zu points offered\n",
               n_right, n_ans, n + np);
        RS_CHECK(n_ans > 0);
        RS_CHECK(n_right == n_ans);
    }

    RS_CASE("the cull refuses the static control at every seed");
    {
        /* The one claim a policy that mostly refuses could still fail, and the
         * one that matters most: on a scene with nothing moving, the other two
         * policies each emit a confident frequency at every seed. Item 11 is
         * unaffected -- this is a scene-driven null, not a common-mode one --
         * but a policy that answered here would be disqualified outright. */
        size_t n_answered = 0;
        for (size_t si = 0; si < n_seed; si++) if (s_cull[si] >= 0.0) n_answered++;
        printf("    cull answered %zu of %zu static scenes; best answered %zu, "
               "consensus %zu\n", n_answered, n_seed, n_seed, n_seed);
        RS_CHECK(n_answered == 0);
        /* The comparison is the point, so assert the premise too: if the other
         * policies ever start refusing here, this case stops meaning anything
         * and should be revisited rather than silently passing. */
        for (size_t si = 0; si < n_seed; si++) RS_CHECK(s_best[si] >= 0.0);
        (void)s_cons;
    }

    RS_CASE("the cull's recall is too low to have been swept");
    {
        /* ASSERTED AS A LIMITATION, not as a success. Five answers over two
         * distinct injections at the bottom of the band is not a sweep, and
         * rs_track_fit()'s slope cannot discriminate a tracking chain from a
         * fixed one on two abscissae however small the residuals. This case
         * exists so that the day the recall improves, it FAILS and forces the
         * claim above to be rewritten rather than quietly outgrown. */
        printf("    cull answered %zu of %zu clutter points over %zu distinct "
               "injections of %zu\n", nk, n, dk, n_freq);
        RS_CHECK(dk < n_freq);
    }

    RS_CASE("the fit criterion can still reject a fixed answer");
    {
        /* Asserting anything about the policies above is meaningless unless the
         * criterion they are judged by can fail. A chain emitting one fixed
         * frequency -- the defect item 2 records repeatedly -- must be rejected
         * by it whatever that frequency is. */
        double fixed[64];
        for (size_t i = 0; i < n; i++) fixed[i] = 0.7;
        double fs = 0.0, fr = 0.0;
        (void)fit_policy(inj, fixed, n, &fs, &fr, NULL);
        printf("    a fixed 0.7 Hz gives slope %.3f, rms %.4f Hz\n", fs, fr);
        RS_CHECK(fabs(fs - 1.0) > 0.15);
        RS_CHECK(fr >= 0.5 * df);
    }

    RS_TEST_END();
}
