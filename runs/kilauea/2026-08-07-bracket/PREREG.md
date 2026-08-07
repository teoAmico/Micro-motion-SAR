# PREREG — kilauea / bracket

Filled in on: `2026-08-07`   git commit: `cf8f2b1` (the change itself is not yet committed; see §5)
Status: **[x] confirmatory   [ ] exploratory**

## 1. Question

Item 113 fixed item 108's *cause* and left a residual: its permutation null is
still anti-conservative, so the motionless collect's 0.997 Hz survived at
p = 0.010. Does bracketing p between an under- and an over-correlated null, and
gating on the conservative end, refuse it — and what does it cost?

## 2. Ground truth

Unchanged from `../2026-08-07-clustermass/`.

## 3. Data freeze

Identical to `../2026-08-07-nomination-fix/PREREG.md` §3.

## 4. Hypotheses

- **H4 the target.** C14's motionless control, which has led with 0.997 Hz
  against a sought 1.00 Hz since item 108, is **refused outright** by the chain.
- **H3 specificity, real.** Both motionless controls refused, **2 of 2**, now by
  the chain rather than by `--stable`.
- **H1 recovery.** At least **4 of 6** injected runs report within half a bin of
  0.998 Hz at 128 looks. *Item 113 scored 6 of 6.* Two lost is the priced cost.
- **H3b synthetic — kill criterion.** At most **1 of 12** motionless scenes
  survives, injected recall at least **5 of 6**.

**Recorded predictions, from an offline measurement of both nulls against item
113's published `ev` table, before these arms:**

- **H4 passes.** Already verified on the collect: it now returns *nothing recurs*,
  with 0.997 Hz at **p 0.342** against a conservative null reaching 37.7. The p
  on that artefact has gone 0.001 → 0.010 → 0.342 across three items.
- **H1 lands at 5 of 6**, losing only C10 at 0.13 mm — the point item 113
  already recorded as sitting exactly on p = 0.050, "the threshold, not a
  recovery", and which `--stable` abstains on anyway. Offline the conservative
  critical evidence is ~37, against C10's 21.6 / 39.7 / 48.1 and C14's
  49.9 / 58.6 / 61.6.
- **H3b holds**, and synthetic refusals should rise above item 113's 4 of 12.

## 5. Pipeline freeze

Commands byte-for-byte those of `../2026-08-07-clustermass/rerun.sh` and
`../../synthetic/2026-08-07-clustermass-null/stabsweep_v5.sh`.

**The change:** `rs_modal_null()` is run twice. The **shift** draw keeps each
window's own nominations (under-correlates, 66% of observed adjacent sharing);
the **dilate** draw makes the whole 2×2 tile share one member's nomination
pattern while each window keeps its own typical ratio (over-correlates, 152%).
`p_chance` and `p_chance_max` are both reported; **admission gates on the
conservative one.**

## 6. Null model

The two nulls are the model, and they bracket rather than estimate.

## 7. Confounds considered

- [x] **Neither null is exact and that is now stated rather than assumed.** The
      gate is the upper bound, so the family-wise rate is controlled at or below
      nominal by construction.
- [x] The dilated draw could be *too* conservative and cost real recall. H1's
      bar is set at 4 of 6 to price exactly that, and §10 records what it is
      expected to lose.
- [x] Correlation beyond 2×2 — **measured and refuted**, see §10.

## 8. Blinding

- [x] Predictions from offline measurement, recorded before the arms.
- [ ] Not blind: item 113's outcome is known.

## 9. Kill criteria

- **If H4 fails** the change has not done the one thing it is for; revert.
- **If H1 falls below 4 of 6**, or H3b fails, revert and keep item 113's state,
  recording that an honest gate costs more recall than this chain can spare.

## 10. What the measurements already establish

**Item 113's stated residual was WRONG, and measuring it first is what caught
that.** It supposed correlation reached beyond the 2×2 tile. Measured on the
motionless C14 collect, shared nominations by window separation:

| separation | 0 | 1 | 2 | 3+ | random pair |
|---|---|---|---|---|---|
| shared of 6 | 6.00 | **2.16** | 0.59 | 0.57–0.63 | 0.63 |

**The correlation is confined to ADJACENT windows and is at baseline by
separation 2** — exactly the pixel-sharing range of a 32 px window at stride 16.
The 2×2 tile is the right scale.

**The real residual is that any FIXED partition destroys correlation across its
own boundaries.** Half of all adjacent pairs straddle one, so the tile-shift null
reproduces only **66%** of the observed adjacent sharing. Jittering the tile
origin does not help (**63%**), and a boundary-free shift field needs a copy
probability of **~0.99** to reproduce it — a globally constant field, which
preserves the very structure the null exists to destroy.

**So a permutation null on the nominations cannot be made exact**, and the honest
instrument is a bracket. Offline, the conservative draw puts the critical
evidence at ~37 against the optimistic ~23, and item 108's artefact at 28.4 falls
between: admitted by one, refused by the other, which is the correct reading of
a scene that cannot be placed.
