# Run: 2026-08-04 giza / offset-chase

**Question:** item 40 found a zero-offset injection landing one window from the
grid's geometric centre, 16 m in each axis, and could not say why. A height
assumption, a geolocation offset and an off-by-one in the window convention all
look identical from the window indices alone.

**Answer: none of those. The geometry is exact.** The reported window is chosen
from a cluster of overlapping windows that all track the same scatterer, and the
correctly-centred one scores LOWEST of the cluster. Recorded as item 41, which
also withdraws item 40's headline number.

## Measuring where the target actually is

Two `focus` runs over the same collect and grid, identical but for
`--inject-vib`, differenced. This is independent of tracking, windows, spectra
and every selection policy.

```
max change 1.584e+04 at azimuth px 47, range px 48
grid origin sits at pixel 47.5 in both axes    (focus.c:193, (n-1)/2 with n=96)
  -> target is -0.5 m azimuth, +0.5 m range from the origin
```

**Sub-pixel exact.** The origin lies between pixels 47 and 48, and the target's
energy straddles them. `--inject-at` addresses the frame it claims to, the
backprojector puts the scatterer where asked, and `rs_grid_t` and the window
convention agree: window centres are at pixel `15.5 + 16k`, so window (2,2) is
centred on 47.5 -- the origin -- and is the target's window.

## Why the reported window is not that one

Per-window prominence for the zero-offset 2 mm run, with the target's true
window marked:

```
      irg=0     1       2       3       4
iaz=0  12.0    6.6     6.6    14.0     5.5
iaz=1  14.8   38.6    38.6    39.1     5.5
iaz=2  14.8   38.6   [38.6]   39.1    17.6      <- (2,2) is the target's window
iaz=3   8.8   38.5    39.1    39.1    15.5
iaz=4   8.8    4.7     8.6     8.6     4.9
```

Six windows sit between 38.5 and 39.1 -- a spread of **1.5 percent** -- and
thirteen pairs of windows are BIT-IDENTICAL across `dominant_hz`, `prominence`,
`quality` and `probe_psd` at once. That is not overlap similarity. At `win 32`
on a `stride 16` these windows overlap by half, they all contain the same
injected dominant scatterer, and the phase estimator tracks precisely that: one
dominant scatterer's phase. Several windows tracking the same physical
scatterer return the same series, so they return the same everything.

**The correctly-centred window is the LOWEST of the cluster**, 38.56 against
39.14. Whichever neighbour wins is decided by a 1.5 percent difference, and item
37 already recorded that winner moving with amplitude.

The image itself is not the cause: all 96 azimuth rows and 96 range columns of
the focused scene are distinct, and there is no periodicity at any shift.

## What it does to item 40

Item 40 scored localisation against the window a zero-offset injection reports.
That window is itself one off. Rescored against the geometry measured here:

```
 offset m |   target px   | true win |  best_window   | scene null
 -32, -32 | (15.5, 15.5)  | (0,0)    | (0,1)  d=1     | (0,1)  d=1
 -16, +16 | (31.5, 63.5)  | (1,3)    | (0,4)  d=1     | (1,4)  d=1
  +0, -32 | (47.5, 15.5)  | (2,0)    | (1,1)  d=1     | (1,1)  d=1
 +16, +32 | (63.5, 79.5)  | (3,4)    | (4,4)  d=1     | (2,4)  d=1
 +32,  +0 | (79.5, 47.5)  | (4,2)    | (3,3)  d=1     | (3,3)  d=1

EXACT: 0/5 and 0/5.   WITHIN ONE WINDOW: 5/5 and 5/5.
```

**Never exact, never worse than one window.** Item 40's "4 of 5 exact" is
withdrawn. The true statement is that localisation is good to one window, 16 m
here, and systematically not better.

## A harness mistake, recorded because this project keeps making it

The first attempt built the focus command with `EXTRA="--inject-vib ..."` and
passed `$EXTRA` unquoted. **zsh does not word-split unquoted parameters**, so it
arrived as one argument, `--inject-vib` was never matched, and the differenced
images showed `max |difference| = 0`. Read carelessly that is a dramatic finding
-- "the injection never reaches focus". It was the harness. `CLAUDE.md` documents
this exact trap, from the time it cost a wrong diagnosis about `--max-pulses`.
