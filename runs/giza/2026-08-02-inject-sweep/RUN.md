# Run: 2026-08-02 giza / inject-sweep

**Question this run is meant to answer:** item 29 recovered one injected
frequency from the real Giza collect. One point is a coincidence by this
project's own standard (item 2). Does a SWEEP of injected frequencies recover,
by slope and rms, on real phase history?

## Provenance

- commit: `1050390` (figure changes landed after and do not affect these numbers)
- collect: `CAPELLA_C13_SP_CPHD_HH_20241004001939_20241004002012.cphd`
- six runs of ~30 min: one uninjected control and five injections, sequential,
  each resumable (`/tmp/claude-501/sweep/run.sh`)

## Band

The sub-aperture response, not Nyquist, sets what is testable. At 128 looks and
0.90 overlap `t_sap` is 2.45 s, so response stays above the ~0.5 item 13
requires only below about 0.25 Hz, while the bin spacing is 0.0326 Hz. The five
injections sit on bin centres 3, 5, 7, 9, 11 -- two bins apart so a one-bin error
cannot merge two points -- at responses 0.91 down to 0.58.

## Commands

```sh
./build/micromotion mmotion --cphd "$C" --at 29.979175,31.134186 \
    --estimator phase --n 128 --overlap 0.90 \
    --size 256 --cell 1.0 --win 32 --rbins 4096 --coherence 0 \
    --inject-vib "$F,2.0,20" --out runs/giza/2026-08-02-inject-sweep/sweep_$F
```

## Result: the tracker passes the bar, the selection policies fail it

Window 112 is the centre window of the 15x15 tracking grid -- the one holding the
injected scatterer.

```
 injected  win112 dom    prom     d_a   hit      consensus   cull
  CONTROL    0.065134   13.87   0.542             0.065      0.065
    0.098    0.097701   28.71   0.376   YES       0.065      0.065
    0.130    0.130268   31.25   0.475   YES       0.130      0.065
    0.163    0.162835   31.37   0.556   YES       0.163      0.163
    0.196    0.195402   29.88   0.610   YES       0.065      0.065
    0.228    0.227969   25.60   0.645   YES       0.065      0.065

  window 112 alone : slope +0.9990  rms 0.00033 Hz   PASS (bound 0.0163)
  consensus        : slope -0.202   rms 0.0947  Hz   FAIL
  cull             : slope +0.000   rms 0.0990  Hz   FAIL
```

The uninjected control returns 0.065 Hz in the same window, below the swept band
of 0.098-0.228, so the static-control condition is met as well.

Every run printed `NO FREQUENCY REPORTED` at 19-24% window agreement.

## What it means

**The chain extracts injected motion from real phase history, and the reporting
policy discards it.** That is items 7-9, confirmed on a real collect against a
known truth rather than on a fixture, and it is the first time anything in this
project has met the README bar on real data.

**It is not a detection method.** The window was known because the target was put
there. Testing whether the chain CAN extract motion is what a positive control
does; finding an unknown target needs a selection policy, and every policy here
failed on this scene. The gap between those two is now measured rather than
argued.

## Bounds and caveats

- One collect, one operating point, one amplitude (2 mm at 20x the median
  non-zero sample magnitude).
- The injected scatterer is ISOTROPIC. Item 24 established that real dominant
  scatterers are aspect-dependent and item 25 that aspect dependence is what
  breaks the phase route on fixtures. This injection does not test that.
- The band is narrow -- 0.098 to 0.228 Hz -- because the response ceiling allows
  no more at this overlap. A slope over 0.13 Hz of span is a weaker constraint
  than the synthetic sweeps' 1.0 Hz.
