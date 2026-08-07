# Run: 2026-08-07 synthetic / blockmedian-null

**Question this run is meant to answer:** does summarising a mode's strength over
its own block cost specificity?

**This is the KILL CRITERION** for `runs/kilauea/2026-08-07-blockmedian/`,
pre-registered there at `eb0775f` §9: more than 1 of 12 motionless scenes
surviving, or injected recall below 6 of 6, reverts the change whatever the
recall arm says.

- git commit: `eb0775f`
- started:    2026-08-07T13:41Z
- host:       Darwin arm64

## Collect

Item 107's fixture, now on its fourth measurement: twelve motionless
`--clutter-vib` scenes (seeds 3-41) and six injected at 2 mm, each at 128 and
256 looks. `df` 0.0504 Hz at both counts, 128-look Nyquist 3.23 Hz.

## Commands

`stabsweep_v4.sh`, which is `../2026-08-07-refbins-null/stabsweep_v3.sh` with
only the output and fixture paths changed. Scored by
`../2026-08-07-nomination-fix-null/score.py`, item 107's rule, run against all
four result sets so the comparison is like for like.

## Result

**H3b PASSES, and improves for the first time since item 107 established it.**

| | item 107 | item 110 | item 111 | item 112 |
|---|---|---|---|---|
| motionless: report | 1 of 12 | 1 of 12 | 1 of 12 | **0 of 12** |
| motionless: reject | 7 | 6 | 8 | **9** |
| motionless: abstain | 4 | 5 | 3 | **3** |
| injected: report | 6 of 6 | 6 of 6 | 6 of 6 | **6 of 6** |

**Seed 31 is finally rejected.** It survived items 107, 110 and 111 at 0.958 Hz
on both look counts; here its 256-look answer moves to 0.151 Hz and the test
refuses it. All six injected scenes still give **0.504 Hz at both counts to
three decimals**.

The mechanism is the same one the recall arm shows: a motionless scene's
strongest block is not internally clean, so a statistic taken over the block
rather than over every nominator stops flattering it, and the frequency it leads
with at 128 looks is no longer the one it leads with at 256.

## Bounds

**0 of 12 is not a zero false-positive rate.** Twelve realisations put a wide
interval on it — one survivor would have been entirely consistent with the same
underlying rate, and three of the four measurements of this fixture found
exactly one. What is fair to say is that the rate did not rise, and that the
scene which had survived three times no longer does.

One fixture family, one operating point, one injected amplitude.
