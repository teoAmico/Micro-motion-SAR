# Handoff — 2026-08-07 (fifth pass)

State of play at commit `HEAD`, written so a new session can pick up without
re-reading 113 follow-up items. **Read `CLAUDE.md` first; this is the delta.**

Tree is clean, 23/23 tests pass, ASAN clean, nothing running in the background.

---

## 1. Start here

**Items 109 → 113 are one thread: the selection stage, taken apart.**

| | before 110 | 110 | 111 | 112 | 113 |
|---|---|---|---|---|---|
| H1, 128-look modal answer correct | 3/6 | 5/6 | 5/6 | 6/6 | **6/6** |
| H3 real controls refused | 2/2 | 2/2 | 2/2 | 2/2 | **2/2** |
| H3b motionless synthetic reporting | 1/12 | 1/12 | 1/12 | 0/12 | **0/12** |
| motionless synthetic REFUSED outright | 0/12 | 0/12 | 0/12 | 0/12 | **4/12** |
| modes admitted per injected run | — | 8-10 | 8-10 | 8-10 | **1** |

Four defects, each visible only once the previous was fixed:

1. **Admission** (110) — the binomial `support_min` is a fraction of the whole
   grid, so a localised mode could not reach it: 28 of a required 34.
2. **Band-edge bias** (111) — 10 reference bins at the edges against 20
   mid-band; 39% of the band took 72% of the maxima on empty scenes. Fixed by
   NARROWING, not widening.
3. **Strength term** (112) — `median_ratio` was a median over every nominator
   and ~22 of 225 nominate any bin by chance, so a 5x change in signal moved it
   7%. Now taken over the mode's own block.
4. **The null** (113) — it assumed windows nominate independently where 50%
   overlap makes neighbours correlated. **This was item 108's cause.**

**Item 113 is worth reading in full**: it is the seventh time a literature search
found the field ahead of this project, and the match is exact — Eklund, Nichols &
Knutsson (PNAS 2016) on cluster-extent inference invalidated by a mis-modelled
spatial autocorrelation, with permutation nulls and cluster MASS as the remedies.
Both were adopted and both worked.

## The one thing still open

**Item 108's false positive is improved tenfold and NOT solved.** C14's
motionless collect still leads with 0.997 Hz against a sought 1.00 Hz, now at
p 0.010 rather than 0.001 with 2 admitted modes rather than 8. The residual is
most likely that correlation extends **beyond the 2×2 tile** the null
decorrelates — a fully dilated null puts the same block at p ≈ 0.5, bracketing
the truth. `--stable` remains the only thing that rejects it.

The next step is measurable: estimate the correlation length from the data
(the shared-nomination excess as a function of window separation) and shift
tiles of that size, rather than assuming 2×2. TFCE (Smith & Nichols 2009) would
additionally remove the arbitrary nomination threshold, `RS_MODAL_PER_WINDOW`.

**Bound to carry forward: C10 at 0.13 mm sits exactly on p = 0.050.** One Monte
Carlo trial moves it across. It is the threshold, not a recovery.

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

1. **Item 108's residual** — §1 above: estimate the correlation length instead
   of assuming 2×2, and consider TFCE to remove the nomination threshold.
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
