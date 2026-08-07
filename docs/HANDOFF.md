# Handoff — 2026-08-07 (third pass)

State of play at commit `HEAD`, written so a new session can pick up without
re-reading 111 follow-up items. **Read `CLAUDE.md` first; this is the delta.**

Tree is clean, 23/23 tests pass, ASAN clean, nothing running in the background.

---

## 1. Start here: three items closed, two defects named

**Items 109 → 110 → 111 are one thread and it is now resolved.** Item 109 said
`rs_local_ratio()`'s guard band lost a localised target. That was wrong — the
third wrong explanation of one failure. Item 110 found the target was lost
**twice**: refused at the binomial `support_min` gate (28 of a required 34,
because that threshold is a fraction of the whole window grid), and then
out-ranked on **extent** by artefacts covering one more window. Item 111 removed
the **band-edge bias** that was manufacturing those artefacts.

| | before 110 | item 110 | item 111 |
|---|---|---|---|
| H1 recovery (real, 128 looks) | 3 of 6 | 5 of 6 | **5 of 6** |
| H3 real controls refused | 2 of 2 | 2 of 2 | **2 of 2** |
| H3b motionless synthetic | 1 of 12 | 1 of 12 | **1 of 12** |
| injected synthetic recall | 6 of 6 | 6 of 6 | **6 of 6** |
| synthetic abstentions | 4 of 12 | 5 of 12 | **3 of 12** |

**What item 111 bought is decisiveness, not rate.** The band-edge bias was
measured on 200 realisations of noise containing nothing: 39% of the band took
**72% of the maxima**, 4.0x the per-bin rate. Fixing it made the 10.148 Hz
artefact that led C10's motionless control vanish — it was bin 61, inside the
starved zone — and dropped abstentions from 5 of 12 to 3 of 12, because fewer
answers land on edge bins whose 256-look partner falls above the 128-look
Nyquist.

**The fix direction is the lesson.** Widening every neighbourhood to the count
mid-band has is the obvious fix, it equalises the count, and `test_tracking`'s
red-floor case kills it: on a floor rolling off as sinc² it reaches past the
first null. **Narrowing to the count the band FLOOR can supply** equalises the
count the other way and never enlarges the span. The header had said a steep
floor needs a narrower neighbourhood since item 47.

### The two named defects, in the order I would take them

1. **`median_ratio` is a median over CHANCE nominators** (item 111, measured, not
   fixed). Every window nominates `RS_MODAL_PER_WINDOW` bins wherever they fall,
   so each bin collects ~`n_win * M / K` nominations from noise alone — **22 of
   225 at the 65-bin operating point**, against reported supports of 28-46.
   Planting a line at gain 40 and at gain 200 moves `median_ratio` from **5.97
   to 6.39**: a factor of five in signal, nearly invisible to the statistic that
   ranks it. Fix: median over the **largest block**, the mode's measured
   footprint, not over every nominator. Needs `nom[k]` indexed by window and
   `rs_largest_block()` returning membership. Changes `evidence`, so it needs
   both arms re-run behind a pre-registration.
2. **Item 108's false positive.** C14's motionless control leads with
   **0.997 Hz at `ev` 28.0** against an injected 1.00 Hz. Three items have now
   changed the selection around it and it has not moved; only `--stable` rejects
   it, on a 256-look answer of 9.327 Hz.

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

1. **`median_ratio`'s chance dilution** and **item 108's false positive** — both
   in §1 above, with their measurements.
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
