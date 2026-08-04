# Run: 2026-08-03 giza / amplitude-sweep-fmin

**Question:** the unfloored sweep beside this one collapsed to bin 1 below 2 mm
(item 37). With the lowest bins excluded, how far down does the injected
frequency actually survive?

**Answer: to 0.0625 mm, the smallest amplitude tried.** A 32-fold improvement on
the 1-2 mm the unfloored sweep bottomed out at.

## Design

Identical to the sweep beside it and to item 35, plus `--fmin 0.098`. That is
`ceil(0.098 / 0.0326) = 4` bins, one more than the default
`RS_SPECTRUM_LEAKAGE_BINS` of 3 this run motivated — chosen before the constant
existed, so it excludes bins 1-3 where the shipped default excludes 1-2.

## Result

```
amp mm | 2.0  | 1.0  | 0.5  | 0.25 | 0.125 | 0.0625
freq   | .163 | .163 | .163 | .163 | .163  | .163     <- all correct
prom   | 38.5 | 40.7 | 41.6 | 42.9 | 45.3  | 48.9
v p-p  |  7.5 |  2.5 |  1.5 |  1.1 |  0.9  |  0.8  mm/s
window | 17   |  8   |  8   |  8   |  8    |  8
```

## Three things this does not let anyone claim

**Prominence rises as the signal weakens**, which no real detection does. Until
that is explained this is not a sensitivity curve. Two candidates, neither
measured: 2 mm is a 0.78 rad phase swing, near item 14's lambda/4 wrapping
limit, so the strongest injection may be the most degraded; and a weaker target
leaves a quieter spectrum for a fixed peak to stand out against. The 2 mm point
also selects window 17 rather than the injected 8 — right frequency, wrong
window, consistent with the wrapping story but not evidence for it.

**Frequency and amplitude have different floors.** Peak-to-peak velocity flattens
at 0.8-1.1 mm/s while the true amplitude falls sixteen-fold across the same
points, so below about 1 mm the reported velocity is tracking noise. It is taken
raw before detrending, by design, but it is printed beside a frequency that IS
recovered with nothing marking it floor-limited.

**Nothing here is adjudicated by these numbers alone.** Removing bins moves the
null as well as the measurement, so item 35's threshold of 23.8 does not carry
over. The 19 trials at the foot of `run.log` are the recalibration, below.

## Cost

Six sweep points at ~5 min each; the 19 null trials dominate the run. Trials ran
at ~30 min each rather than item 33's 3.5 min, because item 34 raised the null's
scatterer density 33-fold — a cost item 34 did not record and this run pins.


## The recalibrated null, and the verdict

```
19 trials, bins 1-3 excluded    min 11.5  mean 16.3  worst 22.9
item 35's null, bins 1-2 in     min 12.0  mean 17.0  worst 23.8
```

The floor moves the null by -0.9 at the worst and -0.7 at the mean, so it is the
measurement that moved, not the threshold.

```
 amp mm |  prom | x worst null | verdict
    2.0 |  38.5 |        1.68x | p = 0.0500  ADJUDICATED
    1.0 |  40.7 |        1.78x | p = 0.0500  ADJUDICATED
    0.5 |  41.6 |        1.82x | p = 0.0500  ADJUDICATED
   0.25 |  42.9 |        1.87x | p = 0.0500  ADJUDICATED
  0.125 |  45.3 |        1.98x | p = 0.0500  ADJUDICATED
 0.0625 |  48.9 |        2.14x | p = 0.0500  ADJUDICATED
```

**The sweep never found a floor.** Every amplitude adjudicates, and 0.0500 is
the smallest p 19 trials can express, so all six are pinned at the bound rather
than approaching it. This does not bracket the sensitivity; it says the bracket
is somewhere below 0.0625 mm and that this design cannot see it.

## The real uninjected control, under the same floor

The measurement that matters most, and it was not in the original sweep:

```
strongest peak in window 6: 0.130 Hz, prominence 17.9, quality 0.445,
                            peak-to-peak velocity 124.8 mm/s
backed by 6 windows, largest touching block 5
```

**0.130 Hz is bin 4 — the first admissible bin.** The trend did not go away when
its bins were removed; it moved to the edge of the exclusion and reported from
there, which is what the regression test predicted and what a band floor can
never prevent.

**It does not false-positive: 17.9 against the null's worst of 22.9, so it is
refused.** But the margin is 1.28x, and a trend-only window in
`test_tracking.c`'s fixture reaches prominence 28.09 -- above this null. The
floor is necessary and is not sufficient. What refuses this scene is the null,
not the floor, and the null has little room.

The 124.8 mm/s peak-to-peak is the clearest evidence yet that the velocity
readout is uncalibrated: it is a motionless desert.
