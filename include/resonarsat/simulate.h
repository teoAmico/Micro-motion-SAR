/* A static scene with a real collect's geometry, for null testing.
 *
 * WHY THIS EXISTS, AND WHY THE EXISTING NULL WAS NOT ENOUGH.
 *
 * The shuffled null permutes the time order of ALREADY FORMED sub-looks. That
 * controls for a great deal -- scene, brightness, coherence and look count are
 * preserved exactly -- and it is the right test for a decomposition whose
 * sub-looks are independent.
 *
 * It is the wrong test for one whose sub-looks overlap. The decomposition of
 * Biondi & Malanga (2022) section 3.1 holds out a fraction of the Doppler band
 * and steps the remainder across the spectrum in rigid shifts, so adjacent
 * sub-looks share most of their bandwidth and therefore most of their speckle
 * realisation. Their measured shifts are correlated BEFORE any target moves.
 * That correlation makes the shift series smooth, a smooth series has its energy
 * at low frequency, and shuffling destroys the smoothness. So the unshuffled
 * series outscores its shuffles for a reason that has nothing to do with the
 * ground, and the test reports significance on a scene where nothing moves.
 *
 * Measured on the Panama collect: the paper decomposition returned p = 0.08 at
 * a frequency whose period is three times shorter than the sub-aperture that
 * measured it. A sub-look integrating three full cycles averages that motion
 * away, so the peak cannot be the motion it appears to be. The overlap is the
 * candidate explanation, and the shuffled null cannot rule it in or out.
 *
 * WHAT THIS DOES INSTEAD. It synthesises phase history for a scene of purely
 * STATIC scatterers, using the reference collect's own pulse times, platform
 * positions, carrier and range sampling. Running the identical chain over it --
 * same sub-aperture route, same tracker, same spectral estimator -- produces the
 * prominence a perfectly motionless world yields through this exact processing.
 * Overlap, tracker bias and estimator behaviour are all inherited, because the
 * same code computes them. Only the motion is absent.
 *
 * A measurement that does not beat that distribution has not demonstrated
 * motion, whatever it does against a shuffle. */

#ifndef RESONARSAT_SIMULATE_H
#define RESONARSAT_SIMULATE_H

#include <stddef.h>

#include "resonarsat/resonarsat.h"
#include "resonarsat/readers.h"

/* Synthesise a static-scene collect with 'ref's geometry.
 *
 * Pulse times, platform positions, per-pulse reference ranges, carrier and
 * range bin spacing are copied from 'ref', so the simulated aperture, dwell and
 * Doppler history are the real ones rather than a plausible substitute. The
 * scene is 'n_target' point scatterers placed pseudo-randomly within
 * 'extent_m' metres of the grid origin, with Rayleigh-distributed reflectivity
 * so the focused image has realistic speckle statistics. None of them moves.
 *
 * 'n_rbin' caps the simulated range extent, which the caller should set just
 * large enough to cover its processing grid: copying the reference's full swath
 * would allocate gigabytes to synthesise range bins no window ever reads. Pass
 * 0 to use the reference's own count. The near range is recentred so the grid
 * origin sits mid-swath whatever the count.
 *
 * 'seed' selects the realisation. Distinct seeds give independent speckle over
 * the same geometry, which is what makes a distribution of prominences rather
 * than a single number.
 *
 * On success '*out' owns its arrays and must be released with rs_cphd_free().
 * Returns RS_ERR_ARG on a NULL argument or a reference carrying no pulses, and
 * RS_ERR_ALLOC if the phase history cannot be sized. */
resonarsat_status_t rs_simulate_static_like(const rs_cphd_t *ref, unsigned seed,
                                            size_t n_target, const double centre[2],
                                            double extent_m,
                                            size_t n_rbin, rs_cphd_t *out);

/* Inject a vibrating point scatterer into a real collect's phase history.
 *
 * THE POSITIVE CONTROL, AND WHY A NULL WITHOUT ONE IS NOT A RESULT.
 *
 * rs_simulate_static_like() answers "what does this processing report when
 * nothing moves". It cannot answer the question that matters when a real collect
 * comes back empty: was the scene still, or is this chain incapable of seeing
 * motion in this data? Those produce the same output, and every real run this
 * project has made returns it. Item 19 addressed that indirectly -- the phase
 * route's precondition was unmet at Giza, so the run could not have succeeded --
 * but indirectly is the most a precondition can do.
 *
 * This is the direct answer. It adds a scatterer of known frequency and known
 * line-of-sight amplitude to the REAL phase history, before sub-aperture
 * formation, and the identical chain then runs over real clutter, real
 * coherence, the real orbit and the real look geometry. If the injected
 * frequency comes back, the chain can see motion in THIS data and a null
 * elsewhere in the scene means something. If it does not, the null was never
 * evidence about the ground.
 *
 * INJECTED BEFORE THE SUB-APERTURE STAGE, WHICH IS WHAT MAKES IT A CONTROL ON
 * THE WHOLE CHAIN. A control injected into the tracked series would exercise
 * only the spectral estimator, and one injected into the tomographic
 * observations only the inversion; neither would say whether sub-aperture
 * formation and sub-pixel tracking can extract motion from this collect, which
 * is the step in doubt.
 *
 * 'centre' is the scatterer's position in the same scene frame as the focusing
 * grid, so passing the grid origin puts it where the analysis windows are.
 * 'freq_hz' and 'amp_m' are its vibration; amp_m is a DISPLACEMENT AMPLITUDE in
 * metres along z, which the collect's own geometry projects onto the line of
 * sight, so the observable is smaller than the number passed. For the phase
 * estimator keep the projected amplitude below about lambda/8: the observable
 * wraps beyond lambda/4 and an injection that wraps tests nothing.
 *
 * 'rel_amp' scales the scatterer against the median of the scene's own
 * NON-ZERO sample magnitudes, so a value of 1 makes it typical of the clutter it sits in and 10
 * makes it a dominant. It is relative and BOUNDED for the reason item 21
 * records: an unbounded injected gain produced a result there that had to be
 * retracted, because the outliers rather than the mechanism carried it. Values
 * far above ~100 make the control easier than any real target and prove little.
 *
 * The written phase follows the collect's own phase_ref_srp convention, so the
 * injected scatterer focuses by the same arithmetic as everything already in the
 * data (see item 27 for what happens when those disagree).
 *
 * WHAT 'report' IS FOR, AND WHY IT IS NOT OPTIONAL IN PRACTICE. A pulse whose
 * target range falls outside the loaded range window deposits nothing, and
 * silently: a caller that crops with --rbins can inject into a collect and have
 * NOTHING happen. That failure is invisible in the result -- a null with no
 * injection in it looks exactly like a null with an injection the chain could
 * not see -- which would make this control worthless in precisely the case it
 * exists for. Measured at Giza on 2026-08-02: an injection that changed the
 * output produced no dominant window, and there was no way to tell a chain that
 * missed the motion from a target that never landed.
 *
 * 'report' may be NULL, but a caller reporting a null to anyone should pass it
 * and check 'n_deposited' against 'n_pulse'.
 *
 * Modifies 'cphd' in place, adding to the existing samples rather than
 * replacing them. Returns RS_ERR_ARG on a NULL argument, a collect carrying no
 * pulse geometry, or a non-finite or non-positive frequency, and RS_ERR_RANGE
 * if NO pulse deposited -- an injection that lands nowhere is an error, not a
 * quiet no-op. */
typedef struct {
    size_t n_pulse;       /* pulses in the collect */
    size_t n_deposited;   /* of those, pulses whose target fell in the window */
    double fbin_min;      /* range bin the target reached, min over pulses */
    double fbin_max;      /* and max; both NAN when nothing deposited */
    double scale_ref;     /* the median non-zero sample magnitude scaled against */
    double amp;           /* rel_amp * scale_ref, the amplitude actually written */
} rs_inject_report_t;

resonarsat_status_t rs_simulate_inject_vibrator(rs_cphd_t *cphd,
                                                const double centre[2],
                                                double freq_hz, double amp_m,
                                                double rel_amp,
                                                rs_inject_report_t *report);

#endif /* RESONARSAT_SIMULATE_H */
