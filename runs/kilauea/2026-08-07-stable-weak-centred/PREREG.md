<!-- Why this form exists, and the failures it is aimed at:
     docs/PREREGISTRATION.md -->
# PREREG — kilauea / stable-weak-centred

Filled in on: `2026-08-07`   git commit: `79ac667`
Status: **[x] confirmatory   [ ] exploratory (sections 4-9 not binding)**

## 1. Question

Item 108 could not test whether `--stable` preserves recall, because its target
at +20 m sat **exactly on a window boundary** -- halfway between the window
centres at +16 and +24 m -- and the signal was lost at 128 looks before stability
could be assessed. This repeats it with the target on an exact window CENTRE,
+24 m, changing nothing else.

Two things are asked at once: does recall return, and if it does, does `--stable`
keep it?

## 2. Ground truth

Exact by construction: Naples segment 02 at a known 1.00 Hz, REL 20, at 0.13,
0.26 and 0.53 mm, placed at **+24,+24 m = the centre of window index 10** in both
axes (grid 256 px at 0.5 m, window 32, stride 16, so centres fall every 8 m from
the origin). The scene's own ground motion is 306x below the clutter floor.

## 3. Data freeze

| | |
|---|---|
| collect / fixture | the same 2 Kilauea collects as item 108 |
| dwell `T` | 6.0 s (seconds-matched) |
| pulses, PRF, range bins | per `kilauea_trunc.tsv` |
| looks `N`, overlap | 128 and 256, overlap 0 |

Derived and written down **before processing**, because they bound what can be
claimed:

- `df` 0.1664 Hz, first admissible 0.50 Hz, common band 0-10.7 Hz
- target at 1.00 Hz. Inside.

## 4. Hypotheses

State each with its threshold **now**.

- **H1 RECALL RETURNS.** At least **4 of 6** injected runs must recover 1.00 Hz
  within half a bin **at 128 looks**, against item 108's 0 of 6 for C10 and 3 of 3
  for C14.
  **Prediction: H1 PASSES**, because item 103 recovered 0.26 and 0.53 mm on these
  same collects with the target at the grid origin, which is itself a window
  centre (index 7).
- **H2 NOT SCORED.** No localisation claim; that needs item 40's two-`focus`
  differencing.
- **H3 SPECIFICITY HOLDS.** Both motionless controls must still be refused by
  `--stable`. These are the same two controls item 108 measured, so this is a
  reproducibility check on them, including C14's 0.997 Hz false positive.
- **H4 STABILITY PRESERVES RECALL -- the question item 108 could not reach.**
  Of the injected runs that recover at BOTH look counts, `--stable` must report
  **all** of them. Any recovered-at-both case called MOVED is a false rejection
  and makes the test a sensitivity trap.

## 5. Pipeline freeze

Every flag, verbatim, including defaults being relied on:

```sh
mmotion --cphd <collect> --estimator phase --n {128,256} --overlap 0 --size 256 \
  --cell 0.5 --win 32 --upsample 200 --coherence 0 --max-pulses <matched> \
  --inject-wave <naples seg02>,<rate>,<amp>,20 --inject-at 24,24
mmotion ... --n 128 ... --stable <the 256-look run>_windows.csv
```
Identical to item 108 except `--inject-at 24,24` in place of `20,20`.

- estimator: **phase**   *(if `correlation`: state the sub-look resolution cell and
  the expected `sigma_px`, and confirm it is below the injected amplitude BEFORE
  building anything on the series -- item 76)*
- `--overlap`: **0** *(item 48: do not raise it for the phase route)*
- `--fmin`: **default** *(item 47: prominence is not comparable across settings)*
- `--pulse-stride`: **must be absent from any measurement run** -- it lowers the
  effective PRF and so the observable band.

## 6. Null model

- construction: the collect against itself at another look count
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

- **If H1 and H4 both pass**, `--stable` preserves recall on a weak localised
  target and is a discriminator rather than only a specificity instrument. Item
  108's pessimistic summary is then withdrawn.
- **If H1 passes but H4 fails**, the test discards true positives near the floor
  and item 107 must be re-stated as applying only well above it.
- **If H1 fails even on a window centre**, the recall loss is not about placement
  and items 40-41 do not explain item 108; something else removes a weak
  localised target at 128 looks.

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
