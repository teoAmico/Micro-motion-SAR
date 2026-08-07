# Run: 2026-08-07 synthetic / refbins-null

**Question this run is meant to answer:** does narrowing `rs_local_ratio()`'s
neighbourhood, which removes a measured band-edge bias, cost specificity?

**This is the KILL CRITERION** for `runs/kilauea/2026-08-07-refbins/`,
pre-registered there at `8885312` §9: if more than 1 of 12 motionless scenes
survives the stability test, or injected recall falls below 6 of 6, the narrowing
is reverted and the bias goes back into `docs/CODE-REVIEW.md` as unfixed.

- git commit: `8885312`
- started:    2026-08-07T12:07Z
- host:       Darwin arm64

## Collect

Item 107's fixture, unchanged through three measurements now: twelve motionless
`--clutter-vib` scenes (seeds 3-41) and six injected at 2 mm, each at 128 and 256
looks. `df` 0.0504 Hz at both counts, so the 128-look Nyquist is 3.23 Hz and a
256-look answer above it is NOT COMPARABLE rather than unstable.

## Commands

`stabsweep_v3.sh`, which is `../2026-08-07-nomination-fix-null/stabsweep_v2.sh`
with only the output and fixture paths changed — the fixture paths so it could
run beside the Kilauea arm without either overwriting the other's files. Scored
by `../2026-08-07-nomination-fix-null/score.py`, item 107's rule, run against all
three result sets so the comparison is like for like.

## Result

**H3b PASSES.**

| | item 107 (`f6ad9f4`) | item 110 (`fce5869`) | item 111 (`8885312`) |
|---|---|---|---|
| motionless: report | **1 of 12** | **1 of 12** | **1 of 12** |
| motionless: reject | 7 | 6 | **8** |
| motionless: abstain | 4 | 5 | **3** |
| injected: report | **6 of 6** | **6 of 6** | **6 of 6** |

All six injected scenes still give **0.504 Hz at both look counts to three
decimals**, and the surviving static is still **seed 31 at 0.958 Hz**, as in item
110.

**The visible gain is in the abstentions, not the rate.** They fall from 5 to 3,
because fewer answers land on the band-edge bins that the old neighbourhood
over-selected and that put a 256-look answer above the 128-look Nyquist. The test
now decides on 9 of 12 motionless scenes where it decided on 7. A discriminator
that abstains less at the same false-positive rate is strictly better, and this
is the arm that shows it.

## Bounds

One fixture family, one operating point, twelve realisations. The rate is 1 in 12
with a wide interval, and it has now been 1 in 12 three times running with
different survivors and different mechanisms behind them — which is a statement
about the fixture as much as about the method.
