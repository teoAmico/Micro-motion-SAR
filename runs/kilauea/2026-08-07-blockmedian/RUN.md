# Run: 2026-08-07 kilauea / blockmedian

**Question this run is meant to answer:** does summarising a mode's strength over
its own block, instead of over every window that nominated the bin, hold the
recall and the false-positive rate items 110 and 111 measured?

- git commit: `eb0775f` (pipeline and hypotheses committed before the first run)
- started:    2026-08-07T13:40Z
- host:       Darwin arm64

## Collect

Identical to `../2026-08-07-refbins/` in every flag. **Only the binary differs.**
`PREREG.md` §5 has the freeze.

## Commands

`rerun.sh`, which is `../2026-08-07-refbins/rerun.sh` with only the output path
changed. Per-window evidence in `windows/`.

## Result

**H1 PASSES at 6 of 6 — the first time.** H3 passes 2 of 2. The kill criterion
H3b, in `../../synthetic/2026-08-07-blockmedian-null/`, passes and improves:
**0 of 12** motionless scenes survive against a bar of 1, with injected recall
held at **6 of 6**.

**Read H1 and the `--stable` column as two different things**, because here they
differ for the first time. H1 is on the **128-look modal answer**, which is what
`PREREG.md` §4 states and what items 110 and 111 scored 5 of 6 on. `--stable`
then adjudicates that answer against the 256-look run, and it **abstains** on
C10 at 0.13 mm because the 256-look answer, 19.631 Hz, is above the 128-look
Nyquist of 10.65 Hz. So the selection recovers 6 of 6 and the stabilization test
reports 5 of 6, and neither number is the other.

| collect | amp | @128 | @256 | `--stable` | @128 in item 111 |
|---|---|---|---|---|---|
| C10 | 0.00 control | 5.823 | 3.161 | MOVED -> reject | 5.823 |
| C10 | 0.13 | **0.998** | 19.631 | not comparable | 0.499 (wrong) |
| C10 | 0.26 | **0.998** | **0.998** | **STABLE -> report** | 0.998 |
| C10 | 0.53 | **0.998** | **0.998** | **STABLE -> report** | 0.998 |
| C14 | 0.00 control | 0.997 | 9.327 | MOVED -> reject | 0.997 |
| C14 | 0.13 | **0.997** | **0.999** | **STABLE -> report** | 0.997 |
| C14 | 0.26 | **0.997** | **0.999** | **STABLE -> report** | 0.997 |
| C14 | 0.53 | **0.997** | **0.999** | **STABLE -> report** | 0.997 |

### The predicted risk did not materialise, and it was the right risk to name

`PREREG.md` §4 recorded that removing the dilution raises **every** candidate's
strength, artefacts included, and named item 108's C14 control -- a small,
internally clean block -- as the thing that could gain more than the injected
line and fail H3. Measured, it gained almost nothing:

| | item 111 `ev` | item 112 `ev` | change |
|---|---|---|---|
| C14 motionless control, 0.997 Hz | 28.0 | **28.4** | +0.4 |
| C14 injected 0.13 mm | 42.3 | **49.9** | +7.6 |
| C14 injected 0.26 mm | 46.7 | **58.6** | +11.9 |
| C14 injected 0.53 mm | 46.7 | **61.6** | +14.9 |

The control's block of 17 windows is **not** internally clean -- its ratio is
5.3, barely above the scene -- so it gained nothing from a statistic that
rewards a clean block. That is the mechanism the fix was aimed at, working: an
injected line's block is pure and an artefact's is not, and the old statistic
could not tell them apart because it averaged both against the same chance
background.

### The evidence table, 128 looks

| amp | C10 injected | C10 competition | C14 injected | C14 competition |
|---|---|---|---|---|
| 0.00 | — | **18.6** | — | **28.4** |
| 0.13 | **21.6** | 18.6 | **49.9** | 26.9 |
| 0.26 | **39.7** | 18.6 | **58.6** | 18.9 |
| 0.53 | **48.1** | 19.2 | **61.6** | 23.7 |

The target's own `ratio` is what moved: C10 at 0.53 mm went from **9.0 to 40.3**,
at 0.26 mm from 4.5 to 21.2. The competition barely moved. **C10 now recovers at
0.13 mm**, where it failed in items 109, 110 and 111.

**That lowers the competition floor below item 103's 0.13-0.26 mm**, and the
right way to state it is that the floor is a property of the scene AND the
selection together, not of the scene alone. Item 103 measured it with the
selection of the time. A better selection moved it. **Do not quote 0.13-0.26 mm
as a scene property.**

### Bounds

- C10 at 0.13 mm is recovered by the modal set and **not adjudicated** by
  `--stable`, which abstains. It is not a detection.
- 0 of 12 on the synthetic null is a rate over twelve realisations with a wide
  interval, not a zero false-positive rate.
- Two real collects, one target placement, one operating point. The target was
  put where it was found. **This remains a selection result.**
