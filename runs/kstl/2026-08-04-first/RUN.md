# Run: 2026-08-04 KSTL — the first collect with independently confirmed motion

`docs/KSTL_ADSB_POSITIVE_TEST.md` paired a Capella stripmap collect with
ADSB.lol and found two aircraft inside the footprint during the aperture. The
CPHD was not downloaded. It is now.

## The collect reads

```
CAPELLA_C11_SM_CPHD_HH_20250319191515_20250319191532   6,773,947,240 bytes
  172997 pulses of 173034 (37 flagged invalid), 9721 range bins
  9.55 GHz, lambda 0.0314 m, PRF 10174 Hz, dwell 17.003 s, near range 679.7 km
  footprint 10 x 100 km; KSTL, both aircraft and the scene reference all inside
```

The whole product needs 13.5 GB against this machine's 25.8, so `--rbins` is
required as it is for ICEYE.

**The footprint check needed the corners ordered.** The five `<Lat>/<Lon>` pairs
in the header are the scene reference followed by four corners in an order that
is not a ring; taken as given, point-in-polygon says KSTL is OUTSIDE its own
scene. Sorted by angle about the centroid it is inside, and so is everything
else. That is a trap for anything reading these footprints.

## What ADS-B actually gives, which is not what this project needs

The doc's own caveat is right and worth sharpening. Both aircraft are
**airborne and translating**:

```
  N510CN   1394 m altitude,  69.8 m/s,  moved  975 m across the in-aperture reports
  N707VM    914 m altitude, 126.5 m/s,  moved 1386 m
```

The focusing grid sits at height 0, so a target 1.4 km above it is mislocated
and defocused before anything else happens. And a moving target is displaced in
azimuth by `R*v_r/V`, which at 680 km range and 7500 m/s platform speed is up to
**6.3 km** for a fully radial 70 m/s. These are moving-target signatures --
smeared, displaced streaks -- which is `ccd.c`'s question, not the vibration
chain's.

**ADS-B is a PROXY.** It establishes that something was present and moving. It
supplies no displacement waveform, so it cannot validate a frequency or an
amplitude, which is what every open item here needs.

## Ground aircraft: none, and the tool said so for the wrong reason

A parked aircraft would be a far better target -- at ground level, not
translating, bright, and vibrating if its APU is running. Scanned the whole
day's archive against this footprint:

```
  200 reports inside the footprint box +/- 120 s
   16 in aperture
    0 ON GROUND, at any time
```

**`adsblol_cphd_crossmatch.py` reported `on_ground` from `point[6]`, which is
the flags word.** readsb puts the STRING `"ground"` in the ALTITUDE field for a
surface aircraft. The column therefore read 0 for everything, which looks like a
scan finding no ground traffic rather than like a bug. Fixed. The corrected scan
still finds zero, so the conclusion stands and the reason for believing it is
now sound.

## The sensor join, and why the polygon matters

`tools/footprint_sensor_join.py` implements the classification: MEASUREMENT
(an instrument recording a time series inside the footprint during the
aperture), OBJECT (a structure known from a map), PROXY (ADS-B, AIS, METAR).

```
scene                       box  in  rec  verdict
GIZA_C13_SP_20241004          0   0    0  nothing in the box
ICEYE_X47_HOUSTON             0   0    0  nothing in the box
KSTL_C11_SM_20250319         10   0    0  nearby but outside
```

**Eleven FDSN stations fall in the KSTL bounding box and NONE in the 10 km
strip** -- including three building-mounted strong-motion instruments, one of
them `NP.2490` at One Bell Center, a downtown tower. Comparing scene centres, or
boxes, would have claimed a match that does not exist. That is the single
strongest argument for doing the intersection properly.

## What would change the answer

A MEASUREMENT-class hit needs a collect whose footprint contains an instrument
with data across the aperture. Three CPHDs is not a search. The catalogue has
thousands, and the tool is written to be pointed at them.

Two cautions for when it is:

- **Strong-motion instruments in buildings are usually TRIGGERED**, so they are
  open for decades and hold data for minutes of it. Station metadata says
  "operating"; only the availability service says "recorded".
- **Ambient ground motion is far below this instrument's floor.** A seismometer
  in a vault sees roughly 0.1 um of displacement at 1 Hz, against the 5.5 um RMS
  floor of item 53. A building's response to wind or traffic reaches tens of
  microns and is the plausible target; bare ground is not.
