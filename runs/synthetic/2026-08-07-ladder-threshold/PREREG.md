# PREREG — synthetic / ladder-threshold

Filled in on: `2026-08-07`   git commit: `b7db626` (the change itself is not yet committed; see §5)
Status: **[x] confirmatory   [ ] exploratory**

## 1. Question

Item 115's ladder statistic is sound and the p attached to it is wrong by four
orders of magnitude. The fix replaces the p with a **persistence threshold
measured from the null's own distribution**. That threshold was read off item
115's twelve motionless scenes — **so re-running those scenes tests nothing.**
Does it hold on twelve seeds it was never fitted to?

## 2. Ground truth

Two arms, and the distinction between them is the whole point of this run:

- **Consistency arm (NOT a test):** item 115's twelve motionless seeds
  3–41 and six injected. The threshold was fitted to these. Re-running them can
  only confirm the arithmetic, and is reported as such.
- **INDEPENDENT arm (the test):** **twelve new motionless seeds — 43, 47, 53, 59,
  61, 67, 71, 73, 79, 83, 89, 97 — and six new injected seeds 43–67**, none of
  which the threshold has seen.

## 3. Data freeze

Ladder 96 / 128 / 160 / 192 / 224 / 256 looks, `--size 128 --cell 0.5 --win 32
--overlap 0 --upsample 200 --coherence 0`, `--estimator phase`, injections at
2 mm. Identical to item 115 in every respect except the seeds.

## 4. Hypotheses

- **H6 the test.** On the twelve INDEPENDENT motionless seeds, **at most 1 of 12**
  reports. *Item 115's ladder-with-p reported 1 of 12 on its own seeds and did so
  for the wrong reason.*
- **H1 recall, independent.** At least **5 of 6** independent injected scenes
  report.
- **H7 consistency.** On item 115's original seeds the threshold reproduces what
  it was fitted to: **0 of 12** motionless report (seed 31's chain of 4 now falls
  short of 5) and **6 of 6** injected report. This is arithmetic, not evidence.

**Recorded predictions:**

- **H7 passes trivially** — already verified on seed 31 (chain 4 of 5 → reject)
  and on injected seed 7 (chain 6 → report).
- **H6 passes at 0 or 1 of 12.** The measured null had eleven scenes at chain 0
  and one at 4; if a new seed reaches 5 or 6, the threshold is fitted to noise
  and I want that on the record.
- **H1 passes at 6 of 6.** Every injected scene in item 115 chained all six
  rungs, with no case near the boundary.
- **The risk I am naming**: 5 was chosen from a null whose maximum was 4, so it
  sits ONE rung above the largest observed artefact. That is a margin of one on
  twelve scenes, and a thirteenth scene reaching 5 would not be surprising. **If
  H6 fails at 2 or more, the honest conclusion is that no fixed chain length
  separates these populations and the ladder must report its chain without a
  verdict.**

## 5. Pipeline freeze

`rs_stable_p()` is **removed**. `--stable` reports the longest chain of
consecutive agreeing rungs against `RS_STABLE_MIN_CHAIN = 5`, and **refuses to
give a verdict at all when the ladder is shorter than 5 rungs** rather than
applying a weaker criterion. Nothing else changes.

## 6. Null model

The motionless arms are the null. There is no analytic null any more, and §10
records why there cannot be one.

## 7. Confounds considered

- [x] **Circularity** — the reason this run exists; handled by the independent
      arm, and the consistency arm is labelled as arithmetic.
- [x] **A threshold fitted to twelve scenes has a wide interval.** Twelve more
      scenes narrows it; it does not settle it, and §4 says what a failure means.
- [x] Rung correlation is not modelled and cannot be — §10.

## 8. Blinding

- [x] The independent seeds were chosen before running, and are listed above.
- [x] Predictions recorded before the arm.

## 9. Kill criteria

- **If H6 fails at 2 or more of 12**, the fixed threshold is abandoned and
  `--stable` reports the chain length with no verdict — which is what the OMA
  literature does.
- **If H1 falls below 5 of 6**, likewise.

## 10. Why there is no p-value, checked against the literature

**The field attaches none.** A stabilization diagram is an ACCEPTANCE CRITERION,
not a significance test: poles are marked stable when frequency, damping and mode
shape persist across consecutive model orders under tolerances, and the result is
refined by CLUSTERING. Searched twice — the number of consecutive orders required
is explicitly **application-dependent and chosen by the user**, not derived. The
commonly quoted five is a convention.

**Two nulls were considered here and both fail**, which is why the threshold is
measured rather than modelled:

- **Per-window chains as an empirical null** — fails, because a scene-wide
  artefact and a scene-wide injection are structurally identical in window
  statistics. That is item 11.
- **Re-dividing the dwell to build the null** — fails, because re-dividing
  returns the same answers; there is no randomisation that destroys a real mode
  while preserving a scene-pinned artefact. That is item 114's wall, on a second
  axis.

So the number comes from the null's own measured distribution, which is item 80's
rule for exactly this situation. On item 115's twelve motionless scenes the
longest chain reached was **0 on eleven and 4 on one**, against **6 on all six
injected**. Five separates them with one rung of margin on each side and
coincides with the literature's usual figure — **corroboration, not derivation.**
