# Run: 2026-08-04 giza / centroid

**Question:** item 41 bounded this tool's localisation at exactly one window --
5 of 5 placements, never better, never worse -- and blamed the window geometry:
at 50% overlap a target sits in four windows that carry the same evidence, and
the one it is centred in scores LOWEST. Item 41 suggested a centroid over the
agreeing cluster and did not try it. This tries it.

**Answer: 16.0 m becomes 0.1 m.** Recorded as item 42.

## Design

Five INTERIOR placements. Item 40's runs used +-32 m, which puts the target on
the grid boundary where half its footprint is off the grid; those are the wrong
test and their residual error is that truncation. These stay within +-16 m so
the truth sits at window index 1, 2 or 3 of 0..4.

Truth is the injection geometry, verified in `../2026-08-04-offset-chase/` by
differencing two focused images: the target lands at pixel `47.5 + offset`,
sub-pixel exact, and window centres are at `15.5 + 16k`.

## Result

```
 offset m |        truth |        argmax |           centroid | cluster
   +0, +0 | (2.00,2.00)  |  (3,2)  1.00  | (2.00,2.01) 0.01   | 9
  -16,-16 | (1.00,1.00)  |  (0,2)  1.00  | (1.00,1.01) 0.01   | 9
  -16, +0 | (1.00,2.00)  |  (0,3)  1.00  | (1.00,2.01) 0.01   | 10
   +0,+16 | (2.00,3.00)  |  (1,4)  1.00  | (2.00,3.01) 0.01   | 9
  +16,-16 | (3.00,1.00)  |  (2,2)  1.00  | (3.00,1.01) 0.01   | 9

  argmax   mean 1.000 windows = 16.0 m, worst 1.00
  centroid mean 0.008 windows =  0.1 m, worst 0.01
```

**argmax is wrong by exactly one window every time; the centroid is right to a
hundredth of one.** A factor of 125, and finer than the 1.0 m grid cell -- a
centre of mass over nine windows is no more limited by the window spacing than
a star centroid is by the pixel pitch.

The residual +0.01 in range is systematic across all five and is not explained.
It is a sixth of a metre and has not been chased.

## What the flag had to become

`clipped` first meant "the agreeing cluster touches the grid edge". Measured,
that fires on FOUR of these five placements -- all accurate to 0.01 -- and on
none of the failures it was meant to catch. Even restricting it to the
weight-bearing windows fires on the same four, carrying 33-55% of the weight at
the edge.

What actually biases a centroid is the TARGET being at the boundary, so half its
own footprint is off the grid. The flag now tests whether the CENTROID lies
within one window of the edge, which separates item 40's +-32 m placements from
these.

## What this does not do

**It locates; it does not detect.** The cluster is grown from a window the
caller has already chosen to believe, and on a scene where nothing is known to
move the tool still reports a null.

**One collect, one frequency, one amplitude, five placements**, all at 2 mm --
the strong end. Item 37 showed the reported window moving with amplitude, so
this needs repeating at 0.125 mm before anyone claims 0.1 m localisation
generally.

**The truth is geometric, not independent.** It rests on the offset-chase
measurement that the injection lands where asked. That was verified once, at
zero offset, sub-pixel.
