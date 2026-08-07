# Run: 2026-08-07 synthetic / clustermass-null

**Question this run is meant to answer:** does gating on cluster mass against a
correlation-preserving permutation null cost specificity or recall?

**This is the KILL CRITERION** for `runs/kilauea/2026-08-07-clustermass/`,
pre-registered there at `87d27eb` §9.

- git commit: `87d27eb`
- started:    2026-08-07T15:22Z
- host:       Darwin arm64

## Collect

Item 107's fixture, fifth measurement: twelve motionless `--clutter-vib` scenes
(seeds 3-41) and six injected at 2 mm, each at 128 and 256 looks.

## Commands

`stabsweep_v5.sh`, which is `../2026-08-07-blockmedian-null/stabsweep_v4.sh` with
only output and fixture paths changed. Scored by
`../2026-08-07-nomination-fix-null/score.py` against all five result sets.

## Result

**H3b PASSES, and it breaks something item 96 established.**

| | 107 | 110 | 111 | 112 | 113 |
|---|---|---|---|---|---|
| motionless: report | 1/12 | 1/12 | 1/12 | 0/12 | **0/12** |
| motionless: **refused outright @128** | 0/12 | 0/12 | 0/12 | 0/12 | **4/12** |
| injected: report | 6/6 | 6/6 | 6/6 | 6/6 | **6/6** |

**Item 96 measured twelve of twelve motionless scenes returning a confident
frequency — a 100% answer rate — and called a per-scene null control the only
thing between this chain and it.** Four of those twelve now return **no modal
answer at all**, before `--stable` is consulted: seeds 3, 11, 13 and 41 at 128
looks, and seeds 13 and 17 at 256.

Recall is untouched: all six injected scenes still give **0.504 Hz at both look
counts to three decimals**.

## Why the abstention count rose

Seven motionless scenes are now scored "abstain" against three in item 112, and
the reason is different from item 111's: six of those are `REFUSED` — one or
both look counts produced no modal set at all, so there is nothing to compare.
**An abstention because the chain refused to answer is not the same failure as
an abstention because the answer fell outside the common band**, and the scorer
lumps them together. Read the `REFUSED` column in `stab_results_v5.txt`, not the
tally.

## Bounds

Twelve realisations, one fixture family, one operating point, one injected
amplitude. 0 of 12 reporting is a rate with a wide interval, and 4 of 12
refusing outright is the first measurement of that quantity — there is no
earlier value to compare it against except item 96's 0 of 12.
