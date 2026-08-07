# PREREG — kilauea / clustermass

Filled in on: `2026-08-07`   git commit: `c490cbf` (the change itself is not yet committed; see §5)
Status: **[x] confirmatory   [ ] exploratory**

## 1. Question

Item 108: a motionless real collect leads with 0.997 Hz against a sought 1.00 Hz,
and only `--stable` rejects it. The diagnosis (§10) is that `rs_modal_null()`
assumed windows nominate independently when 50% overlap makes them strongly
correlated, so `p_chance` was anti-conservative by an order of magnitude. Does a
correlation-preserving permutation null on cluster MASS fix it, and what does it
cost in recall?

## 2. Ground truth

Unchanged from `../2026-08-07-blockmedian/`: the injection geometry on two real
Kilauea collects, and twelve motionless synthetic scenes.

## 3. Data freeze

Identical to `../2026-08-07-nomination-fix/PREREG.md` §3. `df` 0.16637 Hz at 128
looks, injected 1.000 Hz at bin 6, band top 10.65 Hz.

## 4. Hypotheses

**The primary target is SPECIFICITY, so the thresholds are stated on that side
and H1 is the cost being measured.**

- **H2 calibration (the point of the change).** On the two motionless real
  collects, the number of admitted modes falls by at least half against item
  112. *Item 112 admitted 8 on C14 and 4 on C10.*
- **H3 specificity, real.** Both motionless controls are still rejected by
  `--stable`, **2 of 2**.
- **H1 recovery.** At least **5 of 6** injected runs report within half a bin of
  0.998 Hz at 128 looks. *Item 112 scored 6 of 6.* A fall to 5 is the expected
  price and is recorded as such; below 4 the trade is not worth making.
- **H3b synthetic — kill criterion unchanged.** At most **1 of 12** motionless
  scenes survives the stability test, injected recall at least **5 of 6**.

**Recorded predictions, made after the diagnostic in §10 and before these arms:**

- **H2 passes.** Already observed on C14: 8 admitted modes become 2.
- **The primary aim is NOT fully met, and I am saying so in advance.** Item 108's
  own false positive is **not** refused: measured, its `ev` is 28.4 against a
  critical 23.3, so it survives at **p = 0.010** where it used to be quoted at
  0.001. The fix makes the p-value honest; it does not make this scene silent.
  The residual is most likely that correlation extends beyond the 2x2 tile this
  null decorrelates.
- **H1 falls to 5 of 6.** C10 at 0.13 mm carried `ev` 21.6 in item 112, below a
  critical value of roughly 23, so it should now be refused. C10 at 0.26/0.53
  (39.7/48.1) and all three C14 runs (49.9-61.6) are well clear.
- **C10's motionless control should be refused ENTIRELY** — its best `ev` was
  18.6, below the critical value — which would be the first time a real
  motionless collect returns no modal answer at all.

## 5. Pipeline freeze

Commands byte-for-byte those of `../2026-08-07-blockmedian/rerun.sh` and
`../../synthetic/2026-08-07-blockmedian-null/stabsweep_v4.sh`.

**The two behavioural changes under test, and they are one idea:**

1. **The null preserves the spatial correlation.** `rs_modal_null()` keeps every
   window's own observed nomination pattern and ratios, and gives each **2x2
   tile** of windows an independent circular shift of the admissible band.
   Within a tile the correlation is preserved exactly as observed; across tiles
   it is destroyed. The 2x2 tile is the same half-width stride the block floor
   and `freq_se`'s `n_eff = n/4` already come from — it is geometry, not tuning.
2. **The gate tests MASS, not EXTENT.** Admission is now `evidence`
   (`n_contiguous * log(median_ratio)`) against that null, plus the unchanged
   2x2 geometric floor. `p_chance` becomes P(a chance run reaches this
   evidence). Extent stays reported.

Nothing else moves — not the nomination, not the neighbourhood, not the
`evidence` formula, not `--stable`.

## 6. Null model

The permutation above IS the null model, and the motionless arms test it.

## 7. Confounds considered

- [x] **The null may still be anti-conservative**, because correlation can
      extend past a 2x2 tile when one bright scatterer dominates a wider patch.
      §10 brackets this: a fully-dilated null (assuming neighbours identical)
      puts the observed block at p ~ 0.5. The truth is between, and the residual
      false positive is the evidence of it.
- [x] **Mass could favour a different artefact than extent did.** That is what
      H2 and H3 measure; the motionless scenes are the only defence.
- [x] The `evidence` values are unchanged by this commit — only the gate moves —
      so item 112's measured `ev` figures are valid inputs to the predictions
      above, which is what makes them falsifiable.

## 8. Blinding

- [x] Predictions recorded from a measurement on the motionless scene and on
      item 112's already-published `ev` table, before the arms were run.
- [ ] Not blind: item 112's outcome is known.

## 9. Kill criteria

- **If H3b fails** — more than 1 of 12 motionless synthetic scenes survives, or
  injected synthetic recall drops below 5 of 6 — the change is reverted.
- **If H1 falls below 4 of 6**, reverted: an honest p-value that costs most of
  the recall is a worse instrument, and the right answer would then be to report
  both p-values rather than gate on the new one.
- **If H2 fails** the change has not done the one thing it was built for and is
  reverted regardless of H1.

## 10. The diagnosis this rests on, measured before any of it

On the C14 motionless collect, `--shifts` dumped and the nomination replicated
offline (the replica reproduces the binary exactly: support 35, block 17).

**Two candidate explanations were tested and both died to their own controls.**
Phase coherence at the artefact's bin across its block is 0.673 — but random
contiguous 17-window blocks elsewhere reach up to 0.703, and bin 6 ranks only
5th of 62 bins on those same windows. Amplitude is **1.54 mm**, the clutter noise
level and ~1000x the seismometer truth. It is noise.

**What is real is the correlation.** An adjacent window pair shares **2.37 of 6**
nominations against **0.71** for a random pair. Three nulls on the same observed
block of 17:

| null | median | p95 | p(block >= 17) |
|---|---|---|---|
| independent draw (shipped until now) | 6 | 8 | **0.001**, above its own 300-trial max of 9 |
| 2x2 tile-shift (observed correlation kept) | 11 | 15 | **0.013** |
| 2x2 dilated (neighbours assumed identical) | 16 | 24 | ~0.5 |

And that one motionless scene had **eight** bins clearing p <= 0.05 where the
family-wise design intends 0.05 in total.

**This is a published failure mode, not a new one** — the seventh time a search
has found the field ahead of this project. Eklund, Nichols & Knutsson (PNAS
2016), *Cluster failure: why fMRI inferences for spatial extent have inflated
false-positive rates*: parametric cluster-extent inference is invalid because the
assumed spatial autocorrelation is wrong, while nonparametric permutation gives
nominal rates. Window overlap is their smoothing; `n_contiguous` is their cluster
extent. The remedy for the power cost is theirs too — cluster **mass** is
reported as more powerful than extent and specifically better for *small but
intense* clusters, which is exactly the injected target (block 13 at ratio 40)
that an extent threshold would refuse.

Item 108's false positive is **the largest block in its scene and nearly the
weakest mass**, so no extent threshold separates it and mass can — which is why
both halves of the change are one idea.
