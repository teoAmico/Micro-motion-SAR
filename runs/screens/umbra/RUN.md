# Umbra: the reader verified against the vendor's own image

`2023-09-12-02-36-33_UMBRA-04_CPHD.cphd`, Panama Canal, 0.84 GB.

**Question:** `DATASETS.md` recorded Umbra as untried with a named risk — item 3's
SGN override is keyed on `CollectorName`, so a non-Capella product takes the
standard branch, and if Umbra mislabelled SGN the same way Capella does, every
image would come back mirrored in range.

**Answer: it reads correctly, and needs no override.**

```
CollectorName      Umbra-04
SGN                -1          <- declared, and honestly
SignalArrayFormat  CF8         <- float32, where Capella ships CI4
NumBytesPVP        240         <- 30 words, where Capella uses 33
AmpSF              ABSENT from the PVP
9693 vectors x 7218 samples, SPOTLIGHT, dwell 1.991 s, 599.7 km
```

Umbra declares `SGN = -1`, which is exactly what Capella's override *produces*.
Both therefore take the inverse FX-to-delay transform — Umbra by being labelled
correctly, Capella by exception. 9680 usable pulses of 9693; focused in 17 s.

## The check item 3 demands

Item 3 is explicit that only the imagery settles a transform direction.
`umbra_panama_wide.png` is this reader's 1024 x 1024 at 2 m; `*_GEC.tif` from the
same acquisition folder is Umbra's own geocoded product.

Both show the canal running upper-right to lower-left, the lock chambers at
lower-centre, the light-toned basins **east** of the channel and the built-up
strip **west** of it. A range mirror would have swapped those sides. **Not
mirrored.**

That is the first evidence that keying the override on the collector rather than
inverting globally was right: item 3 argued a conformant product from anyone else
should still read correctly, and one now does.

**The check is qualitative** — two images at different scales and projections,
compared by eye on distinctive asymmetric features. A correlation against a
resampled GEC would be stronger and has not been done.

## Not measurement-verified

It reads and it focuses. Nothing has tracked, taken a spectrum or adjudicated on
Umbra data. The 1.991 s dwell gives 0.50 Hz frequency resolution, which is coarse
for the 1–3 Hz band this project targets; Umbra's longer collects were not
surveyed.
