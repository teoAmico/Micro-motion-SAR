#!/bin/sh
C="/Volumes/ZX20 II/sar-data/capella/CAPELLA_C13_SP_CPHD_HH_20241004001939_20241004002012/CAPELLA_C13_SP_CPHD_HH_20241004001939_20241004002012.cphd"
exec ./build/micromotion mmotion --cphd "$C" --at 29.979175,31.134186 \
    --estimator phase --n 128 --overlap 0.90 \
    --size 96 --cell 1.0 --win 32 --rbins 4096 --coherence 0 \
    --null-static 5 \
    --out runs/giza/2026-08-03-null-density/derived
