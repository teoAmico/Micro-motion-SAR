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

---

# Follow-on: all three policies on the same data

The static false positives above are a POLICY failure, so the three selection
policies were scored on identical spectra. `policy_sweep.c` and
`policy_run.log` beside this file.

```
           | PROMINENCE (best_window)  | LOCAL PEAK (item 47)      | PS SELECTOR
 lobe_frac |   slope     rms verdict   |   slope     rms verdict   |
      1.00 |  1.0417  0.0333 RECOVERS  |  0.7500  0.2555 fails     | answered 0/5
           | static in band 1/3        | static in band 2/3        | static 0/3
      0.50 |  1.0417  0.0289 RECOVERS  |  1.0833  0.0527 fails     | answered 0/5
           | static in band 0/3        | static in band 0/3        | static 0/3
      0.25 |  1.2083  1.1252 fails     |  2.0833  0.8528 fails     | answered 0/5
           | static in band 2/3        | static in band 0/3        | static 0/3
      0.12 |  0.9583  0.0236 RECOVERS  |  0.9583  0.0289 RECOVERS  | answered 0/5
           | static in band 0/3        | static in band 1/3        | static 0/3
```

**None of the three is both safe and useful here.**

- **Prominence** recovers at three of four lobe widths and returns an in-band
  frequency on 3 of 12 static scenes. Useful and unsafe.
- **The local peak** recovers at ONE of four and is equally unsafe, 3 of 12. It
  is strictly worse than prominence on this fixture.
- **The PS selector** answers nothing at all -- 0 of 5 at every lobe width -- so
  it has no false positives and no recall.

## The local peak's scope, which this pins

Item 47's local peak was a large win on the real Giza collect: the injected
versus control separation went from 3.3x to 37x, because that failure was a RED
NOISE FLOOR and a local background is exactly the right correction for one.

**It is a loss here.** Aspect dependence does not produce a coloured noise floor;
it produces genuine spectral content, because a facet lit over part of the
aperture amplitude-modulates the return and a modulation has real sidebands. A
local background cannot tell a real sideband from a real tone -- both stand above
their neighbours -- so normalising by it removes the low-frequency bias without
touching this failure, and costs recall.

The two failures look alike in the output and are not alike, and the statistic
built for one does not transfer.

## What that leaves

Item 25's reading is unchanged and now sharper: the PS selector is safe because
it REFUSES, and at this configuration it refuses everything. Item 25 measured 6
answers of 18 at `lobe_frac` 0.12; here no window meets `D_A <= 0.25` at any
lobe width, so the recall it had is configuration-dependent and this
configuration has none of it.

**No policy here is fit for an aspect-dependent scene**, which is the scene type
real structures produce.

---

# Follow-on 2: the AM/PM discriminator, which does not work

Item 55 left the failure isolated to the selection stage. The physics suggested
a discriminator: a VIBRATION modulates phase and not brightness; ASPECT
DEPENDENCE modulates brightness and not range. So a frequency present in both
the displacement and the amplitude spectrum should be brightness, and one
present only in displacement should be motion.

`rs_spectrum_am_check()` implements it. `am_sweep.c` and `am_run.log` beside
this file.

```
  lobe   injected AM ratio    static AM ratio    in-band statics rejected
  1.00        0.4 - 36.8          8.7 - 12.7                1 of 1
  0.50        1.1 - 97.5         26.8 - 444.6               0 of 0
  0.25        0.6 - 28.1          0.7 - 7.8                 0 of 2
  0.12        0.3 - 12.2          0.7 - 13.2                0 of 0
```

**It does not separate them.** The ranges overlap at every lobe width, the
static ratios EXCEED the injected ones at lobe 0.50, and one of the three
in-band false positives is caught. No threshold on this quantity works.

## Why, which is the part worth keeping

**The false positives are not amplitude modulation at the reported frequency.**
At lobe 0.25 the two in-band statics have ratios of 0.7 to 7.8 -- no amplitude
peak at all where the displacement peaked.

The mechanism is different from the one assumed. The aspect lobe makes the
tracked pixel FADE over part of the aperture, and the pixel is chosen once from
the reference look, so during the fade its phase is noise-dominated. The series
is then non-stationary -- good phase for part of the record, noise for the rest
-- and the spectrum of that has structure at no particular frequency. The
amplitude signature is a smooth ENVELOPE at the bottom of the band, not a tone
where the displacement peaked.

## What it re-derives

The right question is **"does this window's brightness vary at all"**, not "does
it vary at this frequency". That question is amplitude dispersion, which is what
`rs_spectrum_ps_window()` applies and why item 25 found it the only safe policy.

This function reaches item 25's conclusion the long way round. The threshold and
the rejection flag were removed after the measurement; it reports a ratio and
gates nothing.
