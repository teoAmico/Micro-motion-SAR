# ICEYE dwell truncation, and the check that could not see it

**Question:** item 36 screened ICEYE's Houston `dwell-precise` collect and it
FAILED the observable band for a 2 Hz target -- because of the 15.345 s dwell,
not the collect. Item 4's remedy is to truncate with `--max-pulses`. Does it
work?

**Answer: yes, and `validate` could not tell you so.** Item 44.

## The arithmetic, from the metadata alone

The observable band is set by the SUB-APERTURE duration, `f_max = 1/(2*alpha*T)`,
so shortening the dwell opens it:

```
 dwell s   pulses  f_max @3.6%   df = 1/T     az res
   15.35   100802       0.905Hz    0.0652Hz      1.00x   <- fails 2 Hz
    6.14    40320       2.263Hz    0.1629Hz      2.50x   <- passes
    4.60    30240       3.017Hz    0.2172Hz      3.33x
```

**Item 4 named the band and not the price.** Frequency resolution IS `1/T_dwell`,
and azimuth resolution scales the same way, so reaching 2 Hz costs 2.5x on both.
The band and the resolution trade directly against each other.

## Measured, end to end

```
info  --max-pulses 40320     pulses 40320, dwell 6.138 s     <- matches 6.14 predicted

validate, FULL dwell
  FAIL  observable band   averaging ceiling 0.905 Hz, sampling 1.509 Hz
                          "The target is ABOVE the band"
  VERDICT: FAIL

validate --max-pulses 40320
  PASS  metadata          dwell 6.138 s, 40320 pulses
  PASS  frequency resolution  0.1629 Hz from a 6.138 s dwell; 2.000 Hz is 12.3 bins
  PASS  observable band   averaging ceiling 2.263 Hz, sampling 3.771 Hz
  VERDICT: WARN
```

Every figure matches the prediction to the digit: 2.263 Hz and 0.1629 Hz.

## The defect this uncovered

**`validate` accepted `--max-pulses` and ignored it.** Its reader options were
`{ .rbin_window = 8 }` with no `max_pulses` and no `pulse_first`, so it read the
whole collect and answered about the untruncated dwell -- reporting FAIL for a
configuration that passes.

That is the worst possible place for it. Truncation is the documented remedy for
exactly this failure, and `validate` is the command a caller runs to decide
whether the remedy will work. It said no. Fixed; both flags are now honoured and
appear in the usage line.

**No test covers it**, and cannot as things stand: `validate` reads real CPHD
only and refuses `sim_cphd` output, which is a known gap recorded in the
USER_GUIDE gotchas. The fix is verified against the real ICEYE product above.

## ICEYE signal read for the first time

`houston_focus.png` -- 1024 x 1024 at 2 m from 40320 pulses, 4 min 26 s. A
recognisable urban scene: street grid, building blocks, a circular structure and
large parking areas. **The `CI4` sample decode works on this vendor**, which item
36 listed as unverified. The swath is a curved strip because `--rbins 4096` reads
a window of the collect's 48000 range bins.

Reading the whole product needs 38.7 GB against this machine's 25.8 GB, and the
reader says so and names the remedy rather than failing at the allocation.
