# Run: 2026-08-04 giza / scene-null

**Question:** build a null from the real scene rather than a simulator, and
check it against the amplitude sweep. Also run the negative injection that
critique of item 37 asked for -- zero amplitude through the identical injection
code path -- to check whether rewriting the phase history at all moves the
statistic.

**The negative injection is the result, and it is bad.** Recorded as item 38.

## The negative control

```
./build/micromotion mmotion ... --inject-vib "0.163,0.0,20"     <- ZERO amplitude

strongest peak in window 8: 0.098 Hz, prominence 56.3, quality 0.895,
                            peak-to-peak velocity 0.8 mm/s
  backed by 9 windows, largest touching block 9
```

**A bright scatterer that does not move at all scores higher than any real
injection.** Against the recalibrated `--null-static` worst of 22.9 it
ADJUDICATES, at 2.5x the bar.

| run | reported | prominence | scene-null z |
|---|---|---|---|
| **zero amplitude (negative)** | **0.098 Hz** | **54.7** | **8.23** |
| uninjected, no injection at all | 0.130 Hz | 17.6 | 1.56 |
| 0.0625 mm | 0.163 Hz | 46.7 | 6.38 |
| 0.125 mm | 0.163 Hz | 44.4 | 5.99 |
| 0.5 mm | 0.163 Hz | 39.0 | 5.34 |
| 2.0 mm | 0.163 Hz | 38.0 | 5.28 |

Every amplitude statistic ranks the motionless bright target FIRST. Prominence,
`--null-static` and the new scene-derived null all agree, and all three are
wrong.

**The only thing that separates them is the reported FREQUENCY.** Zero amplitude
answers 0.098 Hz, which is bin 3 -- the band floor's own edge, item 37's trend
arriving at the first door it is allowed through. Every real injection answers
0.163 Hz, the injected value. Nothing about the peak's SIZE carries the
distinction; only its POSITION does.

## What that costs item 35

Item 35 adjudicated a 2 mm injection at p = 0.05 against 19 synthetic null
trials. A zero-amplitude injection through the same code path would have
adjudicated too, more strongly. **That p-value measured "a bright scatterer was
added to the phase history", not "the scatterer vibrated."**

This does not touch the frequency results. `rs_track_fit()`'s slope-and-rms bar
(item 30) is scored on WHICH frequency comes back, and a zero-amplitude control
does not return 0.163 Hz. Recoverability stands. Detectability was never
measured, and the statistic that claimed to measure it responds to the injection
machinery rather than to motion.

## The scene-derived null

Built and reported (`rs_spectrum_scene_null()`), and it is worth having on its
own terms:

- it refuses the real uninjected scene, z 1.56 against a control-scene
  family-wise maximum of 2.53 measured over 225 windows;
- it separates a localised target from a common-mode artefact in the test
  fixture, z 34.6 against 2.21, which is the case `--null-static` cannot see;
- it costs nothing, and it carries the collect's own trends by construction.

**It does not fix the negative-injection problem**, because a bright static
scatterer IS localised -- it is genuinely unusual for its scene. A spatial null
answers "is this window unusual here", and the answer is correctly yes. The
question that needed asking was "did it move", and no amplitude statistic
computed after the fact can answer it.

## What would

A paired increment at the nominated frequency, `P_injected(f0) -
P_zero_amplitude(f0)`, comparing the same window against the same window with
the injection machinery run at zero. That needs the PSD at a nominated
frequency, which `PREFIX_windows.csv` does not currently carry -- it records
each window's dominant frequency and the prominence there, so the zero-amplitude
run cannot be interrogated at 0.163 Hz. Not attempted.

**Any future positive control here must ship its zero-amplitude twin.** A
positive control that has never been run at zero amplitude does not establish
that the effect came from the motion.
