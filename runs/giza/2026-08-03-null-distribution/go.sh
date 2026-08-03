#!/bin/sh
# Does the SIMULATED static null match the REAL motionless scene?
# No injection: the real scene's own prominence IS the real null, and the
# --null-static trials are the simulated one. Same read, same chain, same
# settings -- only the scene differs.
C="/Volumes/ZX20 II/sar-data/capella/CAPELLA_C13_SP_CPHD_HH_20241004001939_20241004002012/CAPELLA_C13_SP_CPHD_HH_20241004001939_20241004002012.cphd"
exec ./build/micromotion mmotion --cphd "$C" --at 29.979175,31.134186 \
    --estimator phase --n 128 --overlap 0.90 \
    --size 96 --cell 1.0 --win 32 --rbins 4096 --coherence 0 \
    --null-static 8 \
    --out runs/giza/2026-08-03-null-distribution/nulldist
