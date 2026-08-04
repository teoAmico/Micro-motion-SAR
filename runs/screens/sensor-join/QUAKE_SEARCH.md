# Inverting the search: earthquakes first

Item 61 found no synchronised instrument recording motion above the 5.5 um floor
and proposed starting from the earthquake catalogue instead — there are 939
spotlight collects and rather more earthquakes.

**It does not work either, and now there is a number for why.**

## The search

11,977 events of M >= 5.0 worldwide, 2020–2026, against the 939 spotlight and
sliding-spotlight collects. For each pair, P (8 km/s), S (4.5 km/s) and Rayleigh
(3.5 km/s) arrival times at the footprint centre, asking whether any phase
arrives inside the aperture.

```
  7 collect-earthquake pairs with a phase arriving during the aperture

  M5.8 at 16440 km  P          2024-09-07T22:39:08   dwell 33.5s
  M5.4 at  8281 km  P          2023-08-31T10:29:28   dwell 27.0s
  M5.4 at 10757 km  S          2024-07-28T22:13:08   dwell 32.3s
  M5.4 at 12635 km  S          2024-08-29T20:19:26   dwell 19.3s
  M5.1 at 17974 km  P          2024-05-15T01:41:04   dwell 29.1s
  M5.0 at 14971 km  P          2025-02-26T08:56:51   dwell 13.7s
  M5.0 at 17105 km  P          2022-03-02T11:10:02   dwell 24.8s
```

**All seven are teleseismic, 8,000 to 18,000 km, M5.0–5.8.** P-wave displacement
at that range is well under 1 um — an order of magnitude below the floor. None is
usable. Not one Rayleigh arrival coincided, which is the phase that would have
carried real displacement.

## Why it cannot be fixed by looking harder

```
  total aperture      20,013 s across 2,036 days
  duty cycle          1.14e-04  -- the radar is looking 114 millionths of the time
  M>=5.0 within 200 km of a collect site, any time:   1,242
  expected number coinciding with an aperture:        0.14
```

**The expected count is 0.14.** Finding zero usable ones is the outcome the
arithmetic predicts. Reaching an expectation of one needs about seven times this
archive — roughly 39 hours of spotlight over the same sites.

And 0.14 is generous: it counts M>=5 out to 200 km, where ground motion is
already below 5.5 um. The requirement is nearer M>=5 inside ~100 km, M>=7 inside
~1000 km, or M>=8 teleseismic, so the true expectation is lower still.

**Earthquakes of opportunity are not a route to a positive control here.** The
duty cycle is the whole problem: a satellite that stares for thirty seconds at a
time cannot be expected to be looking when the ground moves.

## What that leaves

Not a search problem. Three routes, none of which is more catalogue-mining:

1. **A structure that moves all the time.** Bridges under traffic, tall buildings
   in wind and wind turbines reach tens of microns continuously — no coincidence
   required. Their instruments are in structural-health-monitoring archives
   (NTNU's open data, Zenodo) rather than FDSN, and the join then needs a SAR
   collect over that specific structure during its monitoring period.
2. **A tasked collect**, which is a commercial arrangement rather than open data.
3. **A lower floor.** Items 51–53 moved it from 0.125 mm to 0.0055 mm by removing
   carrier residual. Ambient ground motion at the quiet sites screened here runs
   1–2 um, so roughly another factor of four would bring ordinary ground into
   range — and would make every one of the 315 hits a candidate rather than none
   of them.

Route 3 is the only one this project can pursue on its own, and item 53 recorded
the returns falling fast (38x then 4.5x from successive carrier terms).
