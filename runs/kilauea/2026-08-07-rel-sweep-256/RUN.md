# Run: 2026-08-07 kilauea / rel-sweep-256

**Question this run is meant to answer:** Does the brightness transition move with look count, as a sub-look SCR penalty would require?

- git commit: `9051541`
- started:    2026-08-07T06:48:58Z
- host:       Darwin arm64

## Collect

Two complete Kilauea Capella spotlights, 6.0 s seconds-matched truncation,
**256 looks**. Naples segment 02 at a fixed 0.26 mm and 1.00 Hz; only REL varies.

## Commands

See `relsweep256.sh`. Pre-registered in `PREREG.md` at commit 833c160.

## Result

**H1 FAILS, exactly as pre-registered.** The brightness transition is identical
at 256 looks and at 128: C10 recovers only at REL 20, C14 at REL 20 and 10.

The sub-look SCR penalty therefore does not bite at low REL either, and is dead
in every regime tested. The 11-17 dB gap against PS-InSAR is not about sub-look
resolution.

A detail kept: the uninjected control's frequency MOVES with look count, and
differently per collect. C10 goes 0.665 -> 1.331 Hz, exactly 2x and the same
fraction of Nyquist, so its artefact is periodic in LOOK INDEX rather than in
time -- sixteen looks per cycle at both settings. C14 goes 0.499 -> 20.486 Hz,
band floor to near Nyquist, which is not the same behaviour at all.

Full write-up, and a literature correction to items 104 and 70, in
`docs/FOLLOW-UPS.md` item 106.
