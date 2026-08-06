
---

## Truncation fixed: matched in SECONDS, not in pulses (2026-08-05)

The first version of `kilauea.sh` passed a literal `--max-pulses 60000` to every
collect, on the reasoning that a correlation across collects must hold its
command line identical. That reasoning inverted here, and the flag was the
confound rather than the protection.

**PRF over these 16 runs 7,945 to 10,327 Hz -- a 1.30x spread.** A fixed pulse
count therefore gives apertures of **5.81 to 7.55 s**, a 30% spread in dwell and
so in frequency resolution and in what each sub-look averages. Matching in
seconds requires a *different* pulse count per collect.

`kilauea_trunc.tsv` holds `max_pulses = round(PRF * 6.0 s)` per scene, derived
from each product's own `NumVectors` and the harvested dwell. Spread in seconds
kept: **0.0018%**, against 30%.

Every row is **verified against the file's own header** (`NumVectors` and the
data-array `NumSamples`) before the run, and a mismatch skips the collect. The
guard was tested both ways -- correct pairing returns `OK 56645 6.0000`, and the
same file against another scene's row returns
`ERR header 283005/27650 != table 311043/28800`.

### Why truncate at all, measured rather than assumed

Full dwell is **refused by the reader before it allocates**: *"282972 pulses x
27650 range bins needs 62.6 GB, and this machine has 25.8 GB"*. `info` fails on
these products for the same reason. The alternatives were rejected on their own
terms: `--rbins` cuts **bandwidth** on an FX-domain product rather than range
(`docs/CODE-REVIEW.md`), and `--pulse-stride` lowers the effective PRF, which no
measurement run may use.

At T = 6.0 s the first collect runs in **3.84 GB peak RSS** and reports
**0.1664 Hz**, matching the predicted 0.1667 to four figures. The gate is the
reader's *check*, which is much more conservative than the resident set actually
reached; T must satisfy the check.

**The cost is real and bounds what this test can say:** df 0.1667 Hz against
0.0389 Hz at full dwell, so the first admissible bin is 0.500 Hz. Acceptable only
because the pre-registered primary is median AMPLITUDE against the seismometer's
RMS, not a frequency.

### Second defect fixed in the same pass

The script did not pass `--estimator`, so it would have run the **default
`correlation` route** -- the one item 76 measured at `sigma_px` 46.7 m per look
and withdrew items 69-74 over. It now runs `--estimator phase`.

### Standing caveat, unchanged

Every one of the 16 truths is **below** item 53's 5.5 um floor (1.728 um at the
top, 0.137 um at the bottom). This is not a positive control and cannot become
one. A strong positive correlation would be surprising and would mean the floor
is pessimistic; a null teaches nothing, being what both the floor arithmetic and
item 82's threshold effect predict.

---

## STOPPED, 2026-08-06 — the test cannot answer its question

Abandoned before completion, deliberately, and recorded here rather than
deleted.

**Why.** `runs/kilauea/2026-08-06-floor-prediction/` measured the phase noise of
three completed collects and predicted this scene type's floor at **0.520-0.529
mm**. The sixteen collects were selected for seismometer truth spanning
**0.137-1.728 um**, so the strongest is **306x below the floor** and the weakest
3900x. Item 67 scoped this as a CORRELATION test on the grounds that it needs no
collect above the floor; item 82's threshold effect says that reasoning fails
here, because below threshold the estimator decouples from the truth and the
correlation is zero by construction. A null would have measured nothing.

**State at the stop.** 5 of 16 complete and byte-exact against source
`Content-Length`, verified at four offsets each; 11 incomplete leaving 90.1 GB of
partial data on disk and 225.1 GB never fetched. One `mmotion` run completed
(`CAPELLA_C10_SP_CPHD_HH_20240609091921`), at the seconds-matched truncation.

**What the five kept collects are still for.** Real clutter with real coherence,
which is the substrate the injection framework of item 102 needs. Five
independent real scenes is ample for that; sixteen was only ever needed for the
correlation across truth values, and that is what died.

**What this cost, and the lesson.** The download ran to 36.6% before anyone
measured what the floor on this scene actually was. The measurement takes one
uninjected run with `--shifts`. **Predict the floor before spending the
collect** — item 102's protocol exists because of this.
