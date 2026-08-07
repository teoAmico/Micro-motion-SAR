/* Does the joint transient-and-mode fit recover parameters that were put in?
 *
 * These are unit tests on the estimator itself, not on the chain: the series is
 * synthesised here rather than tracked, so a failure is arithmetic and nothing
 * else. The chain-level question -- whether a mode survives sub-aperture
 * formation and tracking -- is what the sweeps in runs/synthetic answer, and a
 * pass here says nothing about it.
 *
 * The bar is deliberately the one this project uses everywhere else: a sweep
 * scored by slope and rms (rs_track_fit), never a single point. */

#include "resonarsat/microm.h"
#include "rs_test.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

/* State for the generator below, fixed so a failure reproduces exactly. */
static unsigned long long g_state = 0x243F6A8885A308D3ULL;

/* Uniform in [0,1) by xorshift64*. rand() is a property of the C library, so a
 * tolerance measured against it would not hold on another machine. */
static double urand(void)
{
    unsigned long long x = g_state;
    x ^= x >> 12; x ^= x << 25; x ^= x >> 27;
    g_state = x;
    return (double)((x * 2685821657736338717ULL) >> 11)
         * (1.0 / 9007199254740992.0);
}

/* Zero-mean noise of unit variance, by the central limit theorem over twelve
 * uniforms -- adequate for a noise floor and dependency-free. */
static double gauss(void)
{
    double s = 0.0;
    for (int i = 0; i < 12; i++) s += urand();
    return s - 6.0;
}

/* Fill 'y' with one damped sinusoid of known parameters plus optional noise. */
static void synth(double *y, size_t n, double fs, double f, double zeta,
                  double onset_s, double amp, double noise)
{
    const size_t i0 = (size_t)(onset_s * fs + 0.5);
    const double om = 2.0 * M_PI * f, alpha = zeta * om;
    for (size_t i = 0; i < n; i++) {
        y[i] = noise > 0.0 ? noise * gauss() : 0.0;
        if (i >= i0) {
            const double tau = (double)(i - i0) / fs;
            y[i] += amp * exp(-alpha * tau) * cos(om * tau);
        }
    }
}

/* Run every case and return the ctest exit status. */
int main(void)
{
    enum { N = 128 };
    const double fs = 6.4;                 /* 20 s of dwell, as the fixtures use */
    const double df = fs / (double)N;      /* 0.05 Hz */
    double y[N];
    rs_transient_fit_t fit;

    /* A clean sustained tone must come back exactly, including zero damping.
     * This is the case the periodogram already handles, and the fit must not be
     * worse on it -- item 72's max-hold was, which is why it is not a result. */
    RS_CASE("sustained tone recovers with zero damping");
    synth(y, N, fs, 0.5, 0.0, 0.0, 1.0, 0.0);
    RS_CHECK_OK(rs_transient_fit(y, N, fs, 0.0, 0.0, 1, &fit));
    RS_CHECK(fit.n_mode == 1);
    if (fit.n_mode == 1) {
        RS_CHECK_NEAR(fit.mode[0].freq_hz, 0.5, 0.5 * df);
        RS_CHECK_NEAR(fit.mode[0].zeta, 0.0, 0.02);
        RS_CHECK_NEAR(fit.mode[0].amp, 1.0, 0.05);
        RS_CHECK(fit.mode[0].var_frac > 0.95);
    }

    /* The transient case, which is the whole reason this exists: a decaying
     * mode whose energy is concentrated early. A Hann window would taper away
     * exactly the part that carries it. */
    RS_CASE("damped tone recovers frequency AND damping");
    synth(y, N, fs, 0.7, 0.05, 0.0, 1.0, 0.0);
    RS_CHECK_OK(rs_transient_fit(y, N, fs, 0.0, 0.0, 1, &fit));
    RS_CHECK(fit.n_mode == 1);
    if (fit.n_mode == 1) {
        RS_CHECK_NEAR(fit.mode[0].freq_hz, 0.7, 0.5 * df);
        RS_CHECK_REL(fit.mode[0].zeta, 0.05, 0.35);
        RS_CHECK(fit.mode[0].var_frac > 0.95);
    }

    /* An onset partway through the record. The estimator must place it, not
     * absorb it into a wrong frequency or a wrong decay. */
    RS_CASE("a late onset is located");
    synth(y, N, fs, 0.6, 0.03, 5.0, 1.0, 0.0);
    RS_CHECK_OK(rs_transient_fit(y, N, fs, 0.0, 0.0, 1, &fit));
    RS_CHECK(fit.n_mode == 1);
    if (fit.n_mode == 1) {
        RS_CHECK_NEAR(fit.mode[0].freq_hz, 0.6, 0.5 * df);
        /* The onset grid is a sixteenth of the record, 1.25 s here, so one grid
         * step is the tightest this can be asked to be. */
        RS_CHECK_NEAR(fit.mode[0].onset_s, 5.0, 1.30);
    }

    /* Two modes at once, which is the case rs_spectrum_best_window() loses on
     * (item 69): a structure divides its energy and no single line is tallest. */
    RS_CASE("two modes are separated");
    {
        double a[N], b[N];
        synth(a, N, fs, 0.45, 0.02, 0.0, 1.0, 0.0);
        synth(b, N, fs, 1.10, 0.04, 0.0, 0.8, 0.0);
        for (size_t i = 0; i < N; i++) y[i] = a[i] + b[i];
        RS_CHECK_OK(rs_transient_fit(y, N, fs, 0.0, 0.0, 2, &fit));
        RS_CHECK(fit.n_mode == 2);
        if (fit.n_mode == 2) {
            /* Greedy: the stronger comes first, but check the SET rather than
             * the order, since amplitude and damping both set which wins. */
            const double f0 = fit.mode[0].freq_hz, f1 = fit.mode[1].freq_hz;
            const double lo = f0 < f1 ? f0 : f1, hi = f0 < f1 ? f1 : f0;
            RS_CHECK_NEAR(lo, 0.45, 0.5 * df);
            RS_CHECK_NEAR(hi, 1.10, 0.5 * df);
            RS_CHECK(fit.resid_frac < 0.05);
        }
    }

    /* THE BAR. A frequency sweep scored by slope and rms, with noise, because
     * one point matching is a coincidence this project has recorded several
     * times. Half a bin is the bound used everywhere else here. */
    RS_CASE("a swept frequency tracks: slope and rms under noise");
    {
        const double want[] = { 0.35, 0.55, 0.75, 0.95, 1.15, 1.35 };
        const size_t n_pt = sizeof want / sizeof want[0];
        double got[6];
        for (size_t i = 0; i < n_pt; i++) {
            synth(y, N, fs, want[i], 0.04, 0.0, 1.0, 0.25);
            got[i] = 0.0;
            if (rs_transient_fit(y, N, fs, 0.0, 0.0, 1, &fit) == RS_OK &&
                fit.n_mode > 0)
                got[i] = fit.mode[0].freq_hz;
        }
        double slope = 0.0, rms = 0.0;
        RS_CHECK(rs_track_fit(want, got, n_pt, &slope, &rms) == 1);
        RS_CHECK_NEAR(slope, 1.0, 0.05);
        RS_CHECK(rms < 0.5 * df);
        printf("    sweep: slope %.4f, rms %.5f Hz against a %.5f bound\n",
               slope, rms, 0.5 * df);
    }

    /* The damping sweep, which is the parameter the periodogram cannot report
     * at all. Scored the same way, and swept INSIDE the grid's ceiling: at
     * 0.8 Hz over this 20 s record zeta_max is 8/(2*pi*0.8*20) = 0.0796, so a
     * sweep past that measures the ceiling rather than the estimator. */
    RS_CASE("a swept damping tracks");
    {
        const double f_d = 0.8, T = (double)N / fs;
        const double zeta_max = RS_TFIT_DECAY_MAX / (2.0 * M_PI * f_d * T);
        const double step = zeta_max / (double)(RS_TFIT_N_DECAY - 1);
        const double want[] = { 0.010, 0.020, 0.030, 0.045, 0.060, 0.075 };
        const size_t n_pt = sizeof want / sizeof want[0];
        double got[6];
        RS_CHECK(want[n_pt - 1] < zeta_max);
        for (size_t i = 0; i < n_pt; i++) {
            synth(y, N, fs, f_d, want[i], 0.0, 1.0, 0.10);
            got[i] = 0.0;
            if (rs_transient_fit(y, N, fs, 0.0, 0.0, 1, &fit) == RS_OK &&
                fit.n_mode > 0)
                got[i] = fit.mode[0].zeta;
        }
        double slope = 0.0, rms = 0.0;
        RS_CHECK(rs_track_fit(want, got, n_pt, &slope, &rms) == 1);
        /* The bound is one grid step, which is what the quantisation costs --
         * not a tolerance picked to pass. */
        RS_CHECK_NEAR(slope, 1.0, 0.25);
        RS_CHECK(rms < step);
        printf("    damping sweep: slope %.4f, rms %.5f against a %.5f step "
               "(ceiling %.4f)\n", slope, rms, step, zeta_max);
    }

    /* The ceiling is a documented property, so pin it: a mode damped past what
     * the grid can express comes back AT the ceiling rather than wrong or
     * refused, and a reader must know that value means "at least". */
    RS_CASE("damping past the ceiling saturates rather than misreports");
    {
        const double f_d = 0.8, T = (double)N / fs;
        const double zeta_max = RS_TFIT_DECAY_MAX / (2.0 * M_PI * f_d * T);
        synth(y, N, fs, f_d, 3.0 * zeta_max, 0.0, 1.0, 0.0);
        RS_CHECK_OK(rs_transient_fit(y, N, fs, 0.0, 0.0, 1, &fit));
        RS_CHECK(fit.n_mode == 1);
        if (fit.n_mode == 1) {
            /* The saturation is exact in ALPHA, which is what the grid holds.
             * zeta = alpha/(2*pi*f) inherits whatever the frequency grid
             * snapped to, so asserting on zeta would be asserting on f. */
            const double alpha = fit.mode[0].zeta * 2.0 * M_PI
                               * fit.mode[0].freq_hz;
            RS_CHECK_NEAR(alpha, RS_TFIT_DECAY_MAX / T, 1e-9);
            RS_CHECK_NEAR(fit.mode[0].zeta, zeta_max, 0.05 * zeta_max);
        }
    }

    /* THE TWIN CHANGE STATISTIC. It lives in this file rather than its own
     * because it is the other half of the same question -- what a paired run can
     * say about a frequency -- and the harness is already here. Values are
     * checked against the direct likelihood evaluation, not against themselves. */
    RS_CASE("twin LLR matches the closed form and is scale-free");
    {
        double l = 0.0, p = 0.0;
        RS_CHECK_OK(rs_twin_llr(2.0, 1.0, &l, &p));
        RS_CHECK_NEAR(l, 0.117783, 1e-5);
        RS_CHECK_OK(rs_twin_llr(10.0, 1.0, &l, &p));
        RS_CHECK_NEAR(l, 1.106913, 1e-5);
        RS_CHECK_OK(rs_twin_llr(100.0, 1.0, &l, &p));
        RS_CHECK_NEAR(l, 3.238784, 1e-5);
        /* Scale-free: only the ratio may matter. */
        double l2 = 0.0;
        RS_CHECK_OK(rs_twin_llr(2.0e-9, 1.0e-9, &l2, NULL));
        RS_CHECK_NEAR(l2, 0.117783, 1e-5);
        RS_CHECK_OK(rs_twin_llr(2.0e9, 1.0e9, &l2, NULL));
        RS_CHECK_NEAR(l2, 0.117783, 1e-5);
    }

    RS_CASE("twin LLR is one-sided and zero at no change");
    {
        double l = 1.0, p = 0.0;
        RS_CHECK_OK(rs_twin_llr(1.0, 1.0, &l, &p));
        RS_CHECK_NEAR(l, 0.0, 1e-12);
        RS_CHECK_NEAR(p, 1.0, 1e-12);
        /* Less power in the injected run is the H0 boundary, not evidence. */
        RS_CHECK_OK(rs_twin_llr(1.0, 5.0, &l, &p));
        RS_CHECK_NEAR(l, 0.0, 1e-12);
        RS_CHECK_NEAR(p, 1.0, 1e-12);
    }

    /* The exact F(2,2) tail, and the ceiling it puts on a single-look pair:
     * the ratio must exceed 19 before p < 0.05, whatever the difference looks
     * like. That number is the reason multilooking is the stated remedy. */
    RS_CASE("twin p-value is the exact F(2,2) tail");
    {
        double p = 0.0;
        RS_CHECK_OK(rs_twin_llr(19.0, 1.0, NULL, &p));
        RS_CHECK_NEAR(p, 0.05, 1e-12);
        RS_CHECK_OK(rs_twin_llr(99.0, 1.0, NULL, &p));
        RS_CHECK_NEAR(p, 0.01, 1e-12);
        RS_CHECK_OK(rs_twin_llr(3.0, 1.0, NULL, &p));
        RS_CHECK_NEAR(p, 0.25, 1e-12);
        double p18 = 0.0;
        RS_CHECK_OK(rs_twin_llr(18.0, 1.0, NULL, &p18));
        RS_CHECK(p18 > 0.05);
    }

    RS_CASE("twin LLR error contract");
    {
        double l = 0.0, p = 0.0;
        RS_CHECK_ERR(rs_twin_llr(0.0, 1.0, &l, &p), RS_ERR_ARG);
        RS_CHECK_ERR(rs_twin_llr(1.0, 0.0, &l, &p), RS_ERR_ARG);
        RS_CHECK_ERR(rs_twin_llr(-1.0, 1.0, &l, &p), RS_ERR_ARG);
        /* A refused call must leave the outputs at their neutral values. */
        RS_CHECK_NEAR(l, 0.0, 1e-12);
        RS_CHECK_NEAR(p, 1.0, 1e-12);
    }

    /* THE PER-WINDOW FLOOR. Constructed by hand so the arithmetic is checked
     * against a known circular sd rather than against another run. */
    RS_CASE("window floor: a coherent window has a low floor");
    {
        enum { NL = 128 };
        static double ph[NL];
        rs_microm_t mm; memset(&mm, 0, sizeof mm);
        mm.phase = ph; mm.n_win = 1; mm.n_looks = NL;
        const double LAM = 0.031;
        /* Zero phase everywhere: R = 1, so the circular sd is 0 and so is the
         * floor. This is the degenerate end and must not divide by zero. */
        for (int i = 0; i < NL; i++) ph[i] = 0.0;
        double f = -1.0, sd = -1.0;
        RS_CHECK_OK(rs_microm_floor(&mm, LAM, 0, &f, &sd));
        RS_CHECK_NEAR(sd, 0.0, 1e-9);
        RS_CHECK_NEAR(f, 0.0, 1e-15);

        /* A known circular sd: phase alternating +-a gives R = cos(a). */
        const double a = 0.5;
        for (int i = 0; i < NL; i++) ph[i] = (i % 2) ? a : -a;
        RS_CHECK_OK(rs_microm_floor(&mm, LAM, 0, &f, &sd));
        const double want_sd = sqrt(-2.0 * log(cos(a)));
        RS_CHECK_NEAR(sd, want_sd, 1e-9);
        RS_CHECK_NEAR(f, want_sd * LAM / (4.0 * M_PI) * sqrt(2.0 / (double)NL), 1e-15);
        printf("    alternating +-%.1f rad -> sd %.4f rad, floor %.5f mm\n",
               a, sd, f * 1e3);
    }

    RS_CASE("window floor: uniform phase saturates rather than diverging");
    {
        enum { NL = 360 };
        static double ph[NL];
        rs_microm_t mm; memset(&mm, 0, sizeof mm);
        mm.phase = ph; mm.n_win = 1; mm.n_looks = NL;
        for (int i = 0; i < NL; i++)
            ph[i] = -M_PI + 2.0 * M_PI * (double)i / (double)NL;
        double f = 0.0, sd = 0.0;
        RS_CHECK_OK(rs_microm_floor(&mm, 0.031, 0, &f, &sd));
        RS_CHECK(isfinite(sd) && isfinite(f));
        RS_CHECK(sd > 3.0);       /* a full circle is maximal scatter */
    }

    RS_CASE("window floor: error contract");
    {
        enum { NL = 8 };
        static double ph[NL];
        rs_microm_t mm; memset(&mm, 0, sizeof mm);
        mm.phase = ph; mm.n_win = 1; mm.n_looks = NL;
        double f = 0.0, sd = 0.0;
        RS_CHECK_ERR(rs_microm_floor(NULL, 0.031, 0, &f, &sd), RS_ERR_ARG);
        RS_CHECK_ERR(rs_microm_floor(&mm, 0.031, 5, &f, &sd), RS_ERR_ARG);
        RS_CHECK_ERR(rs_microm_floor(&mm, 0.0, 0, &f, &sd), RS_ERR_ARG);
        rs_microm_t nop; memset(&nop, 0, sizeof nop);
        nop.n_win = 1; nop.n_looks = NL;   /* phase NULL: not the phase route */
        RS_CHECK_ERR(rs_microm_floor(&nop, 0.031, 0, &f, &sd), RS_ERR_ARG);
    }

    /* Contract: malformed input produces a status and a message, never a crash
     * or a partly-written result. */
    RS_CASE("error contract");
    RS_CHECK_ERR(rs_transient_fit(NULL, N, fs, 0.0, 0.0, 1, &fit), RS_ERR_ARG);
    RS_CHECK_ERR(rs_transient_fit(y, 4, fs, 0.0, 0.0, 1, &fit), RS_ERR_ARG);
    RS_CHECK_ERR(rs_transient_fit(y, N, 0.0, 0.0, 0.0, 1, &fit), RS_ERR_ARG);
    /* A floor above Nyquist leaves no grid point at all. */
    RS_CHECK_ERR(rs_transient_fit(y, N, fs, 4.0, 0.0, 1, &fit), RS_ERR_RANGE);
    RS_CHECK_ERR(rs_transient_fit(y, N, fs, 0.0, 0.0, 1, NULL), RS_ERR_ARG);

    /* A flat series has no modes and is not an error -- a window over empty
     * ground must not fail the whole scene. */
    RS_CASE("a flat series yields no modes and no error");
    for (size_t i = 0; i < N; i++) y[i] = 0.0;
    RS_CHECK_OK(rs_transient_fit(y, N, fs, 0.0, 0.0, 2, &fit));
    RS_CHECK(fit.n_mode == 0);

    RS_TEST_END();
}
