# What the tracked series actually contains on a motionless scene

## Question

Items 98-100 eliminated the frequency search, multiplicity and the offset-driven
carrier as explanations for item 96's 100% false-positive rate. Item 100 said the
untouched place to look was the tracked series itself. Every item from 91 to 100
argued about statistics computed FROM that series; none had looked at it.

## Command

```sh
./build/sim_cphd ts.cphd 0.5 0.0 --clutter 400 --clutter-vib --seed 7
./build/micromotion mmotion --cphd ts.cphd --estimator phase --n 128 --overlap 0 \
    --size 128 --cell 0.5 --win 32 --upsample 200 --coherence 0 --shifts shifts.csv
```

`--shifts` already existed; nothing needed building.

## Result

```
  phase, circular statistics:  R = 0.643,  circular sd = 0.940 rad
  apparent displacement:       2.32 mm rms, on a scene where NOTHING MOVES
  item 53's quoted floor:      0.0055 mm      ->  the noise is 421x it
  implied sub-look coherence:  ~0.70
  peak frequency per window:   0.302-3.024 Hz, median 1.663, no preferred value
  peak share of band power:    14.8% median (white noise would give 4.8%)
```

The artefact is the periodogram of a noise series. Nothing more.

**This fixture's real floor is 0.29 mm**: 2.32 mm per look, times sqrt(2/128),
which is 53x the 0.0055 mm this project quotes. That figure was measured on an
injected bright coherent point target, not on clutter.

A correction is recorded in item 101: I first read a 14.27 mm peak-to-peak
against a 7.75 mm ambiguity and called the phase saturated. Measured circularly
it is not -- R = 0.643 is well inside the circle. A wrapped quantity must be
summarised circularly.

Full write-up in `docs/FOLLOW-UPS.md` item 101.
