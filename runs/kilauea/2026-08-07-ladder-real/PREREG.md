# PREREG — kilauea / ladder-real

Filled in on: `2026-08-07`   git commit: `5a97133`
Status: **[x] confirmatory   [ ] exploratory**

## 1. Question

Everything in items 115 and 116 is **synthetic**. The ladder statistic, and the
persistence threshold measured from a synthetic null, have never touched real
data. Do they transfer?

**No code changes.** This is a measurement of the shipped chain.

## 2. Ground truth

The injection geometry, unchanged from items 108–114: `--inject-wave` at
1.000 Hz, `--inject-at 24,24`, at 0.13 / 0.26 / 0.53 mm on two real Kilauea
Capella collects, with 0.00 as the motionless control on each.

## 3. Data freeze

Ladder **96 / 128 / 160 / 192 / 224 / 256 looks**, the same rungs as items
115–116. All other flags byte-for-byte those of
`../2026-08-07-bracket/rerun.sh`. 8 configurations × 6 rungs = **48 runs**.

`df` is fixed by the dwell, so all rungs share a bin grid; each adjacent pair is
compared inside the band that pair shares.

## 4. Hypotheses

- **H8 specificity.** Both motionless controls are rejected — chain below 5.
- **H1 recall.** At least **3 of 6** injected runs report. *The bar is low on
  purpose; see the risk below.*
- **H9 transfer.** The threshold behaves on real clutter as it did on synthetic:
  motionless chains stay **at or below 4**, the value measured on 24 synthetic
  motionless scenes.

**Recorded predictions, and the risk is the interesting one:**

- **H8 passes.** Item 114 already refuses both controls outright at 128 and 256
  looks, so most rungs should return nothing and the chain should be 0.
- **H1 is genuinely uncertain and I am not confident.** Item 114's conservative
  gate makes single rungs REFUSE on real data — C10 at 0.13 mm was refused at
  both look counts, and C14 at 0.13 mm answered at 128 and refused at 256. **A
  refusing rung breaks a chain.** If real collects refuse at two or more rungs
  scattered through the ladder, **no chain can reach 5 and recall goes to zero
  even for injections the chain recovers correctly at every rung it answers.**
- **That is the failure mode I expect to see**, and it would be a real finding:
  a threshold of 5 calibrated where 6 of 6 rungs answer does not transfer to
  data where rungs go silent. **If H1 fails that way, the honest conclusion is
  that the criterion must count agreements among ANSWERING rungs rather than
  demand consecutive ones** — which is a change to make deliberately, after
  measuring, not before.
- **H9 is the one that would surprise me if it failed.** A motionless real
  collect reaching chain 5 or 6 would mean real artefacts persist across look
  counts far better than synthetic ones, and would invalidate the threshold
  rather than the ladder.

## 5. Pipeline freeze

```sh
./build/micromotion mmotion --cphd "$DEST/$S.cphd" --estimator phase \
    --n {96,128,160,192,224,256} --overlap 0 --size 256 --cell 0.5 --win 32 \
    --upsample 200 --coherence 0 --max-pulses $MP \
    [--inject-wave "...,$RATE,$amp,20" --inject-at 24,24] --out ...
```
then the 128-look run again with `--stable` naming the other five.

## 6. Null model

The two motionless controls.

## 7. Confounds considered

- [x] **A refusing rung breaks a chain** — named above as the expected failure
      mode rather than discovered afterwards.
- [x] **The threshold is an operating characteristic of a synthetic fixture**
      (item 116) and this run is the first test of whether it is anything more.
- [x] Each pair's common band differs with look count; handled per pair.
- [x] `df` is common across rungs because only `--n` changes.

## 8. Blinding

- [x] Predictions recorded before the run, including which hypothesis I expect
      to fail and what I would conclude from it.
- [ ] Not blind: items 108–116's outcomes are known.

## 9. Kill criteria

- **If H9 fails** — a motionless real collect reaches chain 5 — the threshold is
  withdrawn as a transferable number and `--stable` reports chain length with no
  verdict on real data.
- **If H1 fails at 0 of 6 while every answering rung agrees**, the consecutive
  requirement is the defect, not the ladder, and item 117 changes it.
