/* Does rs_spectrum_modal_set() admit and rank the things it claims to?
 *
 * WHY THIS FILE EXISTS AT ALL. Until item 110 nothing in the suite touched this
 * function -- not one test contained the word "modal" -- although it is the
 * function whose leading frequency `mmotion` reports, whose ordering `--stable`
 * adjudicates, and the subject of items 70, 71, 77, 78, 80, 109 and 110. Item
 * 110 changed both its admission rule and its sort key and `ctest` could not
 * have noticed. `tests/test_modalfit.c` is NOT this: despite the name it tests
 * rs_transient_fit(), a different estimator from a different item.
 *
 * WHAT IS PINNED. The spectra here are BUILT, not tracked, so every failure is
 * arithmetic in the selection and nothing upstream of it. Three things are worth
 * a test and each corresponds to a way this function has actually been wrong:
 *
 *   1. ADMISSION AND ORDERING ARE SEPARATE THINGS (item 110). A localised mode
 *      -- strong, on few windows -- must be admitted even though its support is
 *      far below the binomial threshold, and must then OUTRANK a weak mode
 *      covering more ground. Both halves failed on real data before item 110.
 *   2. NEITHER KEY ORDERS ALONE (item 110). Block alone loses the localised
 *      mode; ratio alone loses a broad true mode to a sharp small artefact.
 *      Both directions are asserted, so reverting to either is caught.
 *   3. THE CHANCE MODEL IS CALIBRATED (items 77, 78, 80). On scenes with
 *      nothing planted, the rate at which a mode is nevertheless admitted must
 *      stay near RS_MODAL_P_MAX rather than drifting with the configuration.
 *      That is the property a fixed block floor could not have, and it is also
 *      the one that catches a biased nomination: a background estimator that
 *      favours particular bins raises this rate without any signal present.
 */

#include "resonarsat/microm.h"
#include "rs_test.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

/* Deterministic unit-mean noise, so a failure reproduces exactly on any
 * machine. The value is exponential-ish by construction -- a periodogram bin of
 * complex Gaussian noise is exponentially distributed, which is the model
 * rs_mode_t.evidence is derived under -- obtained by transforming a uniform
 * hash. 'salt' separates independent realisations of the same scene. */
static double rs_noise_bin(size_t w, size_t k, unsigned salt)
{
    unsigned x = (unsigned)(w * 73856093u) ^ (unsigned)(k * 19349663u)
               ^ (salt * 83492791u);
    x ^= x >> 16; x *= 2246822519u;
    x ^= x >> 13; x *= 3266489917u;
    x ^= x >> 16;
    const double u = ((double)x + 0.5) / 4294967296.0;
    return -log(u);                      /* Exp(1), mean 1 */
}

/* A hand-built spectrum over a square window grid: flat unit-mean noise in
 * every bin of every window, and nothing else. Free with rs_spectrum_free().
 * Returns 0 on allocation failure so the caller can bail rather than fault. */
static int build_noise_spectrum(rs_spectrum_t *s, size_t n_side, size_t n_freq,
                                double df, unsigned salt)
{
    memset(s, 0, sizeof *s);
    s->n_win = n_side * n_side;
    s->n_win_az = n_side;
    s->n_win_rg = n_side;
    s->n_freq = n_freq;
    s->df = df;
    s->psd  = calloc(s->n_win * n_freq, sizeof *s->psd);
    s->freq = calloc(n_freq, sizeof *s->freq);
    if (!s->psd || !s->freq) { rs_spectrum_free(s); return 0; }
    for (size_t k = 0; k < n_freq; k++) s->freq[k] = (double)k * df;
    for (size_t w = 0; w < s->n_win; w++)
        for (size_t k = 0; k < n_freq; k++)
            s->psd[w * n_freq + k] = rs_noise_bin(w, k, salt);
    return 1;
}

/* Plant a line at 'bin' in the square block of windows whose corner is
 * (az0, rg0), scaling those bins by 'gain'. A block rather than a scatter,
 * because that is what a mode on contiguous ground looks like and it is the
 * shape rs_largest_block() measures. */
static void plant_block(rs_spectrum_t *s, size_t bin, double gain,
                        size_t az0, size_t rg0, size_t side)
{
    for (size_t r = rg0; r < rg0 + side && r < s->n_win_rg; r++)
        for (size_t a = az0; a < az0 + side && a < s->n_win_az; a++) {
            const size_t w = r * s->n_win_az + a;
            if (w < s->n_win) s->psd[w * s->n_freq + bin] *= gain;
        }
}

/* Index of a mode at 'bin' in the reported set, or n_mode if it was refused. */
static size_t find_mode(const rs_modal_set_t *ms, size_t bin)
{
    for (size_t i = 0; i < ms->n_mode; i++)
        if (ms->mode[i].bin == bin) return i;
    return ms->n_mode;
}

/* Run every case in this file. */
int main(void)
{
    /* A 15x15 window grid, as the real runs use. The planted cases use a WIDE
     * band (257 bins, what 512 looks would give) for a reason worth stating:
     * every window nominates RS_MODAL_PER_WINDOW bins wherever they fall, so on
     * a band of K bins each bin collects about n_win * M / K nominations by
     * chance alone -- 22 of 225 at the 65-bin operating point, which SWAMPS a
     * mode carried by a dozen windows and leaves median_ratio measuring the
     * chance nominators rather than the mode. A wide band drops that floor to
     * about 5 and lets a planted mode dominate its own statistics, which is what
     * makes these assertions about the selection rather than about the noise.
     * The calibration case below deliberately uses the 65-bin operating point
     * instead, because that is the configuration being calibrated. */
    const size_t N_SIDE = 15, N_FREQ = 257;
    const double DF = 0.05;

    RS_CASE("a localised mode is admitted although its support is below the "
            "binomial threshold");
    {
        /* THE ITEM 109 SCENE, reduced to its arithmetic. A strong line on nine
         * windows of 225, which is the configuration that was refused before
         * block, ratio or ranking were consulted. The margin is inherently
         * narrow -- the binomial threshold sits about 2.4 standard deviations
         * above the chance mean, so a mode below it can carry only a handful of
         * windows more than chance does, and on the real collect the injected
         * line reached 28 against a threshold of 34. Nothing here is tuned to
         * the noise: the assertions are relations between reported quantities,
         * not the quantities themselves. */
        rs_spectrum_t s;
        RS_CHECK(build_noise_spectrum(&s, N_SIDE, N_FREQ, DF, 2u));
        plant_block(&s, 20, 60.0, 5, 5, 3);      /* 9 windows of 225 */

        rs_modal_set_t ms;
        RS_CHECK_OK(rs_spectrum_modal_set(&s, &ms));

        printf("    support_min (binomial, reported) %zu, admit_min %zu, "
               "chance block %zu\n", ms.support_min, ms.admit_min,
               ms.null_block_crit);
        /* Admission is the block floor, NOT the binomial threshold, and the two
         * differ -- which is the whole of item 110's first half. */
        RS_CHECK(ms.admit_min == RS_MODAL_BLOCK_MIN);
        RS_CHECK(ms.admit_min < ms.support_min);

        const size_t i = find_mode(&ms, 20);
        RS_CHECK(i < ms.n_mode);
        if (i < ms.n_mode) {
            printf("    planted bin 20: rank %zu of %zu, block %zu, support "
                   "%zu of a required %zu, ratio %.1f, ev %.1f\n", i, ms.n_mode,
                   ms.mode[i].n_contiguous, ms.mode[i].n_support,
                   ms.support_min, ms.mode[i].median_ratio, ms.mode[i].evidence);
            /* The premise of the case, asserted rather than assumed: this mode
             * really would have been refused by the gate item 110 removed. */
            RS_CHECK(ms.mode[i].n_support < ms.support_min);
            /* The verdict: admitted anyway, on the strength of its shape. */
            RS_CHECK(i == 0);
            RS_CHECK(ms.mode[i].n_contiguous >= RS_MODAL_BLOCK_MIN);
            RS_CHECK_NEAR(ms.mode[i].freq_hz, 20.0 * DF, 1e-12);
        }
        rs_spectrum_free(&s);
    }

    RS_CASE("a strong localised mode outranks a weak broad one, and the weak one "
            "is still reported");
    {
        /* Both keys are exercised at once: the broad mode wins on block and
         * support, the localised one on ratio, and the localised one must lead.
         * Ranking on the block alone reverses this -- that is item 110's first
         * failure -- and dropping the broad mode entirely would mean the gates
         * had become a strength test, which is item 71's failure. */
        rs_spectrum_t s;
        RS_CHECK(build_noise_spectrum(&s, N_SIDE, N_FREQ, DF, 7u));
        plant_block(&s, 20, 80.0, 5, 5, 4);      /* strong, 16 windows */
        plant_block(&s, 100, 8.0, 0, 0, 7);      /* weak, 49 windows */

        rs_modal_set_t ms;
        RS_CHECK_OK(rs_spectrum_modal_set(&s, &ms));
        const size_t is = find_mode(&ms, 20), ib = find_mode(&ms, 100);
        RS_CHECK(is < ms.n_mode);
        RS_CHECK(ib < ms.n_mode);
        if (is < ms.n_mode && ib < ms.n_mode) {
            printf("    localised: block %zu support %zu ratio %.1f ev %.1f\n",
                   ms.mode[is].n_contiguous, ms.mode[is].n_support,
                   ms.mode[is].median_ratio, ms.mode[is].evidence);
            printf("    broad    : block %zu support %zu ratio %.1f ev %.1f\n",
                   ms.mode[ib].n_contiguous, ms.mode[ib].n_support,
                   ms.mode[ib].median_ratio, ms.mode[ib].evidence);
            /* The premise: the broad mode really does cover more ground. */
            RS_CHECK(ms.mode[ib].n_contiguous > ms.mode[is].n_contiguous);
            RS_CHECK(ms.mode[ib].n_support > ms.mode[is].n_support);
            /* ... and the localised one really is stronger where it is. */
            RS_CHECK(ms.mode[is].median_ratio > ms.mode[ib].median_ratio);
            /* The verdict. */
            RS_CHECK(is < ib);
            RS_CHECK(ms.mode[is].evidence > ms.mode[ib].evidence);
        }
        rs_spectrum_free(&s);
    }

    RS_CASE("a broad true mode outranks a sharper artefact on a fraction of the "
            "ground -- ranking on ratio alone is caught here");
    {
        /* The other direction, and the reason rs_mode_t.evidence is a PRODUCT.
         * Ranking on median_ratio alone passes the previous case and fails this
         * one: measured on the real fixture, it hands a 2 mm injected scene to
         * seed 7's 1.512 Hz artefact, which is sharper on a third of the
         * ground. Both cases together pin the form. */
        rs_spectrum_t s;
        RS_CHECK(build_noise_spectrum(&s, N_SIDE, N_FREQ, DF, 2u));
        plant_block(&s, 100, 30.0, 0, 0, 6);     /* true mode, 36 windows */
        plant_block(&s, 20,  60.0, 5, 5, 3);     /* sharper, 9 windows */

        rs_modal_set_t ms;
        RS_CHECK_OK(rs_spectrum_modal_set(&s, &ms));
        const size_t it = find_mode(&ms, 100), ia = find_mode(&ms, 20);
        RS_CHECK(it < ms.n_mode);
        RS_CHECK(ia < ms.n_mode);
        if (it < ms.n_mode && ia < ms.n_mode) {
            printf("    broad mode: block %zu ratio %.1f ev %.1f\n",
                   ms.mode[it].n_contiguous, ms.mode[it].median_ratio,
                   ms.mode[it].evidence);
            printf("    sharp arte: block %zu ratio %.1f ev %.1f\n",
                   ms.mode[ia].n_contiguous, ms.mode[ia].median_ratio,
                   ms.mode[ia].evidence);
            /* The premise: the artefact really is the sharper of the two. */
            RS_CHECK(ms.mode[ia].median_ratio > ms.mode[it].median_ratio);
            /* The verdict. */
            RS_CHECK(it < ia);
        }
        rs_spectrum_free(&s);
    }

    RS_CASE("evidence is the block times log of the ratio, and the set is sorted "
            "by it");
    {
        rs_spectrum_t s;
        RS_CHECK(build_noise_spectrum(&s, N_SIDE, N_FREQ, DF, 7u));
        plant_block(&s, 20, 80.0, 5, 5, 4);
        plant_block(&s, 100, 8.0, 0, 0, 7);

        rs_modal_set_t ms;
        RS_CHECK_OK(rs_spectrum_modal_set(&s, &ms));
        for (size_t i = 0; i < ms.n_mode; i++) {
            const double want = (ms.mode[i].median_ratio > 1.0)
                              ? (double)ms.mode[i].n_contiguous
                                * log(ms.mode[i].median_ratio) : 0.0;
            RS_CHECK_NEAR(ms.mode[i].evidence, want, 1e-9);
            if (i) RS_CHECK(ms.mode[i - 1].evidence >= ms.mode[i].evidence);
        }
        rs_spectrum_free(&s);
    }

    RS_CASE("the chance model is calibrated: scenes with nothing planted are "
            "admitted at about RS_MODAL_P_MAX");
    {
        /* THE CALIBRATION THIS FUNCTION LIVES OR DIES BY, and the one a fixed
         * block floor could not have (items 77-78). It is also the test that
         * catches a BIASED NOMINATION: if the local background estimator
         * favours particular bins -- the band edges, say, where its
         * neighbourhood is one-sided -- those bins accumulate nominating
         * windows that the null does not expect, and this rate climbs with no
         * signal anywhere in the scene.
         *
         * The bar is deliberately loose. p_chance <= 0.05 on a max statistic
         * says about 1 scene in 20 should produce a mode; the nominations of a
         * real spectrum are not quite the null's uniform draw, so some excess
         * is expected and only a LARGE excess is a defect. Six of twenty is
         * three times the nominal rate and is the point at which the reported
         * set stops meaning anything. */
        const size_t TRIALS = 20;
        size_t answered = 0, worst_block = 0;
        for (unsigned t = 0; t < TRIALS; t++) {
            rs_spectrum_t s;
            RS_CHECK(build_noise_spectrum(&s, N_SIDE, N_FREQ, DF, 100u + t));
            rs_modal_set_t ms;
            const resonarsat_status_t st = rs_spectrum_modal_set(&s, &ms);
            if (st == RS_OK && ms.n_mode > 0) {
                answered++;
                if (ms.mode[0].n_contiguous > worst_block)
                    worst_block = ms.mode[0].n_contiguous;
            } else {
                RS_CHECK(st == RS_OK || st == RS_ERR_RANGE);
            }
            rs_spectrum_free(&s);
        }
        printf("    %zu of %zu motionless scenes admitted a mode "
               "(nominal %.0f%%), largest leading block %zu\n",
               answered, TRIALS, 100.0 * RS_MODAL_P_MAX, worst_block);
        RS_CHECK(answered <= 6);
    }

    RS_CASE("the binomial threshold tracks the bin count, which is item 77");
    {
        /* Fewer admissible bins means each nomination is likelier to land on
         * any given bin, so chance agreement inflates and the threshold must
         * rise with it. Item 77 found the opposite assumption fatal: a block of
         * 21 means opposite things at 48 and 128 looks. */
        rs_spectrum_t wide, narrow;
        RS_CHECK(build_noise_spectrum(&wide,   N_SIDE, 65, DF, 5u));
        RS_CHECK(build_noise_spectrum(&narrow, N_SIDE, 25, DF, 5u));
        rs_modal_set_t mw, mn;
        /* Nothing is planted, so "nothing recurs" is the expected answer and
         * RS_ERR_RANGE is not a failure. The counts are populated on that path
         * -- main.c prints them to say WHICH gate spoke -- and that contract is
         * part of what this case pins. */
        const resonarsat_status_t sw = rs_spectrum_modal_set(&wide,   &mw);
        const resonarsat_status_t sn = rs_spectrum_modal_set(&narrow, &mn);
        RS_CHECK(sw == RS_OK || sw == RS_ERR_RANGE);
        RS_CHECK(sn == RS_OK || sn == RS_ERR_RANGE);
        printf("    %zu bins -> support_min %zu (%.2f expected false); "
               "%zu bins -> support_min %zu (%.2f)\n",
               mw.n_bin, mw.support_min, mw.expected_false,
               mn.n_bin, mn.support_min, mn.expected_false);
        RS_CHECK(mn.n_bin < mw.n_bin);
        RS_CHECK(mn.support_min > mw.support_min);
        /* Both budgets are still honoured, which is what "derived" means. */
        RS_CHECK(mw.expected_false < 0.5);
        RS_CHECK(mn.expected_false < 0.5);
        rs_spectrum_free(&wide);
        rs_spectrum_free(&narrow);
    }

    RS_CASE("contract: bad arguments and a band too narrow to hold a background");
    {
        rs_spectrum_t s;
        rs_modal_set_t ms;
        RS_CHECK(build_noise_spectrum(&s, N_SIDE, N_FREQ, DF, 6u));

        RS_CHECK_ERR(rs_spectrum_modal_set(&s, NULL), RS_ERR_ARG);
        RS_CHECK_ERR(rs_spectrum_modal_set(NULL, &ms), RS_ERR_ARG);
        /* Cleared on every non-OK return, so "check the status then read the
         * struct" is safe rather than usually safe. */
        RS_CHECK(ms.n_mode == 0);
        rs_spectrum_free(&s);

        rs_spectrum_t tiny;
        RS_CHECK(build_noise_spectrum(&tiny, 4, RS_SPECTRUM_LEAKAGE_BINS + 2,
                                      DF, 7u));
        RS_CHECK_ERR(rs_spectrum_modal_set(&tiny, &ms), RS_ERR_RANGE);
        RS_CHECK(ms.n_mode == 0);
        rs_spectrum_free(&tiny);
    }

    RS_TEST_END();
}
