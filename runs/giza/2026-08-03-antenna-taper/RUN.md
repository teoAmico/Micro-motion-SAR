# Run: 2026-08-03 giza / antenna-taper

**Question this run is meant to answer:** item 21 listed "an aperture amplitude
taper the synthetic scenes do not have" as an unexcluded candidate for the
real-vs-synthetic `D_A` gap — real collects floor at 0.38–0.52 where the
fixtures reach 0.079. Does the antenna gain pattern, varying across the dwell,
produce enough per-sub-look amplitude variation to account for it?

**Answer: no, by four orders of magnitude.** Recorded as item 23e.

## Provenance

- commit: `0c717d9`
- collect: `CAPELLA_C13_SP_CPHD_HH_20241004001939_20241004002012.cphd`
- reads the PVP block only (70 MB at offset 12352); the 39 GB signal block is
  never touched, so this runs in seconds
- `antenna_taper.py` beside this file is the whole measurement

## Why the data was there and unread

Capella's own `CPHD_by_Example.ipynb` spends four cells on `Antenna/AntPattern`.
The Giza XML carries it populated — `GainZero` 49.58 dBi, a 2×2 `Array/GainPoly`
in direction cosines, `AntCoordFrame` giving the antenna axes as 5th-order
polynomials in time — and nothing in this project reads any of it.

## Command

```sh
C=CAPELLA_C13_SP_CPHD_HH_20241004001939_20241004002012
python3 antenna_taper.py "$C.cphd" "$C.xml"
```

## Two checks that say the numbers are being read right

- **335141 of 335149 vectors** pass the validity screen, which is the count
  `rs_read_cphd()` reports on the same file. The PVP parse agrees with the
  project's own.
- The gain polynomial implies a 3 dB beamwidth of **0.676°** azimuth and 0.680°
  elevation. Capella's notebook states the nominal beamwidth as 0.7°.

## Result

```
off-boresight at the SRP   |dcx| max 0.007 deg, |dcy| max 0.002 deg
one-way gain across the whole 32.87 s dwell        span 0.001 dB

  offset from SRP    gain span across dwell    D_A from the taper alone
        0 m               0.001068 dB               0.000029
      128 m               0.002190 dB               0.000049
      500 m               0.008743 dB               0.000064
     2500 m               0.160751 dB               0.001024
```

Against an observed real-scene floor of 0.38. At the edge of the 256 m patches
the Giza runs used, the taper contributes 0.00005.

**The mechanism is that the spacecraft tracks the target almost exactly.** The
antenna frame rotates 18.04° across the dwell at 0.549°/s while the target stays
within 0.007° of boresight, so a 0.676° beam never shows the scene anything but
its flat top.

## What this does not cover

The **elevation** pattern across the swath is a static per-pixel gain, not a
per-look one, so it cannot move `D_A` whatever its size — `D_A` is a variation
across sub-looks at a fixed pixel.

And this is **one collect**, well tracked. A squinted or poorly-pointed
acquisition would not behave this way, which is an argument for reading the
pattern rather than assuming it is always negligible.
