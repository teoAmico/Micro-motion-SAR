# Run: 2026-08-07 kilauea / ladder-real

**Question this run is meant to answer:** everything in items 115–116 is
synthetic. Does the ladder, and its threshold calibrated on a synthetic null,
transfer to real Kilauea data?

- git commit: `d949eb1` (hypotheses committed before the first run). **No code
  changes** — this measures the shipped chain.
- host:       Darwin arm64

## Result

**H8 passes. H9 passes decisively. H1 passes at exactly its bar, 3 of 6 — and
the reason it is only 3 is the failure mode the pre-registration named.**

| collect | amp | 96 | 128 | 160 | 192 | 224 | 256 | chain | verdict |
|---|---|---|---|---|---|---|---|---|---|
| C10 | 0.00 | — | — | — | — | — | — | 0 | reject |
| C10 | 0.13 | — | — | — | — | — | — | 0 | reject |
| C10 | 0.26 | — | **0.998** | **0.997** | **0.997** | **1.000** | — | **4** | **reject** |
| C10 | 0.53 | 0.997 | 0.998 | 0.997 | 0.997 | 1.000 | 0.998 | 6 | **report** |
| C14 | 0.00 | — | — | — | — | — | — | 0 | reject |
| C14 | 0.13 | — | 0.997 | — | — | — | — | 0 | reject |
| C14 | 0.26 | 0.998 | 0.997 | 0.999 | 0.999 | 1.000 | 0.999 | 6 | **report** |
| C14 | 0.53 | 0.998 | 0.997 | 0.999 | 0.999 | 1.000 | 0.999 | 6 | **report** |

(— is REFUSED: that rung's modal set admitted nothing.)

### Specificity on real data is total

**Not one of the twelve motionless rungs answered.** Both controls refuse at
every look count from 96 to 256, so the chain is 0 by absence rather than by
disagreement. That is the strongest specificity result this project has on real
clutter, and it settles H9: real artefacts did **not** persist across look
counts better than synthetic ones — they did not persist at all.

### Every rung that answered on an injected scene was right

Across the four injected configurations that answered anywhere, **every single
answering rung agreed with the injected 1.00 Hz**, spanning 0.997–1.000 Hz over
look counts from 96 to 256. There is not one disagreeing rung in the table.

### And that is exactly why C10 at 0.26 mm is the finding

```
   96 looks: REFUSED
  128 looks: 0.998 Hz     160 looks: 0.997 Hz
  192 looks: 0.997 Hz     224 looks: 1.000 Hz
  256 looks: REFUSED
```

**Four consecutive agreeing rungs, every answering rung correct, and it is
rejected** — because the two missing rungs are REFUSALS, not disagreements, and
the criterion demands five *consecutive*.

`PREREG.md` §4 named this in advance: *"a refusing rung breaks a chain... if that
happens the criterion must count agreements among ANSWERING rungs rather than
demand consecutive ones."*

**The literature agrees, and it is what automated OMA already does.** Poles are
not identified at every model order — MATLAB's `modalsd` returns the missing ones
as NaN — and automated methods CLUSTER poles across the whole diagram and
threshold on **minimum cluster size**, not on an unbroken run. Requiring
consecutiveness is this project's own addition and it is the wrong one.

## What this establishes and what it does not

- **The ladder transfers.** Specificity is perfect on real data and every
  answering rung on an injected scene is correct.
- **The threshold does not need re-measuring upward** — H9 passed with motionless
  chains of 0, far below the 4 seen synthetically.
- **The CONSECUTIVE requirement is a defect**, demonstrated on real data by a
  case where the chain is unbroken in every rung that spoke.
- Recall is **3 of 6** against item 114's 5 of 6 on the same collects at 128
  looks. The ladder costs recall as it stands; counting answering rungs would
  recover C10 at 0.26 mm and is the next item.
- Two real collects, one placement, one operating point, target put where it was
  found. **Still a selection result.**
