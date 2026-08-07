# Run: 2026-08-07 kilauea / bracket

**Question this run is meant to answer:** does bracketing p between an under- and
an over-correlated null, and gating on the conservative end, refuse item 108's
false positive, and what does it cost?

- git commit: `6fb1cb4` (pipeline and hypotheses committed before the first run)
- started:    2026-08-07T17:05Z
- host:       Darwin arm64

## Collect

Identical to `../2026-08-07-clustermass/` in every flag. **Only the binary
differs.**

## Result

**H4 PASSES — item 108 is refused.** H3 passes 2 of 2, H1 lands at 5 of 6
against a bar of 4, and H3b passes with injected recall 6 of 6.

**Every pre-registered prediction was correct**, including the exact recall
figure and which point would be lost.

| collect | amp | @128 | @256 | `--stable` |
|---|---|---|---|---|
| C10 | 0.00 control | **REFUSED** | **REFUSED** | no comparison |
| C10 | 0.13 | **REFUSED** | REFUSED | no comparison |
| C10 | 0.26 | **0.998** | REFUSED | no comparison |
| C10 | 0.53 | **0.998** | **0.998** | **STABLE -> report** |
| C14 | 0.00 control | **REFUSED** | **REFUSED** | no comparison |
| C14 | 0.13 | **0.997** | REFUSED | no comparison |
| C14 | 0.26 | **0.997** | **0.999** | **STABLE -> report** |
| C14 | 0.53 | **0.997** | **0.999** | **STABLE -> report** |

### Item 108, finally

The motionless collect that has led with 0.997 Hz against a sought 1.00 Hz since
item 108 now returns **nothing recurs across the windows**, and says why:

```
the closest was 0.997 Hz, nominated by 35 of 225 windows in a
largest block of 17 at evidence 28.4 (p 0.342 against chance,
which reaches 37.7 here).
```

Its p across four items: **0.001 -> 0.010 -> 0.342.** And C10's motionless
control is refused at both look counts, with its best candidate at p 0.999.

### The bracket does the work it was built for

| run | p bracket | verdict |
|---|---|---|
| C14 motionless (item 108) | **0.342** | refused |
| C10 motionless | **0.999** | refused |
| C10 injected 0.13 | **0.942** | refused |
| C10 injected 0.26 | **0.001 – 0.043** | admitted |
| C10 injected 0.53 | **0.001 – 0.012** | admitted |
| C14 injected 0.13 | **0.001 – 0.006** | admitted |
| C14 injected 0.26 / 0.53 | **0.001 – 0.001** | admitted |

C10 at 0.26 mm is the only admitted mode whose bracket is wide (0.001–0.043) —
it is admitted on the conservative end, but only just, and that width is the
honest statement about it.

## Costs, stated plainly

- **H1 is 5 of 6.** C10 at 0.13 mm is refused, at p 0.942. Item 113 had it at
  exactly p = 0.050 and recorded it as "the threshold, not a recovery"; the
  conservative null puts it firmly on the chance side. That is the priced cost.
- **`--stable` now has fewer pairs to compare: 3 of 6 report, against 5 of 6.**
  Not because it rejects anything — it rejects nothing here — but because the
  **256-look runs now refuse**, so there is no second answer to compare against.
  C10 at 0.26 and C14 at 0.13 both recover correctly at 128 and lose their
  partner. **This is a new cost and it was not predicted.**
- Two real collects, one placement, one operating point. Still a selection
  result, not a detection.
