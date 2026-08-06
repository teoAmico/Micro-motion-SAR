<!-- Why this form exists, and the failures it is aimed at:
     docs/PREREGISTRATION.md -->
# PREREG — kilauea / injection-floor

Filled in on: `2026-08-06`   git commit: `e32f765`
Status: **[x] confirmatory   [ ] exploratory (sections 4-9 not binding)**

## 1. Question

Item 102 predicts a floor of ~0.53 mm on these collects from their own
uninjected phase noise. Does that prediction actually separate recoverable from
unrecoverable injections on REAL clutter?

A null is informative: if recovery does not follow the prediction, the
noise-to-floor arithmetic of item 101 is not the operative constraint and the
whole items 96-102 chain needs re-opening.

## 2. Ground truth

Exact by construction: a real building's ambient displacement record (Naples,
Zenodo 20667124, segment 02) is injected into REAL Capella phase history at a
known amplitude and a known frequency.

The scene itself is proven quiet for this purpose: item 102 measured its
seismometer truth at 0.137-1.728 um, which is 306x BELOW the predicted floor, so
nothing in the ground can contribute at the amplitudes injected here.

## 3. Data freeze

| | |
|---|---|
| collect / fixture | 5 complete Kilauea Capella spotlights, byte-verified |
| dwell `T` | 6.0 s (seconds-matched truncation, `kilauea_trunc.tsv`) |
| pulses, PRF, range bins | per collect, from the same table |
| looks `N`, overlap | 128, overlap 0 |

Derived and written down **before processing**, because they bound what can be
claimed:

- `df` = **0.1664** Hz (measured on these collects), first admissible **0.50** Hz,
  top of band **10.7** Hz
- target placed at **1.00 Hz**, comfortably inside. Yes.

## 4. Hypotheses

State each with its threshold **now**.

- **H1 THE FLOOR PREDICTS RECOVERY.** Amplitudes at 0.25x, 0.5x, 1x, 2x and 4x
  the predicted 0.53 mm floor (0.13, 0.26, 0.53, 1.06, 2.12 mm), on 2 independent
  collects. H1 passes if the injected frequency is recovered within half a bin
  **at every amplitude at or above 1x** and **at neither amplitude below it** --
  i.e. the prediction is the boundary, not merely correlated with it.
  **Prediction recorded now: H1 PASSES, with the 1x point the ambiguous one.**
- **H2 NOT TESTED.** The target sits at the grid origin, so localisation is
  untested by construction (item 40) and no localisation claim may be made.
- **H3 specificity.** The **uninjected** run of each collect is the control. H3
  passes if neither reports 1.00 Hz. Note this is a real scene, so a null there
  is interpretable only because item 102 measured the ground 306x below the
  floor.
- **H4 twin.** Each injected run is differenced against its own uninjected run
  with `--probe-hz 1.00 --twin`, which on REAL data is a genuine independent
  pairing rather than the deterministic one item 97's caveat covers -- the two
  runs share the collect but the injection is the only difference. H4 passes if
  the twin LLR rises monotonically with injected amplitude.

## 5. Pipeline freeze

Every flag, verbatim, including defaults being relied on:

```sh
# floor prediction, per collect
mmotion --cphd <collect> --estimator phase --n 128 --overlap 0 --size 256 \
   --cell 0.5 --win 32 --upsample 200 --coherence 0 --max-pulses <matched> \
   --shifts <out>.csv --probe-hz 1.00
# injection at each amplitude
mmotion ... --inject-wave <naples seg02>,<rate>,<amp_mm> --probe-hz 1.00 \
   --twin <uninjected>_windows.csv
```

- estimator: **phase**   *(if `correlation`: state the sub-look resolution cell and
  the expected `sigma_px`, and confirm it is below the injected amplitude BEFORE
  building anything on the series -- item 76)*
- `--overlap`: **0** *(item 48: do not raise it for the phase route)*
- `--fmin`: **default** *(item 47: prominence is not comparable across settings)*
- `--pulse-stride`: **must be absent from any measurement run** -- it lowers the
  effective PRF and so the observable band.

## 6. Null model

- construction: **the collect's own uninjected run**, which is the strongest
  control available on real data and is what `--twin` consumes
- realisations: 2 collects
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

- **If H1 passes**, the floor is predictable before a run on real data, and every
  future experiment here must state its predicted floor in its pre-registration.
- **If recovery occurs BELOW the predicted floor**, item 101's arithmetic is too
  pessimistic and the `sqrt(2/N)` averaging gain is being understated.
- **If recovery FAILS at 4x the floor**, the noise-to-floor model is not the
  operative constraint on real clutter and items 96-102 must be re-opened.

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
