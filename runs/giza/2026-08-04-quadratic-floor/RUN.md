# Run: 2026-08-04 giza / quadratic-floor

Item 51 diagnosed the sensitivity floor as a quadratic phase residual the linear
carrier removal leaves behind, and predicted that removing it would lower the
floor. This is the implementation measured.

## The artefact

```
  static bright scatterer, zero injected motion, REL 20
    linear carrier only              12,060.1x at 0.092 Hz
    quadratic, UNCENTRED basis       10,697.4x     -- 1.13x, a failure
    quadratic, CENTRED basis            317.7x     -- 38x
```

## The floor

```
      amp mm   local peak    freq          verdict
       2.0     1,311,807   0.153           signal
       0.125      32,502   0.153           signal
       0.0625      7,773   0.153           signal
       0.03125     1,866   0.153           signal
       0.015625      472   0.153           signal      <- last correct point
       0.0078125     348   0.092  THE ARTEFACT (317.7)
       0.00390625    347   0.092  THE ARTEFACT

  floor: 0.0078 - 0.0156 mm, against 0.0625 - 0.125 mm before
```

**An eight-fold improvement**, and the ratio at 0.015625 mm is 472 against the
artefact's 317.7 -- a margin of 1.49, so the bracket is tight rather than
generous.

Item 51 predicted "toward 0.003 mm" from the offline fit's 2,000x. The phasor
search achieves 38x, not 2,000x, and sqrt(38) = 6.2 gives 0.125/6.2 = 0.020 mm
against a measured 0.0156. **The reasoning was right and the number it was
applied to was the wrong one** -- an offline least-squares fit to the
displacement is not what maximising the phasor sum does, and the gap between
2,000x and 38x is that difference.

## The bug worth keeping

The first implementation used `nu*k + mu*k*k` directly and reduced the artefact
by 1.13x. `k` and `k*k` are strongly correlated over a finite record, so adding
`mu*k*k` shifts the mean rate by about `mu*N` -- at the curvature actually
present, 0.04 rad per look against a coarse step of 0.049 -- and scanning `mu` at
the linear-only `nu` made every trial worse. The search returned `mu = 0` every
time.

Centring the basis (`kc = k - (N-1)/2`, `q = kc^2 - (N^2-1)/12`) makes the two
terms orthogonal over the record, so a staged search is valid. That is the whole
difference between 1.13x and 38x.

## Against the literature, and a caution

0.0156 mm zero-to-peak is 0.011 mm RMS, against Vattulainen et al.'s smallest
confirmed 0.10 mm RMS -- **nine times below the published floor**, where item 50
had just brought this project into agreement with it.

That is a reason for caution, not celebration. Their 0.10 mm is a REAL vibrating
object with synchronous ground truth, carrying every real-world effect. Ours is a
synthetic tone injected into real clutter as a perfectly coherent point target,
brighter and cleaner than any real structure, at a frequency known in advance.
The correct reading is that the INSTRUMENT's floor is now 0.011 mm RMS on this
collect, and that says nothing about what a real structure would give.
