# Run: 2026-08-07 kilauea / refbins

**Question this run is meant to answer:** Item 110 left `rs_local_ratio()`'s
band-edge bias unfixed because the obvious fix broke a test. Narrowing rather
than widening removes the bias and keeps the test — does it hold the recall and
the false-positive rate item 110 measured?

- git commit: `8885312` (pipeline and hypotheses committed before the first run)
- started:    2026-08-07T12:05Z
- host:       Darwin arm64

## Collect

Identical to `../2026-08-07-nomination-fix/` in every flag. **Only the binary
differs.** `PREREG.md` §5 has the freeze.

## Commands

`rerun.sh`, which is `../2026-08-07-nomination-fix/rerun109.sh` with only the
output path changed. Per-window evidence in `windows/`. `measure_edge_bias.c`
is the bias measurement itself; `edge_bias_after.txt` is its output on this
build.

## Result

**H1 PASSES (5 of 6). H3 PASSES (2 of 2).** The kill criterion H3b is in
`../../synthetic/2026-08-07-refbins-null/` and also passes: **1 of 12**
motionless scenes survives with injected recall at **6 of 6**.

**All three pre-registered predictions were correct** — H1 and H3b hold, and
C10 at 0.13 mm remains the miss. That is the first time a prediction recorded in
advance here has been right on every point; items 103, 105, 109 and 110 each had
one wrong.

| collect | amp | @128 | @256 | verdict | item 110 was |
|---|---|---|---|---|---|
| C10 | 0.00 control | 5.823 | 4.825 | **MOVED -> reject** | not comparable |
| C10 | 0.13 | 0.499 | 20.630 | not comparable | not comparable |
| C10 | 0.26 | **0.998** | **0.998** | **STABLE -> report** | STABLE |
| C10 | 0.53 | **0.998** | **0.998** | **STABLE -> report** | STABLE |
| C14 | 0.00 control | 0.997 | 9.327 | MOVED -> reject | MOVED |
| C14 | 0.13 | **0.997** | **0.999** | **STABLE -> report** | STABLE |
| C14 | 0.26 | **0.997** | **0.999** | **STABLE -> report** | STABLE |
| C14 | 0.53 | **0.997** | **0.999** | **STABLE -> report** | STABLE |

### What the fix visibly removed

**The 10.148 Hz artefact that led C10's motionless control in item 110 is gone.**
It was bin 61 — inside the starved zone, where the neighbourhood was clipped to
10 references against mid-band's 20. C10's control now leads with 5.823 Hz, a
mid-band bin, and the whole competition dropped with it:

| | item 110 | item 111 |
|---|---|---|
| C10 competition `ev` | 23.8 – 25.0 | **17.1 – 18.8** |
| C10 leading control frequency | 10.148 Hz (bin 61, starved) | 5.823 Hz (mid-band) |
| real-arm abstentions | 2 of 8 | **1 of 8** |

That is the intended effect measured end to end: an artefact that existed because
the background under it was estimated from half as many bins as everything else.

### The evidence table, 128 looks

| amp | C10 injected | C10 competition | C14 injected | C14 competition |
|---|---|---|---|---|
| 0.00 | — | **17.1** | — | **28.0** |
| 0.13 | 15.1 (3rd) | 18.8 | **42.3** | 19.1 |
| 0.26 | **19.6** | 18.7 | **46.7** | 19.1 |
| 0.53 | **28.6** | 18.7 | **46.7** | 21.2 |

C10 still crosses between 0.13 and 0.26 mm, so item 110's agreement with item
103's 0.13–0.26 mm competition floor is unchanged. **The margin at 0.26 mm is
thin — 19.6 against 18.7** — and was thin in item 110 too (24.0 against 23.8), so
that point should not be quoted as a comfortable recovery in either run.

### What did not change

- **C14's motionless control still leads with 0.997 Hz**, now at `ev` 28.0
  against 28.3. Item 108 is untouched by this and by item 110: a scene with
  nothing in it still answers 0.003 Hz from the frequency being sought, and
  `--stable` is still the only thing that rejects it.
- C10 at 0.13 mm is still a miss. It now reports the band floor at 128 looks
  where it reported 10.148 Hz, so the answer changed and the verdict did not.
- This remains a **selection** result. The target was put where it was found.
