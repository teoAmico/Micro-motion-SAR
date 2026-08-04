# Screening 315 synchronised measurements for motion above the floor

Item 60 measured Oroville Dam at 0.5–0.8 um against item 53's 5.5 um floor and
asked the obvious next question of the other 314: **did any instrument record
something this radar could have seen?**

`motion_screen.csv` is the result. Response removed, band-limited 0.03–3 Hz, RMS
displacement over each collect's own aperture, best channel per station.

## Answer: no, and the reason is instructive

```
  315  synchronised hits
  305  returned a waveform that survived response removal
  303  on an actual seismometer or accelerometer channel
   52  above the 5.5 um floor -- before any auditing
   14  above it once HV.UWB is excluded
    0  that survive the neighbour test
```

## Three layers of artefact, each caught by a different check

**1. Non-seismic channels.** The two largest readings in the whole screen were
`IU.RAR` `LWD` at 317 metres and `BK.ORV` `LCE` at 34 metres. `LWD` is WIND
DIRECTION and `LCE` is CLOCK ERROR. The SEED channel code's middle letter gives
the instrument: `H`/`L` seismometer, `N` accelerometer, `W` wind, `D` pressure,
`C` clock. Filtering on it removed both.

**2. Chronically high stations.** Per-station behaviour across all its collects:

```
  station        n    median        max  reading
  HV.UWB        39    38.23   1918.28  CHRONICALLY HIGH -- instrument
  HV.BYL         5     9.03     64.35  CHRONICALLY HIGH -- instrument
  HV.WRM        31     3.50    109.96  occasionally high
  HV.UWE        40     1.93      3.68  quiet
  HV.RIMD       40     1.73     14.98  occasionally high
  HV.KKO        40     1.42      3.07  quiet
  HV.OTLD       40     1.22      2.45  quiet
```

`HV.UWB` reads 1–1.9 mm on collect after collect for six weeks. Ambient ground
does not do that, and neither does an earthquake at every satellite overpass.

**3. The neighbour test, which is the decisive one.** Kilauea has six to eight
stations inside the same footprint, so every reading has controls on the same
ground at the same instant:

```
  CAPELLA_C10_SP_CPHD_HH_20240709202225  UWB 1918.28 um
     WRM 4.73   UWE 2.24   RIMD 1.88   BYL 1.52   KKO 1.49   SDH 1.18   OTLD 1.11
```

UWB reads **400x** its neighbours on the same ground at the same moment. That
settles it without needing to know what is wrong with the station.

## The one candidate that looked real

`CAPELLA_C10_SP_CPHD_HH_20240609091921`, 2024-06-09T09:19:21Z — WRM 44.50 um and
RIMD 14.98 um, with the rest elevated at 2.3–3.7 um against a 1.6 um median.
Spatially coherent, which is what a real source looks like.

It does not hold up:

- **No catalogued event.** USGS lists nothing in the aperture; the nearest is
  M1.8 at 09:25:53, six and a half minutes later and after the radar had gone.
- **WRM is chronically high** — median 3.50, and above 5.5 um on eight separate
  collects. Its 44.5 um is WRM being WRM.
- That leaves **RIMD at 14.98 um** against its own median of 1.73, with the
  others at about twice background. Consistent with weak volcanic tremor during
  Kilauea's June 2024 activity, and too weak and too unattributed to build a
  positive control on.

## What this establishes

**No collect in the open Capella archive has independently confirmed motion
clearly above this instrument's floor.** That is a stronger and better-founded
statement than `DATASETS.md` has carried until now, which was that no such
collect is *known*.

What would count, stated so the next search is cheap: an instrument reading
**above 5.5 um RMS in the 0.03–3 Hz band**, on a seismometer channel, at a
station that is NOT chronically high, corroborated by a neighbour or a catalogued
event, inside a spotlight or dwell collect. The screen above applies every one of
those tests and can be re-run against any new catalogue.

The most likely place to find one is a site **during a real earthquake**. That
inverts the search: start from the earthquake catalogue and look for a collect
over the epicentre within the aperture, rather than starting from collects.
