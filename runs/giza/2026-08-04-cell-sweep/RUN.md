# Where a factor of four would come from, and two places it does not

Item 62 reframed the data problem: the floor sits 3–4x above ordinary ground
motion, and closing that gap turns all 315 synchronised hits into candidates.
Two routes tested here.

## 1. Finer grid cells — real, and only half of one factor

The carrier the phase estimator must remove is, from `microm.c:623`,
`(4*pi/lambda) * dX * dx / R` where **`dx` is the scatterer's offset from its
pixel centre**. The Giza grid uses 1.0 m cells on a collect whose azimuth
resolution is 0.051 m — twenty times coarser — so a scatterer can sit half a
metre from its pixel centre, and that offset IS the carrier.

```
  cell 1.000 m   worst offset 0.5000 m   carrier 0.553 rad/look   11.3 cycles
  cell 0.250 m                0.1250 m           0.138            2.8
  cell 0.125 m                0.0625 m           0.069            1.4
```

The measured carrier on Giza is 1.1–1.9 rad/look, matching the 1.0 m cell.
Zero-amplitude artefact, same compute (96 cells either way):

```
  cell 1.000 m   artefact  70.7x
  cell 0.250 m   artefact  36.3x     <- halved
  cell 0.125 m   artefact 159.0x     <- WORSE, and the reason matters
```

**Not monotone.** 0.25 m halves the artefact — a factor of 1.4 in the amplitude
floor, since the floor goes as the square root. 0.125 m is four times worse than
that.

The likely reason is scene extent, not sampling: at 96 cells, 0.125 m spans a
12 m patch and a 32-pixel window is 4 m across. Item 15's precondition is one
dominant scatterer per sub-look resolution CELL, and a 4 m window holds too
little scene to satisfy the surrounding statistics the estimator leans on. That
is a hypothesis; what is measured is the non-monotonicity.

**So finer cells give 1.4x of the 4x needed, and only at one setting.** Worth
taking and not sufficient.

## 2. Umbra — no, and the arithmetic is item 58's

Umbra's open archive holds **4048 CPHD products** against Capella's 707
spotlight, and its resolution is finer. Neither is what this measurement needs.
Dwell, sampled from 250 of them:

```
   median             3.50 s
   90th percentile    7.62 s
   maximum           23.50 s

   dwell >= 10 s:  2.9%  ->  ~116 of 4048
   dwell >= 15 s:  0.4%  ->  ~ 16
   dwell >= 30 s:  0.0%  ->     0
```

Against Capella spotlight: **median 22.5 s, 646 collects at >= 15 s, 156 at
>= 30 s.**

Umbra's median 3.5 s gives `df` = 0.29 Hz, coarser than most of the 0.3–3 Hz
band this project targets, and a 128-look stack over 3.5 s gives 27 ms per look.
**Umbra is disqualified by exactly the arithmetic that disqualified stripmap in
item 58** — per-target observation time in seconds — and having six times the
products does not change it.

Item 36 noted the 1.99 s Panama collect was "coarse" without quantifying. This
quantifies it and generalises it to the archive: Umbra is built for resolution
and revisit, not for staring.

## What is left for the factor of four

Finer cells give 1.4x. The carrier terms gave 38x then 4.5x and a quartic is
worth about 2x in artefact, so 1.4x in amplitude. Together that is roughly 2x,
not 4x.

The untried lever is the one the estimator has never used: it reads **ONE pixel
per window** and discards the other 1023. Combining the K brightest coherently
would give up to sqrt(K) if they share the motion — 3x for K=9, which is the
missing factor. Whether they do share it is exactly item 15's precondition, and
it has never been tested.
