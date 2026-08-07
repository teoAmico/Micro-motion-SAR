# PREREG — kilauea / nomination-fix

Filled in on: `2026-08-07`   git commit: `bd9eec8` (the change itself is not yet committed; see §5)
Status: **[x] confirmatory   [ ] exploratory**

## 1. Question

Item 109 named `rs_local_ratio()`'s guard band as the step that loses a localised
target. **A diagnostic run has already refuted that** (see §11). Two different
defects were found in the same machinery. Does fixing them recover item 109's
target at 128 looks, and what does it cost in false positives?

## 2. Ground truth

The injection geometry. `--inject-wave naples_wave/seg02.txt` resampled so the
record's dominant displacement mode lands at **1.000 Hz**, placed at
`--inject-at 24,24` — an exact window centre (index 10; centres every 8 m) — at
0.13, 0.26 and 0.53 mm peak, on real Kilauea clutter. Amplitude 0.00 is the
motionless control on the same collect.

Identical in every respect to `../2026-08-07-stable-weak-centred/`, which is what
makes this a controlled comparison: **only the binary differs.**

Truth for the false-positive arm is `runs/synthetic/2026-08-07-look-stabilization/`:
twelve motionless `--clutter-vib` scenes and six injected at 2 mm, where the
truth is that nothing moves in the first twelve.

## 3. Data freeze

| | |
|---|---|
| collect | `CAPELLA_C10_..._20240609091921` and `CAPELLA_C14_..._20240610013244` |
| dwell `T` | 6.01 s (128 x 0.046959 s), truncated per `kilauea_trunc.tsv` |
| looks `N`, overlap | 128 and 256, overlap 0 |

- `df = 1/T` = **0.16637 Hz** at 128 looks
- first admissible bin `3*df` = **0.499 Hz**
- top of band = **10.65 Hz**
- injected 1.000 Hz is inside the band, at **bin 6**. Yes.

## 4. Hypotheses

- **H1 recovery.** Of item 109's six injected runs (2 collects x 3 amplitudes),
  **at least 5 of 6** report a 128-look modal leading frequency within half a bin
  of 0.998 Hz. *Item 109 scored 3 of 6.* Threshold set at 5 rather than 6 because
  the offline replica predicts C10 recovers and C14 already did, so 6 of 6 is the
  expected outcome and 5 leaves one for a mechanism the replica cannot see.
- **H2 localisation.** Not scored. This run changes selection, not localisation,
  and `--inject-at` is unchanged from item 109.
- **H3 specificity, real.** Both motionless controls (C10 and C14 at amplitude
  0.00) are rejected by `--stable`, as in item 109, in **2 of 2**.
- **H3b specificity, synthetic — THE ONE THAT CAN KILL THIS.** Over the twelve
  motionless scenes of item 107, the number surviving the 128-vs-256 stability
  test stays at **1 of 12 or fewer**. Item 107 measured 1 of 12 before this
  change.
- **H4 twin.** Not run. Item 109 established H4 passes for `--stable`; this
  change does not touch it.

Recorded prediction, so it can be scored: **I expect H1 to pass at 6 of 6 and
H3b to be the one at risk**, because the ranking change alters which admitted
mode leads and a motionless scene's leading mode may become more repeatable
across look counts than it was.

## 5. Pipeline freeze

```sh
./build/micromotion mmotion --cphd "$DEST/$S.cphd" --estimator phase \
    --n {128,256} --overlap 0 --size 256 --cell 0.5 --win 32 --upsample 200 \
    --coherence 0 --max-pulses $MP \
    --inject-wave "$D/naples_wave/seg02.txt,113.7786,$amp,20" --inject-at 24,24 \
    [--stable <the 256-look windows.csv>] --out ...
```

Byte-for-byte the commands of `../2026-08-07-stable-weak-centred/stablecentred.sh`.

- estimator: `phase` (item 76)
- `--overlap`: 0 (item 48)
- `--fmin`: unset
- `--pulse-stride`: absent

**The two behavioural code changes under test, frozen now:**

1. **Admission.** The binomial `support_min` no longer gates; it is reported.
   Admission is `RS_MODAL_BLOCK_MIN` (4) on support -- the 2x2 block floor
   restated, so it refuses only what the block gate refuses anyway -- plus the
   unchanged block gate and `p_chance <= 0.05`. `rs_modal_null()` is drawn under
   the **same** rule, so the chance blocks rise to match and the family-wise
   correction is not lost with the threshold.
2. **Ranking.** `rs_cmp_mode()` orders by `evidence` = `n_contiguous *
   log(median_ratio)`, then block, then support, where it ordered by block,
   support, ratio.

`rs_local_ratio()` is additionally collapsed from three copies to one shared
implementation, with its semantics unchanged.

**Two other changes were written, measured and REVERTED before this run**, and
both are recorded because a pre-registration that hides its dead ends is worth
nothing:

- *Constant reference count in `rs_local_ratio()`* -- fixes the band-edge bias of
  §11 item 1, and fails `test_tracking`'s red-floor case: on a floor rolling off
  as sinc^2 the extension reaches past the first null and inflates the low bins
  it was meant to demote. Bias real, fix wrong. Now in `docs/CODE-REVIEW.md`.
- *Ranking on `median_ratio` alone* -- recovers item 109's target and **breaks
  the injected synthetic fixture**, reporting seed 7's 1.512 Hz common-mode
  artefact (ratio 38.8, block 11) over the true 0.504 Hz (ratio 25.0, block 30).
  Six of item 107's recoveries lost to buy one. The `evidence` form above is what
  survives both cases.

## 6. Null model

The motionless arm is the null: real (C10/C14 at 0.00) and synthetic (twelve
seeds). No shuffle null — invalid for `--estimator phase`.

## 7. Confounds considered

- [x] common-mode artefact — the motionless controls are the only defence, and
      they are run
- [x] **band-floor / trend field** — this is the confound at issue. The answer
      item 109 reported, 0.499 Hz, IS the first admissible bin
- [x] red noise from overlap — `--overlap 0`, and the measured floor on this
      collect is flat (median psd within 1.3x across the band)
- [ ] aspect-dependent fading — not excluded; real clutter, unquantified
- [x] the injected scatterer's mere presence — item 109's H4 covered it
- [x] gates inert rather than passing — `--coherence 0`, so that gate is off by
      construction and is not evidence

## 8. Blinding

- [x] the answer at 128 looks is produced before the 256-look comparison is read
- [ ] **NOT blind: the fix was chosen after seeing item 109's failure**, on the
      diagnostic in §11. That is what makes this a confirmatory run on the SAME
      configuration rather than a new claim — the thresholds above are set
      before the fixed binary is run on all sixteen.
- [x] every window reported; `*_windows.csv` committed

## 9. Kill criteria

- **If H3b fails** — more than 1 of 12 motionless synthetic scenes survives the
  stability test — **the ranking change is reverted**, whatever H1 says. Recall
  bought with specificity is what items 96-99 spent five items refusing.
- **If H1 fails at fewer than 5 of 6**, the ranking change is not the mechanism
  either, and item 110 is recorded as a THIRD wrong explanation of item 109
  rather than as a fix.
- If H1 passes and H3b passes, this is still not a detection claim. It is a
  claim about the selection stage only.

## 10. Reporting rules

- Every figure quoted with its look count, collect and amplitude.
- The offline replica's predictions (§11) are reported beside the measured
  outcome, including where they were wrong.
- A null stays in `RUN.md`.

## 11. What the exploratory diagnostic already established

Run before this pre-registration, on C10 at 0.53 mm and 128 looks with
`--shifts`, replicating `rs_spectrum_compute_opts` + `rs_local_ratio` +
`rs_spectrum_modal_set` in Python. The replica reproduces the C binary exactly
(0.499 Hz, block 14, support 42).

**Item 109's stated mechanism is refuted.** Sweeping the guard band 2→8 bins
never recovers the injected line:

| guard | leading answer |
|---|---|
| 2 (current) | 0.499 Hz |
| 3 | 0.499 Hz |
| 4 | 0.499 Hz |
| 5 | 0.499 Hz |
| 6 | 0.499 Hz |
| 8 | 1.497 Hz |

The injected bin's block sits at 13 and its support at 26-32 at every guard
width. **Widening the guard is not the lever**, and the Hann-skirt argument does
not survive: on this collect the floor is flat, so the skirt of an isolated line
is not what sets the background.

What the diagnostic found instead, and what §5 fixes:

1. The neighbourhood is **truncated** at both band edges — 10 reference bins at
   the first admissible bin against 20 mid-band. The two bins the modal set
   reported (bin 3 and bin 61) are both in the starved zone. Extending it drops
   bin 3's block from 14 to 9.
2. The injected line is **first by median local ratio (8.98 against 4.5-5.5) and
   first by max ratio (73.7 against 24)**, and fourth by block, losing to three
   artefacts by exactly one window. The target window itself nominates the
   injected bin at ratio **41.5**, its top pick, with a psd peak 107x its own
   median. **The evidence is in the struct and the sort threw it away.**

3. **And the sort never saw it at all.** The offline replica had applied no
   support threshold; the binary's is **34 of 225**, derived from the binomial
   null, and the injected bin's support is **28**. It was refused at the SUPPORT
   gate, before block, ratio or ranking were consulted. That gate is a fraction
   of the whole window grid, so a mode occupying a handful of windows cannot
   reach it however strong it is — CLAUDE.md's standing rule about localised
   targets, in the one gate it had not been checked against.

**So item 109's target is lost twice over**, and both had to be fixed: refused by
support, and then out-ranked on extent. Verified before this run on the same
C10 0.53 mm configuration — with both changes the modal set leads with **0.998 Hz
at evidence 28.5**, against the artefacts' 23.8, 21.8 and 21.1, and chance rose
from block 7 to block 9 as the relaxed admission requires. The synthetic injected
fixture keeps **0.504 Hz at evidence 96.6**.

Recorded prediction for the arms below: **H1 passes at 6 of 6.** The risk is
H3b — nine modes are now admitted on C10 where four were, and whether the extra
admissions make a motionless scene's leading frequency more repeatable across
look counts is exactly what the twelve-scene arm measures and what §9 kills on.
