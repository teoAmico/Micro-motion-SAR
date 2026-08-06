# Predicting the floor from an uninjected run, on real Kilauea data

## Question

Item 101 established that the tracked series' phase noise sets the floor, with
the arithmetic `sd * lambda/(4*pi) * sqrt(2/N)`. That is a prediction obtainable
from an UNINJECTED run. What is it on real data, and does the Kilauea
correlation test have a chance?

## Commands

```sh
./build/micromotion mmotion --cphd <collect> --estimator phase \
    --n 128 --overlap 0 --size 256 --cell 0.5 --win 32 --upsample 200 \
    --coherence 0 --max-pulses <from kilauea_trunc.tsv> --shifts <out>.csv
```

Three complete collects, at the seconds-matched truncation.

## Result

| collect | circular sd | coherence | mm/look | predicted floor |
|---|---|---|---|---|
| C10 2024-06-09 | 1.716 | 0.381 | 4.233 | 0.529 mm |
| C10 2024-06-11 | 1.709 | 0.382 | 4.215 | 0.527 mm |
| C14 2024-06-10 | 1.686 | 0.387 | 4.159 | 0.520 mm |

**The Kilauea truth is 0.137 to 1.728 um. The floor is 529 um -- the best scene
is 306x below it.** The correlation test cannot work: below threshold the
estimator decouples from the truth (item 82), so the reported amplitude is noise
and the correlation is zero by construction.

Real data is 1.8x WORSE than the synthetic fixture, not better. The cause is in
the same run's output: amplitude dispersion median 0.567 and 0 of 225 windows
meeting D_A <= 0.25, so Kilauea lava has no persistent scatterers and item 15's
precondition is unmet scene-wide -- item 19's Giza finding on a second scene.

The "real collect's 0.85" in item 12f is the CORRELATION peak, not the sub-look
PHASE coherence. They are different quantities and I conflated them.

Full write-up in `docs/FOLLOW-UPS.md` item 102.
