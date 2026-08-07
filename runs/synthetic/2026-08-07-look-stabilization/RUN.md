# Run: 2026-08-07 synthetic / look-stabilization

**Question this run is meant to answer:** Does requiring a frequency to survive a change of LOOK COUNT kill the 100% false-positive rate?

- git commit: `4190933`
- started:    2026-08-07T07:10:13Z
- host:       Darwin arm64

## Collect

`sim_cphd`, 400 clutter scatterers, `--clutter-vib`. Twelve motionless scenes
(amplitude 0.0, seeds 3-41) and six injected (2 mm, seeds 3-17). Each processed
at 128 AND 256 looks; nothing else differs between the paired runs.

## Commands

See `stabsweep.sh`. Pre-registered in `PREREG.md` at commit f6ad9f4.

## Result

**Both hypotheses PASS.**

```
  H1  motionless scenes still reporting:  1 of 12   (item 96 measured 12 of 12)
  H2  injected scenes still reporting:    6 of 6
```

Seven statics moved within the band by 0.15 to 2.07 Hz; four moved out of it
entirely. Every injected scene reported 0.504 Hz at both look counts to three
decimals.

This is the first discriminator in this project that needs **no twin, no null
control and no ground truth** -- only the collect, processed twice.

Full write-up, with the bounds that matter, in `docs/FOLLOW-UPS.md` item 107.
