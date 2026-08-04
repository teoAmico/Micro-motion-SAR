# Run: 2026-08-04 synthetic / real-waveform

**Question:** every injection this project has run was a pure sinusoid. Does the
reporting stage still work when the motion is a real structure's -- multi-modal,
non-stationary, no dominant line?

Commit: see `git log` for `6d42f29..`; host: local. Recorded as `FOLLOW-UPS.md` item 69.

## The waveform

Oroville Dam, `BK.ORV.00.HNZ` (Kinemetrics EpiSensor ES-T, 100 Hz), the M5.5
Lake Almanor earthquake of 2023-05-11T23:19:41Z, fetched from NCEDC via FDSN,
instrument response removed to DISPLACEMENT with `pre_filt=(0.05,0.1,40,45)`,
then the highest-energy 20 s slice taken (`fetch.py`, `slice.py`).

```
almanor_hnz_20s   peak 155.67 um   rms 38.61 um
  lines: 1.200Hz(1.00) 1.150Hz(0.80) 1.250Hz(0.78) 0.750Hz(0.77) 1.400Hz(0.54) 1.100Hz(0.48)
```

**Six lines within a factor of two, and no dominant mode.** A sine has one line
at 1.00 and nothing else. That is the entire difference under test.

Oroville was chosen over the Zenodo SHM packages of `GROUND_TRUTH_DATASETS.md`
because it is free, public, already this project's nominated static negative
control (item 60), and a real structure -- and item 68 showed the Zenodo sites
have no collect over them either, so the multi-gigabyte download buys shape and
nothing else. Shape is all this experiment needs.

## Commands

```sh
./build/sim_cphd s05.cphd 0.5 0.02 --clutter 400 --clutter-vib --seed 7
./build/sim_cphd w05.cphd 0.5 0.02 --clutter 400 --clutter-vib --seed 7 \
    --wave "almanor_hnz_20s.txt,41.6667"

./build/micromotion mmotion --cphd s05.cphd --n 128 --overlap 0 --size 128 \
    --cell 0.5 --win 32 --upsample 200 --coherence 0 --out r_s05
./build/micromotion mmotion --cphd w05.cphd --n 128 --overlap 0 --size 128 \
    --cell 0.5 --win 32 --upsample 200 --coherence 0 --out r_w05
```

Identical scene, identical seed, identical amplitude (0.02 m peak, which is what
`--wave` normalises the record to), identical processing. The ONLY difference is
the shape of the displacement in time.

`41.6667 Hz` replays the 100 Hz record at 1/2.4 speed, dividing every frequency
by 2.4: the mode cluster moves from 1.10-1.40 Hz to 0.46-0.58 Hz. That is
necessary, not cosmetic -- a 1.20 Hz SINE does not recover in this fixture
either (reported 2.520 Hz), because of the sub-aperture response ceiling of item
13. Comparing a recoverable sine against an unrecoverable waveform would have
measured the ceiling, not the waveform. Time-scaling preserves every ratio
between the modes and the whole envelope.

## Result

| | injected | reported | consensus |
|---|---|---|---|
| sine | 0.500 Hz | **0.504 Hz** correct | 0.504 Hz, 4/49 |
| real record | six modes, 0.31-0.58 Hz | **1.966 Hz**, no mode within 1.3 Hz | 0.605 Hz, 4/49 |

Per-window, scoring a window as a hit if its dominant frequency is within one
bin of ANY of the record's six modes:

```
  sine          10 of 49 windows (20%)
  real record    5 of 49 windows (10%)
```

**The window-level hit rate halves and the reported answer becomes wrong.** The
comparison is generous to the waveform by construction: the sine is scored
against a target six bins wide when it only has one line in it. Even with six
times the target, the real record scores half.

The sine result reproduces the 2026-08-01 e2e run to three decimals, so the
fixture and the operating point are unchanged; what moved is the motion.

## What this does and does not say

It does NOT say the tracker cannot see the record. Consensus lands at 0.605 Hz
against a true mode at 0.583 -- within half a bin of the fifth-strongest line --
so some of the structure survives into the per-window answers. It is
`rs_spectrum_best_window()`, prominence, that picks 1.966 Hz over all six.

That is items 7-9 again, and worse than items 7-9: there the selection policy
discarded a carrier the tracker had recovered in most windows. Here the
recovered energy is SPLIT across six modes, so no single one is ever prominent
enough to win, and the prominence statistic goes to whatever noise line is
tallest. **A statistic that reports one frequency is the wrong shape of answer
for a structure**, and no threshold on it fixes that.

The honest reading of every earlier synthetic recovery in this project is now
that it was measured on the easiest possible motion.
