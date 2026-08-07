# Run: 2026-08-07 kilauea / rel-sweep

**Question this run is meant to answer:** At what target BRIGHTNESS does a real structure's motion stop being recoverable against real clutter?

- git commit: `8898dfa`
- started:    2026-08-07T06:16:40Z
- host:       Darwin arm64

## Collect

Two complete Kilauea Capella spotlights at the 6.0 s seconds-matched truncation.
Naples segment 02 injected at a fixed 0.26 mm and 1.00 Hz; only REL varies.

## Commands

See `relsweep.sh`. Hypotheses pre-registered in `PREREG.md` at commit c0de7f5.

## Result

**H1 and H2 both PASS.** Recovery depends sharply on target brightness, and the
transition sits between **14 and 26 dB** of signal-to-clutter.

| REL | SCR | C10 | floor | C14 | floor |
|---|---|---|---|---|---|
| 20 | 26 dB | 0.998 recovered | 0.0022 mm | 0.997 recovered | 0.0023 mm |
| 10 | 20 dB | 0.665 artefact | 0.486 mm | 0.997 recovered | 0.0025 mm |
| 5 | 14 dB | 0.665 | 0.486 mm | 0.499 artefact | 0.499 mm |
| 2 | 6 dB | 0.665 | 0.486 mm | 0.499 | 0.499 mm |
| 1 | 0 dB | 0.665 | 0.486 mm | 0.499 | 0.499 mm |

The target-window floor is **bimodal** -- 0.002 mm when the target dominates its
pixel, the clutter floor of ~0.49 mm the moment it does not, nothing between.
Below the transition the reported frequency is exactly the uninjected scene's own
artefact.

**My pre-registered prediction was wrong**: I said the transition would fall
between REL 2 and 5, and it falls between 5 and 20.

This does not contradict item 51, which swept REL against the target's OWN
quadratic residual (scaling as REL^2 like the signal, hence brightness-neutral).
Here the competition is the SCENE's clutter artefact, which does not scale with
the target.

Full write-up, including the corroborating literature, in `docs/FOLLOW-UPS.md`
item 104.
