# Handoff — 2026-08-07 (sixth pass)

State of play at commit `HEAD`, written so a new session can pick up without
re-reading 114 follow-up items. **Read `CLAUDE.md` first; this is the delta.**

Tree is clean, 23/23 tests pass, ASAN clean, nothing running in the background.

---

## 1. Start here: the selection arc is finished, and item 108 is closed

**Items 109 → 114 took the selection stage apart, one defect at a time.**

| | before 110 | 110 | 111 | 112 | 113 | 114 |
|---|---|---|---|---|---|---|
| H1, 128-look answer correct (real) | 3/6 | 5/6 | 5/6 | 6/6 | 6/6 | **5/6** |
| real motionless controls refused | 0/2 by chain | 0/2 | 0/2 | 0/2 | 1/2 | **2/2** |
| motionless synthetic reporting | 1/12 | 1/12 | 1/12 | 0/12 | 0/12 | **0/12** |
| motionless synthetic REFUSED @128 | 0/12 | 0/12 | 0/12 | 0/12 | 4/12 | **9/12** |
| item 108's artefact, p | 0.001 | 0.001 | 0.001 | 0.001 | 0.010 | **0.342, refused** |

Five defects, each visible only once the previous was fixed: the **admission**
threshold (110), the **band-edge bias** (111), the **strength term** (112), the
**null's independence assumption** (113), and the fact that **no permutation null
on the nominations can be exact** (114).

**Item 108 is closed.** The collect that led with 0.997 Hz against a sought
1.00 Hz now returns *nothing recurs*, and both real motionless controls are
refused by the chain rather than by `--stable`.

## 2. The two costs item 114 charged

- **H1 fell 6/6 → 5/6.** C10 at 0.13 mm is refused at p 0.942. Item 113 had it at
  exactly p = 0.050 and already called it "the threshold, not a recovery".
- **`--stable` reportable fell 5/6 → 3/6, and this was NOT predicted.** It rejects
  nothing; the **256-look runs now refuse**, so there is no partner to compare
  against. A more specific chain gives the stabilization test less to work with.

## 3. What I would do next, in order

1. **`--stable` at 128 vs 192 looks.** Promoted to the top *by* item 114's second
   cost. Closer counts share more band (fewer "not comparable" abstentions,
   item 108's old weakness) **and** both runs are likelier to clear the gate.
   Cheap, needs no new data, repairs what 114 damaged.
2. **TFCE** (Smith & Nichols 2009) — the literature's third remedy, untaken. It
   removes the arbitrary cluster-forming threshold, here `RS_MODAL_PER_WINDOW`,
   which item 71 measured as consequential.
3. **Item 98's remaining two**: the CCD double change map, and Bayer & Seljak's
   self-calibrating look-elsewhere correction.

**But the strategic point: items 109–114 are all SELECTION.** Six items made the
chain much better at NOT answering and none of them changed what the tracker can
see. Nothing here has detected real motion; the target is still put where it is
found. The archive search is finished and negative (item 85), and the field's own
validations either bolt corner reflectors to the structure (item 104) or use a
steel bridge that supplies its own 20-26 dB (item 106). The next real frontier is
a collect over something that moves and is independently instrumented — **Naples
(item 94) is the cheapest approximation**, and it needs spatially-varying
injection that `--inject-wave` does not support.

## 4. A method lesson worth keeping

**Read a bracket's ORDER, not just its width.** Item 114's first pass was
discarded because the printed interval read `54.0 to 42.9` — the "conservative"
null below the optimistic one, because that draw was not monotone in the thing it
was supposed to vary. It would have reintroduced the very anti-conservatism the
item exists to remove.

**And measure the explanation before building on it.** Four explanations in this
arc were wrong (window boundary, local clutter, guard band, correlation length);
the last was caught *before* anything was built on it, by measuring the
correlation as a function of window separation first.

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

1. **`--stable` at 128 vs 192 looks**, and **TFCE** — §3 above.
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
