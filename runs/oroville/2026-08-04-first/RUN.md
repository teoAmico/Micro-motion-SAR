# Oroville Dam: the first scene where an instrument says what the ground did

Item 59 found `BK.ORV` — a seismic station on Oroville Dam — inside a 30.4 s
Capella spotlight footprint with 168 KB of waveform spanning the aperture. The
strongest candidate in the archive. This measures the waveform BEFORE spending
32 GB on the CPHD.

```
  scene    CAPELLA_C10_SP_CPHD_HH_20240803004650_20240803004720   32.13 GB
  aperture 2024-08-03T00:46:51Z .. 00:47:21Z   (30.4 s)
  station  BK.ORV  39.554508,-121.500359  [NCEDC]  Oroville Dam, CA
  channels HH? 100 Hz velocity, HN? 100 Hz acceleration, both 3-component
```

## Ground displacement during the aperture

Instrument response removed, band-limited to 0.03–3 Hz, which is what this
project targets:

```
   channel          rms   peak-to-peak
       HHE    0.7024 um      2.1772 um
       HHN    0.3349 um      1.1421 um
       HHZ    0.7837 um      2.4332 um
       HNE    0.0132 um      0.0653 um
       HNN    0.0105 um      0.0588 um
       HNZ    0.0117 um      0.0623 um
```

Cross-checked on HHZ by a second route — stage-zero sensitivity plus
frequency-domain integration rather than full response removal — giving
0.5050 um against 0.7837 um. Two different treatments of the response agree to
36%, which settles the order of magnitude and is what the conclusion rests on.

**This project's floor is 5.5 um RMS (item 53). The dam moved 7 to 11 times
below it.**

## So it is not a positive control. It is better than nothing else here.

The SAR could not have seen this. Oroville does not test detection.

What it does give is something this project has never had. `USER_GUIDE`
section 7 item 0 says a null on real data means nothing, because "nothing moved"
and "this chain cannot see motion in this data" produce identical output. **Here
an independent instrument separates them.**

```
  pipeline reports NO frequency   ->  a null that is CORRECT, and provably so
  pipeline reports A frequency    ->  a FALSE POSITIVE, and provably so
```

Items 25 and 55 measured static aspect-dependent FIXTURES returning confident
in-band frequencies, and item 55 concluded no policy here is fit for that scene
type. This is the same test on real data, on a real structure, with independent
proof that the structure was static.

The CPHD download is running. The waveform is the part that decides what the
result will mean, and it is done first for that reason.

## Two service notes worth keeping

**`service.iris.edu/irisws/timeseries` does not hold BK data.** It returned 404
for every `BK.ORV` request including uncorrected ones, while the documented
`IU.ANMO` example returns 29000 bytes through the identical syntax — so the 404
is data holdings, not a malformed request. BK is Berkeley, archived at NCEDC;
`service.ncedc.org/fdsnws/dataselect` serves it. The control is what
distinguishes those two cases and it is worth running every time.

**The location code is `00`, not empty.** `loc=--` returns 404 here. The station
service reports the code and it should be read rather than assumed.
