# Run: 2026-08-04 giza / blind-localisation

**Question:** the README has long said that "automatically locating an unknown
vibrating target among hundreds of windows remains untested". It was untested
because it could not be tested: `--inject-vib` always landed on the grid origin,
so every experiment put the target in the same place and "which window" was
never asked. `--inject-at DX,DY` moves it.

**Answer: the reported window follows the target, 4 of 5 exactly, for both
policies.** Recorded as item 40.

## Design

96 m grid, 1.0 m cells, 32-pixel windows on a 16-pixel stride, so the window
grid is 5x5 and one window is 16 m. Five placements, none at the origin, 2 mm at
0.163 Hz, everything else as item 37's floored sweep.

**Ground truth is the injection geometry, not the data.** An offset of
`(dx, dy)` metres should move the target by `(dx/16, dy/16)` windows from where
the same injection at zero offset lands, which is window 8 = `(iaz 1, irg 3)`.
Scored on the EXACT window.

An earlier scoring used "the window with the largest probe prominence at
0.163 Hz" as truth and gave a different answer. It was wrong: the aliasing
ghosts carry the injected frequency too, so that statistic finds a ghost as
readily as the target. It is recorded here because it looked reasonable.

## Result

```
 offset m |          expected |      best_window |       scene null
 -32, -32 | (0,1)  CLIPPED    | win  1 (0,1) hit | win  1 (0,1) hit
 -16, +16 | (0,4)             | win  4 (0,4) hit | win  9 (1,4) MISS
  +0, -32 | (1,1)             | win  6 (1,1) hit | win  6 (1,1) hit
 +16, +32 | (2,4)  CLIPPED    | win 24 (4,4) MISS| win 14 (2,4) hit
 +32,  +0 | (3,3)             | win 18 (3,3) hit | win 18 (3,3) hit

EXACT window: best_window 4/5, scene-derived null 4/5
```

Chance of naming one window in 25 is 4 percent. Both policies are far above it,
and they fail on different placements.

## Four things this does not establish

**The absolute position is unverified.** The reference used above is empirical:
a zero-offset injection lands in window `(1,3)` where the grid's geometric
centre is `(2,2)`. That is a systematic offset of one window in azimuth and one
in range -- 16 m each way -- between the ENU frame `--inject-at` addresses and
where the target actually focuses. All five placements share it, so RELATIVE
displacement is tested and absolute placement is not. The cause is not known and
is worth chasing: a height assumption, a geolocation offset, or an off-by-one in
the window-centre convention would all look like this.

**Two placements clipped.** At +-32 m the target falls outside the 5x5 window
grid and the expected window is the clipped edge, which is a weaker test than an
interior one. That is a design error in the placements, not a finding; a repeat
should stay within +-16 m.

**Five points, one collect, one frequency, one amplitude.** 2 mm is the strong
end of the sweep. Nothing here says localisation survives at 0.125 mm, and item
37 showed the reported window moving with amplitude even at a fixed position.

**This is not detection.** The policies were not told WHERE, but they were
searching a scene that certainly contains a loud injected target, and the run
that matters -- a scene where nothing is known to move -- still returns a null.
Locating a target that is known to exist is a strictly easier problem than
deciding whether one does.
