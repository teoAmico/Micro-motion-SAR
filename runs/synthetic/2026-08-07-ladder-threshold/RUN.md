# Run: 2026-08-07 synthetic / ladder-threshold

**Question this run is meant to answer:** the persistence threshold was read off
item 115's twelve motionless scenes, so re-running those tests nothing. Does it
hold on twelve seeds it was never fitted to?

- git commit: `21594bc` (pipeline and hypotheses committed before the first run)
- host:       Darwin arm64

## Result

**H6 passes at 0 of 12. H1 passes at 6 of 6. H7 passes.** The independent arm
reproduces the fitted arm exactly.

| arm | scenes | chain 0 | chain 4 | chain 6 | reported |
|---|---|---|---|---|---|
| `fitstatic` — item 115's seeds (arithmetic) | 12 | 11 | 1 (seed 31) | 0 | **0** |
| `newstatic` — **twelve unseen seeds (the test)** | 12 | 11 | **1 (seed 67)** | 0 | **0** |
| `fitinj` — item 115's injected | 6 | 0 | 0 | 6 | **6** |
| `newinj` — **six unseen injected** | 6 | 0 | 0 | 6 | **6** |

### The null's shape reproduces on seeds it never saw

Twelve new motionless scenes gave **eleven chains of 0 and exactly one chain of
4** — the same distribution as the twelve the threshold was fitted to, which had
eleven 0s and one 4. That is the operating characteristic reproducing, not a
threshold fitted to noise.

**Over all 24 motionless scenes: 22 at chain 0, 2 at chain 4, none at 5 or
above. Over all 12 injected: every one at chain 6.**

**The gap at the threshold is empty.** Nothing in 36 scenes landed on 5. The
criterion sits in a void a rung wide on each side, which is the strongest form
this evidence could take and is why the margin named as a risk in `PREREG.md` §4
did not bite.

### Combined, against everything before it

| | item 107 pair | item 115 ladder+p | item 116 ladder+threshold |
|---|---|---|---|
| motionless reported | 1/12 | 1/12 | **0/24** |
| motionless given a definite verdict | 1/12 | 12/12 | **24/24** |
| injected reported | 6/6 | 6/6 | **12/12** |

**Item 107's surviving false positive is gone**, and so is item 115's — they are
the same scene class, a motionless artefact holding four consecutive rungs, and
the threshold is set above it by measurement rather than by assumption.

## Bounds

- **One fixture family, one operating point, one amplitude** (2 mm, 6.9x this
  fixture's floor). The threshold is an operating characteristic of THIS fixture
  and must be re-measured elsewhere before being quoted.
- **Not run on real data.** Everything here is synthetic; the real-data ladder
  is still the deferred 60-minute arm.
- **Two motionless scenes in 24 reached chain 4.** The criterion is one rung
  above that, so a fixture whose artefacts persist one rung longer would defeat
  it. Nothing here bounds how far that persistence can go.
