# Run: 2026-08-03 giza / alpha-verdict

**Question:** with the null generator matched to real clutter (item 34) and the
gate reading a conformal p-value, does a known injected signal on a real collect
clear a properly-sized null control at alpha = 0.05?

**Answer: yes. ADJUDICATED at p = 0.0500.** The first real-data measurement this
project has produced that a null control of adequate size could not reproduce.

## Provenance

- commit: `99896bc` plus the mmotion `--pulse-start` exposure
- collect: `CAPELLA_C13_SP_CPHD_HH_20241004001939_20241004002012.cphd`
- 19 null trials, which is the MINIMUM for alpha = 0.05 since `p_min = 1/(M+1)`
- 96 m grid, so a trial is minutes; item 30's runs were 256 m
- `go.sh` beside this file, run under `stdbuf -o0 -e0`

## Result

```
injection      0.163 Hz, 2.0 mm, 20x the median non-zero sample magnitude
               deposited into 335141 of 335141 pulses, bins 3316.6-3507.4

null scene     13091 scatterers, 0.250 per sub-look resolution cell (derived)
19 trials      min 12.0, mean 17.0, sd 3.6, worst 23.8

measurement    0.163 Hz, prominence 32.0, backed by 9 windows, block 9
               1.88x the null mean, 1.35x the worst
               0 of 19 reached it -- empirical p = 0.0500

VERDICT        ADJUDICATED: p = 0.0500 <= alpha 0.05
```

**The reported frequency is the injected one.** 0.163 Hz asked for, 0.163 Hz
returned, by the policy `mmotion` actually prints.

## What it establishes, and what it does not

*It establishes that the chain can extract and ADJUDICATE a known signal on real
phase history.* Every earlier attempt failed at one of the two: item 30 extracted
and could not report; item 33's null refused everything.

*It is a positive control, not a detection.* The target was put there, at 2 mm and
twenty times the median sample magnitude, and the window was known. Item 30's
distinction stands: finding an unknown target needs a selection policy over the
scene, which this does not test.

*p = 0.0500 is the weakest possible pass.* Nineteen trials is the minimum for
alpha = 0.05, so a clean sweep gives exactly alpha; a single null reaching the
measurement would have given 0.10 and failed. The margin in PROMINENCE is
comfortable -- 32.0 against a worst null of 23.8 -- but the p-value has no
headroom, and a stronger claim needs more trials, not a better result.

*The null density is calibrated on this same scene.* Item 34 set 0.25 scatterers
per cell so the null's prominence matches Giza's uninjected value. Adjudicating a
Giza measurement against it is therefore in-sample. That is not circular in the
fatal sense -- the calibration target was the MOTIONLESS scene and the test is of
an INJECTED one -- but the density has not been shown to transfer to any other
collect, and until it is, a verdict elsewhere rests on an untested constant.

*One frequency, one amplitude, one collect, one grid size.* Nothing here bounds
sensitivity. The obvious next sweep is amplitude, downward, until adjudication
fails.
