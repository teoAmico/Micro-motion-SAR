# Run: 2026-08-07 synthetic / cluster-count

**Question this run is meant to answer:** under counting rather than
consecutiveness, what is the null's support distribution — and does one threshold
serve both fixtures?

- git commit: `7ce1396` (hypotheses committed before the arm)
- host:       Darwin arm64

## Result

**H10 passes. H12 passes — and it is the finding. H11 fails in its letter and
passes in its purpose.**

**No verdict moved on either fixture, exactly as predicted.**

### The synthetic null under counting

| support | 0 | 1 | 2 | 3 | 4 | 5+ |
|---|---|---|---|---|---|---|
| 24 motionless scenes | 4 | 14 | 3 | 1 | **2** | **0** |
| 12 injected scenes | 0 | 0 | 0 | 0 | 0 | **12 (all at 6)** |

**Motionless: 0 of 24 report. Injected: 12 of 12 report.** Identical to item 116
under the old statistic.

**H11 as written said the distribution would be UNCHANGED from item 116's chain
distribution. It is not.** Counting gives many scenes a support of 1–3 where the
chain was 0, because an isolated answer now counts. The property that mattered
survived — **maximum 4, nothing at 5 or above** — so the kill criterion did not
fire, but the prediction was wrong in detail and is recorded as such.

### And no single threshold serves both fixtures

Real supports, from item 117's measured per-rung answers, with C10 at 0.26 mm
re-run directly to verify:

| | support |
|---|---|
| C10 / C14 motionless controls | **0** — every rung refuses |
| C14 injected 0.13 mm | 1 |
| **C10 injected 0.26 mm** | **4** |
| C10 0.53, C14 0.26, C14 0.53 | 6 |

**A real injection sits at 4. Two synthetic motionless scenes also sit at 4.**
The populations overlap **across fixtures**:

- at **5**, the synthetic null stays clean (0 of 24) and the real injection at
  0.26 mm stays rejected;
- at **2–4**, that injection is recovered and the two synthetic motionless
  scenes become false positives.

**`RS_STABLE_MIN_CHAIN` is therefore a per-fixture operating characteristic, not
a constant.** Item 116 stated that as a caveat; this measures it as a fact.

## Why make the change at all, then

Because the statistic is right whether or not a verdict moves. Requiring an
unbroken run was this project's own addition — poles are routinely absent at some
model orders, and automated OMA thresholds on cluster size. Item 117 measured
that requirement rejecting a real injection whose every answering rung was
correct. The count now says what it means: *"0.998 Hz is carried by 4 of 6 rungs
able to express it"*.

## Bounds

- Two fixture families, one operating point each, one injected amplitude each.
- **The real arm's supports are arithmetic over item 117's measured ladders**,
  with only C10 at 0.26 mm re-run end to end. The per-rung answers are the
  measurement; the count over them is not a separate one.
- Nothing here bounds how far a motionless artefact's support can go on a
  fixture not tested.
