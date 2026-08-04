# Run: 2026-08-04 giza / cubic-floor

Item 51 measured that after a quadratic fit a further cubic removed another 3.6x
of the static-scatterer artefact offline. Item 52 implemented the quadratic. This
adds the cubic.

## The basis

```
  kc = k - (N-1)/2
  q  = kc^2 - (N^2-1)/12          orthogonal to kc by parity
  c  = kc^3 - kc*(3N^2-7)/20      orthogonal to BOTH
```

**The cubic's constant is not decoration.** `c` is odd and so is `kc`, so parity
does NOT separate them; the coefficient is `sum(kc^4)/sum(kc^2)`, which for
`k = 0..N-1` is exactly `(3N^2-7)/20`. Verified to machine precision at N = 16,
64, 128 and 512. Without it the cubic stage would have failed exactly as the
uncentred quadratic did in item 52.

## Result

```
  static bright scatterer, zero motion, REL 20
    linear only                 12,060.1x
    + quadratic                    317.7x     38x
    + cubic                         70.7x     a further 4.5x, 171x in total

  2 mm signal   1,207,566 -> 1,311,807 -> 1,277,488     unchanged within 3%

      amp mm   local peak    freq          verdict
       0.015625      556   0.153           signal
       0.0078125     144   0.153           signal      <- last correct point
       0.00390625     72   0.092   THE ARTEFACT (70.7)
       0.001953125    71   0.092   THE ARTEFACT
       0.0009765625   71   0.092   THE ARTEFACT

  floor 0.0039 - 0.0078 mm, against 0.0078 - 0.0156 with the quadratic alone
```

Item 51's offline cubic predicted a further 3.6x; the search delivers 4.5x, and
`sqrt(4.5) = 2.1` predicts the floor halving, which is what it does.

## The progression, and where it stops being worth it

```
                floor mm          artefact   gain
  linear      0.0625 - 0.125      12,060.1     --
  quadratic   0.0078 - 0.0156        317.7    38x
  cubic       0.0039 - 0.0078         70.7   4.5x
```

Each stage costs another O(N^2) pass per window. The returns are falling fast --
38x then 4.5x -- so a quartic would be worth perhaps a factor of two for a fourth
pass, and is not obviously worth it. **Not added.**

## Eighteen times below the published floor, which is now a real concern

0.0078 mm zero-to-peak is 0.0055 mm RMS, against Vattulainen et al.'s smallest
confirmed 0.10 mm RMS. Item 50 had this project in agreement with that number;
item 52 put it 9x below; this puts it 18x.

The gap is not evidence of anything except that **the injected target is not like
a real one.** `rs_simulate_inject_vibrator()` writes a perfectly coherent point
scatterer with analytically exact phase, present in every pulse, at a frequency
known in advance. A real structure decorrelates, is aspect-dependent, and is
never a point. Items 24-25 measured what aspect dependence alone does to this
estimator, and it was not small.

The defensible claim is narrow: **the instrument's own floor on this collect is
0.0055 mm RMS.** Anything about real structures needs a real structure.
