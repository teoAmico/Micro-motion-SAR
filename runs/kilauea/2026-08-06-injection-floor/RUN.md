# Run: 2026-08-06 kilauea / injection-floor

**Question this run is meant to answer:** Does the floor predicted from an uninjected run actually predict recovery on real clutter?

- git commit: `e32f765`
- started:    2026-08-06T22:26:01Z
- host:       Darwin arm64

## Collect

5 complete Kilauea Capella spotlights, byte-verified. Two used here:
C10 2024-06-09 (29.22 GB) and C14 2024-06-10 (25.26 GB), at the seconds-matched
6.0 s truncation. Injected waveform: Naples segment 02 (Zenodo 20667124), a real
building's ambient displacement, placed at 1.00 Hz.

## Commands

See `injfloor.sh`, committed here. Hypotheses pre-registered in `PREREG.md` at
commit 201d897, BEFORE the sweep ran.

## Result

**H1 FAILED as pre-registered, H3 passed, H4 weak.** Recovery occurs BELOW the
predicted floor -- at 0.5x on one collect and 0.25x on the other -- so the
prediction is not the boundary between recoverable and unrecoverable.

The cause is that item 102 predicted the floor from the scene's CLUTTER and the
injection places a BRIGHT POINT TARGET: `--inject-wave` defaults to `rel = 20`.
Measured on the injected run, the target's window is **34x quieter** than the
scene median -- circular sd 0.050 rad against 1.693 -- so its own floor is
0.0154 mm, not 0.522 mm.

Three floors, 34x apart:

| floor | here | describes |
|---|---|---|
| target | 0.015 mm | a bright coherent scatterer (item 53's 0.0055 mm is this) |
| clutter | 0.52 mm | the scene's distributed return (item 102) |
| competition | 0.13-0.26 mm | what it takes to beat the scene's own artefact |

**The competition floor is the operative one.** Item 102's conclusion about
Kilauea still stands, because a seismometer measures the ground, which is
clutter, at 0.137-1.728 um against a 0.52 mm clutter floor.

Full write-up in `docs/FOLLOW-UPS.md` item 103.
