# Handoff — 2026-08-07 (eighth pass)

State of play at commit `HEAD`, written so a new session can pick up without
re-reading 116 follow-up items. **Read `CLAUDE.md` first; this is the delta.**

Tree is clean, 23/23 tests pass, ASAN clean, nothing running in the background.

---

## 1. The rule this session produced

**In this chain, anything built by RE-DIVIDING ONE DWELL is correlated with
everything else built the same way, and no chance model may assume otherwise.**

Three chance models assumed independence and were wrong:

| model | assumed independent | actually shares | error | now |
|---|---|---|---|---|
| `rs_modal_null()` before 113 | window nominations | **pixels** | p 0.001 for an honest 0.013 | permutation + bracket (113, 114) |
| `rs_stable_p()` (115) | ladder rungs | **pulses** | p 1.3e-5 for a 1-in-12 event | **removed** (116) |

And item 116 established the limit: **for the ladder there is no valid null at
all.** Per-window chains cannot separate a scene-wide artefact from a scene-wide
injection (item 11); re-dividing the dwell returns the same answers (item 114's
wall). So the threshold is an *operating characteristic measured from the null's
own distribution*, and it is **not a probability**.

## 2. Where the selection stage ended up (items 109–116)

| | before 110 | 114 | 116 |
|---|---|---|---|
| H1 recovery, real | 3/6 | **5/6** | not re-run |
| real motionless controls refused by the chain | 0/2 | **2/2** | — |
| motionless synthetic reported | 1/12 | 0/12 | **0/24** |
| motionless synthetic given a DEFINITE verdict | — | 1/12 | **24/24** |
| injected synthetic | 6/6 | 6/6 | **12/12** |

**Item 108 is closed** (114). **Item 107's surviving false positive is gone**
(116). `--stable` is a ladder scored on the longest chain of consecutive agreeing
rungs against a threshold of 5, and refuses a verdict when the ladder is shorter.

## 3. What I would do next

1. **Run the ladder on real Kilauea data.** Everything in items 115–116 is
   synthetic. This is the deferred 60-minute arm (6 rungs × 8 configurations at
   ~75 s) and it is now the single largest gap in the evidence.
2. **Re-measure `RS_STABLE_MIN_CHAIN` wherever it is used.** It is an operating
   characteristic of one fixture at one operating point, not a constant. Two
   motionless scenes in 24 reached chain 4, one rung below it.
3. **TFCE** (Smith & Nichols 2009) — removes the arbitrary cluster-forming
   threshold, here `RS_MODAL_PER_WINDOW`, which item 71 measured as consequential.

**The strategic point, unchanged after eight items**: 109–116 are all SELECTION.
They made the chain much better at NOT answering — item 96's 100% false-positive
rate is gone, item 108 is closed, 0 of 24 motionless scenes report — and **none
of them changed what the tracker can see.** Nothing here has detected real
motion; the target is still put where it is found. The next real frontier is a
collect over something that moves and is independently instrumented; Naples
(item 94) is the cheapest approximation and needs spatially-varying injection
that `--inject-wave` does not support.

## 4. Method lessons that earned their keep

- **Search the literature BEFORE designing.** Eight times now. Items 113, 115 and
  116 were all solved — or explicitly declined — in another field, and in each
  case this project had built the degenerate special case or invented a statistic
  the field knows better than to compute.
- **A threshold fitted to data must be tested on data it was not fitted to.**
  Item 116's whole design; the independent arm reproduced the fitted one exactly,
  which is the only reason the number can be trusted at all.
- **Read a bracket's ORDER, not just its width** (114).
- **Measure the explanation before building on it** (114) — four explanations in
  this arc were wrong; the last was caught in advance.

---

## 2. What was built across these two sessions, and what each is for

| flag / function | what it does | item |
|---|---|---|
| `mmotion --twin CSV` | differences against a paired run at `--probe-hz`; reports the LLR and its exact F(2,2) p-value | 97, 98 |
| `rs_twin_llr()` | two-sample GLRT for exponential periodogram bins, `2 log((1+r)/2) − log r`, `p = 1/(1+r)` | 98 |
| `rs_microm_floor()` | per-window detectable-amplitude floor from that window's circular phase sd; `floor_mm` in the CSV | 103 |
| `mmotion --stable CSV` | keeps only frequencies surviving a change of **look count** | 107, 109 |
| `rs_mode_t.evidence` | `n_contiguous * log(median_ratio)`, the modal set's ranking key; `ev` in the report | 110 |
| `tests/test_modalset.c` | the first test over `rs_spectrum_modal_set()`; pins admission against ranking in both directions | 111 |
| block-median `median_ratio` | strength summarised over the mode's footprint, not over every nominator | 112 |
| permutation null on cluster mass | 2x2 tile-shift preserves the correlation overlap creates; the gate tests `ev` | 113 |
| bracketed p, gated on the conservative end per scene | no permutation null here can be exact, so bound it both ways | 114 |
| `--stable CSV,CSV,...` | a LADDER of look counts; longest chain of consecutive agreeing rungs | 115 |
| `RS_STABLE_MIN_CHAIN` | measured operating characteristic, not a p; no valid null exists for it | 116 |
| `rs_transient_fit()` / `--tfit` | damped-sinusoid fit with onsets; works, changes nothing at chain level | 81 |
| `docs/PREREGISTRATION.md` | the form; `tools/new-run.sh` seeds `PREREG.md` per run | 92 |

**`--stable` is the important one.** It is the only control here that needs **no
twin, no null control and no ground truth** — just the collect processed twice.
It took a motionless fixture from 12/12 false positives to 1/12 (item 107), and
on real data it rejected a motionless scene reporting **0.997 Hz against an
injected 1.00 Hz** (item 108) that every other statistic in this project would
have endorsed.

---

## 3. Numbers a new session will need, with their caveats

- **Three floors, ~34x apart, and they are not interchangeable** (item 103):
  *target* ~0.015 mm (a bright scatterer; item 53's 0.0055 mm is this quantity),
  *clutter* ~0.52 mm (the scene's distributed return), *competition* 0.13–0.26 mm
  (what beats the scene's own artefact). **The competition floor governs.**
- **Recovery needs 20–26 dB signal-to-clutter** (items 104, 106), and the
  transition is sharp and **independent of look count**.
- **Item 53's 0.0055 mm does not apply to clutter.** It was measured on a bright
  coherent point target. Quote a floor with the scatterer *and window* attached.
- **`--estimator phase` throughout.** Item 76: the correlation route's `sigma_px`
  was 46.7 m/look and could not see the signal at all.
- **The look count was fixed at 128 for reasons never measured**; 256 gives a 23%
  lower floor (item 105) and is what `--stable` compares against.

---

## 4. Practice adopted, and it caught things

**Pre-register before running.** `tools/new-run.sh <scene> <suffix> "<question>"`
seeds `PREREG.md`; fill it in and **commit it before the first processing
command**, so the history shows the hypotheses predate the data.

It earned its keep this session: item 103's H1 failed in a direction the form had
named in advance, item 105 refuted a prediction I had recorded, and item 109's
H1 failure pointed straight at the real mechanism. **Predictions recorded before
the run are how three wrong explanations got caught.**

---

## 5. Assets on disk

- **5 complete Kilauea Capella spotlights**, ~135 GB on `/Volumes/ZX20 II`,
  byte-verified against source `Content-Length` at four offsets each. Real
  clutter at coherence ~0.38 — the substrate for injection work.
- The other 11 are **partial and the download was stopped deliberately** (item
  102): the seismometer truth is 0.137–1.728 µm against a 0.52 mm clutter floor,
  so the correlation test cannot answer its question. Partials are valid
  resumable prefixes if ever wanted.
- **Real-sensor waveforms**: Naples (21 segments, in-band at 2.64 Hz, mode-shape
  geometry) and Granada (16 segments) in the session scratchpad — **regenerate
  from `runs/synthetic/2026-08-0{5,7}-*` if the scratchpad is gone**; both are
  public Zenodo records cited in items 89 and 94.
- `kilauea_trunc.tsv` holds the **seconds-matched** truncation per collect. PRF
  varies 1.30x across them, so a fixed `--max-pulses` gives unequal dwells.

---

## 6. Where the field is, so nothing here is re-invented

- **A real bridge has been measured** — Lotti et al. (Struct. Control Health
  Monit., Jan 2026), South Portland Street Suspension Bridge, Umbra-04, **no
  corner reflectors**, spectral correlation to ground truth **up to 0.88**,
  `df` 0.06 Hz from 16 s. The bridge supplies its own SCR through steel and sharp
  corners (item 106). **Item 70 quotes the older conference figures (0.33–0.47);
  use the journal ones.**
- **Six times a literature search has found the field already had what was being
  built here** (items 82, 92, 98, 104, 106, 107). Search before designing.
- The archive search is **finished and negative** (item 85): three public
  archives, four sensor networks, one usable pairing, and no instrumented dam
  anywhere.

---

## 7. Open, in the order I would take them

1. **The real-data ladder arm**, and re-measuring the threshold — §3 above.
2. **Item 98's remaining two**: the CCD *double change map* (two twins), and
   Bayer & Seljak's self-calibrating look-elsewhere correction, which needs no
   Monte Carlo and works per window where `p_chance` works on the block.
3. **`--stable` with closer look counts** (128 vs 192): it still abstained on
   1 of 8 real comparisons in item 111 because the 256-look answer landed above
   the 128-look Nyquist. Closer counts share more band, and item 111 shows this
   axis is worth something — removing the edge bias alone took synthetic
   abstentions from 5 of 12 to 3.
4. **Naples mode shapes** (item 94) — sensor *x,y,z* exist, so a real mode shape
   could be injected. Needs spatially-varying injection, which
   `--inject-wave` does not support.
