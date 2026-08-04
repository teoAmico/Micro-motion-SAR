# Capella spotlight catalogue joined to FDSN: 315 synchronised measurements

Item 58 made mode the first filter. This applies it to the whole catalogue and
then does the join item 57 built the tool for.

**Result: 315 station-collect pairs where an instrument was recording a waveform
inside the footprint during the aperture.** `measurement_hits.csv` is the list.
This project has never had one.

## The funnel

```
  1174  CPHD in the Capella open archive
   939  spotlight (707 SP) or sliding spotlight (232 SS)   <- item 58's filter
   939  headers harvested by range request (256 KB each)
        dwell: median 22.5 s, 646 at >= 15 s, 156 at >= 30 s

 76741  unique FDSN network.station worldwide, 22 data centres
 70490  after dropping network SY, which is SYNTHETIC and not an instrument

  3877  polygon hits over 257 distinct collects
   553  of those at dwell >= 25 s
   315  WITH A REAL WAVEFORM across the aperture
```

## The two that matter most: an instrument ON a structure

```
  BK.ORV   167,936 B   dwell 30.4 s   Oroville Dam, Oroville, CA
           CAPELLA_C10_SP_CPHD_HH_20240803004650_20240803004720   32.13 GB

  NN.CC12   23,552 B   dwell 25.7 s   Clark County Firehouse 12, ANSS strong motion
           CAPELLA_C09_SP_CPHD_HH_20231119132416_20231119132442   22.19 GB
```

**Oroville Dam is the strongest candidate in the archive.** A large concrete
structure, an instrument on it, a 30.4 s spotlight collect over it, and 168 KB of
waveform spanning the aperture. Everything this project has needed and not had.

## The repeat coverage, which is worth as much

```
  HV.KKO, HV.OTLD, HV.RIMD, HV.UWE, HV.WRM     40 collects each
  HV.SDH, HV.UWB                               39
  HV.PAUD                                      10
```

Kilauea caldera is covered by **forty separate spotlight collects** with six or
more recording stations each. That is a repeatability experiment rather than a
single shot: the same instruments, the same ground, forty independent
acquisitions. Mount Etna appears with four INGV stations in one collect
(IV.ECNE, EBCN, EPLC, ECPN).

## What this does NOT establish

**A waveform existing is not a waveform showing anything.** Ambient ground
motion is of order 0.1 um at 1 Hz, against the 0.0055 mm = 5.5 um RMS
instrument floor of item 53 -- fifty times below it. A seismic station in a vault
on quiet ground will show nothing this radar could have seen, and the comparison
would be a null on both sides.

What could clear the floor: a dam or a building responding to wind, traffic or
machinery, where displacement reaches tens of microns; or any of these sites
during an actual earthquake, which the timestamps can be checked against.

**The instrument and the radar do not measure the same thing.** A seismometer
records ground velocity at one point; this pipeline records line-of-sight
displacement of scatterers across a window. Co-located is not co-measured, and
the comparison needs the station's own structure to be a scatterer the radar
resolves.

**Nothing has been downloaded or run.** The two structural candidates are 32 GB
and 22 GB. This is a search result, not a measurement.

## Reproducing

`harvest.py` pulls footprints and dwells from CPHD headers by range request;
`tools/footprint_sensor_join.py` does the classification. The station list came
from the FDSN federator at `service.iris.edu/irisws/fedcatalog`, and waveform
existence from per-data-centre `dataselect` endpoints.

**`service.iris.edu/fdsnws/availability` returns HTML, not data.** A first pass
used it and reported 0 of 553 hits with data, which looked like a clean negative
result and was a dead endpoint. `service.earthscope.org/fdsnws/dataselect`
answers correctly. Any future run should assert a known-good control before
believing a zero.
