# Run: 2026-08-07 kilauea / stable-weak

**Question this run is meant to answer:** Does the stabilization test keep a WEAK LOCALISED target on real clutter, and still reject the motionless controls?

- git commit: `7787e4f`
- started:    2026-08-07T07:33:30Z
- host:       Darwin arm64

## Collect

Two complete Kilauea Capella spotlights, real clutter, 6.0 s seconds-matched
truncation. Naples segment 02 at 1.00 Hz, REL 20, placed 20 m off the grid origin
via `--inject-at`, at 0.13, 0.26 and 0.53 mm plus a motionless control. Each run
at 128 and 256 looks. The injection deposited into 56645 of 56645 pulses.

## Commands

See `stableweak.sh`. Pre-registered in `PREREG.md` at commit 25f7351.

## Result

**H1 and H3 both pass as written, but the useful result is narrower than that.**

**C14's motionless control reported 0.997 Hz at 128 looks** against an injected
1.00 Hz -- a real scene with nothing in it, answering 0.003 Hz from the signal
being sought. `--stable` rejected it, because at 256 looks the same scene says
7.828 Hz. Every statistic in items 38-99 would have endorsed that 0.997.

Recall did not transfer. Only 2 of 6 injected runs report, because C10 recovers
at 256 looks and not at 128 -- at amplitudes item 103 recovered on the same
collect with the target at the grid ORIGIN. The only change is
`--inject-at 20,20`, which is items 40-41's split across overlapping windows,
and it removes the signal before `--stable` is consulted.

Three of eight comparisons abstained, the 256-look answer landing above the
128-look Nyquist.

Full write-up in `docs/FOLLOW-UPS.md` item 108.
