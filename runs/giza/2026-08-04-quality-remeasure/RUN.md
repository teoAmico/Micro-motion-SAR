# Run: 2026-08-04 giza / quality-remeasure

**Question:** item 46 changed the phase route's `quality` from amplitude
stability to spatial dominance. That quantity feeds the shared gate every
selection policy applies, and item 31's whole finding was that a gate discarded
the true positive. Item 37's floored amplitude sweep was measured with the old
definition. **Does any recorded conclusion move?**

**Answer: none. All seven points reproduce exactly.**

```
                   item 37 recorded    re-measured 2026-08-04
  2.0    mm        38.5, 0.163 Hz      38.5, 0.163 Hz
  1.0    mm        40.7, 0.163 Hz      40.7, 0.163 Hz
  0.5    mm        41.6, 0.163 Hz      41.6, 0.163 Hz
  0.25   mm        42.9, 0.163 Hz      42.9, 0.163 Hz
  0.125  mm        45.3, 0.163 Hz      45.3, 0.163 Hz
  0.0625 mm        48.9, 0.163 Hz      48.9, 0.163 Hz
  uninjected       17.9, 0.130 Hz      17.9, 0.130 Hz
```

Only `quality` itself differs -- the control reads 0.704 where it read 0.445 --
which is the point of the change and not a result.

## A comparison I got wrong first

The control initially came back at 22.1 rather than 17.9, which looked like the
change moving a recorded number. It was not: item 37's control was run at the
DEFAULT floor while this sweep used `--fmin 0.098`, and prominence is measured
against the admissible band, so a different floor gives a different denominator.
Repeating the control at the original floor returns 17.9 exactly.

**Prominence is not comparable across `--fmin` settings.** That is not recorded
anywhere else and it is easy to trip over, because the reported window and
frequency are unchanged while the number moves.

## Why nothing moved, stated so it is not read as luck

The shared gate is `quality >= 0.5 * q_max`, which is relative. Under the old
definition the injected window scored close to the scene maximum on Giza --
desert clutter is amplitude-stable and so is a 2 mm scatterer at this
amplitude -- so it passed either way. Item 45's ICEYE failure needed an urban
scene whose brightest windows were much more stable than the vibrating target;
Giza has no such contrast. The change fixes a failure Giza never exhibited.

## Local peak, measured on the same runs

```
  uninjected control   0.163 Hz in window 13, 39.2x its own neighbourhood
  2 mm injected        0.163 Hz in window 17, 431.8x
```

The control's 39.2x is a look-elsewhere maximum over every window and bin, and
it lands on 0.163 Hz by coincidence -- one bin of some sixty. It is the number a
detection has to clear, and the injected run clears it by 11x.
