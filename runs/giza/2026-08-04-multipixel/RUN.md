# Multi-pixel combination: it does not work, and the literature says why

Item 63 identified the missing factor: the phase estimator reads ONE pixel per
window and discards the other 1023 of a 32x32. Combining the K brightest should
give sqrt(K) if they share the motion — 3x at K = 9, exactly the gap.

**It gives nothing. The artefact grows with K.**

```
 pixels   artefact  signal @0.03125mm  signal/artefact  vs K=1
      1       70.7             2310.4             32.7   1.00x
      4      181.9             4340.7             23.9   0.73x
      9      159.9             4363.1             27.3   0.83x

  sqrt(K) predicts 2.00x at K=4 and 3.00x at K=9
```

The signal does rise — 2310 to 4341 — but the artefact rises faster, from 70.7 to
182. The ratio that decides detection gets WORSE.

## Why, and it was findable before writing the code

The literature has done this properly and for fifteen years. **SqueeSAR**
(Ferretti et al. 2011) and the phase-linking family combine pixels in exactly
this situation, and they differ from what was implemented here in two ways that
turn out to be the whole result:

**1. Which pixels.** SqueeSAR selects STATISTICALLY HOMOGENEOUS PIXELS by a
two-sample Kolmogorov–Smirnov test on their amplitude distributions — pixels
drawn from the same scattering population. This implementation took the K
BRIGHTEST, which is a different set and generally the wrong one: the second and
third brightest pixels in a window are usually *different scatterers*, each with
its own sub-pixel offset, its own carrier, and its own carrier residual.
Averaging them adds artefacts rather than averaging noise down. That is precisely
what the numbers show.

**2. How to combine.** Phase linking estimates the phase history from the sample
COVARIANCE MATRIX by maximum likelihood, not by averaging phases estimated
independently per pixel. Independent estimation throws away the cross-terms that
carry most of the information.

## This project already had the right estimator

`src/core/phaselink.c` implements the maximum-likelihood phase estimate over a
stack — the coherence matrix and the fixed-point iteration
`x_n <- exp(j*arg(sum_m Gamma_nm x_m))`, which is the standard phase-linking
solver. It was written for the split-band route and never applied to the
sub-aperture stack.

So the correct experiment is not the one run here. It is: select statistically
homogeneous pixels within the window, form the covariance across sub-looks, and
hand it to the existing linker.

## What this measurement is worth

It bounds the naive version, which is worth knowing because the naive version is
what anyone would try first, and it looked like free sqrt(K).

It does not test item 15's precondition, which was the stated reason for trying:
whether several pixels share the motion. The brightest-K construction cannot
answer that, because it does not select pixels that share a scatterer.

`--pixels` is left in place, defaulting to 1, which reproduces every earlier
measurement exactly.
