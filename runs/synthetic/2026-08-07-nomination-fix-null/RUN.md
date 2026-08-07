# Run: 2026-08-07 synthetic / nomination-fix-null

**Question this run is meant to answer:** Item 110 relaxes the modal set's
admission and changes its ranking to recover a localised target. Does that buy
recall with specificity?

**This is the KILL CRITERION** for `runs/kilauea/2026-08-07-nomination-fix/`,
pre-registered there at `fce5869` §9: if more than 1 of 12 motionless scenes
survives the stability test, the ranking change is reverted whatever the recall
arm says.

- git commit: `fce5869`
- started:    2026-08-07T10:20Z
- host:       Darwin arm64

## Collect

Item 107's fixture, unchanged: twelve motionless `--clutter-vib` scenes (seeds
3-41) and six injected at 2 mm, each processed at 128 and 256 looks with nothing
else varied. `df` is 0.0504 Hz at both counts because the dwell is fixed, so the
128-look Nyquist is 3.23 Hz and anything above it at 256 looks is NOT COMPARABLE
rather than unstable.

## Commands

`stabsweep_v2.sh`, which is `../2026-08-07-look-stabilization/stabsweep.sh` with
only the scratchpad path changed. Scored by `score.py`, which applies item 107's
rule and is run against the OLD results too so the comparison is like for like.

```sh
./runs/synthetic/2026-08-07-nomination-fix-null/stabsweep_v2.sh
python3 runs/synthetic/2026-08-07-nomination-fix-null/score.py stab_results_v2.txt 19.8413
```

## Result

**H3b PASSES. The false-positive rate is unchanged and recall is unchanged.**

| | item 107 (`f6ad9f4`) | item 110 (`fce5869`) |
|---|---|---|
| motionless: report | **1 of 12** | **1 of 12** |
| motionless: reject | 7 | 6 |
| motionless: abstain | 4 | 5 |
| injected: report | **6 of 6** | **6 of 6** |

All six injected scenes still give **0.504 Hz at both look counts, to three
decimals**, which is the figure item 107 is quoted at.

**The surviving static is a different scene.** Item 107's was seed 17 at
1.210 Hz; here it is seed 31 at 0.958 Hz. That is item 96 again — each clutter
realisation has its own artefact, so a change to the selection renames the
survivor rather than removing it, and 1 in 12 is a rate rather than a property of
any one seed.

**What this does NOT establish.** It is one fixture family, one operating point,
and twelve realisations, so the rate is 1 in 12 and its confidence interval is
wide. Relaxing admission does admit more modes — nine on the real C10 collect
where four were admitted before — and this arm says the extra admissions do not
survive a change of look count on THIS fixture. It says nothing about a fixture
whose artefacts are more repeatable.
