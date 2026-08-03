# Run: 2026-08-03 giza / null-distribution

**Question this run is meant to answer:** item 31 made `--null-static` the
verdict. A killed run left one datum suggesting the simulated null out-scores the
real desert. Does the simulated static null's prominence distribution match the
real motionless scene's, at a cost small enough to find out?

**Answer: no, by a factor of two, and the gate therefore cannot adjudicate Giza.**
Recorded as item 33.

## Design

No injection. The real scene's own prominence *is* the real null; the
`--null-static` trials are the simulated one. Same read, same chain, same
settings — only the scene differs. A 96 m grid rather than item 30's 256 m, so a
trial is minutes instead of ~35, which is what made eight trials affordable.

## Provenance

- commit: `7629d89` plus the argmax estimator of item 32 (which this run does not
  use — `--estimator phase`, as item 30)
- collect: `CAPELLA_C13_SP_CPHD_HH_20241004001939_20241004002012.cphd`
- `go.sh` beside this file is the exact command; run under `stdbuf -o0 -e0`, after
  the previous attempt lost two hours of stdout to block buffering

## Result

```
  static trial 1/8: prominence 31.4      5/8: 36.0
              2/8: 33.5                  6/8: 31.1
              3/8: 38.7                  7/8: 38.8
              4/8: 24.0                  8/8: 25.1
  mean 32.3, sd 5.3, worst 38.8
  detection 16.6 is 0.51x the mean and 0.43x the worst
  8 of 8 reached it -- empirical p = 1.0000
NO FREQUENCY REPORTED
```

The real scene scores **16.6**; every simulated motionless scene beats it. The
same figure on the 256 m grid is 17.48 (item 31), so the real value is stable
across grid size and it is the null that is displaced.

## What it would do to the known true positives

Item 30's five injected runs measured 25.60–31.37 — **inside** this null's range,
while the real motionless scene sits **below** it:

```
  injected run   nulls reaching it   empirical p
     0.098            6 of 8            0.78
     0.130            5 of 8            0.67
     0.163            5 of 8            0.67
     0.196            6 of 8            0.78
     0.228            6 of 8            0.78
```

All five refused. The gate is not separating signal from nothing; it is
separating the simulator from reality.

## Cost, for budgeting

~32 minutes for the whole thing — one read plus eight trials at a 96 m grid,
against the ~3.5–4 hours the same eight would cost at 256 m. Cutting the grid was
what made the question answerable.

## What this does not show

Not that gating on a null is wrong — item 11's argument for it is untouched, and
no scene-wide fraction would do better. What fails is this particular null on
this collect.

Not the mechanism, either. Items 21 and 23d make sparse isotropic scatterers the
obvious suspect, but this run compares two scalars, not two distributions. See
item 33 for the measurement that would settle it.
