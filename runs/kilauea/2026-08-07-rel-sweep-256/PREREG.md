<!-- Why this form exists, and the failures it is aimed at:
     docs/PREREGISTRATION.md -->
# PREREG — kilauea / rel-sweep-256

Filled in on: `2026-08-07`   git commit: `9051541`
Status: **[x] confirmatory   [ ] exploratory (sections 4-9 not binding)**

## 1. Question

Item 105 refuted the sub-look SCR penalty at REL 20, where the target's phase sd
was flat from 32 to 256 looks. It left one way the penalty could still be real:
**it might bite only at LOW REL**, where the target no longer dominates a cell
that N sub-looks have made N times larger. If so, the brightness transition item
104 measured at 128 looks must MOVE UPWARD at 256.

A null -- the transition unchanged -- closes that possibility and leaves the
11-17 dB gap against PS-InSAR unexplained by anything to do with sub-look
resolution.

## 2. Ground truth

Exact by construction: Naples segment 02 at a known 1.00 Hz and a fixed 0.26 mm,
into real Capella phase history whose own ground motion is 306x below the clutter
floor (item 102).

## 3. Data freeze

| | |
|---|---|
| collect / fixture | 2 complete Kilauea Capella spotlights |
| dwell `T` | 6.0 s (seconds-matched) |
| pulses, PRF, range bins | per `kilauea_trunc.tsv` |
| looks `N`, overlap | **256**, overlap 0 (item 104 used 128) |

Derived and written down **before processing**, because they bound what can be
claimed:

- `df` = 0.1664 Hz, first admissible 0.50 Hz, top of band **21.3 Hz** at 256 looks
- target at 1.00 Hz. Inside. Yes. 129 bins against 65 at 128 looks.

## 4. Hypotheses

State each with its threshold **now**.

- **H1 THE TRANSITION MOVES UP.** REL 20, 10, 5, 2, 1 at 256 looks on 2 collects.
  H1 passes -- i.e. the SCR penalty is real and REL-dependent -- only if the
  transition sits at a HIGHER REL than at 128 looks, where C10 needed 20 and C14
  needed 10.
  **Prediction recorded now: H1 FAILS.** Item 105 found the phase sd flat across
  a 8x change in look count at REL 20; I expect the transition to be unchanged or
  to move DOWN, because item 105 also measured a 23% lower floor at 256.
- **H2 NOT TESTED.** Target at the grid origin; no localisation claim.
- **H3.** Below the transition the reported frequency must be the uninjected
  scene's own artefact, as at 128 looks (0.665 and 0.499 Hz). If it is something
  else, the failure mode has changed with look count and that is itself a result.
- **H4.** Not scored; item 103 established the single-look twin ceiling.

## 5. Pipeline freeze

Every flag, verbatim, including defaults being relied on:

```sh
mmotion --cphd <collect> --estimator phase --n 256 --overlap 0 --size 256 \
  --cell 0.5 --win 32 --upsample 200 --coherence 0 --max-pulses <matched> \
  --inject-wave <naples seg02>,<rate>,0.26,<REL> --probe-hz 1.00
```
Identical to item 104 except `--n 256`.

- estimator: **phase**   *(if `correlation`: state the sub-look resolution cell and
  the expected `sigma_px`, and confirm it is below the injected amplitude BEFORE
  building anything on the series -- item 76)*
- `--overlap`: **0** *(item 48: do not raise it for the phase route)*
- `--fmin`: **default** *(item 47: prominence is not comparable across settings)*
- `--pulse-stride`: **must be absent from any measurement run** -- it lowers the
  effective PRF and so the observable band.

## 6. Null model

- construction: each collect's uninjected run at the same look count
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

- **If the transition is unchanged or lower**, the sub-look SCR penalty is dead
  in every regime tested and the 11-17 dB gap against PS-InSAR must be explained
  by something other than resolution -- most likely that the two methods measure
  different things and their thresholds are not commensurable.
- **If it moves up**, the penalty is real and REL-dependent, item 105's flat
  phase sd was a property of REL 20 only, and the look count becomes a trade
  against brightness rather than a free improvement.

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
