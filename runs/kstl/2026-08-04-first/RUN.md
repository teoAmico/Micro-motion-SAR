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

---

# The image does not focus, and the reason disqualifies the collect

`kstl.png` — 1024 x 1024 at 2 m from all 172997 pulses, 21 min 55 s — is not a
recognisable airport. It is speckle with faint smeared streaks.

**This is the first STRIPMAP collect this project has processed**, and the
backprojector integrates every pulse onto every grid cell, which is correct for
spotlight and wrong here. In stripmap the beam sweeps: a point is lit for
`lambda*R/(2*rho*V)` and no longer.

```
  2 m azimuth resolution at 680 km needs a 5335 m aperture = 0.71 s = 7238 pulses
  of the 172997 available, so 95.8% of pulses have the target OUTSIDE the beam
```

Those pulses contribute random phase. Signal grows as `N_sig` and noise as
`sqrt(N_noise)`, so integrating the whole 17 s makes the image worse than using
the right 4%.

## Spotlight or dwell is a hard requirement

The quantity this measurement needs is PER-TARGET observation time, which in
stripmap is not the collect duration:

```
mode                            collect  per-target   df = 1/T  usable
Capella KSTL   (stripmap)         17.0s       0.71s    1.406Hz  NO
Capella Giza   (spotlight)        32.9s      32.90s    0.030Hz  yes
ICEYE Houston  (dwell-precise)    15.3s      15.30s    0.065Hz  yes
Umbra Panama   (spotlight)         2.0s       1.99s    0.503Hz  NO
```

At KSTL the frequency resolution is **1.4 Hz**, coarser than the whole 0.3-3 Hz
band this project targets, and 0.71 s split into 128 sub-looks gives 5.5 ms each
with no useful azimuth resolution. **No amount of processing fixes that**: the
target was not observed for long enough.

So KSTL was never a candidate for the vibration chain, aircraft or not. It is a
moving-target scene and nothing else. The same table disqualifies Umbra's 2 s
spotlight, which item 36 noted as coarse and did not quantify.

**A collect is usable here only if one point stays in the beam for seconds.**
That is spotlight or a dwell mode, and it should be the first thing checked
about any candidate — before the footprint, before the sensors.

## A flag ordering defect found on the way

`focus --pulse-start 96000 --max-pulses 16000` fails with "pulse window
[96000, 112000) outside available 16000 pulses". `--max-pulses` truncates the
READ to the first 16000 pulses and `--pulse-start` then indexes into that
truncated buffer, so the two cannot select a window late in a collect —
which is exactly what a stripmap sub-aperture needs. `USER_GUIDE` describes
`--pulse-start` as "first pulse to read", which is not what it does.

Not fixed, because the mode is disqualified above and the fix should be made
when something needs it.
