# Run: 2026-08-07 synthetic / ladder

**Question this run is meant to answer:** does a stabilization LADDER with
consecutive-persistence beat item 107's two-point test?

- git commit: `c2a0849` (pipeline and hypotheses committed before the first run)
- host:       Darwin arm64

## Commands

`laddersweep.sh`. Twelve motionless and six injected scenes, each processed at
**96 / 128 / 160 / 192 / 224 / 256 looks**; only `--n` differs between a scene's
rungs. Results in `ladder_results.txt`.

## Result

**H1 passes 6 of 6. H3b passes 1 of 12, at the bar. H5 passes decisively.**

| | two-point (item 114) | ladder (item 115) |
|---|---|---|
| injected reported | 6/6 | **6/6**, every one on a full 6-rung chain |
| motionless reported | 0/12 | **1/12** |
| motionless given a DEFINITE verdict | **1/12** | **12/12** |

**H5 is the win.** The two-point test could only decide 1 of 12 motionless
scenes — the rest abstained, because item 114 made single rungs refuse and a
pair with a refusing partner has nothing to compare. The ladder decides **all
twelve**: eleven reject outright with no two rungs agreeing, one reports.

Every injected scene chains all six rungs, e.g. 0.502 / 0.504 / 0.500 / 0.508 /
0.510 / 0.504 Hz.

## The one that reports is the finding

Seed 31 — the scene that survived items 107, 110 **and** 111 — reports on a chain
of **four**, at a claimed p of **0.0000**:

```
   96 looks:  0.954 Hz
  128 looks:  0.958 Hz   <- this run
  160 looks:  0.950 Hz
  192 looks:  0.965 Hz
  224 looks:  0.153 Hz
  256 looks:  0.151 Hz
```

**A motionless scene holds ~0.95 Hz across four consecutive look counts.** Under
`rs_stable_p()` that is 3 x (1/62)^3 = 1.3e-5. It happened on 1 of 12 scenes.
**The derived p is wrong by four orders of magnitude, and the cause is the
assumption it rests on: that rungs are independent.**

They are not. Every rung uses the **same pulses over the same dwell**, divided
differently — 192 looks and 224 looks build sub-apertures out of largely the same
phase history — so an artefact pinned to the scene persists across rungs exactly
as it persists across overlapping windows.

**This is item 113's error on a second axis**, and the third time in this project
that a chance model has assumed independence where the construction manufactures
correlation. The instrument is better; its p-value is not calibrated.

## What stands and what does not

- **The persistence STATISTIC is sound and is a real improvement**: 12 of 12
  definite verdicts against 1 of 12, and full 6-rung chains on every injection.
- **The p attached to it is not.** Read the chain length; do not quote the p.
- H3b passes at its bar of 1 of 12, but it passes for the wrong reason — the
  ladder did not remove item 107's surviving false positive, it re-detected it
  with more confidence.

## Bounds

One fixture family, one operating point, one injected amplitude (2 mm, 6.9x this
fixture's floor). The ladder was not run on real Kilauea data; at ~75 s a run and
6 rungs x 8 configurations that is a 60-minute arm, deferred.
