# Run: 2026-08-01 synthetic / e2e-check

**Question this run is meant to answer:** does the CLI chain run end to end with figures and a null control

- git commit: `eafc526`
- started:    2026-08-01T10:58:54Z
- finished:   2026-08-01T11:21Z (23 minutes wall)
- host:       Darwin arm64

## Collect

```
synthetic, from tools/sim_cphd
  0.5 Hz injected, 0.02 m amplitude
  400 Rayleigh clutter scatterers, --clutter-vib (patch moves coherently)
  seed 7
  8000 pulses x 256 range bins, 20 s dwell, PRF 400 Hz
```

## Commands

```sh
tools/new-run.sh synthetic e2e-check "does the CLI chain run end to end ..."
./build/sim_cphd scene.cphd 0.5 0.02 --clutter 400 --clutter-vib --seed 7
./build/micromotion mmotion --cphd scene.cphd \
    --n 128 --overlap 0 --size 128 --cell 0.5 --win 32 \
    --upsample 200 --coherence 0 --null-static 5 --out e2e \
    > stdout.log 2> stderr.log
```

## Result

**The chain runs end to end and correctly refuses to report a frequency.** Both
independent checks decline, which is the point of the run.

*Consensus.* The strongest window reports 0.504 Hz against an injected 0.500 -- a
near-miss that reads as a recovery in isolation. Only 4 of 49 windows agree (8%),
across 36 distinct answers, largest contiguous block 2. That is below the
geometric bound of 4, so it is refused.

*Null control.* Five simulated motionless collects through the identical chain:

```
mean 7.8, sd 0.8, worst 9.1
detection 8.3 is 1.06x the mean and 0.91x the worst
1 of 5 reached it -- empirical p = 0.3333
```

A motionless scene reached the measurement. **The two checks are independent and
they agree**, which is the pairing item 11 of `FOLLOW-UPS.md` argues is necessary:
consensus catches scene-driven noise, the null catches common-mode artefacts, and
neither substitutes for the other. Here both fire on the same peak.

Note this is one seed at one operating point, and settles nothing about the
configuration. It establishes that the machinery runs and that the refusals work.

## Cost, which is a finding in itself

The measurement takes about 9 s. `--null-static 5` took **23 minutes** -- each
trial re-simulates and refocuses the whole 128-look stack, and `--upsample 200`
makes the correlator dominate. That is roughly a 150x multiplier on the check this
project calls the one that matters, which is a standing incentive to skip it.

Worth investigating whether the null trials can reuse the focused stack rather
than rebuilding it per trial. Not attempted here.

## Artifacts

`e2e_freq.png`, `e2e_quality.png`, `e2e_scene.png`, `e2e_spectrum.png`,
`e2e_windows.csv`, `stdout.log`, `stderr.log`.

**`e2e_scene.png` did not exist when this run was made** and was produced later
the same day, by the same regeneration described below, once `mmotion` learned to
write the scene its windows sit on. `stdout.log` therefore predates it and its
`wrote ...` line does not name it.

It is worth looking at. The sub-look is heavily oversampled -- 0.5 m cells
against the 8.26 m sub-look resolution printed above -- so what fills the frame
is the sub-look point-spread function, not scatterers: sixteen cells per
resolution element, drawn out into tilted streaks. The 32 px correlation patch
outlined in red spans about two resolution cells. That is the geometry every
number in this run was measured in, and it was not visible in any output the run
originally produced. Note this is a different quantity from the aliasing warning
in `stderr.log`, which compares the cell to the FULL-aperture resolution of
0.064 m.

**`e2e_spectrum.png` was regenerated later the same day**, after the y axis
learned to state its units: it read a bare `POWER` and now reads
`POWER, (M/S)^2/HZ`, which is what the correlation estimator's observable
actually is. Nothing else about it changed. The regeneration re-ran `sim_cphd`
and the `mmotion` line above verbatim, minus `--null-static 5`, which touches no
figure; `e2e_freq.png`, `e2e_quality.png` and `e2e_windows.csv` came back
bit-for-bit identical and every number printed to stdout matched, so the plot
below is this run's plot with a corrected label rather than a second run's.
`stdout.log` and `stderr.log` are the originals and still carry the null-control
trials, which were not repeated.

The scene and its truth file are gitignored (`*.cphd`, `*.cphd.truth`);
regenerate with the `sim_cphd` line above, which is deterministic under
`--seed 7`.
