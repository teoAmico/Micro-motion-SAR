# Run: 2026-08-03 giza / null-static — **INCOMPLETE**

**Question this run was meant to answer:** item 31 made `--null-static` the gate.
It has never been run on the Giza collect. Does a motionless realisation of
Giza's own geometry reach the prominence a known injected true positive produces?

**Status: stopped after ~2 hours with one null trial of five.** Kept rather than
deleted because the one trial is informative and the timings change what a
sensible next attempt looks like.

## Provenance

- commit: `7629d89`
- host: 8 cores, 24 GB RAM, macOS; collect on external USB (exFAT)
- collect: `CAPELLA_C13_SP_CPHD_HH_20241004001939_20241004002012.cphd`

```sh
./build/micromotion mmotion --cphd "$C" --at 29.979175,31.134186 \
    --estimator phase --n 128 --overlap 0.90 \
    --size 256 --cell 1.0 --win 32 --rbins 4096 --coherence 0 \
    --inject-vib "0.163,2.0,20" --null-static 5 --out inj0163
```

## What survived, and what did not

Only `stderr` survived. `mmotion` writes its report to `stdout`, which is
block-buffered when redirected to a file, so the injection report, the headline
and the verdict were still in the buffer when the process was killed. The PNGs
and `_windows.csv` are written at the very end and never got there.

**A relaunch should capture stdout unbuffered**, or the first two hours of any
interrupted run are unrecoverable. That is a lesson about the harness, not about
the measurement.

## The one datum

```
static trial 1/5: prominence 36.5 at 0.065 Hz
```

Set against measurements from the same collect and the same settings (item 31):

| | max prominence |
|---|---|
| five injected runs | 25.77 – 35.35 |
| **real uninjected control** | **17.48** |
| **simulated static null, trial 1** | **36.5** |

The simulated null scored **2.1x the real desert**, and above every injected run.

## What that does and does not support

**It is one trial.** No distribution, no spread, and a single draw from a
distribution nobody has characterised cannot establish a bias.

**The configurations are not like for like either.** The null trials synthesise a
collect at `sim_rbin = 1024` where the run itself used `--rbins 4096`, so a
difference in prominence is not attributable to scene content alone.

**But the direction is the one that matters, and it is the opposite of the
expected failure.** Before this ran the anticipated bad outcome was "the null
reaches the measurement, so the gate is too strict". If a *simulated* motionless
scene routinely out-scores the *real* motionless scene by a factor of two, the
problem is not the threshold — it is that `rs_simulate_static_like()` is not
statistically comparable to the scene it stands in for, and gating on it would
refuse real measurements that the real scene's own noise never would.

Item 27 fixed that function so its output *focuses*. Nothing has ever checked
that its prominence distribution matches a real collect's. That check is cheap
and has not been done.

## What a better next attempt looks like

Not a relaunch of this. Two changes:

1. **Characterise the null instead of consuming it.** The question is whether the
   simulated static scene's prominence distribution matches a real motionless
   scene's. That can be asked at a much smaller operating point — a 64 m grid at
   32 looks, where a trial is minutes — by running `--null-static N` beside a real
   uninjected run at the same settings and comparing the two distributions. If
   the simulated null is systematically harsher there, it is at Giza scale too,
   and the expensive run is not needed to establish it.

2. **The real uninjected control is already the better null**, and item 30's
   sweep produced it: prominence 17.48, against 25.77–35.35 for the five injected
   runs. It is n = 1, but it is the right n = 1 — the actual scene, through the
   actual chain.

## Timings, for budgeting

- main run with injection: ~60 min (item 17's uninjected run was 44 min 55 s)
- per null trial: ~35 min at `--size 256`, `--n 128`
- so `--null-static 5` at this operating point is ~3.5–4 hours, not the ~1.5 the
  trial count suggests
