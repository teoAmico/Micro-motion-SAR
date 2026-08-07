# PREREG — kilauea / refbins

Filled in on: `2026-08-07`   git commit: `19c488a` (the change itself is not yet committed; see §5)
Status: **[x] confirmatory   [ ] exploratory**

## 1. Question

Item 110 left `rs_local_ratio()`'s band-edge bias unfixed, on the grounds that
the obvious fix broke a test. A different fix — narrowing rather than widening —
removes the bias and keeps the test. Does it hold the recall and the
false-positive rate item 110 measured?

## 2. Ground truth

Unchanged from `../2026-08-07-nomination-fix/`: the injection geometry on two
real Kilauea collects, and twelve motionless synthetic scenes where the truth is
that nothing moves.

## 3. Data freeze

Identical to `../2026-08-07-nomination-fix/PREREG.md` §3. `df` 0.16637 Hz at 128
looks, first admissible bin 0.499 Hz, band top 10.65 Hz, injected 1.000 Hz at
bin 6.

## 4. Hypotheses

Item 110's thresholds, unchanged, because this is a re-measurement of the same
claim under a changed estimator and moving the bar would make the comparison
meaningless.

- **H1 recovery.** At least **5 of 6** injected runs report within half a bin of
  0.998 Hz at 128 looks. *Item 110 scored 5 of 6.*
- **H3 specificity, real.** Both motionless controls rejected, **2 of 2**.
- **H3b specificity, synthetic — THE KILL CRITERION.** At most **1 of 12**
  motionless scenes survives the stability test, and injected recall stays at
  **6 of 6**.

Recorded prediction: **H1 and H3b both hold, and C10 at 0.13 mm remains the
miss.** The bias being removed is a variance effect on which bin wins a maximum,
and the injected line at 0.13 mm loses on evidence rather than on nomination, so
I do not expect the miss to turn. If C10 0.13 mm now recovers, that is a better
result than predicted and should be reported as unpredicted.

## 5. Pipeline freeze

Commands byte-for-byte those of `../2026-08-07-nomination-fix/rerun109.sh` and
`../../synthetic/2026-08-07-nomination-fix-null/stabsweep_v2.sh`.

**The one behavioural change under test:** `rs_local_ratio()` takes the
`RS_LOCAL_REF_BINS` NEAREST reference bins outside the guard, from whichever side
has them, where it took every bin in a `±RS_LOCAL_HALF_BINS` interval clipped at
the band edges. `RS_LOCAL_REF_BINS` becomes `RS_LOCAL_HALF_BINS -
RS_LOCAL_GUARD_BINS` = 10, the count the band floor itself can supply, so the
count is levelled DOWN and the span never exceeds what it was anywhere.

Beside it, `tests/test_modalset.c` is added. It is a test and changes no
behaviour, but it is listed here because it was written first and two of its
cases were tuned before the arms were run.

## 6. Null model

The motionless arms, real and synthetic, as in item 110. No shuffle null.

## 7. Confounds considered

- [x] The change is a pure variance effect on a null with no signal; the
      measurement in §10 is on scenes containing nothing, so no injected signal
      can flatter it.
- [x] **Red floor** — the reason the previous attempt failed. Narrowing makes the
      neighbourhood MORE locally flat, not less, and `test_tracking`'s red-floor
      case is the standing check.
- [x] Losing precision in the background estimate: the median is now over 10
      values rather than 20, so every ratio is noisier. The chance model absorbs
      a uniform shift; H3b is what tests whether it does.

## 8. Blinding

- [x] The fix was chosen from a measurement on scenes with nothing planted
      (§10), not from the injected arm.
- [ ] Not blind: item 110's outcome is known, and these hypotheses restate it.
      That is what makes this a re-measurement rather than a new claim.

## 9. Kill criteria

- **If H3b fails**, the narrowing is reverted and the band-edge bias goes back
  into `docs/CODE-REVIEW.md` as unfixed, with a second wrong fix recorded.
- **If H1 falls below 5 of 6**, likewise: a bias fix that costs recall is not a
  fix, and item 110's state is the one to keep.

## 10. What the measurement already establishes, before these arms

The bias was measured directly, on **200 realisations of flat unit-mean noise
containing nothing at all**, recording which bin won the local-ratio maximum:

| | starved bins (39% of band) | full-neighbourhood bins |
|---|---|---|
| before | **72% of maxima**, 2.98% per bin | 0.75% per bin |
| after | 36% of maxima, 1.50% per bin | 1.68% per bin |

**4.0x over-representation becomes 0.89x.** The two frequencies item 109's report
actually named — its first admissible bin and bin 61 — both sit in the starved
zone, which is what made this worth chasing.

`test_tracking`'s red-floor case still finds its tone (1.2500 Hz against a
planted 1.25, local ratio 155.7), which is the case the previous attempt failed.
