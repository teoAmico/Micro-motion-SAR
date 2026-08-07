# Handoff — 2026-08-07

State of play at commit `febde44`, written so a new session can pick up without
re-reading 109 follow-up items. **Read `CLAUDE.md` first; this is the delta.**

Tree is clean, 22/22 tests pass, ASAN clean, nothing running in the background.

---

## 1. Start here: the one named defect

**`FOLLOW-UPS.md` item 109.** For the first time the *selection* failure has a
specific function and a specific constant attached, rather than being a general
complaint.

On a real Kilauea collect with a localised target injected at +24,+24 m (an exact
window centre), at 128 looks:

```
  injected 1.00 Hz    15 windows   largest 4-connected block 13
  artefact 0.665 Hz   11 windows   largest block  4
  REPORTED 0.499 Hz    7 windows   largest block  6
```

The injected line **wins on both of the modal set's stated ranking criteria** —
most support, much the largest contiguous block, centred on the target — **and is
still not reported.**

So the loss is not in the ranking. It is in the **nomination**:
`rs_spectrum_modal_set()` (`src/core/spectrum.c`) nominates via
`rs_local_ratio()`, which scores each candidate peak against its own *spectral
neighbourhood*. A strong isolated line inflates the background it is measured
against through its own Hann skirt, if `RS_LOCAL_GUARD_BINS` does not exclude
enough of it. **The cleaner the target, the worse it scores.**

### The next experiment, concretely

1. Read `rs_local_ratio()` and the constants `RS_LOCAL_HALF_BINS` /
   `RS_LOCAL_GUARD_BINS` in `src/core/spectrum.c`.
2. A Hann main lobe is ±2 bins (`RS_SPECTRUM_LEAKAGE_BINS` is 3 for this
   reason). Check whether the guard actually excludes it.
3. **Pre-register before running** — `tools/new-run.sh` seeds the form; see §4.
4. Re-run item 109's exact configuration and see whether C10 recovers at 128
   looks. The script is `runs/kilauea/2026-08-07-stable-weak-centred/stablecentred.sh`;
   the expected answer is 1.00 Hz and the current wrong answer is 0.499 Hz.
5. Guard against the obvious trap: widening the guard makes *every* line score
   higher, so re-run the **12 motionless scenes** of item 96 too and check the
   false-positive rate does not climb back. `runs/synthetic/2026-08-07-look-stabilization/stabsweep.sh`
   already does both arms.

**Do not assume this is the whole story.** Two explanations were offered for the
same failure in items 108 and 109 and both were wrong (window placement, then
local clutter). Measure before believing.

---

## 2. What was built this session, and what each is for

| flag / function | what it does | item |
|---|---|---|
| `mmotion --twin CSV` | differences against a paired run at `--probe-hz`; reports the LLR and its exact F(2,2) p-value | 97, 98 |
| `rs_twin_llr()` | two-sample GLRT for exponential periodogram bins, `2 log((1+r)/2) − log r`, `p = 1/(1+r)` | 98 |
| `rs_microm_floor()` | per-window detectable-amplitude floor from that window's circular phase sd; `floor_mm` in the CSV | 103 |
| `mmotion --stable CSV` | keeps only frequencies surviving a change of **look count** | 107, 109 |
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

1. **`rs_local_ratio()`'s guard band** — item 109 above. Specific, cheap,
   testable.
2. **Item 98's remaining two**: the CCD *double change map* (two twins), and
   Bayer & Seljak's self-calibrating look-elsewhere correction, which needs no
   Monte Carlo and works per window where `p_chance` works on the block.
3. **`--stable` with closer look counts** (128 vs 192): it abstained on 3 of 8
   real comparisons because the 256-look answer landed above the 128-look
   Nyquist (item 108). Closer counts share more band.
4. **Naples mode shapes** (item 94) — sensor *x,y,z* exist, so a real mode shape
   could be injected. Needs spatially-varying injection, which
   `--inject-wave` does not support.
