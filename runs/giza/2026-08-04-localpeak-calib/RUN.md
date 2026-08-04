# Run: 2026-08-04 giza / localpeak-calib

**Question:** item 47 introduced the local peak and left it gating nothing,
because its control maximum was known from two scenes. Two samples is not a
distribution.

**Answer: nine disjoint grids of real desert give 15.1 to 34.4, median 20.8.**
Recorded as item 49.

## Design

Nine 96 m grids on the same real uninjected Giza collect, on a 3x3 lattice at
150 m spacing so no two share a window. `--overlap 0.5`, which the sweep beside
this makes the setting worth calibrating rather than 0.90.

**No simulator anywhere in this.** The null is real clutter through the real
chain, which is the property `--null-static` never had (items 33, 37) and the
reason the scene-derived null was built. This applies the same idea to
frequency: the collect provides its own controls, and they carry whatever it
does.

## Result

```
  15.1  16.2  19.2  19.5  20.8  20.9  22.5  25.0  34.4

  min 15.1   median 20.8   MAX 34.4
```

A tight distribution -- the largest is 2.3x the smallest across nine independent
patches of desert. For comparison the ICEYE Houston control gave 47.5 at overlap
0.90, and a 2 mm injection at this overlap gives **1,207,566**, which is
**35,104x the worst control grid**.

## What it is worth, exactly

**Nine controls give a smallest possible p of 1/10 = 0.10.** A measurement
exceeding all nine is significant at 0.10 and no better, because a conformal
p-value cannot express more than the control count allows -- the same arithmetic
item 35 recorded for `--null-static`. Nineteen disjoint grids would reach 0.05.
The collect is large enough to supply them; this run did not.

That said, 35,104x is not a marginal result and the p-value is not what is doing
the work. The number worth carrying forward is the RANGE: anything under about
35x on this collect at this setting is inside what empty desert produces.

**It is one collect, one setting, one grid size.** The control maximum is a
maximum over every window and every bin, so it scales with the number of windows
searched; a larger grid will produce a larger control maximum and this number
does not transfer to one.
