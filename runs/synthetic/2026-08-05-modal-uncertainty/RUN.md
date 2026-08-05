# Per-mode uncertainty and the chance model for the block

Commit: see `git log` for the commit that added `p_chance` to
`rs_spectrum_modal_set()`. Host: darwin, Release build.

## Question

Item 78 said the block gate needs a chance model rather than a matched static
run, and item 79 said the field reports a per-mode posterior rather than
thresholding a spectral statistic. Build both as far as the evidence supports:
does pricing the block for chance change any verdict the fixed floor of 4 got
wrong?

## Commands

`sweep80.sh` -- 12 injected burst frequencies x 2 clutter seeds + 2 static
controls, at 128 AND 48 looks, `--estimator phase` at 2 mm.

## Result

No. The threshold it derives is correct and configuration-dependent (block 6 at
128 looks, block 20 at 48, against a fixed floor of 4), which fixes item 77's
hazard. But at 128 looks no verdict moves, at 48 it refuses two answers that
were already wrong, and **both static controls still pass at both look counts**
because they carry the common-mode artefact rather than chance agreement.

Item 11 for the third time: a chance model over nominations is not a null over
scenes. Full write-up in `docs/FOLLOW-UPS.md` item 80.

`sweep80_results.txt` is the raw table. The parser was verified against a known
line (`0.302 31 0.001 34/49`) before the sweep was launched -- see the sweep
parser defect recorded in item 73's arc.
