# Run: 2026-08-01 giza / phase-highoverlap

**Question this run is meant to answer:** does the repaired phase estimator
(FOLLOW-UPS items 14-15) survive contact with a real collect at the high overlap
that real sub-look coherence requires, and does the common-mode sawtooth artefact
it used to emit reappear?

## Provenance

- commit: see `git log -1` at the time of the run (working tree clean apart from
  this directory, which is gitignored)
- host: 8 cores, 24 GB RAM, macOS; collect on external USB
- collect: `CAPELLA_C13_SP_CPHD_HH_20241004001939_20241004002012.cphd`, 36 GB,
  335141 usable vectors of 335149, dwell 32.869 s, PRF 10196 Hz
- reader independently cross-checked against SARPy (item 12b): every derived
  field agrees

## Commands

```sh
./build/micromotion validate --cphd "$C" --frequency 1.0 --overlap 0.90 --amplitude 2.0
./build/micromotion focus --cphd "$C" --at 29.979175,31.134186 \
    --size 512 --cell 1.0 --rbins 4096 --max-pulses 16696 --out khufu.png
./build/micromotion mmotion --cphd "$C" --at 29.979175,31.134186 \
    --estimator phase --n 128 --overlap 0.90 \
    --size 256 --cell 1.0 --win 32 --rbins 4096 --coherence 0 \
    --out runs/giza/2026-08-01-phase-highoverlap/khufu
```

`--at` resolves to `--offset -152,-552`. `focus` confirms the pyramid is inside
the range window before the expensive run; the quicklook shows its four edges
clearly. 44 min 55 s for the mmotion run, 442% mean CPU.

`validate` returned **FAIL**, and was overridden deliberately: three of its four
failing checks are the correlation tracker's limits in tracking pixels and do not
apply to this estimator, while `phase floor` -- the one written for this
observable -- passes at 0.13 mm per look. Recorded as FOLLOW-UPS item 16.

## Result: a NULL, correctly refused

```
sub-apertures: 128 looks, dt 0.2399 s
  observable band  f_max 2.08 Hz   AT sub-look resolution 0.70 m
tracked 225 windows (15 x 15); 225 pass the 0.00 coherence mask
spectra: 65 bins, 0.0326 Hz resolution
NO FREQUENCY REPORTED: only 27 of 170 windows agree (16%), which is what a
  MOTIONLESS scene produces.
  strongest window 55: 0.033 Hz, prominence 24.3, quality 0.483
  consensus: 0.195 Hz, 27 of 170 (16%), 12 distinct, largest block 9
  cull: 0.065 Hz from 18 of 170 surviving (neighbours 105 removed)
  NOTE: the cull and the consensus disagree (0.065 vs 0.195 Hz)
```

Per-window: quality min/median/max 0.000 / 0.417 / 0.619; prominence median 8.9.
Dominant frequencies spread flat across the low bins -- 0.163 Hz in 35 windows,
0.195 in 33, 0.098 in 30, 0.065 in 28, 0.130 in 26, 0.228 in 25.

## What it establishes, and what it does not

**Establishes: the artefact is gone on real data.** Item 11 recorded the old phase
estimator returning ONE fixed frequency at 100 percent window agreement, on
moving and motionless scenes alike. Here 225 windows spread over 12 distinct bins
with the modal one taking 16 percent, and the histogram is flat. That is noise,
not a line, and it is the first evidence outside simulation that the carrier
removal of item 14 does what it was built to do.

**Establishes: the chain runs at the high overlap real coherence needs**, end to
end, on a 36 GB product, without producing a confident answer it has not earned.

**Does NOT establish that the estimator can detect real motion.** Nothing in this
scene is known to move; `RS_VALIDATE_GROUND_TRUTH` says so and always will. A
null here is the expected outcome for a pyramid and is uninformative about
sensitivity. It is not a negative result about the method.

**`--null-static` was not run.** A null control adjudicates a POSITIVE; with no
detection to adjudicate it would cost 45 minutes per trial to confirm that
nothing is still nothing. It is required before any positive from this
configuration is believed.
