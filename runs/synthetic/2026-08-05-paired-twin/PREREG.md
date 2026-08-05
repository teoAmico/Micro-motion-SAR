<!-- Why this form exists, and the failures it is aimed at:
     docs/PREREGISTRATION.md -->
# PREREG — synthetic / paired-twin

Filled in on: `2026-08-05`   git commit: `6505b72`
Status: **[x] confirmatory   [ ] exploratory (sections 4-9 not binding)**

## 1. Question

Item 96 measured a 100% false-positive rate on motionless clutter, with a
different invented frequency per scene. Does the remedy this project already
owns -- a ZERO-AMPLITUDE TWIN OF THE SAME SCENE, differenced at the injected
frequency with `--probe-hz` (items 38, 39) -- separate a real injection from that
scene's own artefact?

A null is informative: it would mean the only remaining discriminator this
project has does not work at this operating point.

## 2. Ground truth

Exact by construction: the injected frequency is placed by `--wave` at a known
target, and the twin is the SAME seed and SAME clutter at amplitude 0. The only
difference between the paired runs is whether the target moves.

## 3. Data freeze

| | |
|---|---|
| collect / fixture | `sim_cphd`, 400 clutter, `--clutter-vib`, seeds 7 & 11 |
| dwell `T` | ~20 s |
| pulses, PRF, range bins | fixture default |
| looks `N`, overlap | 128, overlap 0 |

Derived and written down **before processing**, because they bound what can be
claimed:

- `df` = 0.0504 Hz, first admissible 0.151 Hz, top of band 3.2 Hz
- targets 0.30-0.90 Hz, all inside. Yes.

## 4. Hypotheses

State each with its threshold **now**.

- **H1 SEPARATION (the whole question).** For each of 24 injected points, let
  `D = probe_prominence(injected) - probe_prominence(twin)` at the injected
  frequency, in the CENTRE window where the target sits. H1 passes if
  **`D > 0` for at least 20 of 24 points**, i.e. the injection raises the energy
  at its own frequency above what the identical motionless scene puts there.
- **H2 SPATIAL SPECIFICITY.** The same difference taken at the MEDIAN window over
  all 49 must be near zero -- item 39 measured `+0.00` there. H2 passes if
  `median-window D` has magnitude below one tenth of the centre-window `D`.
  Without this, a positive `D` could be a scene-wide gain rather than a target.
- **H3.** Not applicable: this run tests a paired statistic, not a reported
  frequency. Item 96 already measured the reported-frequency false-positive rate
  at 12 of 12.
- **H4.** Subsumed into H1 -- this run IS the twin test.
  **Prediction recorded now:** H1 passes and H2 passes. The twin is paired on the
  same clutter, so the scene's own artefact subtracts out; that is the mechanism
  items 38-39 were built on and it has never been swept.

## 5. Pipeline freeze

Every flag, verbatim, including defaults being relied on:

```sh
# injected, 24 points
sim_cphd  <o> 0.5 0.002 --clutter 400 --clutter-vib --seed {7,11} --wave <seg>,<rate>
micromotion mmotion --cphd <o> --estimator phase --n 128 --overlap 0 \
   --size 128 --cell 0.5 --win 32 --upsample 200 --coherence 0 --probe-hz <target>
# twin, 12 runs: identical but amplitude 0.0 and no --wave
```

- estimator: **phase**   *(if `correlation`: state the sub-look resolution cell and
  the expected `sigma_px`, and confirm it is below the injected amplitude BEFORE
  building anything on the series -- item 76)*
- `--overlap`: **0** *(item 48: do not raise it for the phase route)*
- `--fmin`: **default** *(item 47: prominence is not comparable across settings)*
- `--pulse-stride`: **must be absent from any measurement run** -- it lowers the
  effective PRF and so the observable band.

## 6. Null model

- construction: **paired zero-amplitude twin on the SAME seed** -- the strongest
  null this project has, because it differs from the injection in one thing only
- realisations: 12 twins (2 seeds x 6 probe frequencies)
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

- **If H1 fails**, the paired twin does not separate injection from artefact at
  this operating point, and this project has NO working discriminator --
  prominence, the modal set, the chance model and now the twin having all failed.
  That would make the selection stage, not the estimator, the thing to rebuild.
- **If H1 passes but H2 fails**, the difference is a scene-wide gain and the twin
  is measuring the injection's presence rather than its motion -- item 38's
  original trap, and the statistic would need the amplitude-0 pairing item 39
  specifies rather than an uninjected pairing.
- **If both pass**, the discriminator works and the failure in items 91/95/96 is
  in what is REPORTED, not in what is measurable -- which would make a
  twin-differenced statistic the thing to report.

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
