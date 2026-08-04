/* Micro-motion extraction: sub-pixel offset tracking (SPOT) across a
 * sub-aperture stack, and the per-pixel vibration spectra it feeds.
 *
 * The technique is known in the literature as micro-Doppler SAR (MDSAR) and the
 * tracking step as SPOT; those names are used here so that results are
 * comparable with published work rather than privately defined.
 *
 * The realistic performance envelope, from an independent metrological
 * assessment against synchronous accelerometer ground truth on the same class
 * of X-band spotlight data this project targets (Vattulainen et al. 2026):
 * vibration frequencies 1-4 Hz, RMS radial displacement 10.43 mm down to
 * 0.10 mm, radial velocity error of order 1 mm/s, frequency resolution 0.06 Hz
 * from a 16 s acquisition.
 *
 * THAT BAND IS THE TARGET'S, NOT THE TECHNIQUE'S. It was measured on bridges,
 * which vibrate at a few hertz. The same group, on the same class of Umbra
 * X-band data, recovers 87 Hz from an idling van and 36 Hz from a ship's
 * engine, both confirmed against triaxial accelerometers (Clemente et al.,
 * EuRAD 2025). Quoting 1-4 Hz as a property of the method would understate its
 * reach by more than an order of magnitude.
 *
 * What sets the ceiling is the sub-aperture sampling rate, and reaching those
 * frequencies takes far more sub-looks than seems natural: that work uses
 * sub-apertures of 0.4167 s stepped by about 4 ms -- roughly 99 percent overlap
 * and some 3900 sub-apertures across a 16.4 s dwell, sampling near 240 Hz.
 * Configurations here have typically used tens of looks, which caps the
 * observable band in the single hertz and cannot represent an engine line at
 * all. The overlap is deliberate and is justified there exactly as in
 * time-frequency analysis: it restores time resolution without giving up
 * azimuth resolution.
 *
 * Note also that the observable differs. That work reads the PHASE of each
 * pixel in each sub-aperture directly; this interface defaults to
 * correlation-based offset tracking. See rs_microm_estimator_t.
 *
 * One limitation from that work shapes this interface: frequencies are
 * recovered reliably while relative amplitudes are not. The amplitude field
 * below is therefore a qualitative indicator, and callers must label it as
 * such wherever it is presented. */

#ifndef RESONARSAT_MICROM_H
#define RESONARSAT_MICROM_H

#include <complex.h>
#include <stddef.h>

#include "resonarsat/resonarsat.h"
#include "resonarsat/subaperture.h"

/* Tracking parameters.
 *
 * The defaults come from published working values rather than round numbers:
 * azimuth upsampling 10, range upsampling 20, and a patch tuned per target in
 * the range 51 to 131 pixels. Patch size is not a free parameter -- too small a
 * patch makes the tracker systematically underestimate displacement -- so it is
 * exposed prominently and rs_microm_params_default() picks a mid-range value
 * rather than pretending there is a universally correct one. */
/* Which sub-look each correlation is taken against.
 *
 *   RS_MICROM_REF_FIRST     Every look against look 0. Simple, and the shifts
 *                           come out absolute with no accumulation. But the
 *                           correlation surface of an N-pixel window is only
 *                           unambiguous over +/-N/2 pixels, and a target's
 *                           excursion relative to look 0 spans TWICE its
 *                           amplitude. Exceed that and the shift wraps, folding
 *                           the recovered series and putting its energy at twice
 *                           the true frequency -- a clean, confident-looking
 *                           second harmonic that is entirely an artefact.
 *
 *   RS_MICROM_REF_ADJACENT  Each look against its predecessor, accumulating the
 *                           differentials. Consecutive looks are separated by
 *                           one sampling interval, so the shift between them is
 *                           smaller than the full excursion by a factor of
 *                           2*pi*f*dt -- comfortably inside the unambiguous
 *                           range where the direct comparison is not. They also
 *                           share most of their pulses, so they correlate far
 *                           better than look 0 and look N do.
 *
 * MEASURED TRADE-OFF, and the reason FIRST remains the default. ADJACENT does
 * what it promises -- per-window coherence roughly doubles and the second
 * harmonic disappears from the target's own windows. But accumulating the
 * differentials integrates tracking noise into a random walk, whose 1/f^2
 * spectrum swamps the signal: on the single-target frequency sweep ADJACENT
 * takes recovery from 5 of 6 to 0 of 6, every window reporting the lowest bin.
 * The least-squares detrend in rs_spectrum_compute() removes only the linear
 * part of a random walk, which is not enough. Using ADJACENT profitably needs
 * the accumulation handled -- differencing the series back, or a high-pass --
 * and that is not yet implemented. PAIR below is the principled way to get the
 * same benefit, because it never accumulates in the first place.
 *
 *   RS_MICROM_REF_PAIR      Each look's slave against its own master, the pair
 *                           held B_shift apart. This is what the sources
 *                           describe: WO2024008365A1 [0004] sweeps two bands
 *                           "rigidly held at a distance B_shift" and tracks the
 *                           slave's position against its master, and the Giza
 *                           paper's Eqs. 4-5 give N_D masters and N_D slaves.
 *                           Needs a stack built with rs_subap_params_t.pair.
 *
 *                           Each sample is a displacement difference across one
 *                           FIXED lag dt = B_shift * t_dwell / B_CD, so the
 *                           series is a first difference of the displacement
 *                           rather than ADJACENT's running sum of differences.
 *                           Differencing is a high-pass with a known response,
 *                           |2 sin(pi f dt)|; summing is an integrator that
 *                           turns tracking noise into a random walk. So the
 *                           expectation was that PAIR would recover frequencies
 *                           where ADJACENT cannot.
 *
 *                           IT DOES NOT, AND THE REASON IS NOW UNDERSTOOD. On
 *                           the same single-target fixture that ADJACENT fails,
 *                           PAIR fails too -- and on the distributed-texture
 *                           fixture its series is exactly zero in every window
 *                           that holds the target: at the default B_shift the
 *                           slave-master offset never moves one quantisation
 *                           step, and the mode's occasional nonzero windows are
 *                           near-empty edge windows whose blips do not depend
 *                           on the injection: measured across five seeds and
 *                           six injected frequencies, on and off the fixture's
 *                           blind bins, the mode's answers pool to {0.1 Hz,
 *                           0.5 Hz} whatever is injected.
 *
 *                           Two structural reasons, not one defect. First, the
 *                           paper decomposition's sweep spans the master band's
 *                           own width, so the record length always equals the
 *                           sub-look duration (N*dt = t_sap) and every
 *                           resolvable frequency bin sits at an INTEGER
 *                           observation ratio -- each sub-look integrates a
 *                           whole number of cycles of any frequency the sweep
 *                           can resolve. A displacement-averaging observable
 *                           (which correlation tracking is) cannot both resolve
 *                           a frequency and retain it, at any left_out_frac.
 *                           Second, the pair's lag is one sweep step, so its
 *                           differential is attenuated by |2 sin(pi f dt)| on
 *                           top of that -- at 96 looks, under one quantisation
 *                           step for everything in band.
 *
 *                           The selection-artefact history is worth keeping:
 *                           PAIR used to return the lowest spectral bin for
 *                           every injection, which turned out to be
 *                           rs_spectrum_best_window() choosing windows whose
 *                           whole excursion was one sub-pixel step. The
 *                           quantisation floor removed that, and the remaining
 *                           "recoveries" (2 of 9 at 0.5 Hz) were then shown to
 *                           be coincidence: across five seeds the mode answers
 *                           0.5 Hz at a fixed per-seed rate whatever frequency
 *                           is injected, including with coherent whole-patch
 *                           motion injected at 0.35 Hz.
 *
 *                           AN EARLIER SUSPECT WAS CHECKED AND WAS NOT THE
 *                           CAUSE. rs_microm_ref_t once named a systematic
 *                           drift of the master-slave offset with sweep
 *                           position; the band layout did carry such a defect
 *                           (the sweep began half a step above the lower band
 *                           edge, clipping the last slave's filter), it was
 *                           fixed in rs_subaperture_split(), and the measured
 *                           result did not change. Real defect, wrong suspect.
 *
 *                           So this mode is FAITHFUL TO THE SOURCES AND NOT FIT
 *                           FOR MEASUREMENT. It is implemented and exposed
 *                           because the sources describe it and it should be
 *                           testable, not because it works. Do not read a
 *                           frequency out of it. test_tracking.c records the
 *                           behaviour so a fix would show up as that test
 *                           changing.
 *
 *                           If it is fixed, amplitudes will still be attenuated
 *                           by |2 sin(pi f dt)| and not comparable to FIRST's.
 *
 *   RS_MICROM_REF_LAG       Each look against the one 'ref_lag' places before
 *                           it, with NO accumulation. Untested; added to
 *                           separate the two defects the other three carry.
 *
 *                           FIRST decorrelates: looks i and j share pulses only
 *                           while |i-j| < 1/(1-overlap), so with a fixed
 *                           reference the coherent span is a handful of looks
 *                           out of hundreds. Measured on the single-target
 *                           fixture, the correlation peak of look 0 against the
 *                           rest averages 0.090 at zero overlap and only 0.310
 *                           at 0.90, while ADJACENT PAIRS at 0.90 correlate at
 *                           0.913. The coherence exists; a fixed reference
 *                           discards it.
 *
 *                           ADJACENT keeps that coherence and then integrates
 *                           tracking noise into a random walk. PAIR avoids the
 *                           integrator but inherits the published sweep's
 *                           geometry, where N*dt = t_sap puts every resolvable
 *                           bin at an integer observation ratio -- a null of the
 *                           averaging response.
 *
 *                           LAG is the remaining corner: a fixed short lag on
 *                           the PULSE route. Coherence is set by the lag alone
 *                           and stays high for small 'ref_lag'; nothing
 *                           accumulates, so there is no random walk; and the
 *                           record length is the whole dwell rather than one
 *                           sub-look, so df = 1/T and the bins do NOT land on
 *                           integer observation ratios the way the spectral
 *                           sweep forces them to.
 *
 *                           Each sample is a displacement difference across
 *                           ref_lag*dt, so the series is a first difference with
 *                           response |2 sin(pi f ref_lag dt)|. That nulls at
 *                           f = k/(ref_lag*dt); keep ref_lag small enough that
 *                           the first null sits above the band of interest.
 *                           Frequencies survive differencing, amplitudes are
 *                           attenuated by that response and are NOT comparable
 *                           to FIRST's.
 */
typedef enum {
    RS_MICROM_REF_FIRST = 0,
    RS_MICROM_REF_ADJACENT = 1,
    RS_MICROM_REF_PAIR = 2,
    RS_MICROM_REF_LAG = 3
} rs_microm_ref_t;

/* Which estimator computes the shifts.
 *
 *   RS_MICROM_EST_CORRELATION  Cross-correlation peak finding, refined to
 *                              sub-pixel by local upsampling. The classical
 *                              method, and characterised in the literature as
 *                              achieving "performances in the order of the
 *                              resolution element for a few independent
 *                              samples" -- which is what this project measures.
 *
 *   RS_MICROM_EST_SPLITBAND    Split-band Phase Linking across the whole stack
 *                              (see phaselink.h). Uses all N^2 interferograms
 *                              rather than the N-1 formed against one reference,
 *                              and comes within 0.5 dB of the Cramer-Rao bound.
 *                              Requires interferometric coherence between looks;
 *                              it has nothing to track if that is absent.
 *
 *   RS_MICROM_EST_ARGMAX       The azimuth position of the window's BRIGHTEST
 *                              PIXEL, read off each sub-look and tracked. No
 *                              correlation, no phase, no reference look: a
 *                              vibrating scatterer's paired echoes walk along
 *                              azimuth, and this follows where the brightest
 *                              sample sits.
 *
 *                              THIS IS THE PUBLISHED METHOD, AND THIS PROJECT
 *                              MEASURED IT WORKING BEFORE IT HAD IT. Suppi et
 *                              al., "Vibrational Monitoring of Isolated Targets
 *                              Using Single-Pass SAR Images" (IWSHM 2025), state
 *                              their step 5 as tracking "the azimuthal
 *                              displacement of the brightest pixel in each
 *                              sub-aperture", validated against a corner
 *                              reflector on an electromechanical shaker with an
 *                              LVDT, reaching Pearson correlation 0.98 against
 *                              ground truth. FOLLOW-UPS.md item 6 measured the
 *                              same thing on this project's own stacks: a plain
 *                              integer argmax carried 93 percent of its variance
 *                              at the injected frequency where the correlator on
 *                              the identical stack carried 4.1 percent.
 *
 *                              IT IS QUANTISED AT ONE CELL, which is not a
 *                              defect to be refined away. The published result
 *                              is for the integer argmax, and rs_microm_t.quant_px
 *                              is set to 1.0 so the quantisation floor tests it
 *                              honestly: an excursion under 2.449 cells is
 *                              rounding, and the selection policies refuse it.
 *                              A parabolic fit through the peak's neighbours
 *                              would buy sub-cell resolution and is deliberately
 *                              NOT done here, because what has evidence behind
 *                              it is the integer form.
 *
 *                              ITS PRECONDITION IS A DISTINGUISHED BRIGHTEST
 *                              PIXEL, which is weaker than the phase route's one
 *                              dominant scatterer per resolution cell -- the
 *                              peak has only to be findable, not coherent -- and
 *                              stronger than the correlator's, which needs no
 *                              individual scatterer at all. 'quality' reports it
 *                              directly as one minus the window's mean-to-peak
 *                              amplitude ratio.
 *
 *                              THE OPERATING POINT IS NOT THIS PROJECT'S. Suppi
 *                              et al. use 39-80 sub-apertures over a 5.2-6.1 s
 *                              observation at 0-65 percent overlap and aperture
 *                              fractions of 1.8-4.9 percent. This project has
 *                              run 128-2048 looks over a 33 s dwell. Nothing
 *                              here is validated at the settings the rest of
 *                              this file uses.
 *
 *   RS_MICROM_EST_PHASE        The phase of a single dominant pixel, read
 *                              directly from each sub-look, with the geometric
 *                              carrier removed. This is the observable of
 *                              Clemente et al. (EuRAD 2025), the only one in
 *                              this file with published accelerometer
 *                              validation on this class of data.
 *
 *                              ITS PRECONDITION IS ONE DOMINANT SCATTERER PER
 *                              SUB-LOOK RESOLUTION CELL, and that is a
 *                              quantitative statement, not the loose one this
 *                              header used to make. A lattice of EQUAL bright
 *                              scatterers spaced more finely than the resolution
 *                              cell puts several in every cell and satisfies
 *                              nothing: at 2.75 per cell the estimator fails at
 *                              every seed and every selection policy, and at
 *                              about one per cell all three policies recover.
 *                              On uniform clutter the working band is narrow --
 *                              96 to 128 scatterers over a 24 m patch at 8.26 m
 *                              sub-look resolution, with RAYLEIGH amplitudes,
 *                              whose tail is what gives some cell a scatterer
 *                              clearly brighter than its neighbours. Equal
 *                              amplitudes never work at any count. FOLLOW-UPS.md
 *                              item 15 has the tables.
 *
 *                              HIGH OVERLAP IS FINE FOR THIS ESTIMATOR AND IS
 *                              WHAT A REAL COLLECT NEEDS. Recovery holds to 95
 *                              percent overlap. At 90 percent, t_sap of 1.458 s
 *                              puts a 1.3 Hz injection at a sub-aperture
 *                              response of 0.055 -- a tenth of what the
 *                              correlation estimator needs, see
 *                              rs_subap_params_t.overlap -- and it recovers
 *                              anyway at rms 0.0164 Hz. That is the measurement
 *                              behind the claim that the response ceiling is the
 *                              CORRELATOR'S and not the method's.
 *
 *                              IT IS ALSO THE ONLY ESTIMATOR HERE THAT HAS BEEN
 *                              SHOWN TO RECOVER AN INJECTED FREQUENCY. On the
 *                              synthetic fixtures, swept and pooled over seeds
 *                              with a static control through identical
 *                              processing: slope 1.008 and rms 0.0070 Hz
 *                              against a half-bin bound of 0.0252 Hz, on
 *                              coherently vibrating clutter at three seeds, and
 *                              slope 1.016 with rms 0.0078 Hz on an isolated
 *                              point. Read FOLLOW-UPS.md item 14 before quoting
 *                              that: it is a synthetic result, and rs_sim_scene()
 *                              gives every scatterer analytically exact phase, so
 *                              the sub-look decorrelation a real collect imposes
 *                              on this observable is absent by construction.
 *
 *                              THE CARRIER MUST COME OFF BEFORE THE PHASE IS
 *                              WRAPPED, and for most of this estimator's life it
 *                              did not. A scatterer anywhere but exactly at its
 *                              pixel's centre has a range that changes linearly
 *                              as the aperture sweeps, so its phase is linear in
 *                              the sub-look index at (4*pi/lambda)*dX*dx/R --
 *                              measured at 1.1 to 1.9 radians PER LOOK, tens of
 *                              full cycles across a stack. Folding that into
 *                              (-pi, pi] makes a sawtooth whose line is set by
 *                              the target's sub-pixel offset and the geometry,
 *                              so it does not move when the scene does. That was
 *                              the fixed frequency this estimator reported for
 *                              every injection AND for a motionless scene, at a
 *                              prominence higher than any moving case; it is the
 *                              common-mode artefact FOLLOW-UPS.md item 11 uses
 *                              to show the consensus gate blind. Detrending the
 *                              displacement cannot undo it, because by then the
 *                              wrap has happened and a sawtooth is not a trend.
 *
 *                              IT IS NOT A REFINEMENT OF THE OTHER TWO -- it
 *                              measures something different. Both of those
 *                              estimate WHERE a patch sits, averaged over the
 *                              sub-look, so they live in the displacement-
 *                              averaging regime and go blind at frequencies
 *                              near k/t_sap. Phase responds to sub-wavelength
 *                              line-of-sight motion without that averaging, and
 *                              the same work recovers 36 Hz at an observation
 *                              ratio of exactly 18, where the averaging model
 *                              predicts precisely zero. See
 *                              rs_spectrum_subaperture_response().
 *
 *                              The cost is ambiguity, and it is not a footnote:
 *                              it decides which fixture the estimator can even
 *                              be tested on. Phase wraps every lambda/2 of
 *                              line-of-sight motion -- about 16 mm at X band --
 *                              so a target moving further folds and the
 *                              recovered series is nonsense. The 20 mm
 *                              amplitude the correlation fixtures use is a 6.6
 *                              radian swing, and the phase estimator fails on
 *                              them completely and correctly; the sweeps above
 *                              inject 2.442 mm, which is 0.81 radians. A
 *                              measurement that puts this estimator on a
 *                              correlation fixture is measuring the wrap, not
 *                              the estimator. Correlation has no such limit.
 *                              Prefer phase for small fast motion and
 *                              correlation for large slow motion.
 *
 *                              SUB-LOOK COHERENCE IS SET BY PULSE SHARING, AND
 *                              THAT BOUNDS HOW LONG A SERIES CAN BE UNWRAPPED.
 *                              Measured on a real X-band spotlight collect, the
 *                              coherence between two sub-looks is very nearly
 *                              the fraction of pulses they have in common:
 *
 *                                shared  95%  90%  75%  50%   0%
 *                                gamma  0.85 0.78 0.61 0.39 0.07
 *
 *                              It reaches the noise floor exactly when the
 *                              windows stop overlapping. That is independent
 *                              speckle per pulse subset -- the coherence is the
 *                              shared fraction of the energy -- and it was the
 *                              same on distributed clutter and on a scatterer
 *                              74x above its surroundings, so it is not an SNR
 *                              effect and not a property of the target.
 *
 *                              THE CONSEQUENCE IS COUNTER-INTUITIVE: RAISING
 *                              THE OVERLAP MAKES THE UNWRAP WORSE. Per-step
 *                              phase noise falls as the overlap rises, but the
 *                              number of steps rises faster, and the unwrap
 *                              accumulates them as a random walk of
 *                              sigma*sqrt(N). Worse, gamma does not approach 1
 *                              as the overlap does -- it levels off near 0.9 --
 *                              so sigma stops falling while N keeps growing.
 *                              Over a 33 s dwell the accumulated error is tens
 *                              of radians at every overlap tried, where a
 *                              usable unwrap needs it well under pi.
 *
 *                              So do NOT reach for 99 percent overlap to
 *                              stabilise a phase series; it does the opposite.
 *                              Use the FEWEST looks that still sample the
 *                              frequency of interest, and treat any unwrapped
 *                              phase series spanning a full aperture as suspect
 *                              until its accumulated noise is checked. A
 *                              peak-to-peak line-of-sight velocity near
 *                              lambda/(2*dt) is the tell: that is the ceiling
 *                              the fold imposes, and hitting it means the
 *                              series wrapped rather than that the target moved.
 *
 *                              THE SHUFFLE NULL TEST IS INVALID FOR THIS
 *                              ESTIMATOR. rs_null_floor(), --shuffle-looks and
 *                              --null-trials destroy the sub-look time order and
 *                              hold everything else constant -- for a
 *                              correlation observable. For phase they do not:
 *                              reordering puts non-consecutive looks adjacent,
 *                              which is precisely where a phase series steps
 *                              furthest, so the shuffle inflates the per-step
 *                              noise it is supposed to preserve. Measured on the
 *                              Giza collect at 128 looks and 0.99 overlap, the
 *                              median largest step is 0.052 rad in order and
 *                              1.878 rad shuffled -- a factor of 36. A drifting
 *                              series therefore beats its own shuffles by
 *                              construction: that run cleared 32 of them at
 *                              p = 0.03 while every window on the grid reported
 *                              whatever bin the search was told to start at, and
 *                              8 simulated motionless collects reproduced the
 *                              same frequency at 99 percent of the same
 *                              prominence. Adjudicate a phase result with
 *                              rs_null_static() (--null-static), which a
 *                              motionless scene cannot walk over because it
 *                              carries the same overlap, unwrap and detrend.
 *                              See runs/giza/2026-07-30-uniform-phase-khufu/.
 */
typedef enum {
    RS_MICROM_EST_CORRELATION = 0,
    RS_MICROM_EST_SPLITBAND = 1,
    RS_MICROM_EST_PHASE = 2,
    RS_MICROM_EST_ARGMAX = 3
} rs_microm_estimator_t;

typedef struct {
    rs_microm_estimator_t estimator;  /* how shifts are computed */
    rs_microm_ref_t reference;  /* which correlation reference (correlation only) */
    /* Lag in looks for RS_MICROM_REF_LAG, ignored by every other mode.
     *
     * Small keeps coherence high -- looks share pulses while the lag is under
     * 1/(1-overlap) -- and pushes the differencing null at 1/(ref_lag*dt) up
     * out of the band. Both want it small; the only thing wanting it large is
     * sensitivity, since |2 sin(pi f ref_lag dt)| grows with the lag until that
     * null. Default 1, which is ADJACENT's spacing without ADJACENT's
     * integrator. */
    size_t ref_lag;
    size_t win_az, win_rg;      /* correlation patch size in pixels */
    size_t stride_az, stride_rg;/* step between patch centres, pixels */
    size_t upsample_az;         /* sub-pixel refinement factor along azimuth */
    size_t upsample_rg;         /* sub-pixel refinement factor along range */
    double coherence_min;       /* discard windows below this correlation peak */

    /* Subtract the scene-median shift from every window, look by look.
     *
     * Sub-looks are focused from different pulse windows, so they can carry a
     * systematic offset relative to one another -- residual co-registration
     * rather than target motion. That offset is common to every window, so it
     * appears at the same frequency across the whole scene and can outrank a
     * genuine localised target: on a four-target test scene it produced a
     * prominent peak in static-ground windows that won window selection at every
     * injected frequency, giving the same wrong answer each time.
     *
     * The median is used rather than the mean so that a few genuinely moving
     * windows do not drag the estimate. Real localised motion survives; anything
     * shared by the whole scene cancels.
     *
     * MEASURED: off by default. The median is only as good as the windows it is
     * taken over, and on a scene dominated by empty background -- every
     * synthetic fixture here -- most windows track noise, so the median is noisy
     * and subtracting it injects noise into the one window that had signal:
     * recovery falls from 5 of 6 to 3 of 6. Enable on real scenes with
     * distributed clutter, where the premise that most of the scene is static
     * and well-tracked actually holds. */
    int remove_common_mode;

    /* Nonzero: run the tracker as an unoptimised reference. Set by --no-optimize.
     *
     * Two things change, and only one of them can change a number:
     *
     *   1. The correlator searches the WHOLE zero-padded surface for its peak
     *      instead of the neighbourhood of the strongest integer sample
     *      (RS_COREG_REFINE_EXHAUSTIVE). This CAN move a reported shift, and is
     *      the point of the flag. See rs_coreg_refine_t for the precise
     *      circumstance under which the two disagree -- it is narrower than it
     *      sounds, because the optimised path's integer peak is already a global
     *      maximum over the sampled surface.
     *
     *   2. The loop over windows runs on one thread. This cannot change a number.
     *      Windows are independent and each writes only its own output slots.
     *
     * MEASURED COST, WHICH IS FAR SMALLER THAN THE MECHANISM SUGGESTS. The
     * exhaustive correlator is 1.7x to 3.2x the optimised one per call across
     * every configuration tried: 2.5x at the tomo and mmotion defaults (32x32
     * window, 10x20 upsampling, a 320x640 padded surface at 1.56 MB), 1.7x at
     * 24x24 and 10x10, 3.2x at 64x64 and 10x20.
     *
     * That is not the ratio one guesses from "upsample the whole surface instead
     * of a neighbourhood", and the reason is worth knowing: the optimised path is
     * not cheap either. It evaluates (2*upsample_az+1)*(2*upsample_rg+1) points
     * and each one costs O(win_az*win_rg), so at the defaults it already does
     * about 880 thousand complex multiply-accumulates -- against roughly 3.6
     * million butterflies for the padded transform. Same order. The O(N) per
     * refinement point is what closes the gap.
     *
     * Serialising the window loop costs more than the search does: about 4x
     * wall-clock on eight cores. Both together put a full-scale run in single
     * digits, not orders of magnitude.
     *
     * A run made with this set is NOT a better measurement than one without. It
     * is a second measurement by a slower route, whose only use is comparison
     * with the first. Neither passes a null test on its own -- see README.md. */
    int no_optimize;
} rs_microm_params_t;

/* Per-window micro-motion result over the whole sub-look stack.
 *
 * 'n_win_az' by 'n_win_rg' windows, each carrying a displacement time series of
 * 'n_looks' samples. 'disp_az' and 'disp_rg' hold the tracked shift in pixels,
 * 'disp_los' the line-of-sight displacement in metres derived from phase, and
 * 'quality' the correlation peak value used as a mask.
 *
 * Series are stored window-major: window w's series begins at index
 * w * n_looks. */
typedef struct {
    double *disp_az, *disp_rg;  /* [n_win][n_looks], pixels */

    /* Line-of-sight VELOCITY, m/s, derived from the tracked azimuth shift.
     *
     * This is the primary observable, and the reason is worth stating. A target
     * moving radially is displaced in azimuth by dx = R * v_r / V -- the classic
     * moving-target azimuth shift -- so the tracked shift measures a velocity,
     * not a displacement. Being a geometric shift rather than a phase, it is
     * unambiguous: nothing here wraps, however large the motion, so long as the
     * shift stays inside the correlation window.
     *
     * This is also the quantity the independently validated literature reports
     * (velocity errors of order 1 mm/s against accelerometer ground truth), so
     * results computed from it are directly comparable with published work.
     *
     * A sinusoidal displacement at frequency f produces a sinusoidal velocity at
     * the same f, so frequency recovery is unaffected by working in velocity. */
    double *vel_los;            /* [n_win][n_looks], m/s */

    /* Line-of-sight DISPLACEMENT, m, from the phase taken against the series'
     * own mean phasor. Far finer than the velocity estimate, and ambiguous
     * beyond +/-lambda/4 of TOTAL motion -- about 8 mm at X band -- because it
     * is deliberately not unwrapped. See rs_microm_track() for why unwrapping
     * was removed rather than fixed. */
    double *disp_los;           /* [n_win][n_looks], metres */

    /* Phase relative to the series' mean phasor, rad, kept so a caller can
     * choose a different reference or attempt its own unwrap. Note this is no
     * longer an accumulated total: consecutive values are independent. */
    double *phase;              /* [n_win][n_looks], radians in (-pi, pi] */

    /* [n_win] tracking quality in [0,1] -- BUT IT IS A DIFFERENT QUANTITY PER
     * ESTIMATOR, and on one route it is not independent of 'd_a' below.
     *
     * Correlation and split-band: the mean correlation peak over the looks that
     * produced a shift. That is the reading this field had for its first two
     * years and is what --coherence was written to mask on.
     *
     * PHASE: SPATIAL DOMINANCE of the window's brightest pixel over its own
     * window, 1 - mean/peak on the reference look -- the same measure
     * RS_MICROM_EST_ARGMAX uses.
     *
     * IT WAS AMPLITUDE STABILITY, 1 - sigma_A/mu_A, AND THAT WAS A DEFECT.
     * Scoring a phase window by how constant its PHASE is would reward the
     * windows where nothing moves; the old measure avoided that and then made
     * the same mistake one level down. A scatterer vibrating at 2 mm is not
     * amplitude-stable across sub-looks, because that is what the motion does to
     * it. Measured on ICEYE Houston (item 45): six windows carried the injected
     * frequency at the highest prominence in the scene and every one failed the
     * shared gate at 0.21-0.24 against a trend artefact's 0.54, so the tool
     * reported the artefact. The gate preferred targets that did not move.
     *
     * The precondition being proxied is item 15's -- one dominant scatterer per
     * SUB-LOOK RESOLUTION CELL -- which is a statement about space, and a
     * vibrating dominant is still dominant. After the change the same run
     * reports the injected frequency at quality 0.994.
     *
     * THIS IS NO LONGER 1 - d_a. They were complements computed from the same
     * amplitudes, so a `quality` map and a `d_a` map on a phase run were one
     * measurement shown twice; that is what docs/CODE-REVIEW.md finding 1
     * recorded and it no longer holds. `quality` now answers "is there a
     * dominant scatterer here" and `d_a` answers "is it a persistent one".
     * `--coherence F` is likewise no longer the criterion D_A <= 1 - F.
     *
     * THE SHARED GATE IS NOW INERT ON REAL SCENES, and that is measured rather
     * than suspected. Fully developed speckle over a 1024-pixel window already
     * scores 1 - mean/peak = 0.673 (mean over 20000 realisations, 5th-95th
     * 0.633-0.718), real sub-look imagery runs 0.81-0.94, and a dominant reaches
     * 0.995. The shared gate is quality >= 0.5*q_max, which sits near 0.50 when
     * the max is 0.99, so nothing is removed: 25 of 25 windows pass on every
     * ICEYE run measured. The discriminating range is roughly [0.75, 1.0] and
     * the threshold is far below it.
     *
     * That is a strictly better failure than the old one -- an inert gate
     * removes nothing, where the old gate removed the signal -- but it means the
     * RELATIVE form of the threshold is wrong for this quantity, not the
     * quantity. A floor derived from the speckle expectation above would
     * discriminate; none is imposed here, because this project does not fit
     * constants to two scenes. See FOLLOW-UPS item 46.
     */
    double *quality;            /* [n_win] */

    /* [n_win] mean correlation-surface SNR, and the value that same surface
     * reaches on noise alone. Both zero for the estimators that never form a
     * correlation surface.
     *
     * WHAT THIS ADDS OVER 'quality', WHICH IS THE WHOLE REASON IT EXISTS. The
     * peak value says how alike two sub-looks are. The SNR says whether the
     * surface had ONE maximum or a field of comparable ones, which is a
     * different failure and the one that moves an offset without lowering the
     * coherence. A window over distributed clutter with several scatterers of
     * similar strength correlates well and locates badly; nothing in 'quality'
     * distinguishes it from a window that correlates well and locates well.
     *
     * 'snr_null' is what the surface would score with no signal in it at all --
     * the harmonic number of the window's bin count, derived on
     * rs_coreg_quality_t -- and it travels with the measurement because the
     * threshold is only meaningful relative to it. It depends on win_az and
     * win_rg, so a run with a different window size has a different null and the
     * raw SNRs of the two runs are not comparable. Their ratios to this are. */
    double *snr;                /* [n_win] */
    double  snr_null;

    /* [n_win] AMPLITUDE DISPERSION of the window's brightest pixel across the
     * sub-look stack: sigma_A / mu_A, computed for EVERY estimator because it is
     * a property of the scene rather than of the tracker.
     *
     * WHAT IT IS FOR. rs_microm_estimator_t states the phase route's
     * precondition as one dominant scatterer per sub-look resolution cell, and
     * FOLLOW-UPS.md item 15 measured that it is a real constraint rather than a
     * formality -- at 2.75 equal scatterers per cell the estimator fails at
     * every seed and every selection policy. Until this field existed there was
     * no way to tell from a result whether the precondition had been met, which
     * made a null uninterpretable: "nothing moved" and "the estimator was never
     * applicable here" produce the same output.
     *
     * This is the standard measure of exactly that condition. Ferretti et al.
     * (2001) select persistent scatterers by amplitude dispersion, taking
     * D_A <= 0.25 as the criterion, and the quantity is the same one: a cell
     * whose return is dominated by one scatterer holds its amplitude, while a
     * cell of comparable scatterers fluctuates as their interference changes.
     *
     * THE THRESHOLD'S CALIBRATION IS NOT TRANSFERABLE AND THE FORM IS. That 0.25
     * was established over INDEPENDENT PASSES, where each acquisition is a fresh
     * realisation; sub-looks of one aperture share their scatterers and are not
     * independent, so the null distribution differs and the number should be
     * read as a scale rather than a bright line. Measured on the Giza collect at
     * 0.90 overlap the best window reached 0.381 and the median 0.583, so ZERO
     * of 225 windows met it -- a margin wide enough that the calibration
     * question does not change the reading.
     *
     * THIS IS MEASURED AT THE ARGMAX OF THE REFERENCE LOOK, WHICH BIASES IT
     * UPWARD. Selecting a pixel on one realisation and then measuring it across
     * all of them favours pixels that happened to be high in that one look, and
     * the bias grows with the number of candidates in the window. MEASURED
     * (item 23), tracking one shared stack at both sizes: 4x the candidates
     * moves the synthetic median from 0.452 to 0.477. Real, negligible, and far
     * too small to explain the 0.079-against-0.38 gap between the fixtures and
     * real collects.
     *
     * COMPARE MEDIANS ACROSS RUNS, NOT MINIMA. The best D_A in a run is a
     * minimum over its windows, so a configuration with fewer windows reports a
     * worse best for purely that reason -- on a fixed grid, win 16/32/64 gives
     * 225/49/9 windows. The same caution applies to the per-run "best" the CLI
     * prints.
     *
     * ON REAL DATA D_A RISES STEEPLY WITH WINDOW SIZE AND ON THE FIXTURES IT
     * DOES NOT: the same 4x moves the real median by 0.95 against the
     * fixtures' 0.025. Since it is the same code over the same geometry, that
     * is a statement about scenes -- the brightest return in a real scene is
     * also the most aspect-selective one, which `rs_sim_scene()` does not model.
     * See FOLLOW-UPS.md item 23d.
     *
     * Zero-amplitude windows report D_A = RS_DA_MAX rather than a division by
     * zero: an empty window is maximally dispersed, not perfectly stable.
     *
     * ON THE PHASE ROUTE THIS IS EXACTLY 1 - quality, for the reasons set out on
     * that field above. Read them together or one of them twice. */
    double *d_a;                /* [n_win] */

    /* [n_win] rms over looks of the one-sigma offset uncertainty in AZIMUTH,
     * pixels, from the curvature of each look's correlation peak. Zero for the
     * estimators that form no correlation surface.
     *
     * Azimuth alone because azimuth is the observable: the tracked azimuth shift
     * is what becomes vel_los, and a range uncertainty does not enter the
     * reported spectrum. The range figure is computed per look and discarded
     * here rather than being carried unused.
     *
     * Combined as an rms rather than a mean because variances add and standard
     * deviations do not. Individual looks are clamped at RS_COREG_SIGMA_MAX
     * before the combination, so a window whose peaks were unusable reaches a
     * large finite value instead of poisoning the aggregate with an infinity --
     * see that constant for why a clamp rather than an exclusion.
     *
     * NOT AN ERROR BAR. See rs_coreg_quality_t: the constant factor relating
     * this to a true standard deviation is omitted, and is the same for every
     * window of a run. It ranks windows against each other and is used for
     * nothing else. */
    double *sigma_px;           /* [n_win] */

    size_t n_win_az, n_win_rg, n_win, n_looks;
    size_t win_az, win_rg, stride_az, stride_rg;

    double dt;      /* s between consecutive looks, from the stack */
    double f_max;   /* Hz, vibration Nyquist limit */

    /* Pixel spacings carried through from the sub-look geometry, so that
     * consumers can convert the tracked shifts from pixels to metres without
     * needing the image stack. */
    double az_spacing_m, rg_spacing_m;

    /* Sub-pixel quantisation of the tracked shift, in pixels: 1/upsample_az for
     * the correlation estimator, and zero when the concept does not apply.
     *
     * This is a FLOOR on what the tracker can report, not a precision figure,
     * and it has to travel with the result because a consumer cannot otherwise
     * tell a measurement from a rounding artefact. A window whose whole
     * excursion is one step returns a two-valued series, and a two-valued
     * series has its energy at low frequency whatever produced the transitions.
     * See rs_spectrum_best_window(), which refuses to select such a window.
     *
     * Zero for RS_MICROM_EST_PHASE, whose observable is pixel phase rather than
     * a correlation offset and whose limit is set by phase noise instead. */
    double quant_px;
} rs_microm_t;

/* Per-window vibration spectrum and the summary maps derived from it. */
typedef struct {
    double *psd;            /* [n_win][n_freq] power spectral density */
    double *freq;           /* [n_freq] frequency axis, Hz */
    double *dominant_freq;  /* [n_win] frequency of the largest peak, Hz */
    double *amplitude;      /* [n_win] QUALITATIVE peak strength, not calibrated */
    double *quality;        /* [n_win] copied from the tracking mask */

    /* [n_win] peak-to-peak excursion of the tracked azimuth shift, in PIXELS,
     * before detrending -- and 'quant_px' carried through from rs_microm_t.
     *
     * These two travel together because neither means anything alone. The
     * excursion says how far the tracker saw the patch move; the quantisation
     * says how far it could have seen it move by accident. Their ratio is what
     * decides whether a window holds a measurement, and it is the only quantity
     * here that is not scale-free -- prominence is a ratio of powers and
     * therefore identical for a strong peak and for rounding noise. */
    double *excursion_px;
    double  quant_px;

    /* [n_win] spectral prominence: the dominant peak's power divided by the mean
     * power of the rest of the spectrum.
     *
     * This is the metric that identifies which windows actually contain a
     * vibrating target, and it earns its place by being much better at it than
     * the obvious alternatives. Selecting the window with the largest
     * displacement excursion picks the NOISIEST window, because noise excursions
     * exceed real ones; selecting by tracking coherence picks the window that
     * correlates best, which is usually static ground. Prominence asks the
     * relevant question -- does this window's motion concentrate at one
     * frequency -- and on a synthetic sweep it recovers five injected
     * frequencies of six where excursion-based selection recovers two. */
    double *prominence;

    /* [n_win] the correlation-surface statistics, carried through unchanged from
     * rs_microm_t so that a selection policy can weigh what the TRACKER knew
     * beside what the spectrum shows. Zero, with 'snr_null' zero, for estimators
     * that form no correlation surface; rs_spectrum_ampcor_window() reads that
     * as "these gates do not apply here" rather than as a failure.
     *
     * They are copied rather than referenced because rs_spectrum_t outlives no
     * rs_microm_t in particular -- tests build spectra from hand-made tracking
     * results and free them independently. */
    double *snr;
    double *sigma_px;
    double  snr_null;

    /* [n_win] amplitude dispersion, carried through from rs_microm_t so a
     * selection policy and an evidence file can both report whether the phase
     * estimator's precondition was met. See rs_microm_t.d_a. */
    double *d_a;

    size_t n_win, n_win_az, n_win_rg, n_freq;
    double df;              /* Hz per spectral bin */
} rs_spectrum_t;

/* The amplitude-dispersion criterion for a persistent scatterer.
 *
 * Ferretti et al. (2001). See rs_microm_t.d_a for what it measures, and for why
 * its calibration over independent passes does not transfer unexamined to
 * sub-looks of one aperture. */
#define RS_PS_DA_MAX 0.25

/* The value reported for a window with no amplitude at all. Chosen above any
 * physically meaningful dispersion so that an empty window sorts with the worst
 * rather than, through a zero, with the best. */
#define RS_DA_MAX 9.99

/* Fill 'params' with the published working defaults described above. */
void rs_microm_params_default(rs_microm_params_t *params);

/* Track every window across the sub-look stack and extract displacement series.
 *
 * Sub-look 0 is the reference. For each window and each subsequent look, a
 * two-dimensional normalised cross-correlation against the reference gives the
 * apparent shift, refined to sub-pixel precision by local upsampling of the
 * correlation surface around its peak (the Guizar-Sicairos approach: refine by
 * evaluating an upsampled inverse transform in a small neighbourhood, rather
 * than upsampling the whole surface).
 *
 * The tracked azimuth shift converts to a line-of-sight velocity through
 * v_r = dx * V / R, with dx the shift in metres, V the platform speed and R the
 * slant range -- all measured from the collection rather than assumed. This is
 * the unambiguous observable and the one to prefer.
 *
 * Line-of-sight displacement is separately obtained from the interferometric
 * phase between look and reference, averaged over the window, as
 * d = -lambda/(4*pi) * phi. Phase is far more precise than tracking but wraps
 * modulo lambda/2 -- 15.6 mm at X-band, which real structural motion exceeds.
 *
 * THE PHASE SERIES IS NOT UNWRAPPED. Each look's phase is expressed relative to
 * the series' own mean PHASOR -- psi = arg(z * conj(mean z)) -- which lands in
 * (-pi, pi] with nothing to fold and, crucially, makes every sample independent
 * of every other.
 *
 * It used to unwrap temporally, accumulating folded differences, which is the
 * textbook way to trade total range for per-step range. That is unusable on an
 * aperture whose sub-looks decorrelate, and the failure is not marginal. Sub-look
 * coherence is the fraction of pulses two looks share, so even 95 percent
 * overlap gives gamma = 0.85 and a per-step phase spread of 0.65 rad; over 1548
 * looks the accumulated random walk is sigma*sqrt(N) ~ 25 rad, against an
 * unambiguous range of pi/2. Measured on real data, the recovered series was a
 * random walk in every window, on distributed clutter and on a scatterer 74x
 * above its surroundings alike. Raising the overlap makes it worse, not better.
 * See rs_microm_estimator_t, which carries the measured coherence-versus-lag
 * numbers.
 *
 * WHAT THIS COSTS. The ambiguity the unwrap bought is gone: motion beyond
 * +/-lambda/4 in TOTAL now folds, where before it folded only beyond lambda/4
 * BETWEEN looks. At X band that is about 8 mm of line-of-sight motion. Against
 * it, per-sample noise is now bounded at sigma rather than growing without
 * limit, and a periodic signal still averages down across the periodogram.
 *
 * So prefer 'vel_los' -- the tracked shift -- for motion larger than lambda/4;
 * it has no ambiguity at any amplitude. Use 'disp_los' for small motion, where
 * it is far finer. 'phase' now holds psi rather than an accumulated total, so a
 * caller wanting a different reference or its own unwrap has the raw quantity.
 *
 * The loop over windows is parallelised with OpenMP when available. On success
 * '*out' owns its arrays and must be released with rs_microm_free().
 *
 * Returns RS_ERR_ARG if the stack has fewer than two looks, or if the window
 * size exceeds the image. */
resonarsat_status_t rs_microm_track(const rs_subap_stack_t *stack,
                                    const rs_microm_params_t *params,
                                    rs_microm_t *out);

/* Release everything a micro-motion result owns. */
void rs_microm_free(rs_microm_t *m);

/* Which observable a spectrum is computed from.
 *
 * Default to velocity: it does not wrap, and it is what the validated
 * literature reports. Displacement is finer but inherits the unwrapping caveat
 * on rs_microm_track(). Both give the same peak FREQUENCY for a sinusoid. */
typedef enum {
    RS_SPEC_VELOCITY = 0,
    RS_SPEC_DISPLACEMENT = 1
} rs_spectrum_source_t;

/* Compute per-window vibration spectra from tracked displacement series.
 *
 * A Hann-windowed periodogram of the selected observable's series. The
 * series are short -- one sample per sub-look -- so this is cheap, and a plain
 * periodogram is the honest choice at this record length; high-resolution
 * sparse estimators can resolve closely spaced peaks but invite reading
 * structure into noise when only a few dozen samples support them.
 *
 * The mean is removed before transforming, so a static offset does not appear
 * as a spurious zero-frequency peak, and the zero bin is excluded when the
 * dominant frequency is selected.
 *
 * On success '*out' owns its arrays and must be released with
 * rs_spectrum_free(). */
resonarsat_status_t rs_spectrum_compute(const rs_microm_t *m,
                                       rs_spectrum_source_t source,
                                       rs_spectrum_t *out);

/* Bins always excluded when choosing a dominant frequency, counting from DC.
 *
 * WHY THREE, AND WHY IT IS NOT A TUNABLE. Every series here is Hann-windowed
 * before its transform. A Hann window's main lobe is four bins wide, +/-2 about
 * the component's own bin, so any residual DC or trend deposits energy in bins
 * 1 and 2 no matter how clean the record is. Those bins do not contain a
 * separable measurement of anything; they contain the skirt of whatever sits at
 * zero. Bin 3 is the first that a leakage argument does not condemn.
 *
 * The physical reading agrees. Bin k is k cycles across the dwell, so bins 1
 * and 2 are one and two cycles of a record whose whole length is the dwell.
 * Calling one cycle a "frequency" is not a measurement of periodicity; it is a
 * measurement of the fact that the record has ends.
 *
 * MEASURED, on the real Giza collect at 128 looks over 30.71 s (df = 0.0326 Hz).
 * A 0.163 Hz injection swept downward in amplitude was reported correctly at
 * 2 mm and then, at every smaller amplitude, as 0.033 Hz -- bin 1 exactly -- in
 * 10 of 25 windows at once, with the SAME prominence to within 0.1 in windows
 * far apart. Prominence rose from 32.0 to 56.0 as the injection weakened, so
 * the answer got more confident as it got wronger, and D_A fell to 0.121
 * because a scatterer that barely moves has a stable amplitude -- passing the
 * Ferretti criterion of items 19-20 by failing to vibrate. Quality rose for the
 * same reason. Prominence, quality, D_A and the null control of item 35 all
 * endorsed it. The null could not do otherwise: its trials are synthetically
 * static, contain no dominant scatterer to impose a common trend, and topped
 * out at 23.8 against this artefact's 56.
 *
 * Excluding these three bins, the same sweep returns 0.163 Hz at every
 * amplitude from 2 mm down to 0.0625 mm. See FOLLOW-UPS item 37.
 *
 * A caller wanting the old behaviour does not get it, deliberately. There is no
 * operating point at which bin 1 means something. */
#define RS_SPECTRUM_LEAKAGE_BINS 3

/* As rs_spectrum_compute(), but ignoring every bin below 'f_min' hertz when
 * choosing the dominant peak and when computing prominence.
 *
 * The lowest bins are where a drift lives, and a drift is not a vibration. The
 * linear detrend applied to every series removes the straight-line part of one,
 * but a random walk or any curved trend survives it and piles its energy into
 * the first two or three bins -- see rs_spectrum_best_window() on how that can
 * outscore a genuine target. The quality gate normally keeps such windows out,
 * but the gate is relative, so it stops protecting when every window in the
 * scene tracks poorly.
 *
 * A band floor is the direct remedy and it is also a test. If a detection
 * survives excluding the lowest bins, whatever it is has structure at a real
 * frequency; if it evaporates, it was a trend.
 *
 * 'f_min' can only RAISE the floor, never lower it. RS_SPECTRUM_LEAKAGE_BINS
 * below is always excluded, including when 0 is passed, because those bins are
 * not separable from a trend by any amount of signal. See that constant. */
resonarsat_status_t rs_spectrum_compute_band(const rs_microm_t *m,
                                            rs_spectrum_source_t source,
                                            double f_min,
                                            rs_spectrum_t *out);

/* What is removed from each displacement series before its spectrum is taken.
 *
 *   RS_DETREND_LINEAR  Least-squares straight line. The default, and what the
 *                      two calls above use.
 *   RS_DETREND_MEAN    Mean only, leaving any ramp in the record.
 *   RS_DETREND_NONE    Nothing removed.
 *
 * WHY THIS IS SELECTABLE RATHER THAN FIXED. Detrending is not in the source
 * material; it was added here because temporal phase unwrapping random-walks
 * when the phase is noisy, producing a ramp that puts all the energy in the
 * lowest bin and makes every window report the same spurious "dominant
 * frequency" of one bin width.
 *
 * But it is not a neutral cleaning step, and under one of the models it is
 * actively destructive. A resonance interpretation maps depth as z = v/(2f), so
 * the LOWEST frequencies are the DEEPEST structure. Removing a straight line
 * preferentially attenuates exactly the part of the record that model reads as
 * deep, which means a linear detrend could suppress a real deep signal rather
 * than only a spurious one. Under Model A depth is linear in the transform's
 * bin index and no such asymmetry arises.
 *
 * A choice that helps one model and hurts another must not be a hidden default.
 * Turning it off is also a test in the same spirit as the band floor: if a
 * detection appears only without detrending and sits in the first bins, it is a
 * trend; if it survives both settings, it is not. */
typedef enum {
    RS_DETREND_LINEAR = 0,
    RS_DETREND_MEAN = 1,
    RS_DETREND_NONE = 2
} rs_detrend_t;

/* As rs_spectrum_compute_band(), with the detrend explicit. The two calls above
 * are wrappers passing RS_DETREND_LINEAR. */
resonarsat_status_t rs_spectrum_compute_opts(const rs_microm_t *m,
                                             rs_spectrum_source_t source,
                                             double f_min,
                                             rs_detrend_t detrend,
                                             rs_spectrum_t *out);

/* Release everything a spectrum result owns. */
void rs_spectrum_free(rs_spectrum_t *s);

/* Find the window whose spectrum shows the most prominent peak, among those
 * that resolved any motion at all, writing its index to '*out_window' and its
 * prominence to '*out_prominence' when those are non-NULL.
 *
 * This is how a caller should pick a window to report from a scene when it does
 * not already know where the target is. See rs_spectrum_t.prominence for why
 * the obvious alternatives are worse.
 *
 * IT IS ONE OF THREE POLICIES AND THE WEAKEST OF THEM. It is what the tool
 * reports, and FOLLOW-UPS.md items 7-9 record the finding that it discards
 * carriers the tracker did recover. rs_spectrum_consensus() asks instead what
 * the windows collectively say; rs_spectrum_ampcor_window() asks what the
 * correlation surfaces behind them looked like, which is the only one of the
 * three reading evidence from the tracker rather than from the spectrum. All
 * three apply the same two gates below, so their counts describe one population
 * and can be compared directly.
 *
 * THE QUANTISATION FLOOR, AND WHY PROMINENCE ALONE CANNOT SUPPLY IT.
 * Prominence is peak power over mean power -- a ratio, and therefore
 * scale-free. Multiply a window's whole series by any constant and its
 * prominence does not move. So prominence cannot distinguish a strong peak from
 * rounding noise, and the tracker's output is quantised: shifts are located to
 * 1/upsample of a pixel, and a window whose entire excursion is one such step
 * returns a two-valued series whose energy sits at low frequency whatever
 * produced the transitions. Such a window routinely out-scores real ones. It
 * also passes any coherence gate at the top, because a patch that never moves
 * correlates with itself perfectly.
 *
 * A window is therefore a candidate only if its excursion clears the
 * quantisation noise by three sigma. Quantisation error is uniform on +/-q/2
 * with RMS q/sqrt(12); a sinusoid of peak-to-peak A has RMS A/(2*sqrt(2)); so
 * the ratio is 1.2247*A/q and three sigma is A >= 2.449*q. That is a derived
 * limit rather than a tuned threshold, which matters because the alternative --
 * picking a fraction that works on the scene in front of you -- is how a
 * selection rule ends up fitted to one fixture.
 *
 * WHAT THIS DOES NOT DO. It does not make a marginal observable measurable.
 * On the distributed-texture fixture the floor moved RS_MICROM_REF_PAIR's
 * count from 0/9 to an apparent 2/9 -- and wider reproduction (five seeds,
 * frequencies on and off the fixture's blind bins) then showed those two, and
 * RS_MICROM_REF_FIRST's one, to be artefacts whose answers do not depend on
 * the injection at all: honest zero everywhere. The value is in what it
 * refuses, not in what it finds: without it the
 * answer is a confident wrong frequency, with it the answer is either honestly
 * absent or an artefact that repetition across seeds exposes.
 *
 * Returns RS_ERR_ARG if the spectrum is empty, and RS_ERR_RANGE if no window
 * cleared the floor -- which is a result, not a failure, and callers must
 * report it as "nothing resolved" rather than falling back to a window. When
 * 'quant_px' is zero the floor cannot be evaluated and every window is a
 * candidate, which is the behaviour for estimators whose limit is not a
 * correlation quantisation.
 *
 * HOW MANY WINDOWS WERE ELIGIBLE IS PART OF THE ANSWER, which is why
 * 'out_n_candidates' exists and why callers should present it beside the
 * selection rather than treating it as a diagnostic.
 *
 * It counts windows surviving BOTH gates -- the relative coherence gate and the
 * floor -- so a caller must not describe it as the floor's doing alone. On a
 * phase run, where 'quant_px' is zero and the floor never runs, every exclusion
 * is the coherence gate.
 *
 * The floor is three sigma for ONE window. It is then applied to every window
 * independently and the best survivor returned, with nothing accounting for how
 * many were tried -- so the chance that some window crosses it on quantisation
 * noise alone grows with the grid. Measured on real data: the same scene and
 * the same chain gave 225 of 225 windows at exactly zero excursion and an
 * honest RS_ERR_RANGE at one grid size, and 958 of 961 at zero with two
 * crossings and a confident-looking "0.183 Hz, prominence 29.9" at a larger
 * one. The observable was identical; only the number of opportunities changed.
 *
 * So a small count is the signature of that effect, and there is a
 * non-arbitrary place to draw the line. Windows are laid down at a stride,
 * typically half their width, so they OVERLAP: a target big enough to be
 * resolved at all falls inside a 2x2 block of them at minimum. Fewer than four
 * qualifying windows therefore cannot describe a spatially resolved mode, and
 * two scattered ones describe noise. That bound comes from the window geometry
 * rather than from a tuned constant, which matters here for the same reason it
 * mattered for the floor itself.
 *
 * STILL OPEN, and measured: on one real scene the same chain gave an honest
 * RS_ERR_RANGE at 225 windows and "0.183 Hz, prominence 29.9" at 961, off two
 * chance crossings, with the observable identically zero in 99.7 percent of
 * windows both times. A proper fix -- a multiplicity correction over the
 * EFFECTIVE number of independent windows, which is fewer than n_win because
 * overlapping windows are correlated, or ranking on the qualifying FRACTION
 * and its spatial contiguity instead of on one window's prominence -- is not
 * implemented.
 * Reporting the count is the honest interim: "2 of 961 cleared the floor" reads
 * as noise where "prominence 29.9" reads as a detection, and they are the same
 * result. Pass NULL if the count genuinely is not wanted. */
resonarsat_status_t rs_spectrum_best_window(const rs_spectrum_t *spec,
                                            size_t *out_window,
                                            double *out_prominence,
                                            size_t *out_n_candidates);

/* The frequency the most windows agree on, and how many agree.
 *
 * rs_spectrum_best_window() answers "which single window is most prominent";
 * this answers "what do the windows agree on", which is a different question and
 * the one a detection needs. A window can clear every gate and still report a
 * frequency no other window reports -- measured on a 3.000 Hz injection, 23 of
 * 49 windows made 2.995 Hz their top bin while 16 made 2.604 Hz theirs, and the
 * single most prominent window was one of the sixteen.
 *
 * Windows agree when their dominant frequencies fall in the same bin, tested at
 * half a bin. Only windows passing the SAME gates as rs_spectrum_best_window()
 * vote, so the two functions describe the same population and their counts are
 * comparable.
 *
 * 'out_n_agree' and 'out_n_distinct' ARE THE ANSWER, not diagnostics. The
 * measured behaviour on synthetic fixtures with known ground truth:
 *
 *     agreement      distinct winners     outcome
 *     47-61%         5-11                 3 of 3 recovered the injection
 *     14-24%         15-28                mostly wrong
 *     static scene   19-27                no motion present
 *
 * A fragmented vote and a motionless scene look alike, which is the property
 * this file has otherwise lacked: a single window's argmax is equally confident
 * whether or not anything moved, and that is why prominence turned out to be
 * anti-correlated with correctness. Nineteen distinct winners over 49 windows
 * should read as "no consensus" however prominent the leader is.
 *
 * THAT HOLDS FOR SCENE-DRIVEN NOISE ONLY, AND THE EXCEPTION MATTERS. Agreement
 * detects noise that is INDEPENDENT across windows. An artefact produced by the
 * processing rather than by the scene appears identically in every window, so
 * the windows agree about it unanimously and this statistic reports its highest
 * possible confidence. Measured: the phase estimator returns 0.407 Hz for every
 * injection from 0.2 to 0.7 Hz AND for a motionless scene, with 9 of 9 windows
 * agreeing in all seven cases. No threshold helps, because 100% is the ceiling.
 *
 * The only check that sees a common-mode artefact is a null control -- the same
 * processing over a scene known to be motionless, compared -- because such an
 * artefact is by definition identical whether or not anything moved. That is
 * rs_null_static() and mmotion's --null-static. This function does not replace
 * it and must not be presented as doing so: a run wants both, agreement for
 * scattered noise and a null control for coherent noise.
 *
 * NOT A THRESHOLD, and deliberately no threshold is applied here. The 40%
 * boundary above rests on three correct detections from one seed and one fixture
 * family; it is far too little to hard-code, and a caller that wants to gate on
 * it should do so where the choice is visible. Returning the counts and letting
 * the caller decide is the honest interim, exactly as reporting
 * 'out_n_candidates' is for the function above.
 *
 * 'out_n_contiguous' IS WHERE THE AGREEING WINDOWS ARE, not merely how many.
 * It is the size of the largest 4-connected block of agreeing windows on the
 * window grid, and it separates a mode from a coincidence in a way the count
 * alone cannot: twenty-three windows scattered across the scene and
 * twenty-three forming a patch are the same number and not the same evidence.
 * A vibrating structure occupies contiguous ground; chance crossings do not.
 *
 * There is a non-arbitrary floor for it, and it is the one
 * rs_spectrum_best_window()'s header already derives for the candidate count.
 * Windows are laid at a stride of typically half their width, so they overlap
 * and a target large enough to be resolved at all falls inside a 2x2 block at
 * minimum. A largest block below four therefore cannot describe a spatially
 * resolved mode whatever the agreement percentage says. That bound comes from
 * the window geometry rather than from a tuned constant.
 *
 * Returns RS_ERR_RANGE when no window passes the gates, matching
 * rs_spectrum_best_window(), and RS_ERR_ARG on a NULL or empty spectrum. */
resonarsat_status_t rs_spectrum_consensus(const rs_spectrum_t *spec,
                                          double *out_freq,
                                          size_t *out_n_agree,
                                          size_t *out_n_distinct,
                                          size_t *out_n_voting,
                                          size_t *out_n_contiguous);

/* What a cull kept, what it removed, and at which gate. */
typedef struct {
    size_t window;        /* the selected window; n_win if nothing survived */
    double freq_hz;       /* the frequency the survivors agree on */
    double snr;           /* the selected window's correlation SNR */
    double sigma_px;      /* and its offset uncertainty */

    size_t n_input;       /* entered the cull, i.e. passed the shared gates */
    size_t n_snr_cull;    /* removed by the surface-SNR gate */
    size_t n_sigma_cull;  /* removed by the offset-uncertainty gate */
    size_t n_neigh_cull;  /* removed by the neighbourhood-consistency gate */
    size_t n_survivor;    /* passed all three */
    size_t n_agree;       /* survivors sharing the reported frequency bin */

    double snr_gate;      /* the thresholds actually applied, for the record */
    double sigma_gate;    /* px; twice the entrants' median offset uncertainty */
    int    gates_applied; /* zero when the estimator has no surface statistics */
} rs_spectrum_cull_t;

/* Select a window the way the offset-tracking correlators do: by throwing away
 * everything whose measurement was not well determined, and reading the answer
 * off what is left.
 *
 * WHY A THIRD SELECTION POLICY. rs_spectrum_best_window() ranks by prominence
 * and is what the tool reports; rs_spectrum_consensus() asks what the windows
 * agree on. FOLLOW-UPS items 7-9 record the finding that motivates this one: on
 * the synthetic fixture the TRACKER recovers the injected carrier in most
 * windows and the SELECTION POLICY discards it. Both existing policies read only
 * the spectrum. Neither asks the question the ampcor family of correlators has
 * always asked first -- was this offset series worth transforming at all -- and
 * that question is answerable from quantities the correlator already computes
 * and this pipeline was throwing away.
 *
 * THE THREE GATES, IN ORDER, EACH DERIVED RATHER THAN TUNED.
 *
 * 1. SURFACE SNR. A window must score at least twice what its own window size
 *    scores on noise alone, 'snr_null'. The null is the harmonic number of the
 *    bin count and is derived on rs_coreg_quality_t, so the threshold is stated
 *    as a multiple of what pure noise produces rather than as a bare number.
 *    The factor of two is the one genuinely chosen quantity here; it is chosen
 *    at the scale where "twice what noise gives" is the weakest claim worth
 *    making, and 'snr_gate' is reported so a caller can see what was applied.
 *
 * 2. OFFSET UNCERTAINTY, RELATIVE TO THE SCENE. A window is culled when its
 *    sigma_px exceeds twice the MEDIAN sigma_px of the windows entering the
 *    cull -- ampcor's median-based rejection of offsets that disagree with their
 *    surroundings, applied to the covariance rather than to the offset itself.
 *
 *    THIS GATE WAS ABSOLUTE IN ITS FIRST VERSION AND THAT WAS AN ERROR, recorded
 *    in FOLLOW-UPS.md item 12c. It read 'excursion_px >= 3*sigma_px', which
 *    compares a real pixel excursion against a quantity rs_coreg_quality_t
 *    states explicitly is not calibrated in an absolute sense. Measured: on an
 *    isolated point target whose surfaces scored an SNR near 80 -- ten times the
 *    noise-alone value, so gate 1 culled nothing -- sigma came out at 130 to 200
 *    PIXELS on 32-pixel patches, and the gate removed every window at every
 *    frequency of a sweep. The relative form is what an uncalibrated ranking
 *    statistic supports, and it is the form the sources use.
 *
 *    The excursion is not left unguarded: rs_spectrum_best_window()'s
 *    quantisation floor is applied before this, and it IS calibrated -- it asks
 *    whether the series moved further than the tracker's own grid step. That
 *    remains the absolute test; this is the relative one.
 *
 * 3. NEIGHBOURHOOD CONSISTENCY. A window must have at least TWO of its four
 *    lattice neighbours reporting the same frequency bin, among neighbours that
 *    passed the first two gates. This is ampcor's cull of offsets that disagree
 *    with their neighbours, applied to the frequency rather than to the offset,
 *    and its threshold is the geometric bound this codebase already uses: a
 *    resolvable target falls inside a 2x2 block of overlapping windows, and each
 *    cell of a 2x2 block has exactly two of its four neighbours inside the
 *    block. So two agreeing neighbours is precisely "belongs to a block or
 *    better", and it is what an isolated window cannot produce. A line of three
 *    agreeing windows loses its ends and then its middle, which is intended: a
 *    one-window-wide streak is the shape of a processing artefact along an axis,
 *    not of a structure.
 *
 * HOW THIS RELATES TO THE CONSENSUS GATE, AND WHAT IT STILL CANNOT DO. Gate 3
 * is a stricter, local form of the contiguity figure rs_spectrum_consensus()
 * reports, and gates 1 and 2 are new information -- they come from the
 * correlator, not from the spectrum. That makes this policy sensitive to a
 * failure the consensus is blind to by construction, since a window that tracked
 * nothing can still agree with its neighbours about a common-mode artefact but
 * cannot manufacture a sharp, well-determined correlation peak.
 *
 * IT IS NOT A NULL CONTROL AND DOES NOT BECOME ONE. FOLLOW-UPS item 11's finding
 * survives here unchanged: an artefact produced by the processing appears in
 * every window with a genuine, well-determined correlation behind it, and passes
 * all three gates. Only running a motionless scene through identical processing
 * catches that. This narrows which windows are believed; it does not decide
 * whether the ground moved.
 *
 * WHEN THE STATISTICS DO NOT EXIST. The phase and split-band estimators form no
 * correlation surface, so 'snr' and 'sigma_px' are zero and 'snr_null' is zero.
 * Gates 1 and 2 are then SKIPPED rather than failed -- 'gates_applied' is set to
 * zero to say so -- and gate 3 still runs, because neighbourhood consistency
 * needs only the frequencies. A caller must read 'gates_applied' before treating
 * a survivor count as evidence of anything the correlator vouched for.
 *
 * 'out' receives the counts at every stage, which is the point: a policy that
 * reports only its winner cannot be audited, and the per-gate counts say whether
 * a null came from a scene with no motion or from a threshold that removed
 * everything. Returns RS_ERR_ARG on a NULL or empty spectrum, and RS_ERR_RANGE
 * when nothing survives -- with the counts filled in, so the caller can still
 * see where the population went. */
resonarsat_status_t rs_spectrum_ampcor_window(const rs_spectrum_t *spec,
                                              unsigned char *out_state,
                                              rs_spectrum_cull_t *out);

/* As rs_spectrum_ampcor_window(), with both tuned factors explicit.
 *
 * 'snr_factor' multiplies the noise-alone SNR to give gate 1's threshold, and
 * 'sigma_factor' multiplies the entrants' median uncertainty to give gate 2's.
 * The call above passes the defaults; a non-positive value here disables that
 * gate rather than closing it, so a caller can measure one gate's effect with
 * the other out of the way.
 *
 * 'min_neighbours' is gate 3's threshold, in agreeing 4-neighbours; zero
 * disables that gate. Unlike the other two this one is DERIVED rather than tuned
 * -- two is the in-block neighbour count of a 2x2 block -- and it is exposed
 * anyway, because a derivation is a claim about the world and a claim about the
 * world can be checked. Sweeping it is how one finds out whether the geometry
 * argument survives contact with a real window population, which is exactly what
 * gate 3's first version did not (FOLLOW-UPS.md item 12c).
 *
 * EXPOSED SO THE CONSTANTS CAN BE SWEPT RATHER THAN ARGUED ABOUT. A constant
 * compiled into a selection policy is a claim nobody can check without editing
 * the source and rebuilding. tests/test_cullsweep.c sweeps these across ONE set
 * of spectra, so the threshold is the only thing varying and any difference is
 * attributable to it alone. */
resonarsat_status_t rs_spectrum_ampcor_window_opts(const rs_spectrum_t *spec,
                                                   double snr_factor,
                                                   double sigma_factor,
                                                   size_t min_neighbours,
                                                   unsigned char *out_state,
                                                   rs_spectrum_cull_t *out);

/* 'out_state', on both calls above, is a caller-supplied buffer of spec->n_win
 * bytes, or NULL when the per-window detail is not wanted. Each byte receives
 * what happened to that window:
 *
 *   0  did not enter -- failed the shared coherence gate or the quantisation
 *      floor, so the cull never saw it
 *   1  entered and was removed by gate 1 (surface SNR) or gate 2 (offset
 *      uncertainty)
 *   2  entered, cleared gates 1 and 2, and was removed by gate 3 (too few
 *      agreeing four-neighbours)
 *   3  survived all three
 *
 * It is cleared before any early return, so a caller sees "nothing entered"
 * rather than uninitialised memory when the spectrum is rejected.
 *
 * WHY THIS EXISTS RATHER THAN LETTING A CALLER RECOMPUTE. mmotion's
 * PREFIX_windows.csv used to derive its 'passed_cull' column from the recorded
 * thresholds, which cannot reproduce gate 3 -- that gate reads a neighbourhood,
 * and the recomputation had no way to reconstruct which windows had entered. The
 * column therefore reported 170 survivors on a Giza run whose header said 65,
 * because 105 windows were removed on neighbours and the column could not see
 * it. See docs/CODE-REVIEW.md finding 2. The evidence file is the artefact this
 * project asks a reader to consult before believing a summary line, so it has to
 * carry the selector's own verdict rather than an approximation of it. */

/* How many gated windows agree with ONE NOMINATED frequency, and the largest
 * 4-connected block among them.
 *
 * rs_spectrum_consensus() answers the same two questions about the frequency the
 * plurality picked. This answers them about a frequency the caller names --
 * normally the one being reported -- which is a different question whenever the
 * scene's plurality is not the scene's signal.
 *
 * WHY THAT DISTINCTION IS NOT ACADEMIC. On the Giza injection sweep the plurality
 * lands on a 12-window block at 0.065 Hz that is present in the UNINJECTED control
 * too, so the consensus statistic describes a common-mode artefact while the
 * injected frequency occupies its own 9-12 window block elsewhere on the grid.
 * Asking about the reported frequency finds the second; asking about the
 * plurality finds the first. See FOLLOW-UPS.md item 31.
 *
 * Both counts are taken over the population every selection policy gates on, so
 * they are comparable with rs_spectrum_consensus()'s. 'out_n_block' is left at
 * zero when the window lattice is not rectangular, since neighbours are then
 * undefined. Either output may be NULL. */
resonarsat_status_t rs_spectrum_block_at(const rs_spectrum_t *spec, double freq_hz,
                                         size_t *out_n_agree, size_t *out_n_block);

/* What selecting on amplitude dispersion found. See rs_spectrum_ps_window(). */
typedef struct {
    size_t window;        /* lowest-dispersion window at the reported frequency */
    double freq_hz;       /* what the selected persistent scatterers agree on */
    double d_a;           /* the selected window's amplitude dispersion */
    double da_gate;       /* the criterion applied, for the record */
    size_t n_input;       /* windows passing the shared gates */
    size_t n_candidate;   /* of those, meeting the dispersion criterion */
    size_t n_agree;       /* of those, sharing the reported bin */
} rs_spectrum_ps_t;

/* Which windows hold a persistent scatterer, and what they say.
 *
 * WHY THIS IS A SELECTOR AND NOT A GATE. rs_microm_t.d_a reports amplitude
 * dispersion because the phase route needs a dominant scatterer per resolution
 * cell, and mmotion warns when no window has one. That warning is the wrong
 * shape for a real scene: Biondi et al. (Mosul Dam) -- the origin of the
 * sub-aperture tracking this project implements -- do not gate on D_A, they
 * SELECT on it, measuring displacement only at the candidate persistent
 * scatterers and ignoring the rest of the image. On a bridge over water that
 * distinction decides the result, because two hundred windows of water will
 * drown any scene-wide summary while the handful on the deck are the entire
 * measurement.
 *
 * THE THRESHOLD IS MEASURED HERE RATHER THAN BORROWED. Ferretti et al. give
 * D_A <= 0.25 for persistent scatterers over independent PASSES; sub-looks of
 * one aperture share their scatterers and are not independent, so the
 * calibration had to be checked. Measured over 1800 windows -- four fixtures
 * spanning the recovery boundary, six injected frequencies, three seeds, phase
 * estimator -- by whether each window's own dominant bin matched its injection:
 *
 *     D_A <= t    windows   correct    rate
 *        0.20         234       230   98.3%
 *        0.25         318       302   95.0%
 *        0.30         387       354   91.5%
 *        0.40         627       478   76.2%
 *        0.50        1374       560   40.8%
 *        all         1800       600   33.3%
 *
 * The knee is at 0.25 and the multi-pass constant survives contact with
 * sub-looks: selecting on it takes a 33 percent window-level hit rate to 95.
 * That is the whole case for this function, and it is why the number is the same
 * as the literature's while no longer being taken on faith.
 *
 * 'out' receives the reported frequency, the selected window and the counts at
 * each stage. Returns RS_ERR_RANGE when no window meets the criterion -- which
 * is a result rather than an error, and is what a scene with no persistent
 * scatterer looks like. RS_ERR_ARG on a NULL or empty spectrum. */
resonarsat_status_t rs_spectrum_ps_window(const rs_spectrum_t *spec,
                                          rs_spectrum_ps_t *out);

/* As rs_spectrum_ps_window(), with the dispersion criterion explicit.
 *
 * Exposed for the same reason the cull's factors are: the constant above is
 * measured on synthetic fixtures at one look count, and a caller re-measuring it
 * on other data should not have to edit the source. A non-positive value selects
 * every window that passed the shared gates, which is the no-criterion control. */
resonarsat_status_t rs_spectrum_ps_window_opts(const rs_spectrum_t *spec,
                                               double da_max,
                                               rs_spectrum_ps_t *out);

/* Prominence at a NOMINATED frequency, for one window, rather than at whichever
 * frequency happens to be largest.
 *
 * WHY THIS IS A DIFFERENT QUESTION. Everything else here reports each window's
 * dominant peak and how much it stands out. That answers "what is the strongest
 * thing in this window", which is the wrong question whenever the frequency of
 * interest is already known -- and it is known in every injection experiment
 * this project runs.
 *
 * Item 38 is where the difference became load-bearing. A zero-amplitude
 * injection -- a bright scatterer added to the phase history that does not move
 * -- outscored every real injection on dominant-peak prominence, because its
 * residual trend produced a bigger peak at the band floor than a real 0.0625 mm
 * vibration produced at its own frequency. Compared at the DOMINANT frequency
 * the motionless target wins. The comparison that distinguishes them has to be
 * made at one fixed frequency in both runs, which is what this returns.
 *
 * The paired statistic it exists for is
 *
 *     T(w) = prominence_injected(w, f0) - prominence_zero_amplitude(w, f0)
 *
 * -- did running the injection at a real amplitude ADD evidence at f0, over
 * running the identical code path at zero. That subtracts off everything the
 * injection machinery does regardless of motion, which no absolute statistic
 * can separate.
 *
 * 'freq_hz' is snapped to the nearest bin; the bin actually used is returned
 * through 'out_bin' so a caller can report what it measured rather than what it
 * asked for. Bins below RS_SPECTRUM_LEAKAGE_BINS are ADMISSIBLE here, unlike in
 * peak selection: a caller naming a frequency is not searching, so the
 * look-elsewhere reasoning behind the floor does not apply -- but a value from
 * inside the Hann skirt is still not separable from a trend, and 'out_bin'
 * being below the floor is how a caller sees that.
 *
 * 'out_psd' and 'out_prom' are optional. Prominence uses the same definition as
 * the dominant-peak column -- the bin's power over the mean of the others -- so
 * the two are directly comparable. */
resonarsat_status_t rs_spectrum_prominence_at(const rs_spectrum_t *spec,
                                              size_t window,
                                              double freq_hz,
                                              size_t *out_bin,
                                              double *out_psd,
                                              double *out_prom);

/* What a locally-normalised search found. See rs_spectrum_local_window(). */
typedef struct {
    size_t window;        /* the window holding the strongest local peak */
    size_t bin;           /* its bin; n_freq if nothing qualified */
    double freq_hz;
    double ratio;         /* peak power over the median of its own neighbourhood */
    double ref_median;    /* that median */
    size_t n_ref;         /* reference bins behind it */
    size_t n_searched;    /* window-bin pairs scored -- the family size */
} rs_local_peak_t;

/* Guard bins excluded either side of a candidate when estimating its own
 * background. A Hann main lobe is four bins wide, so a real peak deposits power
 * two bins each way and would otherwise inflate the background it is measured
 * against. Same derivation as RS_SPECTRUM_LEAKAGE_BINS. */
#define RS_LOCAL_GUARD_BINS 2u

/* Half-width of the neighbourhood the background is taken from, in bins. Wide
 * enough that a median over roughly twenty bins is stable, narrow enough that a
 * red noise floor is locally flat across it. Not derived; see the measured
 * sensitivity in FOLLOW-UPS item 47. */
#define RS_LOCAL_HALF_BINS 12u

/* Find the strongest peak measured against ITS OWN neighbourhood rather than
 * against the whole spectrum.
 *
 * WHY THE ORDINARY PROMINENCE IS BIASED, AND IT IS NOT SUBTLE. `prominence` is a
 * bin's power over the mean of every other bin, which is the right statistic
 * only if the noise is white. On this data it is emphatically not. Measured on
 * the real ICEYE Houston collect with nothing injected, 25 windows averaged:
 *
 *     bin  1  0.174 Hz   15.1x the median bin
 *     bin  2  0.349 Hz   18.7x
 *     bin  3  0.523 Hz   15.7x
 *     bin  9  1.569 Hz    5.1x
 *     bin 32  5.580 Hz    0.8x
 *
 * Bins 1-4 carry 24 TIMES the power of the band above Nyquist/2. The noise is
 * red, and the cause is in the processing rather than the scene: at
 * `--overlap 0.90` adjacent sub-looks share nine tenths of their pulses, so
 * their phase errors are correlated and the error series is smooth.
 *
 * That is the trend items 37 and 45 kept reporting. It is not a polynomial --
 * a linear detrend removes 1.4 percent of the spread and a cubic only 17 -- so
 * detrending cannot reach it and excluding the lowest bins only moves the
 * reported answer to the first bin that remains. Against a global mean, ANY
 * low-frequency bin of pure noise scores highly, so an uninjected scene reliably
 * reports the band floor.
 *
 * THE UNCOMFORTABLE PART: item 14 recommends high overlap for this estimator,
 * because overlap is what buys sub-look coherence on a real collect. The same
 * setting manufactures the correlated noise that beats the signal. Items 13 and
 * 14 traded overlap against the response ceiling; this is a third cost neither
 * accounted for.
 *
 * WHAT THIS DOES INSTEAD. Each bin is scored against the median of the bins
 * around it, excluding RS_LOCAL_GUARD_BINS either side so that a real peak does
 * not raise its own background. A median rather than a mean because a second
 * genuine tone inside the neighbourhood would drag an average. That is a
 * frequency-domain CFAR, and it makes the score comparable across frequencies
 * on a coloured floor.
 *
 * MEASURED, on ICEYE Houston, searching from RS_SPECTRUM_LEAKAGE_BINS upward:
 *
 *                       against the global mean   against the local median
 *     no injection      26.2 at 0.523 Hz          47.5 at 1.221 Hz
 *     1.0 Hz injected   87.4 at 1.046 Hz        1757.2 at 1.046 Hz
 *
 * The control stops preferring the band floor, and the separation between the
 * injected run and the control widens from 3.3x to 37x.
 *
 * IT FAILS ON A STEEP ENOUGH FLOOR, which the test found rather than the
 * design anticipating it. Against a random walk -- power falling as 1/f^2 --
 * the slope across a +-RS_LOCAL_HALF_BINS neighbourhood is itself large, so a
 * bin at the bottom of the band still beats its own background and the bias
 * survives. This works because the real floor is not that steep: the ICEYE
 * measurement above is 24x across the whole band, and overlap-induced
 * correlation is a MOVING AVERAGE over shared pulses rather than an
 * integration, so its power rolls off as sinc^2 and is locally flat over twenty
 * bins. A collect whose floor is steeper than that would need a narrower
 * neighbourhood or a fitted slope, and neither is implemented.
 *
 * IT DOES NOT MAKE THE CONTROL SILENT. Pure noise still produces a best local
 * ratio -- 47.5 above -- because this takes a maximum over every window and
 * every bin, so the look-elsewhere cost is inside it. The number to compare
 * against is another scene's maximum, exactly as for rs_spectrum_scene_null().
 *
 * 'out' is cleared on any non-OK return. */
resonarsat_status_t rs_spectrum_local_window(const rs_spectrum_t *spec,
                                             rs_local_peak_t *out);

/* Where a target sits, to better than one window. See rs_spectrum_centroid(). */
typedef struct {
    size_t seed;          /* the window the centroid was grown from */
    double freq_hz;       /* the frequency the cluster agrees on */

    double c_az, c_rg;    /* centroid in FRACTIONAL window indices */
    double az_px, rg_px;  /* and in image pixels, if the caller supplied geometry */

    size_t n_cluster;     /* windows in the agreeing 4-connected block */
    int    clipped;       /* non-zero if the cluster touches a grid edge */
} rs_centroid_t;

/* Locate a target to better than the window spacing, by taking the centre of
 * mass of the windows that agree on its frequency.
 *
 * WHY A SINGLE WINDOW INDEX IS NOT THE ANSWER. Item 41 measured this project's
 * localisation at exactly one window, 5 placements out of 5, never better and
 * never worse. The cause is geometric rather than statistical: at the usual
 * win 32 on stride 16 the windows overlap by half, so a target sits inside FOUR
 * of them at once. Those four contain the same dominant scatterer, the phase
 * estimator tracks that scatterer's phase by construction, and they return
 * bit-identical displacement series -- thirteen such pairs in one Giza run.
 * Their prominences then agree to 1.5 percent, and, measured, THE WINDOW THE
 * TARGET IS CENTRED IN SCORES LOWEST of the group: 38.56 against 39.14.
 *
 * So argmax cannot be right except by accident. It is choosing between windows
 * that carry the same evidence, on a difference that is noise, and item 37
 * recorded the winner moving with injected amplitude for that reason.
 *
 * WHAT THE CLUSTER IS, AND WHY THERE IS NO TOLERANCE PARAMETER. The obvious
 * construction -- "windows within x percent of the peak" -- needs a threshold
 * nobody can derive. This uses the block that already exists: the 4-connected
 * set of windows whose dominant frequency matches the seed's to within half a
 * bin, which is rs_spectrum_block_at()'s notion of agreement. Membership is
 * then decided by the measurement rather than by a constant.
 *
 * THE WEIGHT IS PROMINENCE ABOVE THE SCENE MEDIAN, floored at zero. Weighting
 * by raw prominence lets the background pull the answer toward the middle of
 * the cluster's bounding box; weighting by the excess over what an ordinary
 * window in this scene scores puts the mass where the evidence is. Measured
 * across five placements, raw prominence gives a mean error of 0.463 windows
 * and the excess gives 0.403. Squaring the excess changes nothing (0.405), so
 * the plain excess is used.
 *
 * MEASURED, against target positions taken from the imagery -- two focus runs
 * differing only in --inject-vib, differenced -- not from any reported window:
 *
 *     argmax                    mean error 1.000 windows, worst 1.00
 *     centroid, raw prominence  mean error 0.463 windows, worst 0.59
 *     centroid, excess          mean error 0.403 windows, worst 0.50
 *
 * Those five placements all put the target ON the grid boundary, which is what
 * the residual 0.4 is. Repeated at five INTERIOR placements on the same collect:
 *
 *     argmax    mean error 1.000 windows = 16.0 m, worst 1.00
 *     centroid  mean error 0.008 windows =  0.1 m, worst 0.01
 *
 * A factor of 125, and finer than the 1.0 m grid cell -- a centre of mass over
 * nine windows is not limited by the window spacing any more than a centroid of
 * a star image is limited by the pixel pitch.
 *
 * THAT NUMBER IS AMPLITUDE-DEPENDENT AND MUST BE QUOTED WITH ONE (item 43). The
 * same five placements at 0.125 mm, sixteen times weaker:
 *
 *     centroid @ 2.000 mm   0.0084 windows = 0.13 m
 *     centroid @ 0.125 mm   0.1400 windows = 2.24 m
 *     argmax   @ 0.125 mm   1.0000 windows = 16.0 m
 *
 * A 16x weaker signal costs 17x the error -- linear in 1/amplitude to within the
 * measurement, because what sets the error is the target's excess prominence
 * against the background gradient and that ratio scales with amplitude. argmax
 * stays at exactly one window at both, being quantised to an integer index.
 *
 * The residual is a WEIGHTING artefact, not geometry. The cluster is a symmetric
 * 3x3 block whose members agree to about 1.5 percent -- item 41's spread -- with
 * a monotone gradient toward higher indices, steeper in range, and the centre of
 * mass inherits it. Weighting every member equally is EXACT on a symmetric
 * cluster and was tried: 0.000 on four placements and 0.200 on the fifth, whose
 * cluster came out asymmetric at ten windows. Mean 0.0400 against the excess
 * weighting's 0.0083. Symmetry cannot be assumed and a background window that
 * happened to agree would drag a uniform centroid while contributing nothing
 * here, so the robustness is kept and the bias documented.
 *
 * 'clipped' MEANS THE ANSWER IS NEAR THE BOUNDARY, not that the cluster
 * reaches it. Those are different and the difference was measured: four of five
 * accurate placements had both the agreeing cluster and its weight-bearing
 * windows touching the grid edge, and were still right to 0.01 windows, because
 * the target's own footprint was complete. What biases a centroid is the TARGET
 * sitting at the boundary so that half of its footprint is off the grid --
 * worth close to half a window when it happens. The flag is therefore set when
 * the centroid itself lies within one window of the edge. A clipped answer is
 * pulled toward the scene centre by an amount this does not correct; move the
 * grid rather than trusting it.
 *
 * 'stride_az', 'stride_rg', 'win_az' and 'win_rg' convert the answer to pixels;
 * pass zeros to skip that and read 'c_az' and 'c_rg' only. The convention is
 * rs_microm_track()'s: window w spans pixels [w * stride, w * stride + win), so
 * its centre is w * stride + (win - 1) / 2.
 *
 * This locates; it does not detect. The cluster is grown from a window the
 * caller has already chosen to believe. */
resonarsat_status_t rs_spectrum_centroid(const rs_spectrum_t *spec,
                                         size_t seed,
                                         size_t stride_az, size_t stride_rg,
                                         size_t win_az, size_t win_rg,
                                         rs_centroid_t *out);

/* What a scene-derived null found. See rs_spectrum_scene_null(). */
typedef struct {
    size_t window;        /* the window maximising z; n_win if none qualified */
    double freq_hz;       /* that window's dominant frequency */
    double prominence;    /* and its prominence */

    double z;             /* (prominence - ref_median) / ref_scale */
    double ref_median;    /* the matched reference set's median prominence */
    double ref_scale;     /* 1.4826 * MAD of it, the robust sigma equivalent */
    size_t n_ref;         /* reference windows behind that z */

    size_t n_searched;    /* windows that got a z -- the family size */
    double z_runner_up;   /* the second-highest z anywhere in the scene */
    int    matched;       /* non-zero if D_A matching was applied */
} rs_scene_null_t;

/* Minimum reference windows before a z is computed for a candidate. Below this
 * a median and a MAD are describing noise rather than a background. */
#define RS_SCENE_NULL_MIN_REF 8u

/* Reference windows retained after matching, closest first in |dD_A|. Enough
 * that the MAD is stable, few enough that the retained set still resembles the
 * candidate on a scene of a few hundred windows. */
#define RS_SCENE_NULL_MATCH 32u

/* Score every window against the rest of its OWN scene, and return the strongest.
 *
 * WHY THIS EXISTS. --null-static synthesises motionless scenes and processes
 * them through the same chain, which is the right idea and has one structural
 * limit: it calibrates only against noise the simulator can produce. Item 37 is
 * that limit arriving. A residual trend on the real collect reached prominence
 * 56 while the synthetic null topped out at 23.8, so the artefact was scored as
 * a detection by a control that could not generate it. No amount of extra
 * trials fixes that; the model is missing the term.
 *
 * A real collect already contains hundreds of places where nothing is moving.
 * Those are the null. They cost nothing to evaluate, and they carry the scene's
 * own trends, brightness statistics and processing artefacts by construction --
 * including any this project has not thought of.
 *
 * WHY NOT A SURROGATE OF THE TIME SERIES. Four standard constructions were
 * considered and all of them are circular or benign here:
 *   - random permutation destroys the temporal correlation AND the trend, which
 *     reproduces exactly the too-benign background that made --null-static fail;
 *   - phase scrambling preserves the periodogram magnitude, and prominence is
 *     computed FROM that periodogram, so the surrogate carries the same peak;
 *   - a circular shift preserves the spectrum exactly, so it cannot test whether
 *     a spectral peak is real;
 *   - a residual bootstrap after detrending inherits only what the trend model
 *     left, and the disputed artefact is precisely what that model removes.
 * Every one of them fails because the null must keep the low-frequency nuisance
 * while destroying the localisation, and a time-domain surrogate of a single
 * series cannot separate those. Space can: the nuisance is shared across
 * windows, the target is not.
 *
 * THE GUARD RADIUS is not optional and is not a tuning knob. Windows overlap
 * whenever stride < win, so a target in one window is physically inside its
 * neighbours' pixels too. Including those in the reference set puts the signal
 * in its own null and biases z toward zero. Pass
 *   guard = ceil(win_az / stride_az) - 1
 * which is 1 at the usual 32/16, and 0 only when the windows are disjoint.
 *
 * MATCHING. A bright, low-D_A, well-tracked window is not exchangeable with a
 * dark one, so the reference set is the RS_SCENE_NULL_MATCH windows closest to
 * the candidate in amplitude dispersion. When no D_A is present -- the
 * correlator leaves none -- matching is skipped and 'matched' reports 0, which
 * a caller must read as a weaker null rather than as an equivalent one.
 *
 * WHAT MATCHING COSTS. Each candidate gets its OWN reference set, so z is only
 * comparable between windows whose sets are comparable. A candidate whose 32
 * nearest neighbours in D_A happen to be unusually alike gets a small scale and
 * a large z for that reason alone. This was measured while building the test:
 * with a D_A pattern uncorrelated with position, a window carrying a weak
 * spill-over of the target outscored the target itself, z 51.9 against 34.6,
 * purely because its matched set had a scale of 0.15. Real scenes vary D_A
 * smoothly, which makes matched sets spatially coherent and the effect much
 * smaller, but it is not zero -- read a large z together with 'ref_scale' and
 * treat an anomalously small scale as a reason to distrust the ranking.
 *
 * THE STATISTIC is robust by necessity: the mean and standard deviation of a
 * scene containing a target are shifted by that target. Median and MAD are not.
 * 1.4826 makes the MAD comparable to a standard deviation for Gaussian data;
 * the prominence distribution is not Gaussian, so z is a robust deviation and
 * NOT a normal deviate, and must not be converted to a p-value through a normal
 * tail.
 *
 * FAMILY-WISE BY CONSTRUCTION. Every window is scored and the maximum returned,
 * so the look-elsewhere cost of searching the scene is inside the statistic
 * rather than applied to it afterwards. Calibrating it therefore needs the
 * distribution of this same maximum over scenes where nothing moves.
 *
 * MEASURED, on the real Giza collect at 256 m, 225 windows, --estimator phase:
 *
 *   uninjected control, all 225 windows scored   median -0.04, 90th 1.56,
 *                                                99th 2.46, MAXIMUM 2.53
 *   0.098 Hz injected      z = 9.84 at the injected window
 *   0.130 Hz               z = 8.04
 *   0.163 Hz               z = 8.51
 *   0.196 Hz               z = 7.45
 *   0.228 Hz               z = 5.86
 *
 * All five clear the control's family-wise maximum by 2.3x or better, and the
 * control's own maximum is the null sample that matters: it is what the same
 * window search returns on the same scene with nothing in it.
 *
 * WHAT THIS IS NOT. One control scene is one draw of the family-wise maximum,
 * so 2.53 is an observation and not a calibrated threshold -- it says where this
 * collect sat, not where the next one will. Prominence is scene- and
 * processing-dependent, so a z from one configuration does not transfer to
 * another. And this null shares the estimator, the detrend and the band floor
 * with the measurement, so it cannot test any of them; a defect common to every
 * window is invisible to it exactly as it is to --null-static. It answers one
 * question only: is this window unusual FOR THIS SCENE.
 *
 * 'out' is cleared on any non-OK return. */
resonarsat_status_t rs_spectrum_scene_null(const rs_spectrum_t *spec,
                                           size_t guard,
                                           rs_scene_null_t *out);

/* Return the observation ratio implied by a sub-aperture duration and a measured
 * frequency: t_sap divided by that frequency's period, i.e. how many cycles of
 * the motion each sub-look integrates over.
 *
 * WHAT THIS DOES AND DOES NOT MEAN. This was previously documented as a
 * validity threshold -- above 0.5 the sub-look "smears the target away" and the
 * measurement "should not be trusted". That was wrong, and it was wrong in a
 * way that would have thrown away real results.
 *
 * Integrating a sinusoid over a window of length t_sap does not destroy it. It
 * multiplies its amplitude by |sinc(pi*f*t_sap)|, which falls off but stays
 * finite almost everywhere. The frequency survives; only the amplitude is
 * attenuated, and this project already reports amplitude as qualitative for
 * independent reasons.
 *
 * The correction came from checking the rule against ground truth rather than
 * against intuition. Clemente et al. (EuRAD 2025) recover 87 Hz from an idling
 * van using 0.4167 s sub-apertures, and 36 Hz from a ship using 0.5 s ones,
 * both confirmed against triaxial accelerometers. Those are observation ratios
 * of 36.3 and 18.0 -- seventy times what the old threshold permitted. A rule
 * that rejects two accelerometer-validated measurements is not a rule about the
 * physics. Note that 18.0 is an exact integer, which also rules out the sinc
 * response below as a universal law; see there.
 *
 * USE rs_spectrum_subaperture_response() INSTEAD to judge a measurement. The
 * ratio remains useful as the quantity that response is computed from, and
 * because integer values of it are exactly where the response nulls. */
double rs_spectrum_observation_ratio(double t_sap, double freq);

/* Amplitude the sub-aperture window passes at 'freq', as a factor in [0,1].
 *
 * A sub-look averages the motion over its own duration, which for a sinusoid is
 * a sinc weighting: |sin(pi*f*t_sap) / (pi*f*t_sap)|. Multiply a recovered
 * amplitude by the reciprocal to undo it, if amplitudes are wanted at all.
 *
 * THIS APPLIES TO THE OFFSET-TRACKING OBSERVABLE ONLY, and the boundary is
 * worth stating carefully, because getting it wrong twice is how this function
 * came to be documented at such length.
 *
 * The sinc is the response of a window that AVERAGES DISPLACEMENT. That is what
 * a correlation tracker measures: where the patch sits, averaged over the
 * sub-look. In that picture the response nulls at every integer observation
 * ratio, f = k/t_sap, where a whole number of cycles averages to exactly zero.
 * For 0.4167 s sub-apertures that is a comb at 2.4, 4.8, 7.2 Hz and upward.
 *
 * That picture does not describe phase-based micro-Doppler, and the literature
 * says so plainly. Clemente et al. (EuRAD 2025) recover 36 Hz from a ship using
 * 0.5 s sub-apertures. The observation ratio is exactly 18.0 -- an integer,
 * where the sinc above predicts zero response -- and the measurement is
 * confirmed against triaxial accelerometers. A vibration modulates the PHASE,
 * which puts its energy into micro-Doppler sidebands in the azimuth spectrum;
 * far above 1/t_sap that sideband structure, not the averaged displacement, is
 * what carries the frequency.
 *
 * So the honest reading is a regime boundary rather than a limit:
 *
 *   f well below 1/t_sap   the sub-look tracks quasi-static displacement, the
 *                          sinc describes the amplitude, and this pipeline's
 *                          correlation tracker is in its element.
 *
 *   f near k/t_sap         the AVERAGING observable is blind there. A peak from
 *                          RS_MICROM_EST_CORRELATION beside such a null is
 *                          suspect; a phase measurement need not be.
 *
 *   f well above 1/t_sap   sidebands dominate and the averaging model stops
 *                          applying. Reaching this regime needs the phase
 *                          observable, which this interface does not yet
 *                          provide -- see the note at the top of this file.
 *
 * Returns 1.0 at zero frequency and for a non-positive t_sap, since neither
 * attenuates anything. */
double rs_spectrum_subaperture_response(double t_sap, double freq);

/* Return the number of sub-looks needed for a target of the given vibration
 * frequency and line-of-sight amplitude to sit inside the phase-ambiguity
 * interval, for a collect of the given dwell and wavelength.
 *
 * The condition comes from requiring the peak azimuth shift to fall within three
 * quarters of a sub-look resolution cell:
 *
 *     peak shift  = 2*pi*f*A * R/V
 *     resolution  = lambda*R*M / (2*V*T_dwell),  M = 1 + (N-1)(1-overlap)
 *     require       peak shift < 0.75 * resolution
 *     giving        M > (4*pi/0.75) * f * A * T_dwell / lambda
 *
 * and this returns the N implied for the given overlap. Note the direction,
 * which is the opposite of intuition: SHORTER sub-looks are needed, because a
 * coarser resolution cell makes the ambiguity interval WIDER in metres while the
 * target's shift is fixed by the geometry. Overlap works against the condition,
 * so zero overlap needs the fewest looks.
 *
 * This is a lower bound from one constraint, not an optimum. Pushing the look
 * count far past it eventually starves each sub-look of bandwidth until the
 * resolution cell exceeds the correlation window, and measured performance falls
 * away again. Use rs_vibration_fmax() and rs_observation_ratio() to check the
 * other two constraints, and sweep around the result.
 *
 * Returns 0 for degenerate inputs. */
size_t rs_microm_recommend_looks(double vib_freq, double amp_los,
                                 double t_dwell, double lambda, double overlap);


#endif /* RESONARSAT_MICROM_H */
