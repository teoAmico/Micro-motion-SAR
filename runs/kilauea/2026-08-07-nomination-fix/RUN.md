# Run: 2026-08-07 kilauea / nomination-fix

**Question this run is meant to answer:** Item 109 named `rs_local_ratio()`'s
guard band as the step that loses a localised target. Is that right, and if not,
what is?

- git commit: `fce5869` (pipeline and hypotheses committed before the first run)
- started:    2026-08-07T09:56Z
- host:       Darwin arm64

## Collect

Identical to `../2026-08-07-stable-weak-centred/` in every flag — same two
collects, same `--inject-at 24,24`, same amplitudes, same look counts. **Only the
binary differs.** See `PREREG.md` §3 and §5 for the freeze.

## Commands

`rerun109.sh`, which is `../2026-08-07-stable-weak-centred/stablecentred.sh` with
only the scratchpad path changed. Per-window evidence in `windows/`.

## Result

**H1 PASSES (5 of 6, needed 5). H3 PASSES (2 of 2 controls refused).**
H3b, the kill criterion, is in `../../synthetic/2026-08-07-nomination-fix-null/`
and also passes: **1 of 12** motionless scenes survives, item 107's rate exactly,
with injected recall held at **6 of 6**.

| collect | amp | @128 | @256 | verdict | item 109 was |
|---|---|---|---|---|---|
| C10 | 0.00 control | 10.148 | 15.971 | not comparable | not comparable |
| C10 | 0.13 | 10.148 | 20.630 | not comparable | MOVED -> reject |
| C10 | 0.26 | **0.998** | **0.998** | **STABLE -> report** | MOVED -> reject |
| C10 | 0.53 | **0.998** | **0.998** | **STABLE -> report** | MOVED -> reject |
| C14 | 0.00 control | 0.997 | 5.996 | MOVED -> reject | MOVED -> reject |
| C14 | 0.13 | **0.997** | **0.999** | **STABLE -> report** | STABLE -> report |
| C14 | 0.26 | **0.997** | **0.999** | **STABLE -> report** | STABLE -> report |
| C14 | 0.53 | **0.997** | **0.999** | **STABLE -> report** | STABLE -> report |

**Item 109's stated mechanism is wrong** — the third wrong explanation of the
same failure. Sweeping the guard band 2 to 8 bins never recovers the injected
line; its block stays at 13 and its support at 26-32 throughout. The Hann-skirt
argument does not apply on this collect at all, because at `--overlap 0` the
floor is flat: median psd across the band varies by 1.3x.

**The target was lost twice, and neither loss is the guard band.**

1. **Refused at the SUPPORT gate.** `support_min` is 34 of 225 voting windows;
   the injected bin's support is 28. It never reached the ranking. That
   threshold is a fraction of the whole window grid, so a mode occupying a
   handful of windows cannot reach it however strong it is.
2. **Then out-ranked on EXTENT.** Ranked block-first it came fourth, behind
   three artefacts that beat it by exactly one window, while leading every rival
   on strength — median local ratio 8.98 against 4.5-5.5, max 73.7 against 24.
   The target's own window nominates it at ratio 41.5, its top pick, on a psd
   peak 107x that window's median.

Both are fixed: admission is now the 2x2 block floor restated (so support
refuses only what the block gate refuses anyway, with `rs_modal_null()` drawn
under the same rule so the chance block rises 7 -> 9 to compensate), and ranking
is by `evidence = n_contiguous * log(median_ratio)`.

### The recovery is now a threshold that can be read off

`ev` for the injected 0.998 Hz line against its scene's own competition, at
128 looks:

| amp | C10 injected `ev` | C10 competition | C14 injected `ev` | C14 competition |
|---|---|---|---|---|
| 0.00 | — | **25.0** (10.148 Hz) | — | **28.3** (0.997 Hz) |
| 0.13 | 16.6 (5th) | 23.8 | **40.5** (1st) | 15.5 |
| 0.26 | **24.0** (1st) | 23.8 | **51.6** (1st) | 15.5 |
| 0.53 | **28.5** (1st) | 23.8 | **55.1** (1st) | 17.5 |

So C10 crosses between 0.13 and 0.26 mm and C14 is already across at 0.13 —
**which is item 103's competition floor of 0.13-0.26 mm, reached independently
through a different statistic.** The line is admitted at every amplitude now,
including 0.13 mm where it sits fifth; before this change it was admitted at
none.

### What did not change, and should not be read as fixed

- **C14's motionless control still leads with 0.997 Hz** at `ev` 28.3 against an
  injected 1.00 Hz. Item 108's false positive is untouched by any of this, and
  `--stable` is still the only thing that rejects it (0.997 at 128, 5.996 at
  256). A scene with nothing in it still answers 0.003 Hz from the frequency
  being sought.
- **C10 at 0.13 mm is a miss, not a wrong answer.** It reports 10.148 Hz at 128
  looks and abstains at the comparison. My pre-registered prediction was 6 of 6
  and it is 5 of 6; the prediction was wrong in the direction the amplitude
  table above explains.
- Abstentions from the common-band rule went 3 of 8 to 2 of 8. Item 108's open
  point about closer look counts is unaffected.
- This is a **selection** result. Nothing here says the chain detects real
  motion, and the target was put where it was found.
