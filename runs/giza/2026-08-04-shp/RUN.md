# SqueeSAR done properly: SHP selection plus the existing phase linker

Item 64 found that combining the K brightest pixels fails, and that the
literature's method — statistically homogeneous pixel selection by a two-sample
KS test, then maximum-likelihood phase linking over the covariance — was already
half-implemented here in `phaselink.c`. This does it properly.

**It is worse than the naive version, and much worse than one pixel.**

```
 pixels   artefact  signal @0.03125mm  signal/artefact
      1       70.7             2310.4             32.7
      9     1326.3             5391.8              4.1
     25     1326.3             5391.8              4.1

  item 64's brightest-K, for comparison:  K=9 gave 27.3
```

The signal does rise, 2310 to 5392 — phase linking is genuinely extracting more
from the stack. But the artefact rises nineteen-fold, from 70.7 to 1326, and the
ratio that decides detection falls eightfold.

K=25 is identical to K=9, so the KS test is selecting roughly nine homogeneous
pixels per window and the cap is not binding.

## Why, and it is the same argument as item 47

Averaging beats down noise that is INDEPENDENT between the things averaged. The
carrier residual is not independent between neighbouring pixels: they sit at
similar sub-pixel offsets, so they have similar carriers and similar residuals
after the cubic fit.

**Statistically homogeneous pixels are homogeneous in the artefact too.** The KS
test selects pixels drawn from the same scattering population, which is exactly
the set that shares a residual — and the phase linker, being a maximum-likelihood
estimator over the whole covariance, then estimates that shared residual very
precisely. It amplifies the common-mode term because it is doing its job.

That is item 47's argument in a different place: a null, a normalisation, or an
average only helps against noise that varies across the things being combined.

## What this closes

Items 62 and 63 identified a missing factor of four and named multi-pixel
combination as the one untried lever with enough headroom. **It is not a lever.**
Both the naive form and the literature's form make detection worse on this data,
for a reason that is structural rather than a tuning failure.

So the remaining routes to a lower floor are the ones already measured and
insufficient: finer cells 1.4x (item 63), a quartic carrier term perhaps 1.4x
(item 53's trend). Roughly 2x against the 4x needed, with nothing else identified.

The honest position is that **this instrument's floor is within about a factor of
two of what this approach can reach**, and closing the gap to ordinary ground
motion needs something not yet identified — not more of what is here.

`--pixels` stays at its default of 1, which reproduces every earlier measurement.
