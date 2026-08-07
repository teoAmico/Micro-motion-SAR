# Run: 2026-08-07 synthetic / bracket-null

**Question this run is meant to answer:** does gating on the conservative end of
the bracket cost specificity or recall on the synthetic fixture?

**KILL CRITERION** for `runs/kilauea/2026-08-07-bracket/`, pre-registered at
`6fb1cb4` §9.

- git commit: `6fb1cb4`
- host:       Darwin arm64

## Result

**H3b PASSES, with no synthetic recall cost at all.**

| | 107 | 110 | 111 | 112 | 113 | 114 |
|---|---|---|---|---|---|---|
| motionless: report | 1/12 | 1/12 | 1/12 | 0/12 | 0/12 | **0/12** |
| motionless: **refused outright @128** | 0/12 | 0/12 | 0/12 | 0/12 | 4/12 | **9/12** |
| injected: report | 6/6 | 6/6 | 6/6 | 6/6 | 6/6 | **6/6** |

**Three quarters of motionless scenes now return no modal answer at all** — 19 of
the 24 motionless runs across both look counts refuse — while all six injected scenes
still give **0.504 Hz at both look counts to three decimals**.

Item 96 measured this same fixture returning a confident frequency on **12 of 12**
motionless scenes and called a per-scene null the only defence. Nine of twelve
are now silent without one.

**These are the re-run figures.** The first pass was discarded when the gate was
found not to be conservative on every scene; see the kilauea run's RUN.md. It
gave 8 of 12 refusals rather than 9, and the same 0 of 12 reporting and 6 of 6
injected.

## Bounds

Twelve realisations, one fixture family, one operating point, one injected
amplitude at 2 mm — which is 6.9x this fixture's floor (item 101), the easy case.
The synthetic arm shows no recall cost; **the real arm does show one** (H1 5 of 6,
and `--stable` pairs falling from 5 to 3). Read them together.
