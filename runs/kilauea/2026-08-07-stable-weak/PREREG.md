<!-- Why this form exists, and the failures it is aimed at:
     docs/PREREGISTRATION.md -->
# PREREG — kilauea / stable-weak

Filled in on: `2026-08-07`   git commit: `7787e4f`
Status: **[x] confirmatory   [ ] exploratory (sections 4-9 not binding)**

## 1. Question

Item 107's stabilization test was measured on a WHOLE-SCENE `--clutter-vib`
injection at 2 mm -- strong, scene-wide, and on synthetic clutter. A real
structure is **localised** and **weak**. Does the test still keep the signal, and
still reject the motionless controls, when the target is a point source placed
off-centre at amplitudes bracketing item 103's 0.13-0.26 mm competition floor, on
REAL Kilauea clutter?

The failure worth finding is a true positive that is UNSTABLE -- a real signal
whose reported frequency moves with look count because it is too weak to hold the
selection. That would make the test a sensitivity trap rather than a
discriminator, and item 107's 6-of-6 recall would not transfer.

## 2. Ground truth

Exact by construction: Naples segment 02 at a known 1.00 Hz, placed **20 m off
the grid origin** via `--inject-at`, at 0.13, 0.26 and 0.53 mm. The scene's own
ground motion is 0.137-1.728 um against a 0.42 mm clutter floor (item 102), so it
cannot contribute. The amplitude-0 run of each collect is the motionless control
and is a real scene, not a simulation.

## 3. Data freeze

| | |
|---|---|
| collect / fixture | 2 complete Kilauea Capella spotlights, REAL clutter |
| dwell `T` | 6.0 s (seconds-matched) |
| pulses, PRF, range bins | per `kilauea_trunc.tsv` |
| looks `N`, overlap | **128 and 256**, overlap 0 -- the stabilization axis |

Derived and written down **before processing**, because they bound what can be
claimed:

- `df` = 0.1664 Hz at both look counts (the dwell is fixed), first admissible
  0.50 Hz, common band 0-10.7 Hz set by the 128-look Nyquist
- target at 1.00 Hz. Inside. Yes.

## 4. Hypotheses

State each with its threshold **now**.

- **H1 RECALL SURVIVES LOCALISATION AND WEAKNESS.** Wherever the injected
  frequency is recovered at BOTH look counts, `--stable` must report it. H1 fails
  if any amplitude that recovers at both counts is nonetheless called MOVED.
  **Prediction: H1 PASSES**, and stability tracks recovery rather than amplitude.
- **H2 NOT SCORED.** The target is off-centre, but no localisation claim is made
  here -- that needs the two-`focus` differencing of item 40 and is a separate run.
- **H3 SPECIFICITY ON REAL CLUTTER.** Both motionless controls must be called
  MOVED. Item 107's specificity was measured on the synthetic fixture only; this
  is the first test of it on a real scene. **Prediction: H3 PASSES**, since item
  106 already saw both real controls move (0.665 to 1.331 and 0.499 to 20.486).
- **H4.** Not scored. The whole point of `--stable` is that it needs no twin.

## 5. Pipeline freeze

Every flag, verbatim, including defaults being relied on:

```sh
mmotion --cphd <collect> --estimator phase --n {128,256} --overlap 0 --size 256 \
  --cell 0.5 --win 32 --upsample 200 --coherence 0 --max-pulses <matched> \
  --inject-wave <naples seg02>,<rate>,<amp>,20 --inject-at 20,20
mmotion ... --n 128 ... --stable <the 256-look run>_windows.csv
```

- estimator: **phase**   *(if `correlation`: state the sub-look resolution cell and
  the expected `sigma_px`, and confirm it is below the injected amplitude BEFORE
  building anything on the series -- item 76)*
- `--overlap`: **0** *(item 48: do not raise it for the phase route)*
- `--fmin`: **default** *(item 47: prominence is not comparable across settings)*
- `--pulse-stride`: **must be absent from any measurement run** -- it lowers the
  effective PRF and so the observable band.

## 6. Null model

- construction: **none** -- that is the claim. The comparison is the collect
  against itself at another look count.
- realisations: 2 real collects x 3 amplitudes + 2 motionless controls
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

- **If H1 and H3 both pass**, the stabilization test transfers from a strong
  whole-scene synthetic injection to a weak localised one on real clutter, and it
  is worth quoting as a general discriminator rather than a fixture result.
- **If H1 fails** -- a recovered signal called unstable -- the test is a
  sensitivity trap that discards true positives near the floor, and item 107 must
  be re-stated as applying only well above it.
- **If H3 fails** -- a real motionless collect called stable -- the synthetic
  12-to-1 result does not transfer to real clutter and the test is worth much
  less than item 107 claims.

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
