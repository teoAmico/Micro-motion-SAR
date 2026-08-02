# Run: 2026-08-02 giza / positive-control

**Question this run is meant to answer:** every real collect this project has
processed returns a null, and a null cannot distinguish "nothing moved" from
"this chain cannot see motion in this data". Inject a scatterer of known
frequency into the real phase history before sub-aperture formation and ask
whether the chain returns it.

## Provenance

- commit: `327ff95` for attempt 3; attempts 1 and 2 predate the response guard
- host: 8 cores, 24 GB RAM, macOS; collect on external USB
- collect: `CAPELLA_C13_SP_CPHD_HH_20241004001939_20241004002012.cphd`, 36 GB,
  335141 usable vectors of 335149
- settings identical to `runs/giza/2026-08-01-phase-highoverlap` so the result is
  comparable to the null recorded there

## Commands

```sh
./build/micromotion mmotion --cphd "$C" --at 29.979175,31.134186 \
    --estimator phase --n 128 --overlap 0.90 \
    --size 256 --cell 1.0 --win 32 --rbins 4096 --coherence 0 \
    --inject-vib 0.15,2.0,20 \
    --out runs/giza/2026-08-02-positive-control/khufu_inject3
```

31 min 35 s, 610% mean CPU.

## Three attempts, two of them void

**Attempt 1 (`khufu_inject2*` predecessor, 1.0 Hz).** VOID. The injected target
never focused: `focus --inject-vib` measured peak-to-median 3.5 at this offset,
indistinguishable from speckle. `src/core/simulate.c` wrote the deposit envelope's
width in metres and evaluated it against an offset in bins, making the response a
single-bin spike. Fixed; FOLLOW-UPS.md item 28.

**Attempt 2 (`khufu_inject2*`, 1.0 Hz, after the fix).** VOID for a different
reason. The target focused and the chain saw it -- `D_A` best fell from 0.427 to
0.136 and 2 of 225 windows met the persistent-scatterer criterion where none had
-- but 1.0 Hz sits at sub-aperture response 0.128 at this operating point. Each
sub-look integrates 2.45 cycles and averages the motion away. The reported 0.033
Hz is what a blind band returns. `mmotion` now computes and warns about this.

**Attempt 3 (`khufu_inject3*`, 0.15 Hz, response 0.800).** The first attempt that
asked the question.

## Result

```
  strongest window 127: 0.163 Hz
  consensus:            0.163 Hz, agreed by 32 of 136 windows (24%)
  cull:                 0.163 Hz from 20 of 136 windows
  D_A best 0.427; 0 of 225 windows meet D_A <= 0.25
NO FREQUENCY REPORTED: only 32 of 136 windows agree (24%)
```

0.15 Hz falls in the bin centred at 0.163 at this 0.0326 Hz resolution, so
**0.163 Hz is the injection**. All three spectrum policies land on it together,
having disagreed in both controls:

```
  run                          strongest   consensus   cull
  uninjected (item 17)           0.033       0.195     0.065
  injected 1.0 Hz (blind band)   0.033       0.065     0.261
  injected 0.15 Hz               0.163       0.163     0.163
```

**The chain extracts the injected motion from real Giza phase history, and the
tool refuses to report it.** `NO FREQUENCY REPORTED` fires on 24% window
agreement, and `rs_spectrum_ps_window()` finds nothing because `D_A` never
reaches 0.25 anywhere in this scene. That is items 7-9 -- the tracker recovers
the carrier and the selection policy discards it -- reproduced on real data with
a known truth for the first time.

## What this is not

**One frequency, one run.** The bar in `README.md` is a slope-and-rms fit across
a sweep, because a single frequency matched once is a coincidence this file
records several times. This is one point and is not a recovery.

**It says nothing about whether Giza moves.** The injected scatterer dominates
the answer; the scene's own behaviour is not what was measured.

## Unexplained

`D_A` best was 0.136 with 2 qualifying windows at 1.0 Hz and 0.427 with 0 windows
at 0.15 Hz -- same amplitude, same relative brightness, same geometry, only the
injected frequency differing. Amplitude dispersion should not depend on the
injected frequency. Either sub-look amplitude does depend on it in a way not
accounted for, or one of the two numbers is not measuring what it appears to.
Open.
