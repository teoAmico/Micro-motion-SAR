# Run: 2026-08-04 giza / centroid-lowamp

**Question:** item 42 claimed 0.1 m localisation from five placements **all at
2 mm**, the strong end of the sweep. Item 37 had already shown the reported
window moving with amplitude at a fixed position, so that is where the claim was
most likely to break. Same five interior placements at 0.125 mm, sixteen times
weaker.

**Answer: it breaks gracefully and predictably. 0.13 m becomes 2.24 m.**
Recorded as item 43. Item 42's number is not wrong; it is amplitude-dependent
and was quoted as if it were not.

## Result

```
 offset m |  truth | 2 mm centroid       | 0.125 mm centroid | argmax @0.125
   +0, +0 | (2,2)  | (2.002,2.008) 0.008 | (2.04,2.14) 0.140 | win8  (1,3) 1
  -16,-16 | (1,1)  | (1.002,1.009) 0.009 | (1.04,1.16) 0.160 | win2  (0,2) 1
  -16, +0 | (1,2)  | (1.002,2.008) 0.008 | (1.04,2.13) 0.130 | win3  (0,3) 1
   +0,+16 | (2,3)  | (2.002,3.008) 0.008 | (2.04,3.13) 0.130 | win9  (1,4) 1
  +16,-16 | (3,1)  | (3.002,1.009) 0.009 | (3.03,1.14) 0.140 | win12 (2,2) 1

  centroid @ 2.000 mm : 0.0084 windows = 0.13 m
  centroid @ 0.125 mm : 0.1400 windows = 2.24 m
  argmax   @ 0.125 mm : 1.0000 windows = 16.00 m
```

**A 16x weaker signal costs 17x the error.** That is linear in 1/amplitude to
within the measurement, which says the error is background-limited rather than a
fixed bias: what matters is the target's excess prominence against the
background gradient, and that ratio scales with amplitude.

**argmax stays at exactly 1.000 windows at both amplitudes**, because it is
quantised to an integer index and always lands on a neighbour. The centroid is
7.1x better at the weak end and 125x better at the strong one.

## The +0.01 bias, chased

Item 42 left a systematic `+0.01` in range unexplained. It is not an
off-by-something -- a half-window or half-pixel convention error would give
exactly 0.5.

The cluster is a symmetric 3x3 block centred on the truth, and its weights are
uniform to about 1.5 percent with a slight MONOTONE GRADIENT toward higher
indices in both axes:

```
offset 0,0   scene median 14.84, cluster 9 windows
  azimuth marginal weight:  1: 71.72   2: 71.72   3: 72.20
  range   marginal weight:  1: 71.05   2: 71.73   3: 72.85
```

That gradient is item 41's 1.5 percent spread -- the one that makes several
windows tracking the same scatterer score almost but not exactly alike -- and
the centroid inherits it. Range has the steeper gradient, which is why the bias
shows there. At 0.125 mm the same bias appears at `+0.13`, seventeen times
larger, exactly as the background-limited reading predicts.

## Uniform weighting was tried and is worse

If the cluster is symmetric about the target, weighting every member equally is
EXACT -- and it is, on four of the five:

```
  excess-weighted  0.008  0.009  0.008  0.008  0.009   mean 0.0083 = 0.13 m
  uniform          0.000  0.000  0.200  0.000  0.000   mean 0.0400 = 0.64 m
```

The failure is the one cluster that came out asymmetric, at 10 windows rather
than 9, where uniform weighting is dragged 0.2 windows. Cluster symmetry is not
something the method can rely on, and a background window that happens to agree
would drag a uniform centroid while contributing nothing to an excess-weighted
one. **The 0.13 m mean is bought with robustness and the trade is worth it**, but
uniform weighting being exact on symmetric clusters is why the residual is a
weighting artefact rather than a geometric one.
