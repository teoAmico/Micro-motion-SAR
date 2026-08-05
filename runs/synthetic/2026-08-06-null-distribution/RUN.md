# The empirical null distribution of prominence on motionless clutter

## Question

Item 96 measured a 100% false-positive rate on motionless scenes. Item 98 named
the suspect: the look-elsewhere effect, which is item 1, this project's oldest
open entry. Does a multiplicity correction explain the false positives?

## Commands

`nulldist.sh` -- 12 motionless scenes, `--estimator phase --n 128 --overlap 0`,
every `_windows.csv` kept. `null_prominence.csv` is the pooled result: 588
per-window prominences.

## Result: no

The modelled family-wise threshold over 49 windows x 62 bins is prominence
**11.01**. The twelve scenes report **20.1 to 36.9** -- all twelve clear it.

The empirical null explains why the model is useless here:

```
  measured   mean 14.21  median 12.53  p95 27.06  p99 32.86  max 36.87
  Exp(1)     mean  1.00  median  0.69  p95  3.00  p99  4.61
```

Fourteen times too large in the mean. Since prominence is the peak over the mean
of 62 bins, a prominence of 14 means **one bin holds 23% of the band's power on a
scene where nothing moves** -- a coherent line, not a chance maximum. Item 63's
per-realisation residual carrier is the mechanism.

Both candidate explanations for item 96 are now eliminated: the frequency search
costs ~20% in SNR (item 98) and multiplicity cannot bridge 11 to 37. What remains
is that the artefact is real signal in the tracked series, so the carrier removal
(items 51-53, 63) is where to attack it -- a different half of the codebase from
everything items 91-98 touched.

Full write-up in `docs/FOLLOW-UPS.md` item 99.
