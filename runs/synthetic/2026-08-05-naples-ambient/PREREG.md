<!-- Why this form exists, and the failures it is aimed at:
     docs/PREREGISTRATION.md -->
# PREREG — synthetic / naples-ambient

Filled in on: `2026-08-05`   git commit: `9b43f47`
Status: **[x] confirmatory   [ ] exploratory (sections 4-9 not binding)**

## 1. Question

Does item 91's result -- a real building's AMBIENT motion recovered 1 time in 24,
with 9 of 21 answers equal to a static control's -- REPLICATE on an independent
building, or was it a property of Granada's particular record?

This is a replication, so a null is the informative outcome: two buildings
failing the same way is a far stronger statement than one.

## 2. Ground truth

Truth is the INJECTION, not the scene: a known waveform is placed in synthetic
phase history, so ground truth is exact by construction.

Source: `zenodo.org/records/20667124`, former military hospital Naples, CC-BY-NC.
Reference accelerometer ACC-1 (fixed across all four setups), 200 Hz,
double-integrated to displacement with a **1.0 Hz** high-pass. The corner was
verified not to set the answer (item 94): 0.3 and 0.5 Hz track the corner,
0.8/1.0/1.2 Hz all give 2.64 Hz on a 120 s window.

**Stated in advance because it weakens H1:** on 12 s segments the dominant
displacement frequency is NOT stable -- 1.17, 1.37, 1.76, 2.54, 2.73, 4.88 Hz
across 20 segments, median 1.76. Each segment therefore carries its OWN truth and
the playback rate is computed per segment. Real peak displacement 0.33-1.27 um.

## 3. Data freeze

| | |
|---|---|
| collect / fixture | `sim_cphd`, 400 clutter scatterers, `--clutter-vib` |
| dwell `T` | ~20 s (fixture default) |
| pulses, PRF, range bins | fixture default |
| looks `N`, overlap | **128, overlap 0** |

Derived and written down **before processing**, because they bound what can be
claimed:

- bin spacing `df = 1/T` = **0.0504** Hz (as reported by every run in items 80-91)
- first admissible bin = **0.151** Hz
- top of band `N/(2T)` = **3.2** Hz
- **Inside?** YES by construction: the playback rate PLACES each segment's
  dominant at a target in 0.30-0.90 Hz, well inside. The record's native 2.64 Hz
  would also be inside at this dwell, unlike Granada's 3.78 (item 94).

## 4. Hypotheses

State each with its threshold **now**.

- **H1 recovery.** Across **6** injected frequencies (0.30, 0.40, 0.50, 0.60,
  0.75, 0.90 Hz) over **2 independent segments from DIFFERENT SETUPS x 2 clutter
  seeds**, `rs_track_fit()` gives slope within **0.10** of 1 and rms below
  **0.0252** Hz (half a bin). 24 points total.
  **Prediction, recorded now: H1 FAILS**, on the reasoning of item 91 -- ambient
  response is stationary, broadband and multi-modal, so no line is prominent.
- **H2 localisation.** NOT TESTED in this run -- the target is at the grid
  origin, so localisation is untested by construction (item 40). No localisation
  claim may be made from these results.
- **H3 specificity.** **2** static controls (amplitude 0, same seeds) through
  identical processing. H3 passes only if NO injected run returns a value a
  static control also returns. *Item 91 failed this 9 times in 21.*
- **H4 twin.** NOT RUN. The static controls here are motionless SCENES, not
  zero-amplitude twins of an added scatterer, so item 38's specific hazard is not
  addressed and **no detectability claim may be made from this run** -- only a
  recoverability one.

## 5. Pipeline freeze

Every flag, verbatim, including defaults being relied on:

```sh
sim_cphd  <out> 0.5 0.002 --clutter 400 --clutter-vib --seed {7,11} \
          --wave <seg>,<rate>            # rate = 200 * target / seg_dominant
micromotion mmotion --cphd <out> --estimator phase \
          --n 128 --overlap 0 --size 128 --cell 0.5 --win 32 \
          --upsample 200 --coherence 0
```
Identical to items 74/77/80/81/91 in every flag; only the injected waveform
differs, which is what makes this a replication.

- estimator: **phase**   *(if `correlation`: state the sub-look resolution cell and
  the expected `sigma_px`, and confirm it is below the injected amplitude BEFORE
  building anything on the series -- item 76)*
- `--overlap`: **0** *(item 48: do not raise it for the phase route)*
- `--fmin`: **default** *(item 47: prominence is not comparable across settings)*
- `--pulse-stride`: **must be absent from any measurement run** -- it lowers the
  effective PRF and so the observable band.

## 6. Null model

- construction: **motionless scenes at amplitude 0**, plus the modal set's own
  built-in Monte Carlo (`p_chance`, item 80)
- realisations: **2** static scenes; `p_chance` uses `RS_MODAL_NULL_TRIALS` 1000
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

- **If H1 fails on Naples as it did on Granada**, then item 91 is not a property
  of one record and the conclusion generalises: *this chain does not recover
  ambient structural motion at any amplitude*, and no further injection of an
  ambient record is worth running without a change to the SELECTION stage.
- **If H1 passes here but failed on Granada**, item 91 must be re-opened and its
  cause narrowed to the out-of-band mode rather than to ambient motion.
- **If H3 fails** (injected runs returning static-control values), that is item
  11 again and the answer is the artefact, whatever H1 says.

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
