# Run: 2026-08-07 kilauea / stable-weak-centred

**Question this run is meant to answer:** With the target on a window CENTRE, does recall return, and does --stable keep it?

- git commit: `79ac667`
- started:    2026-08-07T07:58:39Z
- host:       Darwin arm64

## Collect

Identical to `../2026-08-07-stable-weak/` except `--inject-at 24,24` -- an exact
window centre (index 10; centres fall every 8 m for a 256 px grid at 0.5 m with
window 32 and stride 16) in place of the +20 m boundary placement.

## Commands

See `stablecentred.sh`. Pre-registered in `PREREG.md` at commit c3fb48d.

## Result

**H1 FAILS (3 of 6, needed 4). H3 and H4 pass.**

C14 now recovers down to 0.13 mm and `--stable` reports all three, so the test
discards no true positive that reaches it. C10 still fails at 128 looks.

Both explanations offered for that failure were wrong. It is not the window
boundary -- C10 fails identically on an exact centre. It is not local clutter --
at the target window the floor is 0.0044 mm against the clutter's 0.5210 and
`d_a` is 0.056 against 0.574, so the target dominates completely.

The mechanism is the modal set's NOMINATION. At 128 looks the injected 1.00 Hz
has 15 windows and a largest block of 13; the reported 0.499 Hz has 7 and 6. The
injected line wins on both stated criteria and is still not reported, so the loss
is in `rs_local_ratio()`, which scores each peak against its own spectral
neighbourhood -- a background a strong isolated line inflates through its own
Hann skirt.

Full write-up in `docs/FOLLOW-UPS.md` item 109.
