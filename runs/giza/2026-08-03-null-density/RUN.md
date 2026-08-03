# Run: 2026-08-03 giza / null-density

**Question:** item 33 measured the simulated static null scoring twice the real
Giza desert's prominence, refusing every known true positive. What scatterer
density makes it resemble the scene it stands in for?

**Answer: 0.25 per sub-look resolution cell, and at that density it inverts item
33's verdict.** Recorded as item 34.

## Provenance

- commit: `c6c1ed0` plus the `--null-scatterers` option added for this
- collect: `CAPELLA_C13_SP_CPHD_HH_20241004001939_20241004002012.cphd`
- `go.sh` is the sweep, `verify.sh` the confirmation at the derived default
- 96 m grid, so a trial is minutes; item 30's runs were 256 m

## The sweep

```
  scatterers   per cell   null mean prominence
         400     0.0076          34.6      <- the historical default
        4000     0.0764          25.7
       20000     0.3819          13.3
                                 16.6      <- the real motionless scene
```

Log-interpolating the crossing gives ~13,000 scatterers = 0.25 per cell.

## Verification at the derived default

13,091 scatterers, 0.250 per cell, five trials:

```
  17.1  13.2  23.8  20.7  13.9     mean 17.7, sd 4.0
  detection 16.6 is 0.94x the mean and 0.70x the worst
  3 of 5 reached it -- empirical p = 0.6667 -> NO FREQUENCY REPORTED
```

The null now sits **on** the real scene (1.07x) rather than a factor of two above
it (1.95x), and correctly refuses a scene with nothing in it.

## What it does to item 30's known true positives

```
  injected   before (0.008/cell)          after (0.25/cell)
    28.71    6/8 reach it p=0.78 REFUSED    0/5  p=0.17  adjudicated
    31.25    5/8          p=0.67 REFUSED    0/5  p=0.17  adjudicated
    31.37    5/8          p=0.67 REFUSED    0/5  p=0.17  adjudicated
    29.88    6/8          p=0.78 REFUSED    0/5  p=0.17  adjudicated
    25.60    6/8          p=0.78 REFUSED    0/5  p=0.17  adjudicated
```

All five refused before, all five adjudicated after, motionless still refused.

**The grids differ** — this null is measured at 96 m and those runs were at
256 m. The real scene's own value is stable across the two (16.6 and 17.48) but
the null's grid dependence is untested, so that table is indicative.

## Cost

Sweep: ~45 min for three densities x three trials at a 96 m grid. Verification:
~28 min for five trials. The deposit is O(pulses x scatterers), so 13,091
scatterers is 33x the old default's deposit — a few minutes per trial here, and
about eight per trial at the 256 m grid where backprojection already costs ~35.
