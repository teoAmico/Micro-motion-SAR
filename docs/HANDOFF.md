# Handoff — 2026-08-07 (second pass)

State of play at commit `HEAD`, written so a new session can pick up without
re-reading 110 follow-up items. **Read `CLAUDE.md` first; this is the delta.**

Tree is clean, 22/22 tests pass, ASAN clean, nothing running in the background.

---

## 1. Start here: what just closed, and what is open

**Item 109's named defect was wrong and item 110 fixed the real one.** Item 109
said `rs_local_ratio()`'s guard band lost the localised target. Measured — by
replicating the whole nomination offline from a `--shifts` dump, a replica that
reproduces the binary exactly — sweeping the guard 2 to 8 bins never recovers the
line, and the Hann-skirt argument does not apply at `--overlap 0` where the floor
is flat. **That was the third wrong explanation of one failure** (window
boundary, local clutter, guard band).

The target was lost **twice**, in `rs_spectrum_modal_set()`:

1. **The binomial `support_min` gate.** 34 of 225 required, 28 delivered. That
   threshold is a fraction of the whole window grid, so a mode on a handful of
   windows cannot reach it however strong. It is CLAUDE.md's own
   localised-target rule in the one gate nobody had checked it against.
2. **Block-first ranking.** Fourth place, behind three artefacts covering one
   more window each, while leading every rival two-to-one on strength.

Both fixed: admission is `RS_MODAL_BLOCK_MIN` (the 2x2 block floor restated, so
support refuses only what the block gate refuses anyway) with `rs_modal_null()`
drawn under the same rule so the chance block rises to compensate; ranking is
`evidence = n_contiguous * log(median_ratio)`.

**H1 5 of 6** (was 3), **H3 2 of 2**, kill criterion **H3b 1 of 12** unchanged
from item 107, injected recall **6 of 6** unchanged.

### The one thing that did NOT move, and is now the named defect

**Item 108's false positive.** C14's motionless control still leads with
**0.997 Hz at `ev` 28.3** against an injected 1.00 Hz. Nothing in item 110
touches it; `--stable` is still the only thing that rejects it, on the strength
of a 256-look answer of 5.996 Hz. A scene with nothing in it answering 0.003 Hz
from the frequency being sought is the sharpest open problem here.

### And recovery is now a number you can read off the report

`ev` for the injected line against its own scene's competition, 128 looks:

| amp mm | C10 injected | C10 competition | C14 injected | C14 competition |
|---|---|---|---|---|
| 0.00 | — | **25.0** | — | **28.3** |
| 0.13 | 16.6 (5th) | 23.8 | **40.5** | 15.5 |
| 0.26 | **24.0** | 23.8 | **51.6** | 15.5 |
| 0.53 | **28.5** | 23.8 | **55.1** | 17.5 |

C10 crosses between 0.13 and 0.26 mm — **item 103's competition floor, reached
independently through a different statistic.** That is the first time this
project has had a *reportable* quantity that predicts recovery rather than
explaining it afterwards.

---

## 2. What was built across these two sessions, and what each is for

| flag / function | what it does | item |
|---|---|---|
| `mmotion --twin CSV` | differences against a paired run at `--probe-hz`; reports the LLR and its exact F(2,2) p-value | 97, 98 |
| `rs_twin_llr()` | two-sample GLRT for exponential periodogram bins, `2 log((1+r)/2) − log r`, `p = 1/(1+r)` | 98 |
| `rs_microm_floor()` | per-window detectable-amplitude floor from that window's circular phase sd; `floor_mm` in the CSV | 103 |
| `mmotion --stable CSV` | keeps only frequencies surviving a change of **look count** | 107, 109 |
| `rs_mode_t.evidence` | `n_contiguous * log(median_ratio)`, the modal set's ranking key; `ev` in the report | 110 |
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

1. **Item 108's false positive**, now the sharpest thing here: a motionless real
   collect leads with 0.997 Hz at `ev` 28.3 against an injected 1.00 Hz, and only
   `--stable` refuses it. Item 110 raised the ranking's resolution without
   touching this.
2. **A test over `rs_spectrum_modal_set()`**, which has none — item 110 changed
   its admission rule and its sort key and `ctest` could not have noticed. See
   `docs/CODE-REVIEW.md` for what it needs to pin.
3. **`rs_local_ratio()`'s band-edge starvation** — 10 reference bins at the band
   floor against 20 mid-band, measured to matter (bin 3's block 14 → 9) and
   NOT fixed, because the obvious fix fails `test_tracking`'s red-floor case.
   Needs a narrower neighbourhood or a fitted slope. `docs/CODE-REVIEW.md`.
4. **Item 98's remaining two**: the CCD *double change map* (two twins), and
   Bayer & Seljak's self-calibrating look-elsewhere correction, which needs no
   Monte Carlo and works per window where `p_chance` works on the block.
5. **`--stable` with closer look counts** (128 vs 192): it abstained on 2 of 8
   real comparisons in item 110 because the 256-look answer landed above the
   128-look Nyquist. Closer counts share more band.
6. **Naples mode shapes** (item 94) — sensor *x,y,z* exist, so a real mode shape
   could be injected. Needs spatially-varying injection, which
   `--inject-wave` does not support.
