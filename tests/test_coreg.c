/* Sub-pixel offset tracking: can it recover a known shift?
 *
 * Everything the micro-motion stage reports rests on this primitive locating a
 * correlation peak to a fraction of a pixel, so the tolerances here are the
 * tightest in the suite. */

#include "resonarsat/coreg.h"
#include "rs_test.h"

#include <math.h>
#include <stdlib.h>

/* Fill a patch with a Gaussian blob centred at (cy, cx). Sub-pixel centres are
 * representable because the Gaussian is evaluated analytically. */
static void make_patch(float complex *p, size_t n, double cy, double cx, double sigma)
{
    for (size_t a = 0; a < n; a++) {
        for (size_t r = 0; r < n; r++) {
            const double dy = ((double)a - cy) / sigma;
            const double dx = ((double)r - cx) / sigma;
            p[a * n + r] = (float)exp(-0.5 * (dy * dy + dx * dx));
        }
    }
}

/* Remove the mean, as rs_coreg_extract() does for real patches. */
static void demean(float complex *p, size_t n)
{
    float complex s = 0.0f;
    for (size_t i = 0; i < n * n; i++) s += p[i];
    const float complex m = s / (float)(n * n);
    for (size_t i = 0; i < n * n; i++) p[i] -= m;
}

/* Run every case in this file. */
int main(void)
{
    const size_t n = 32;
    const double sigma = 2.0;
    float complex *ref = malloc(n * n * sizeof *ref);
    float complex *cur = malloc(n * n * sizeof *cur);

    /* Integer shifts first: if these fail nothing else is meaningful. */
    RS_CASE("integer shifts recovered exactly");
    {
        const double shifts[][2] = { {0,0}, {1,0}, {0,1}, {2,-3}, {-2,2} };
        for (size_t i = 0; i < sizeof shifts / sizeof shifts[0]; i++) {
            make_patch(ref, n, 16.0, 16.0, sigma);
            make_patch(cur, n, 16.0 + shifts[i][0], 16.0 + shifts[i][1], sigma);
            demean(ref, n); demean(cur, n);

            double sa = 0, sr = 0, pk = 0;
            RS_CHECK_OK(rs_coreg_shift(ref, cur, n, n, 10, 20, &sa, &sr, &pk));
            RS_CHECK_NEAR(sa, shifts[i][0], 0.2);
            RS_CHECK_NEAR(sr, shifts[i][1], 0.2);
            RS_CHECK(pk > 0.5);
        }
    }

    /* Sub-pixel shifts are the real requirement: structural vibration moves a
     * scatterer by a small fraction of a resolution cell, so a tracker that
     * only resolves whole pixels measures nothing. */
    RS_CASE("sub-pixel shifts recovered to better than 0.15 px");
    {
        const double shifts[] = { 0.25, 0.5, -0.4, 1.3, -1.75 };
        for (size_t i = 0; i < sizeof shifts / sizeof shifts[0]; i++) {
            make_patch(ref, n, 16.0, 16.0, sigma);
            make_patch(cur, n, 16.0 + shifts[i], 16.0, sigma);
            demean(ref, n); demean(cur, n);

            double sa = 0, sr = 0, pk = 0;
            RS_CHECK_OK(rs_coreg_shift(ref, cur, n, n, 20, 20, &sa, &sr, &pk));
            RS_CHECK_NEAR(sa, shifts[i], 0.15);
            RS_CHECK_NEAR(sr, 0.0, 0.15);
        }
    }

    RS_CASE("identical patches correlate at unity");
    {
        make_patch(ref, n, 16.0, 16.0, sigma);
        demean(ref, n);
        double sa = 0, sr = 0, pk = 0;
        RS_CHECK_OK(rs_coreg_shift(ref, ref, n, n, 10, 10, &sa, &sr, &pk));
        RS_CHECK_NEAR(pk, 1.0, 0.05);
    }

    /* A blank patch must mask itself out rather than divide by zero. */
    RS_CASE("degenerate patches yield zero peak, not a crash");
    {
        for (size_t i = 0; i < n * n; i++) { ref[i] = 0.0f; cur[i] = 0.0f; }
        double sa = 1.0, sr = 1.0, pk = 1.0;
        RS_CHECK_OK(rs_coreg_shift(ref, cur, n, n, 10, 10, &sa, &sr, &pk));
        RS_CHECK_NEAR(pk, 0.0, 1e-9);
        RS_CHECK_NEAR(sa, 0.0, 1e-9);
    }

    RS_CASE("null arguments are refused");
    {
        double sa, sr, pk;
        RS_CHECK_ERR(rs_coreg_shift(NULL, cur, n, n, 1, 1, &sa, &sr, &pk), RS_ERR_ARG);
        RS_CHECK_ERR(rs_coreg_shift(ref, cur, 0, n, 1, 1, &sa, &sr, &pk), RS_ERR_ARG);
    }

    /* --no-optimize's audit path. The whole value of a baseline is that it agrees
     * with the fast path on cases where the fast path is known to be right, so
     * that a disagreement elsewhere means something. Tested to one lattice step
     * rather than exactly: the two search the same lattice but arrive by
     * different arithmetic -- a padded transform against a direct summation --
     * and single-precision rounding can put the crest one step either way when
     * two neighbouring points are within an ulp of each other. */
    RS_CASE("exhaustive search agrees with local refinement on clean shifts");
    {
        const size_t up = 20;
        const double tol = 1.5 / (double)up;
        const double shifts[][2] = { {0,0}, {1,0}, {0,1}, {2,-3}, {0.25,0},
                                     {-0.4,0.5}, {1.3,-1.75} };
        for (size_t i = 0; i < sizeof shifts / sizeof shifts[0]; i++) {
            make_patch(ref, n, 16.0, 16.0, sigma);
            make_patch(cur, n, 16.0 + shifts[i][0], 16.0 + shifts[i][1], sigma);
            demean(ref, n); demean(cur, n);

            double la = 0, lr = 0, lpk = 0, xa = 0, xr = 0, xpk = 0;
            RS_CHECK_OK(rs_coreg_shift_ex(ref, cur, n, n, up, up,
                                          RS_COREG_REFINE_LOCAL, &la, &lr, &lpk));
            RS_CHECK_OK(rs_coreg_shift_ex(ref, cur, n, n, up, up,
                                          RS_COREG_REFINE_EXHAUSTIVE, &xa, &xr, &xpk));

            /* Against the truth, so a bug that moved both paths equally cannot
             * pass by having them agree with each other. */
            RS_CHECK_NEAR(xa, shifts[i][0], 0.15);
            RS_CHECK_NEAR(xr, shifts[i][1], 0.15);
            RS_CHECK_NEAR(xa, la, tol);
            RS_CHECK_NEAR(xr, lr, tol);
            /* The normalisations must match too, or the coherence mask would
             * threshold two different quantities depending on the mode. */
            RS_CHECK_NEAR(xpk, lpk, 0.02);
        }
    }

    /* The exhaustive path must land on the same 1/upsample lattice as the local
     * one. A padding or index-unwrap error would still produce plausible shifts,
     * just off the lattice -- which is invisible in a tolerance check. */
    RS_CASE("exhaustive shifts land on the 1/upsample lattice");
    {
        const size_t up = 8;
        make_patch(ref, n, 16.0, 16.0, sigma);
        make_patch(cur, n, 16.0 + 0.375, 16.0 - 1.25, sigma);
        demean(ref, n); demean(cur, n);

        double sa = 0, sr = 0, pk = 0;
        RS_CHECK_OK(rs_coreg_shift_ex(ref, cur, n, n, up, up,
                                      RS_COREG_REFINE_EXHAUSTIVE, &sa, &sr, &pk));
        RS_CHECK_NEAR(sa * (double)up, round(sa * (double)up), 1e-9);
        RS_CHECK_NEAR(sr * (double)up, round(sr * (double)up), 1e-9);
    }

    /* A degenerate patch must mask itself out on this path too. The exhaustive
     * branch returns before the search, so this checks the early exit is shared
     * rather than duplicated and forgotten. */
    RS_CASE("exhaustive search masks out a blank patch");
    {
        for (size_t i = 0; i < n * n; i++) { ref[i] = 0.0f; cur[i] = 0.0f; }
        double sa = 1.0, sr = 1.0, pk = 1.0;
        RS_CHECK_OK(rs_coreg_shift_ex(ref, cur, n, n, 10, 10,
                                      RS_COREG_REFINE_EXHAUSTIVE, &sa, &sr, &pk));
        RS_CHECK_NEAR(pk, 0.0, 1e-9);
        RS_CHECK_NEAR(sa, 0.0, 1e-9);
        RS_CHECK_NEAR(sr, 0.0, 1e-9);
    }

    /* An impossible surface must be refused with a described error rather than
     * attempted. The failure mode this guards is a tracker that returns a
     * complete result with every window zero. */
    RS_CASE("an oversized exhaustive surface is refused");
    {
        RS_CHECK_ERR(rs_coreg_surface_check(4096, 4096, 64, 64), RS_ERR_RANGE);
        RS_CHECK_OK(rs_coreg_surface_check(32, 32, 20, 20));

        double sa, sr, pk;
        RS_CHECK_ERR(rs_coreg_shift_ex(ref, cur, n, n, 4096, 4096,
                                       RS_COREG_REFINE_EXHAUSTIVE, &sa, &sr, &pk),
                     RS_ERR_RANGE);
    }

    /* ------------------------------------------------------------------
     * The ampcor-style surface statistics.
     *
     * The bar these have to clear is that they SEPARATE cases the peak value
     * does not. A statistic that merely tracks the coherence adds nothing, so
     * each case below pins the statistic against a control where the peak value
     * alone would give the wrong answer or the same one.
     * ------------------------------------------------------------------ */

    RS_CASE("a clean peak scores far above the noise-alone SNR");
    {
        /* The null is a property of the window size and nothing else, so it can
         * be checked directly before anything is correlated against it. */
        const double null32 = rs_coreg_snr_null(32, 32);
        printf("    null SNR at 32x32: %.2f (ln 1024 + gamma = %.2f)\n",
               null32, log(1024.0) + 0.5772156649);
        RS_CHECK_NEAR(null32, log(1024.0) + 0.5772156649 + 0.5 / 1024.0, 1e-6);
        /* Monotone in the bin count, which is what makes it usable as a scale:
         * a bigger window has more chances to throw up a large bin. */
        RS_CHECK(rs_coreg_snr_null(64, 64) > null32);

        make_patch(ref, n, 16.0, 16.0, sigma);
        make_patch(cur, n, 16.4, 16.0, sigma);
        demean(ref, n); demean(cur, n);

        double sa, sr;
        rs_coreg_quality_t q;
        RS_CHECK_OK(rs_coreg_shift_q(ref, cur, n, n, 10, 20,
                                     RS_COREG_REFINE_LOCAL, &sa, &sr, &q));
        printf("    clean pair: peak %.4f, snr %.1f, sigma_az %.4f px\n",
               q.peak, q.snr, q.sigma_az_px);
        RS_CHECK(q.peak > 0.9);
        /* Well clear of the gate the selection stage applies, which is twice
         * the null. If this ever fails the gate is unreachable and the whole
         * policy is a refusal machine. */
        RS_CHECK(q.snr > 2.0 * null32);
        /* A sharp, coherent peak must be located to well under a pixel. */
        RS_CHECK(q.sigma_az_px < 0.5);
        RS_CHECK(q.sigma_rg_px < 0.5);
    }

    RS_CASE("decorrelated patches fall to the noise-alone SNR");
    {
        /* Two independent noise fields. The peak value is low here and so is
         * the SNR -- this is the easy direction, and it is checked so that the
         * hard direction below has a control. */
        unsigned s1 = 12345u, s2 = 99991u;
        for (size_t i = 0; i < n * n; i++) {
            s1 = s1 * 1103515245u + 12345u;
            s2 = s2 * 1103515245u + 12345u;
            ref[i] = (float)((double)(s1 >> 16) / 32768.0 - 1.0);
            cur[i] = (float)((double)(s2 >> 16) / 32768.0 - 1.0);
        }
        demean(ref, n); demean(cur, n);

        double sa, sr;
        rs_coreg_quality_t q;
        RS_CHECK_OK(rs_coreg_shift_q(ref, cur, n, n, 10, 20,
                                     RS_COREG_REFINE_LOCAL, &sa, &sr, &q));
        printf("    decorrelated: peak %.4f, snr %.1f (null %.1f)\n",
               q.peak, q.snr, rs_coreg_snr_null(n, n));
        /* Within a small factor of the derived null, which is the claim that
         * makes the null usable as a threshold at all. */
        RS_CHECK(q.snr < 3.0 * rs_coreg_snr_null(n, n));
    }

    RS_CASE("a second lobe lowers the SNR at an unchanged peak value");
    {
        /* THE CASE THIS WHOLE STATISTIC EXISTS FOR. Both pairs below correlate
         * about equally well, so the peak value cannot tell them apart. One has
         * a single maximum; the other has a rival five pixels away, which is
         * exactly the configuration that moves an offset without lowering the
         * coherence. Only the SNR sees it. */
        double sa, sr;
        rs_coreg_quality_t q_one, q_two;

        make_patch(ref, n, 16.0, 16.0, sigma);
        make_patch(cur, n, 16.3, 16.0, sigma);
        demean(ref, n); demean(cur, n);
        RS_CHECK_OK(rs_coreg_shift_q(ref, cur, n, n, 10, 20,
                                     RS_COREG_REFINE_LOCAL, &sa, &sr, &q_one));

        /* The same blob plus a near-equal companion, in BOTH patches, so the
         * two remain as alike as before -- the change is to the surface's
         * shape, not to how well the patches match. */
        float complex *tmp = malloc(n * n * sizeof *tmp);
        RS_CHECK(tmp != NULL);
        make_patch(ref, n, 16.0, 16.0, sigma);
        make_patch(tmp, n, 16.0, 21.0, sigma);
        for (size_t i = 0; i < n * n; i++) ref[i] += 0.95f * tmp[i];
        make_patch(cur, n, 16.3, 16.0, sigma);
        make_patch(tmp, n, 16.3, 21.0, sigma);
        for (size_t i = 0; i < n * n; i++) cur[i] += 0.95f * tmp[i];
        free(tmp);
        demean(ref, n); demean(cur, n);
        RS_CHECK_OK(rs_coreg_shift_q(ref, cur, n, n, 10, 20,
                                     RS_COREG_REFINE_LOCAL, &sa, &sr, &q_two));

        printf("    one lobe:  peak %.4f, snr %.1f\n", q_one.peak, q_one.snr);
        printf("    two lobes: peak %.4f, snr %.1f\n", q_two.peak, q_two.snr);
        /* The premise: the peak value really does fail to separate them. If
         * this stops holding the case is no longer testing what it claims. */
        RS_CHECK(fabs(q_two.peak - q_one.peak) < 0.05);
        /* And the claim: the SNR does separate them. */
        RS_CHECK(q_two.snr < q_one.snr);
    }

    RS_CASE("a broad peak is reported as less certain than a sharp one");
    {
        double sa, sr;
        rs_coreg_quality_t q_sharp, q_broad;

        make_patch(ref, n, 16.0, 16.0, 1.2);
        make_patch(cur, n, 16.3, 16.0, 1.2);
        demean(ref, n); demean(cur, n);
        RS_CHECK_OK(rs_coreg_shift_q(ref, cur, n, n, 10, 20,
                                     RS_COREG_REFINE_LOCAL, &sa, &sr, &q_sharp));

        make_patch(ref, n, 16.0, 16.0, 5.0);
        make_patch(cur, n, 16.3, 16.0, 5.0);
        demean(ref, n); demean(cur, n);
        RS_CHECK_OK(rs_coreg_shift_q(ref, cur, n, n, 10, 20,
                                     RS_COREG_REFINE_LOCAL, &sa, &sr, &q_broad));

        printf("    sharp: peak %.4f, sigma_az %.4f px\n",
               q_sharp.peak, q_sharp.sigma_az_px);
        printf("    broad: peak %.4f, sigma_az %.4f px\n",
               q_broad.peak, q_broad.sigma_az_px);
        /* Both correlate essentially perfectly -- the patches differ only by a
         * shift -- so once again the peak value says nothing and the curvature
         * says what matters. */
        RS_CHECK(q_sharp.peak > 0.9 && q_broad.peak > 0.9);
        RS_CHECK(q_broad.sigma_az_px > q_sharp.sigma_az_px);
    }

    RS_CASE("sigma does not depend on the upsample factor");
    {
        /* The curvature baseline is fixed in PIXELS for this reason: a baseline
         * of one lattice step would make the same surface report different
         * uncertainties at different refinement settings, which would silently
         * change every cull when --upsample changed. See
         * RS_COREG_CURV_BASELINE_PX. */
        make_patch(ref, n, 16.0, 16.0, sigma);
        make_patch(cur, n, 16.3, 16.0, sigma);
        demean(ref, n); demean(cur, n);

        double sa, sr;
        rs_coreg_quality_t q10, q100;
        RS_CHECK_OK(rs_coreg_shift_q(ref, cur, n, n, 10, 10,
                                     RS_COREG_REFINE_LOCAL, &sa, &sr, &q10));
        RS_CHECK_OK(rs_coreg_shift_q(ref, cur, n, n, 100, 100,
                                     RS_COREG_REFINE_LOCAL, &sa, &sr, &q100));
        printf("    sigma_az at 1/10 px: %.5f, at 1/100 px: %.5f\n",
               q10.sigma_az_px, q100.sigma_az_px);
        RS_CHECK(q10.sigma_az_px > 0.0 && q100.sigma_az_px > 0.0);
        RS_CHECK(fabs(q100.sigma_az_px - q10.sigma_az_px) <
                 0.25 * q10.sigma_az_px);
    }

    RS_CASE("both refinement modes report the same statistics");
    {
        /* coreg.h's contract is that the two modes are comparable. That claim
         * was previously testable only for the peak value; these two are
         * measured on different surfaces -- a bounded lattice against a padded
         * transform -- so it is worth checking rather than assuming. */
        /* Deliberately NOT a noiseless pair. At gamma exactly one the model
         * says the offset is exactly determined and both modes return zero,
         * which would make this case pass without comparing anything. Noise
         * puts the coherence where real sub-looks sit, and it is the same noise
         * in both calls so the two see one surface. */
        make_patch(ref, n, 16.0, 16.0, sigma);
        make_patch(cur, n, 16.3, 16.2, sigma);
        {
            unsigned s = 4242u;
            for (size_t i = 0; i < n * n; i++) {
                s = s * 1103515245u + 12345u;
                const double e = (double)(s >> 16) / 32768.0 - 1.0;
                cur[i] += (float)(0.25 * e);
            }
        }
        demean(ref, n); demean(cur, n);

        double sa, sr;
        rs_coreg_quality_t q_loc, q_exh;
        RS_CHECK_OK(rs_coreg_shift_q(ref, cur, n, n, 10, 10,
                                     RS_COREG_REFINE_LOCAL, &sa, &sr, &q_loc));
        RS_CHECK_OK(rs_coreg_shift_q(ref, cur, n, n, 10, 10,
                                     RS_COREG_REFINE_EXHAUSTIVE, &sa, &sr, &q_exh));
        printf("    local:      snr %.2f, sigma_az %.4f px\n",
               q_loc.snr, q_loc.sigma_az_px);
        printf("    exhaustive: snr %.2f, sigma_az %.4f px\n",
               q_exh.snr, q_exh.sigma_az_px);
        /* The SNR is measured on surfaces sampled at different densities, so it
         * is compared loosely; the sigma is a local property of the same peak
         * over the same physical baseline and should agree closely. */
        RS_CHECK(fabs(q_exh.snr - q_loc.snr) < 0.5 * q_loc.snr);
        RS_CHECK(fabs(q_exh.sigma_az_px - q_loc.sigma_az_px) <
                 0.25 * q_loc.sigma_az_px);
    }

    RS_CASE("a blank patch reports maximal uncertainty, not zero");
    {
        /* Zero would read as a perfectly determined offset, which is the exact
         * opposite of what a patch with nothing in it means -- and would carry
         * such a window through the cull built to remove it. */
        for (size_t i = 0; i < n * n; i++) { ref[i] = 0.0f; cur[i] = 0.0f; }
        double sa, sr;
        rs_coreg_quality_t q;
        RS_CHECK_OK(rs_coreg_shift_q(ref, cur, n, n, 10, 10,
                                     RS_COREG_REFINE_LOCAL, &sa, &sr, &q));
        RS_CHECK_NEAR(q.peak, 0.0, 1e-9);
        RS_CHECK_NEAR(q.snr, 0.0, 1e-9);
        RS_CHECK_NEAR(q.sigma_az_px, RS_COREG_SIGMA_MAX, 1e-6);
        RS_CHECK_NEAR(q.sigma_rg_px, RS_COREG_SIGMA_MAX, 1e-6);
    }

    RS_CASE("the quality-reporting entry point rejects a null destination");
    {
        double sa, sr;
        RS_CHECK_ERR(rs_coreg_shift_q(ref, cur, n, n, 10, 10,
                                      RS_COREG_REFINE_LOCAL, &sa, &sr, NULL),
                     RS_ERR_ARG);
    }

    free(ref);
    free(cur);
    RS_TEST_END();
}
