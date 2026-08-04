# Run: 2026-08-04 ICEYE Houston — the first measurement run on this vendor

Truncated to 40320 pulses (6.138 s) per item 44, `--estimator phase`, 128 looks
at 0.90 overlap, 96 m grid at 1.0 m, `--coherence 0`, `--probe-hz 1.000`. Three
runs: no injection, 1.0 Hz at 2 mm, and its zero-amplitude twin.

## 1. Urban does NOT meet the persistent-scatterer precondition either

```
                       best D_A   median   windows meeting D_A <= 0.25
  Giza  (desert, 225)     0.381        —              0 of 225   (item 19)
  Houston (urban,  25)    0.444     0.597              0 of  25
```

**Houston is worse than Giza on best `D_A`.** The expectation was that a dense
city would supply the dominant scatterers a desert could not. It does not, and
the reason is in the same output: the sub-look resolution here is **2.89 m**.
Item 15's precondition is one dominant scatterer per SUB-LOOK RESOLUTION CELL,
and a 2.89 m cell in a dense city holds many strong scatterers rather than one.
Urban density works against the criterion at this cell size, not for it.

## 2. The quality gate discards the true positive

The injected 1.0 Hz is recovered — `df` is 0.1629 Hz and six windows report
1.047 Hz, inside half a bin — with the **highest prominence in the scene**. Every
one of them fails the gate:

```
 win  (az,rg)   dominant    prom  quality  gates
   0 (0,0)        0.524    25.4   0.5428      1   <- REPORTED
   6 (1,1)        1.047    35.8   0.2061      0
   7 (1,2)        1.047    35.8   0.2061      0
  11 (2,1)        1.047    35.8   0.2061      0
  12 (2,2)        1.047    35.0   0.2406      0   <- the injected window
  13 (2,3)        1.047    35.0   0.2406      0
```

`mmotion` reported **0.524 Hz** — the first admissible bin above the band floor,
which is item 37's trend arriving at the first door it is allowed through.

**The mechanism is not a tuning error.** On the phase route `quality` is
amplitude STABILITY, the same quantity as `D_A`. A scatterer vibrating at 2 mm is
not amplitude-stable across sub-looks — that is what the motion does to it — so
**the persistent-scatterer criterion and the signal being measured are in direct
conflict at large amplitude.** Item 37 found the complement: a barely-moving
scatterer has LOW `D_A` and passes. The gate prefers targets that do not move.

This is item 31's failure mode again — the tracker recovers and the policy
discards — with a different culprit.

## 3. Two newer statistics recover what the reported policy loses

```
                    plain        injected            zero-amplitude twin
  scene null   0.524 Hz z 3.03   1.047 Hz z 7.13     0.524 Hz z 2.88
               window 1          WINDOW 12           window 12
```

**The scene-derived null names the injected window and the injected frequency**,
at more than twice the z of either control. Window 12 is `(iaz 2, irg 2)` — the
grid origin, where the injection lands. It applies no quality gate, which is why
it survives where `best_window` does not.

The paired increment at 1.000 Hz agrees:

```
  window  6 (1,1)  T = +86.91      window 12 (2,2)  T = +80.32
  window  7 (1,2)  T = +86.91      median over 25 windows: +0.00
  window 11 (2,1)  T = +86.91
```

Median +0.00 — the injection machinery contributes nothing, as in item 39 — and
the mass sits on the 2x2 block around the origin. The centred window again scores
slightly LOWEST of its neighbours, which is item 41 reproduced on a different
vendor and a different scene.

**Item 38 concluded the scene-derived null did not help.** It did not there,
because the confound was a bright static scatterer that genuinely is unusual for
its scene. Here the confound is a gate, and the null is the only reported
statistic that steps around it. Both readings stand; the null is useful against
some confounds and not others.

## What this is not

Nothing in this scene is known to move. The 1.047 Hz is ours. The plain run's
0.524 Hz is a trend artefact at the band floor, and its 105.7 mm/s peak-to-peak
on an urban scene is the uncalibrated readout of item 43 doing what it does.

The `located at` centroid is seeded from `best_window`, so on the injected run it
reports (0.00, 0.00) — the artefact's position, not the target's. **The centroid
is only as good as its seed**, and nothing currently seeds it from the scene-
derived null.
