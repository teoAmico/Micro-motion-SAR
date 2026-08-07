<!-- Why this form exists, and the failures it is aimed at:
     docs/PREREGISTRATION.md -->
# PREREG — synthetic / look-stabilization

Filled in on: `2026-08-07`   git commit: `4190933`
Status: **[x] confirmatory   [ ] exploratory (sections 4-9 not binding)**

## 1. Question

Item 96 measured a **100% false-positive rate**: twelve motionless scenes, twelve
confident frequencies, nine of them distinct. Does requiring a frequency to
SURVIVE A CHANGE OF LOOK COUNT -- appearing at the same Hz at both 128 and 256
looks -- remove those false positives?

**The idea and its provenance.** This is the operational-modal-analysis
STABILIZATION DIAGRAM: *"spurious modes will vary and physical modes will remain
quite constant at different model orders"*. Item 70 already identified that
principle and substituted the SPATIAL WINDOW for model order. That was the wrong
axis. The analogue of model order is a PROCESSING parameter that changes what the
estimator fits, and here that is the number of sub-looks: a real vibration sits
at f Hz however the aperture is sliced, while an artefact produced by the slicing
need not.

Item 106 measured this by accident -- C10's motionless artefact moved 0.665 to
1.331 Hz, exactly twice, the same fraction of Nyquist -- and across its six
injected and control cases the stability verdict agreed with recovery 6 times out
of 6.

A null -- artefacts staying put -- kills the idea and is worth knowing, because
this is the last untried discriminator that needs no twin and no ground truth.

## 2. Ground truth

Exact by construction. The twelve scenes are `sim_cphd` at amplitude 0.0 --
**nothing moves at all** -- so every reported frequency is by definition a false
positive. That is item 96's fixture unchanged, so the before-figure is already
measured at 12 of 12.

## 3. Data freeze

| | |
|---|---|
| collect / fixture | `sim_cphd`, 400 clutter, `--clutter-vib`, amplitude 0.0 |
| dwell `T` | ~20 s (fixture default) |
| pulses, PRF, range bins | fixture default |
| looks `N`, overlap | **128 AND 256**, overlap 0 -- the swept axis |

Derived and written down **before processing**, because they bound what can be
claimed:

- `df` = 0.0504 Hz at BOTH look counts, because `df = 1/T` and the dwell is fixed
- first admissible 0.151 Hz; top of band **3.2 Hz at 128 looks, 6.4 Hz at 256**
- **The comparison is restricted to the COMMON band, 0.151-3.2 Hz.** A frequency
  reported above 3.2 Hz at 256 looks has no counterpart at 128 and cannot be
  called unstable on that ground -- it is simply outside the comparison.

## 4. Hypotheses

State each with its threshold **now**.

- **H1 THE FALSE POSITIVES DIE.** Twelve motionless scenes at both look counts. A
  scene "reports" only if its modal-set frequency at 128 and at 256 agree within
  **half a bin (0.0252 Hz)**. H1 passes if **at most 2 of 12** survive, against
  item 96's 12 of 12.
  **Prediction recorded now: H1 PASSES.** If artefacts sit at a fixed fraction of
  Nyquist, as C10's did, doubling the look count doubles their Hz and none
  survives.
- **H2 RECALL SURVIVES.** The test is worthless if it also kills real signals.
  Six injected scenes (seeds 3,5,7,11,13,17 at 2 mm, the item 95 fixture) must
  still report, with **at least 5 of 6** stable across look count.
- **H3.** Subsumed by H1, which IS the specificity test.
- **H4.** No twin. **That is the entire point** -- every discriminator that has
  worked here needed a paired run (item 97) or a null control, neither of which a
  single real collect provides. This one needs only the collect itself.

## 5. Pipeline freeze

Every flag, verbatim, including defaults being relied on:

```sh
sim_cphd  <out> 0.5 0.0 --clutter 400 --clutter-vib --seed <s>     # motionless
micromotion mmotion --cphd <out> --estimator phase --n 128 --overlap 0 \
   --size 128 --cell 0.5 --win 32 --upsample 200 --coherence 0
micromotion mmotion --cphd <out> --estimator phase --n 256 ...      # same scene
```
Only `--n` differs between the paired runs.

- estimator: **phase**   *(if `correlation`: state the sub-look resolution cell and
  the expected `sigma_px`, and confirm it is below the injected amplitude BEFORE
  building anything on the series -- item 76)*
- `--overlap`: **0** *(item 48: do not raise it for the phase route)*
- `--fmin`: **default** *(item 47: prominence is not comparable across settings)*
- `--pulse-stride`: **must be absent from any measurement run** -- it lowers the
  effective PRF and so the observable band.

## 6. Null model

- construction: **none needed, and that is the claim being tested.** The scenes
  are motionless by construction, so the false-positive count is exact.
- realisations: 12 motionless + 6 injected
- threshold derived how? **From the null's own distribution, not chosen.**
  (item 80's `p_chance`; their 5x came from 400 null runs at ~2% FPR.)
- **A shuffle null is NOT valid for `--estimator phase`** -- a phase series is
  unwrapped across looks, so reordering breaks the observable rather than the
  signal.

## 7. Confounds considered

Tick what has been thought about and say how each is excluded:

- [ ] common-mode artefact -- agreement is blind to it (item 11); only a null catches it
- [ ] band-floor / trend field -- a cluster at the first admissible bin is a trend, not a detection
- [ ] red noise from overlap (item 47)
- [ ] aspect-dependent fading making the tracked pixel non-stationary (items 25, 55-56)
- [ ] the injected scatterer's mere presence (item 38 -- needs the twin)
- [ ] `D_A` / coherence gates being inert rather than passing (items 45-46)

## 8. Blinding

- [ ] the spectrum and the selection are produced before truth is overlaid
- [ ] no threshold or flag is changed after seeing the answer
- [ ] **every window is reported, including the misses** -- `PREFIX_windows.csv`
      is committed with the run

## 9. Kill criteria

Written now:

- **If H1 and H2 both pass**, this project has a blind discriminator for the
  first time -- one needing no twin, no null control and no ground truth -- and it
  should be implemented as a reported statistic.
- **If H1 passes but H2 fails**, the test kills signal and noise alike and is a
  sensitivity trap, not a detector.
- **If H1 fails**, artefacts are stable in Hz across look count, the stabilization
  analogy does not transfer to this axis, and item 106's 6-of-6 was an accident of
  two collects.

## 10. Reporting rules

Binding on whatever this run produces:

- **Every figure is quoted with the configuration that produced it** -- look
  count, sample size, overlap, `--fmin`, time filter. Four separate results in
  this repository had to be qualified or withdrawn for want of this (items 77,
  84, 86, 92).
- A ratio against a null is not a detection. Item 38 reached prominence 56.3 on a
  target that does not move; item 49's empty desert reaches 34.4; the external
  work's Butte artefact reached 1720x.
- A null result is written up in `RUN.md` and kept.
