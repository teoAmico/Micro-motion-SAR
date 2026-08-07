# PREREG — kilauea / blockmedian

Filled in on: `2026-08-07`   git commit: `e5cc13a` (the change itself is not yet committed; see §5)
Status: **[x] confirmatory   [ ] exploratory**

## 1. Question

Item 111 named a defect it did not fix: `median_ratio`, the strength term in the
modal set's ranking key, is a median over every window that nominated the bin,
and most of those nominated by chance. Taking the median over the mode's own
block instead makes the statistic track the signal. Does it hold the recall and
the false-positive rate items 110 and 111 measured?

## 2. Ground truth

Unchanged from `../2026-08-07-refbins/`: the injection geometry on two real
Kilauea collects, and twelve motionless synthetic scenes where the truth is that
nothing moves.

## 3. Data freeze

Identical to `../2026-08-07-nomination-fix/PREREG.md` §3. `df` 0.16637 Hz at 128
looks, first admissible bin 0.499 Hz, band top 10.65 Hz, injected 1.000 Hz at
bin 6.

## 4. Hypotheses

Items 110 and 111's thresholds, restated unchanged, because this is a third
re-measurement of one claim and moving the bar would make the comparison
meaningless.

- **H1 recovery.** At least **5 of 6** injected runs report within half a bin of
  0.998 Hz at 128 looks. *Items 110 and 111 both scored 5 of 6.*
- **H3 specificity, real.** Both motionless controls rejected, **2 of 2**.
- **H3b specificity, synthetic — THE KILL CRITERION.** At most **1 of 12**
  motionless scenes survives, and injected recall stays at **6 of 6**.

**Recorded prediction, and it is a real prediction this time rather than a
restatement.** The dilution suppressed every candidate's strength toward the
chance background, so removing it *raises every `ev` in the scene, artefacts
included*. Whether the injected line gains more than its competition is not
something I can derive — it depends on whether the injected line's block is
purer than an artefact's, which is the whole question. So:

- I expect **H1 to hold at 5 of 6 and H3b at 1 of 12**.
- I expect the **margins to widen** — C10 at 0.26 mm was 19.6 against 18.7,
  and if the fix does what it should that gap grows.
- **C10 at 0.13 mm is the point to watch.** It has missed in both previous runs
  at `ev` 15.1 against 18.8. If the fix helps localised modes specifically, this
  is where it would show, and **6 of 6 is possible**. I am not predicting it.
- **A real risk in the other direction**: an artefact whose block is small but
  internally clean now scores higher than it did, and item 108's C14 control is
  exactly such a thing. If its `ev` rises faster than the injected line's, H3
  could fail. That would be an informative failure, not a surprise.

## 5. Pipeline freeze

Commands byte-for-byte those of `../2026-08-07-refbins/rerun.sh` and
`../../synthetic/2026-08-07-refbins-null/stabsweep_v3.sh`.

**The one behavioural change under test:** `rs_mode_t.median_ratio` is the median
of the local ratios of the windows in the mode's LARGEST BLOCK, where it was the
median over every nominating window. Supporting changes: `nom[k]` is indexed by
window rather than by nomination order, and `rs_largest_block()` optionally
returns the winning component's membership via one extra flood fill. The null's
inner loop passes NULL for that mask and is unchanged.

Nothing else moves — not the admission gates, not the `evidence` form, not the
neighbourhood.

## 6. Null model

The motionless arms, real and synthetic, as in items 110 and 111.

## 7. Confounds considered

- [x] **The change raises every candidate's ratio, not just the target's.** That
      is why H3 and H3b are the load-bearing hypotheses here, and why the
      prediction above names the direction the risk runs in.
- [x] Small blocks are now summarised over few values, so `median_ratio` is
      noisier for a mode at the 2x2 floor. The block gate already refuses below
      four windows, so the minimum is a median of four.
- [x] The chance model is untouched and still drawn under the same admission
      rule, so `p_chance` remains calibrated.

## 8. Blinding

- [x] The fix was chosen from a fixture measurement (§10), not from the arms.
- [ ] Not blind: items 110 and 111's outcomes are known and these hypotheses
      restate them.

## 9. Kill criteria

- **If H3b fails** — more than 1 of 12 motionless synthetic scenes survives —
  the change is reverted, whatever H1 says.
- **If H3 fails** on the real controls, likewise.
- **If H1 falls below 5 of 6**, likewise. A strength statistic that tracks the
  signal on a fixture but costs recall on real clutter is not an improvement.

## 10. What the fixture measurement already establishes

On a 65-bin band — the operating point every figure in `FOLLOW-UPS.md` is quoted
at — planting a line on 16 windows of 225 and varying only its amplitude:

| plant gain | `median_ratio` before | after |
|---|---|---|
| 40 | 5.97 | **14.48** |
| 200 | 6.39 | **67.74** |

**A factor of five in signal moved the old statistic by 7%.** It now moves it by
4.7x, and the ranking flips with it: at gain 40 the broad competing plant leads,
at gain 200 the localised one does. `tests/test_modalset.c` passes unchanged,
with every planted ratio risen and the calibration case still admitting 1 of 20
motionless scenes.
