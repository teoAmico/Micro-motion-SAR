# Run: 2026-08-05 synthetic / naples-ambient

**Question this run is meant to answer:** Does item 91's failure replicate on an independent building, or was it Granada's out-of-band mode?

- git commit: `9b43f47`
- started:    2026-08-05T20:54:53Z
- host:       Darwin arm64

## Collect

sim_cphd fixture, 400 clutter scatterers, --clutter-vib, seeds 7 and 11.
Injected source: Zenodo 20667124 (Naples), reference accelerometer ACC-1,
segments 02 (setup 1) and 13 (setup 3), double-integrated with a 1.0 Hz
high-pass. 128 looks, overlap 0, --estimator phase, 2 mm.

## Commands

See sweep_naples.sh, committed here. Hypotheses pre-registered in PREREG.md at
commit cb47b06, BEFORE the sweep ran.

## Result

**H1 FAILS, as pre-registered.** **H3 FAILS.** The prediction recorded before
running -- that H1 would fail -- was correct.

| | Naples (this run) | Granada (item 91) |
|---|---|---|
| correct within half a bin | **7 of 24** | 1 of 24 |
| `rs_track_fit` slope | **+0.779** | -0.948 |
| rms | 0.7295 Hz | 0.9315 Hz |
| answers equal to a static control's | 7 of 24 | 9 of 21 |
| refused | 0 | 3 |

Both fail the bar (slope within 0.10 of 1, rms < 0.0252 Hz). But Naples is
**substantially better than Granada and the difference is structured**, not noise:

- **The artefact is SEED-BOUND.** All seven 1.512 Hz answers are seed 7; seed 11
  returns it **not once**. Seed 7's own static control reports 1.512 and seed
  11's reports 1.210. So the "common-mode" line of item 76 is a property of a
  particular clutter realisation, not of the processing alone.
- **Source modal prominence predicts recovery.** Segment 02 (setup 1) scores
  5 of 12; segment 13 (setup 3) scores 2 of 12. Both are normalised to unit peak
  before injection, so the difference is spectral SHAPE -- how concentrated the
  record's energy is at its dominant -- not amplitude.
- **Recovery improves with target frequency**: 1, 0, 1, 1, 1, 3 of 4 at 0.30,
  0.40, 0.50, 0.60, 0.75, 0.90 Hz. The low targets sit nearer the artefact-rich
  bins.

## Kill criterion, applied

The pre-registered criterion was: *if H1 fails on Naples as it did on Granada,
the conclusion generalises -- this chain does not recover ambient structural
motion.* H1 fails on both, so it generalises **at the bar**. The nuance that must
be quoted with it: the failure is not total on Naples, and the decomposition
above says where the surviving signal is.

## What this run may NOT claim

Pre-registered and unchanged: **no localisation claim** (target at grid origin,
item 40) and **no detectability claim** (controls are motionless scenes, not
item 38's zero-amplitude twins). Recoverability only.
