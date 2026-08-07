<!-- Why this form exists, and the failures it is aimed at:
     docs/PREREGISTRATION.md -->
# PREREG — kilauea / rel-sweep

Filled in on: `2026-08-07`   git commit: `8898dfa`
Status: **[x] confirmatory   [ ] exploratory (sections 4-9 not binding)**

## 1. Question

At a fixed displacement amplitude, how bright must the target be -- relative to
the scene's mean -- before its motion is recoverable against REAL clutter?

**Item 51 already answered this in a DIFFERENT regime and the distinction is the
point.** There the competition was the target's OWN quadratic phase residual,
which scales as REL^2 exactly as the signal does, so the ratio was
brightness-independent and item 51 concluded "a brighter target does not make
smaller motion detectable". In item 103's regime the competition is the SCENE's
artefact -- Kilauea clutter peaking at prominence 9.9 -- which does NOT scale
with the target. There the ratio should improve with REL, and item 103's own
numbers say it must: the target window's floor was 0.0157 mm at REL 20 against
0.42-0.65 mm everywhere else in the same scene.

A null -- recovery unchanged across REL -- would mean item 51's conclusion holds
in this regime too and the clutter artefact is not the operative competition.

## 2. Ground truth

Exact by construction. Naples segment 02 injected at a KNOWN 1.00 Hz and a fixed
0.26 mm -- the amplitude item 103 measured as the recovery threshold at REL 20 --
into real Capella phase history. The scene's own ground motion is 0.137-1.728 um
against a 0.42 mm clutter floor (item 102), so it cannot contribute.

## 3. Data freeze

| | |
|---|---|
| collect / fixture | 2 complete Kilauea Capella spotlights, byte-verified |
| dwell `T` | 6.0 s (seconds-matched) |
| pulses, PRF, range bins | per `kilauea_trunc.tsv` |
| looks `N`, overlap | 128, overlap 0 |

Derived and written down **before processing**, because they bound what can be
claimed:

- `df` = 0.1664 Hz, first admissible 0.50 Hz, top of band 10.7 Hz
- target at 1.00 Hz. Inside. Yes.

## 4. Hypotheses

State each with its threshold **now**.

- **H1 BRIGHTNESS MATTERS IN THIS REGIME.** REL at 20, 10, 5, 2 and 1 at a fixed
  0.26 mm, on 2 collects. H1 passes if recovery within half a bin holds at high
  REL and FAILS at low REL, i.e. there is a brightness threshold.
  **Prediction recorded now: H1 PASSES, with the transition between REL 2 and 5**,
  because the target must dominate its pixel for the phase route's precondition
  (item 15) to hold at all.
- **H2 THE FLOOR TRACKS THE BRIGHTNESS.** `floor_mm` at the target window should
  RISE as REL falls, approaching the scene's 0.42-0.65 mm clutter floor at REL 1.
  H2 passes if the target-window floor at REL 1 is within 2x of the scene median.
  This is the mechanism test behind H1 and is now reported by the tool itself.
- **H3.** The uninjected run of each collect is the control; it must not report
  1.00 Hz. Already measured in item 103: 0.665 and 0.499 Hz.
- **H4.** Not scored. Item 103 measured the twin LLR plateauing without reaching
  p < 0.05, which is item 98's single-look ceiling, and nothing here changes it.

## 5. Pipeline freeze

Every flag, verbatim, including defaults being relied on:

```sh
mmotion --cphd <collect> --estimator phase --n 128 --overlap 0 --size 256 \
  --cell 0.5 --win 32 --upsample 200 --coherence 0 --max-pulses <matched> \
  --inject-wave <naples seg02>,<rate>,0.26,<REL> --probe-hz 1.00
```
Only REL varies. Amplitude, frequency, waveform, geometry and collect are fixed.

- estimator: **phase**   *(if `correlation`: state the sub-look resolution cell and
  the expected `sigma_px`, and confirm it is below the injected amplitude BEFORE
  building anything on the series -- item 76)*
- `--overlap`: **0** *(item 48: do not raise it for the phase route)*
- `--fmin`: **default** *(item 47: prominence is not comparable across settings)*
- `--pulse-stride`: **must be absent from any measurement run** -- it lowers the
  effective PRF and so the observable band.

## 6. Null model

- construction: each collect's own uninjected run (item 103)
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

- **If H1 passes**, item 51's "a brighter target does not make smaller motion
  detectable" is regime-specific and must be quoted with its competition named.
  It also means this method's reach on a real structure depends on the
  structure's brightness relative to its surroundings, which is a measurable
  property nobody here has ever looked up.
- **If H1 fails** -- recovery unchanged across REL -- item 51 generalises, the
  clutter artefact is not the operative competition, and item 103's three-floor
  picture needs re-opening.
- **If H2 fails while H1 passes**, recovery depends on brightness through some
  route other than the target window's own noise, and the floor reported by
  `rs_microm_floor()` is not the quantity that governs.

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
