# Pre-registration

A run's hypotheses, thresholds and kill criteria, written down **before the first
processing command touches real data**, and committed unchanged.

## Why this exists

Adopted from the validation practice in `github.com/Hassanforeman/subsurface-sar-tomo`
(`docs/VALIDATION_PROTOCOL.md`), recorded here as `FOLLOW-UPS.md` item 92. Its
purpose there and here is the same: separate a detection from **noise
confirmation bias**.

This project already had the components -- a null control, a zero-amplitude twin,
a sweep bar, an evidence CSV beside every run -- and applied them **after** the
fact. Everything in `FOLLOW-UPS.md` was scored once the numbers were in. Exactly
one thing here has ever been pre-registered, `runs/kilauea/2026-08-05-correlation/analyse.py`,
and it was written that way because the answer was expected to be marginal.

The failures this file is aimed at are all recorded in this repository, by
number:

- item 2 and item 30 -- a single frequency matching once, called a recovery
- item 38 -- a positive control with no zero-amplitude twin, so `p = 0.05`
  measured that a scatterer had been ADDED, not that it moved
- item 40 -- localisation scored against a reference that was itself one window
  off, producing a published "4 of 5 exact" that had to be withdrawn
- items 69-74 -- an entire six-item arc built on a tracked series whose
  `sigma_px` was 46.7 m per look against a 20 mm injection
- items 77, 84, 86 and 92 -- a figure quoted without the configuration that
  produced it, four times

None of those would have survived a filled-in form.

## How to use it

`tools/new-run.sh` seeds `PREREG.md` in every new run directory. Fill it in and
**commit it before running anything**, so the git history shows the hypotheses
predate the data. Then fill in `RUN.md` afterwards. If the answer changes what
you would have asked, that is a new run with a new pre-registration, not an edit
to this one.

A run that is exploratory rather than confirmatory should say so in section 1 and
skip to section 10. **Exploration is legitimate; exploration reported as
confirmation is not.**

---

# PREREG — <scene> / <suffix>

Filled in on: `<date>`   git commit: `<sha>`
Status: **[ ] confirmatory   [ ] exploratory (sections 4-9 not binding)**

## 1. Question

What single question does this run answer? One sentence, and phrased so that a
null is an answer rather than a failure.

## 2. Ground truth

- What is independently known to move here, at what frequency and amplitude?
- Source of that truth (instrument, catalogue, injection geometry)?
- **If none: say so.** A real-data null means nothing without it -- "nothing
  moved" and "the chain cannot see" are indistinguishable (USER_GUIDE section 7
  item 0). An interpretable null needs an instrument that says the ground was
  still, as at Oroville (item 60) and Granada (item 91).

## 3. Data freeze

| | |
|---|---|
| collect / fixture | |
| dwell `T` | |
| pulses, PRF, range bins | |
| looks `N`, overlap | |

Derived and written down **before processing**, because they bound what can be
claimed:

- bin spacing `df = 1/T` = ______ Hz
- first admissible bin `3*df` = ______ Hz (`RS_SPECTRUM_LEAKAGE_BINS`, not tunable)
- top of band `N/(2T)` = ______ Hz
- **is the expected frequency inside `[3/T, N/(2T)]`?** If not, stop -- item 89
  found a building at 3.78 Hz against a 2.13 Hz ceiling at 128 looks.

## 4. Hypotheses

State each with its threshold **now**.

- **H1 recovery.** Across a sweep of ______ injected frequencies over ______
  independent clutter realisations, `rs_track_fit()` gives slope within ______ of
  1 and rms below ______ Hz (half a bin = `0.5*df`). *A single point matching is
  not H1 (item 2).*
- **H2 localisation.** Reported centroid within ______ m of the injection
  geometry. **Scored by differencing two `focus` runs that differ only in
  `--inject-vib`** -- never against the window with the most energy, and never
  against a zero-offset run's reported window (item 40 withdrew a result scored
  that way).
- **H3 specificity.** Static controls through identical processing return
  `NO FREQUENCY REPORTED`, or an out-of-band frequency, in ______ of ______ runs.
- **H4 twin.** The zero-amplitude twin's `probe_prominence` at the injected
  frequency is below the injection's by ______ (item 38, item 39).

## 5. Pipeline freeze

Every flag, verbatim, including defaults being relied on:

```sh
```

- estimator: ______   *(if `correlation`: state the sub-look resolution cell and
  the expected `sigma_px`, and confirm it is below the injected amplitude BEFORE
  building anything on the series -- item 76)*
- `--overlap`: ______ *(item 48: do not raise it for the phase route)*
- `--fmin`: ______ *(item 47: prominence is not comparable across settings)*
- `--pulse-stride`: **must be absent from any measurement run** -- it lowers the
  effective PRF and so the observable band.

## 6. Null model

- construction: `--null-static N` / `rs_spectrum_scene_null()` / reshuffle
- realisations: ______
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

Written now, in the form *"if X, then this approach is abandoned / this item is
withdrawn"*:

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
