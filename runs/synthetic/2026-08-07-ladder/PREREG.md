# PREREG — synthetic / ladder

Filled in on: `2026-08-07`   git commit: `10fe380` (the change itself is not yet committed; see §5)
Status: **[x] confirmatory   [ ] exploratory**

## 1. Question

Item 107 implemented the OMA stabilization diagram as a **two-point** test: one
run at 128 looks against one at 256. The literature sweeps model order over a
range and accepts a pole only when it persists across **several consecutive**
orders. Does a ladder beat the pair — and does it repair item 114's cost, which
was that single rungs now REFUSE often enough to leave a two-point test with no
partner?

## 2. Ground truth

Item 107's fixture, unchanged: twelve motionless `--clutter-vib` scenes (seeds
3–41), truth "nothing moves", and six injected at 2 mm with a true 0.500 Hz.

## 3. Data freeze

`--size 128 --cell 0.5 --win 32 --overlap 0 --upsample 200 --coherence 0`,
`--estimator phase`. **The ladder is 96 / 128 / 160 / 192 / 224 / 256 looks**,
six rungs, chosen before running as an even spread around the 128 this project
is quoted at and the 256 item 107 used. `df` is fixed by the dwell, so all rungs
share a bin grid; each pair's common band is set by the lower rung's Nyquist.

## 4. Hypotheses

- **H1 recall.** At least **5 of 6** injected scenes are reported.
- **H3b specificity — kill criterion.** At most **1 of 12** motionless scenes is
  reported. *Item 107 measured 1 of 12 with the pair; items 112–114 have it at
  0 of 12.*
- **H5 the point of the ladder.** The number of scenes on which the test can
  return a verdict at all — neither "no two rungs agree" for want of data nor an
  abstention — is **higher** than the two-point test manages on the same scenes.

**Recorded predictions:**

- **H1 passes at 6 of 6.** Verified on one injected scene before writing this:
  all six rungs answer within 0.500–0.510 Hz, a chain of 6, p ≈ 0.
- **H3b passes at 0 of 12.** Verified on seed 17 — item 107's *surviving* false
  positive, the one scene that beat the two-point test — where **every rung now
  refuses**, so the chain is 0.
- **H5 passes**, because a refusing rung breaks a chain instead of ending the
  test.
- **A two-rung chain will no longer be enough on a six-rung ladder**, and that
  is intended: at 62 bins a lucky pair is p = 0.016 alone but p = 0.081 once
  there are five places for it to appear. If any scene reports on a chain of
  exactly 2, the derived threshold has been mis-specified and I want to see it.

## 5. Pipeline freeze

`--stable` now takes a **comma-separated list**. Each file is a rung; this run is
a rung too. Rungs are sorted by look count, adjacent pairs are compared within
the band that PAIR shares, and the statistic is the **longest chain of
consecutive agreeing rungs**.

The threshold is **derived, not chosen**: under the null a rung's answer falls
anywhere in the admissible band, so a pair agrees with probability about
`1/n_bin` and a chain of `r` rungs needs `r-1` agreements, with `n_rung-r+1`
places to start — `rs_stable_p()`. A mode reports at `p <= RS_MODAL_P_MAX`.
**A longer ladder therefore demands more persistence**, which is the
look-elsewhere cost along the ladder and is the part a two-point test could not
express.

## 6. Null model

The twelve motionless scenes.

## 7. Confounds considered

- [x] **`df` must be common across rungs** — it is, because the dwell is fixed
      and only `--n` changes. A ladder across DWELLS is a different test and
      still untested (item 107).
- [x] **Each pair's common band differs**, since Nyquist scales with the look
      count. Handled per pair rather than globally, so a high-frequency answer
      at the top of the ladder is not silently compared against a rung that
      cannot express it.
- [x] Duplicate look counts are refused — two runs at the same count are the
      same rung and test nothing.

## 8. Blinding

- [x] The ladder was chosen before any sweep, and the two verification scenes
      (one injected, one motionless) were run before the hypotheses were fixed
      and are named above.
- [ ] Not blind: items 107–114's outcomes are known.

## 9. Kill criteria

- **If H3b fails** — more than 1 of 12 motionless scenes reports — the ladder is
  reverted and item 107's pair stands.
- **If H1 falls below 5 of 6**, likewise: persistence bought with recall is the
  trade items 110–114 spent six items avoiding.

## 10. Why the literature says a ladder

Searched before designing, which this project's own rule demands and which has
now paid seven times. Stabilization diagrams in OMA sweep the model order and
mark a pole stable only when frequency, damping and mode shape persist across
**consecutive** orders — commonly quoted as **five consecutive identifications**,
with tolerances like 1% in frequency and MAC ≥ 0.95 between successive orders.
Automated methods (Reynders et al. 2012 and successors) go further and CLUSTER
poles across the whole diagram rather than comparing two of them.

Item 107 took the principle and implemented its two-point special case. This is
the same instrument at the shape the field actually uses. What does NOT transfer:
their tolerances are for SSI poles, far more precise than a periodogram bin, so
the tolerance here stays half a bin; and this chain has no damping or mode shape
to require consistency in, so persistence in FREQUENCY is all that is available.
