# Run: 2026-08-07 kilauea / clustermass

**Question this run is meant to answer:** does a correlation-preserving
permutation null on cluster MASS fix item 108's false positive, and what does it
cost?

- git commit: `87d27eb` (pipeline and hypotheses committed before the first run)
- started:    2026-08-07T15:20Z
- host:       Darwin arm64

## Collect

Identical to `../2026-08-07-blockmedian/` in every flag. **Only the binary
differs.** `PREREG.md` §5 has the freeze; §10 has the diagnosis it rests on.

## Result

**H1 passes 6 of 6. H2 passes decisively. H3 passes 2 of 2.** The kill criterion
H3b, in `../../synthetic/2026-08-07-clustermass-null/`, passes with injected
recall **6 of 6** and **0 of 12** motionless scenes reporting.

| collect | amp | @128 | @256 | `--stable` |
|---|---|---|---|---|
| C10 | 0.00 control | **REFUSED** | 19.631 | no comparison |
| C10 | 0.13 | **0.998** | 19.631 | not comparable |
| C10 | 0.26 | **0.998** | **0.998** | **STABLE -> report** |
| C10 | 0.53 | **0.998** | **0.998** | **STABLE -> report** |
| C14 | 0.00 control | 0.997 | 9.327 | MOVED -> reject |
| C14 | 0.13 | **0.997** | **0.999** | **STABLE -> report** |
| C14 | 0.26 | **0.997** | **0.999** | **STABLE -> report** |
| C14 | 0.53 | **0.997** | **0.999** | **STABLE -> report** |

### Scoring the pre-registered predictions

`PREREG.md` §4 recorded four. **Two right, one right and it was the important
one, and one wrong in the better direction.**

| prediction | outcome |
|---|---|
| H2 passes, admitted modes at least halve | **RIGHT** — 12 admitted modes across the two motionless collects becomes **2** |
| C10's motionless control refused **entirely** | **RIGHT** — first real motionless collect ever to return no modal answer |
| Item 108's own false positive is **NOT** refused | **RIGHT, and it is the honest half of this item** — it survives at `ev` 28.4 against a critical 23.3, **p 0.010** where it was quoted at 0.001 |
| H1 falls to 5 of 6 | **WRONG** — it is **6 of 6**. C10 at 0.13 mm kept its answer at `ev` 21.6 against a critical 21.6, i.e. **p = 0.050 exactly on the threshold** |

### What actually changed: the reports went quiet

The gate is now on mass against a null that models the correlation, and the
effect is not that answers moved but that the *competition disappeared*:

| run | modes admitted, item 112 | item 113 |
|---|---|---|
| C10 motionless | 4 | **0** |
| C14 motionless | 8 | **2** |
| C10 injected 0.13 / 0.26 / 0.53 | 10 / 9 / 9 | **1 / 1 / 1** |
| C14 injected 0.13 / 0.26 / 0.53 | 8 / 8 / 8 | **1 / 1 / 1** |

**Every injected run now admits exactly one mode and it is the injected
frequency.** There is no longer a list to rank.

### Item 108 itself: improved by an order of magnitude, not solved

C14's motionless control still leads with **0.997 Hz** against a sought 1.00 Hz.
What changed is the honesty of the number attached to it:

| | item 112 | item 113 |
|---|---|---|
| p quoted for the false positive | 0.001 | **0.010** |
| modes admitted on that scene | 8 | 2 |

The offline diagnosis predicted 0.013 for this block under the same null design,
so the implementation agrees with the analysis. **The residual is most likely
that correlation extends beyond the 2x2 tile this null decorrelates** —
`PREREG.md` §10 brackets it, with a fully-dilated null putting the same block at
p ~ 0.5. `--stable` remains the thing that rejects it.

### Bounds

- **C10 at 0.13 mm sits exactly on p = 0.050.** One trial either way in a
  1000-trial Monte Carlo moves it across. Do not quote it as a recovery; it is
  the threshold.
- C10 at 0.13 mm is still not adjudicated by `--stable`, which abstains.
- Two real collects, one placement, one operating point, target put where it was
  found. **Still a selection result.**
