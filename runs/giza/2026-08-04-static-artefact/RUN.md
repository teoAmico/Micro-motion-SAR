# Run: 2026-08-04 giza / static-artefact

**Question:** item 50's sensitivity floor at 0.125 mm is not set by noise. It is
set by the zero-amplitude twin: a bright STATIC scatterer produces 12,060x at
the band floor, and below 0.125 mm the real signal loses to it. What is it, and
does it scale with brightness?

**Answer: it scales as the square of brightness, saturates above ~50x, and it is
a QUADRATIC PHASE RESIDUAL that the carrier removal does not remove.** Recorded
as item 51.

## How it scales

Zero injected motion, `--overlap 0.5`, brightness swept:

```
  REL     local peak at the band floor
    1                            37.9      <- desert control is 15.1-34.4
    5                           963.7
   20                        12,060.1
   50                        38,991.6
  100                        44,697.8
```

Roughly `REL^2` up to 20 and saturating between 50 and 100 (2x the brightness
buys 1.15x). Square-law is what a fixed FRACTION of the scatterer's own energy
leaking into one bin looks like: the artefact's power tracks the scatterer's
power while the clutter background around it does not.

**At REL = 1 -- a scatterer typical of the clutter it sits in -- the artefact is
37.9, barely outside the 15.1-34.4 that empty desert produces.** This is a
problem of BRIGHT scatterers specifically.

### The consequence that matters

Signal power and artefact power both scale as `REL^2`, so **their ratio does
not depend on brightness**. A brighter target does not make smaller motion
detectable. Item 50's 0.125 mm floor is a real limit on amplitude and cannot be
bought down by choosing a brighter target.

What the ratio DOES depend on is the size of the residual phase, which is where
the fix has to come from.

## What it is

The estimator removes the geometric carrier by finding the LINEAR ramp that
maximises the de-ramped phasor sum. The series that survives is not white:

```
  window 8, static bright scatterer, zero injected motion
  series rms 1.0653 mm, peak-to-peak 4.0666 mm, phase rms 0.4153 rad

  after fitting and removing:   residual rms      bin-3 local ratio
    linear (what ships)           1.0347 mm             21,601.9
    quadratic                     0.0902 mm                 10.4
    cubic                         0.0054 mm                  2.9
```

**A quadratic removes it.** Residual rms falls 11x and the artefact falls 2,000x.
Across all 25 windows the worst case goes from 21,602 to 10.4.

That is the expected shape: a scatterer's range history is quadratic in time, so
its phase is quadratic, and a de-ramp that fits only `exp(-i*v*k)` leaves the
curvature behind. The phase rms of 0.4153 rad is well inside `(-pi, pi]`, so
nothing is wrapping -- this is not item 11's sawtooth, it is the term below it
that item 14's carrier removal was never designed to reach.

**I had this backwards on first reading** and wrote that no polynomial removes
it, having looked at the residual rms and not at the bin the artefact lives in.
The rms falls by 11x where the artefact falls by 2,000x, because the artefact is
a small part of the total spread and almost all of the bin-3 power.

## The fix, measured but NOT implemented

Two places it could go, and they are not equivalent:

1. **Extend the carrier search to a quadratic term** -- maximise
   `|sum_k z[k] exp(-i(v*k + w*k^2))|` over both. This removes GEOMETRY, which
   is what the residual is, and leaves the signal alone. It costs a second
   search dimension; the existing coarse pass is already O(N^2) per window, so
   it would need an alternating or staged search rather than a 2-D grid.

2. **Raise the detrend order on the displacement series.** Much smaller change,
   and `rs_detrend_t` already exists. But `microm.h` argues against exactly this:
   under a resonance reading the lowest frequencies are the DEEPEST structure, so
   a quadratic detrend attenuates the part of the record that model calls real.
   A 0.163 Hz tone is five cycles and safe; a genuine signal near the band floor
   is not.

Option 1 is the right one and is the larger change. Neither is done here.
