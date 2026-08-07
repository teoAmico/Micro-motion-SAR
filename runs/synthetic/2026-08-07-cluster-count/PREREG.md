# PREREG — synthetic / cluster-count

Filled in on: `2026-08-07`   git commit: `0c0cd8a` (the change itself is not yet committed; see §5)
Status: **[x] confirmatory   [ ] exploratory**

## 1. Question

Item 117 showed `--stable`'s CONSECUTIVE requirement rejecting a real injection
whose every answering rung was correct, and the field counts cluster size rather
than unbroken runs. The statistic is now a count. **What is the null's
distribution under it, and does one threshold serve both the synthetic fixture
and real collects?**

## 2. Ground truth

- **Synthetic:** item 116's 24 motionless scenes (seeds 3–41 and 43–97) and 12
  injected at 2 mm.
- **Real:** item 117's eight configurations on C10 and C14. Their per-rung
  answers are already measured and do not change — only the verdict step is
  re-run.

## 3. Data freeze

Ladder 96/128/160/192/224/256 looks throughout. All other flags as items 116
and 117.

## 4. Hypotheses

**This run is expected to CHANGE NO VERDICT, and to establish why.**

- **H10 the statistic.** Under counting, every real injected configuration's
  support equals the number of rungs that answered, and the two real motionless
  controls have support **0**.
- **H11 the null.** The synthetic motionless support distribution is unchanged
  from item 116's chain distribution: **22 of 24 at 0, 2 of 24 at 4**, none at 5
  or above.
- **H12 the point.** **No single threshold serves both fixtures.** At 5 the
  synthetic null stays clean and C10 at 0.26 mm — a true injection carried by 4
  of 6 rungs — is still rejected. At 2–4 that injection is recovered and the two
  synthetic motionless scenes at support 4 become false positives.

**Recorded predictions:**

- **H10 and H11 pass.** Verified already on C10 at 0.26 mm: it now reports
  *"0.998 Hz is carried by 4 of 6 rungs able to express it, 5 needed"* — the
  statistic reads correctly and the verdict does not move.
- **H12 passes, and it is the finding.** Measured real supports are
  **0, 0, 1, 4, 6, 6, 6** against synthetic motionless reaching **4**. The
  injected and motionless populations OVERLAP at 4 across fixtures, so a single
  constant cannot separate them.
- **Therefore I predict the fix changes no verdict on either fixture**, and that
  the honest conclusion is that `RS_STABLE_MIN_CHAIN` is a **per-fixture
  operating characteristic** — which item 116 stated as a caveat and this
  measures as a fact.

## 5. Pipeline freeze

`--stable` scores a candidate by **how many rungs carry it**, gaps included,
instead of by the longest consecutive run. Rungs whose Nyquist is below the
candidate are excluded from the count entirely rather than treated as
disagreeing, and the report says how many rungs could express it.
`RS_STABLE_MIN_CHAIN` is unchanged at 5 **pending this measurement**.

## 6. Null model

The 24 synthetic motionless scenes, plus the two real motionless controls.

## 7. Confounds considered

- [x] **Changing the statistic re-opens the threshold** — that is what this run
      measures, and item 116 said so in advance.
- [x] **The two fixtures have different nulls.** Real motionless collects refuse
      at every rung (support 0); synthetic motionless scenes carry a persistent
      artefact to support 4. This is the confound, not a nuisance.
- [x] Rungs that cannot express a candidate are excluded, so a high-frequency
      mode is not penalised for rungs that never could see it.

## 8. Blinding

- [x] Predictions recorded before the arms, including that no verdict moves.
- [ ] Not blind: items 115–117's outcomes are known.

## 9. Kill criteria

- **If H11 fails** — a synthetic motionless scene reaches support 5 or more —
  the threshold must rise, and the ladder's margin on that fixture is gone.
- **If H12 fails** because some threshold DOES serve both, adopt it and say so.

## 10. Why counting rather than consecutiveness, from the literature

Poles are routinely **not identified at every model order**; MATLAB's `modalsd`
returns the missing ones as **NaN**. Automated OMA **clusters poles across the
whole diagram and thresholds on minimum cluster size** (Reynders 2012 and
successors), not on unbroken runs. Requiring consecutiveness was this project's
own addition. Item 117 measured its cost on real data: a true injection carried
correctly by every rung that answered, rejected for two REFUSALS.

**The statistic is now right whether or not any verdict moves**, which is the
argument for making the change even if this run confirms it changes nothing.
