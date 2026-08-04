# Run: 2026-08-04 fixtures / aspect-carrier

**Question:** items 52-53 improved the carrier removal by 171x against a static
bright scatterer on real data. Item 25 is where the phase route breaks on a
FIXTURE -- aspect-dependent scattering, the mechanism real structures have and
`rs_sim_scene()` lacks. Does the carrier fix rescue it?

**Answer: it rescues the recovery and not the safety.** Recorded as item 54.

## Result

`aspect_sweep.c` beside this file is the harness.

```
 lobe_frac |    slope   rms Hz | verdict  | static controls (3 seeds)
      1.00 |   1.0417   0.0333 | RECOVERS |  0.25  0.58  0.25  IN BAND
      0.50 |   1.0417   0.0289 | RECOVERS |  0.25  0.25  0.25  outside
      0.25 |   1.2083   1.1252 | fails    |  0.33  0.25  0.33  IN BAND
      0.12 |   0.9583   0.0236 | RECOVERS |  1.83  1.75  1.67  outside
 isotropic |   0.9583   0.0236 | RECOVERS
```

Item 25 measured `rs_spectrum_best_window()` failing at **all four** lobe widths,
with slope negative at three of them. **Three of four now recover.**

**But three of twelve static controls come back inside the swept band**, against
item 25's two of twelve. That failure is untouched.

## Why the split makes sense

The two failures had different causes and only one was a carrier problem.

The slope-and-rms failure was the carrier residual: aspect dependence makes a
scatterer's amplitude vary across the aperture, which perturbs the linear-only
carrier fit, and the leftover curvature swamped the tone. Fitting the curvature
removes it.

**The static false positives are a POLICY failure and always were.** Item 25 said
so: "aspect dependence gives amplitude per-look structure the isotropic model
never had, and the prominence policy reads that structure as a frequency."
Nothing about the carrier changes what prominence does with a real amplitude
modulation on a scene where nothing moves.

## Not a like-for-like reproduction of item 25

Stated plainly because the numbers invite the comparison:

- **5 frequencies over 1 seed**, where item 25 used 6 over 3 -- 5 points against
  18, so the slope and rms are much less constrained.
- **12 s dwell at 64 looks** gives `df = 0.0833 Hz` and a half-bin bound of
  0.0417, where item 25's bound was 0.0252. **A looser bar.**
- Same fixture, same lobe fractions, same estimator and window geometry.

So "three of four recover" is a directional comparison against item 25's "none of
four", not a withdrawal of it. The static-control count -- 3 of 12 against 2 of
12 -- is the one that should be read as unchanged rather than as worse, given
the different seed count.

## What it means for item 25

Item 25's headline was two-part: the reported policy fails, and it fails
unsafely. **The first part weakens and the second stands.** The policy is what
item 25 concluded was at fault, and it still is -- `rs_spectrum_ps_window()` was
the only one that behaved there, and nothing here changes that.
