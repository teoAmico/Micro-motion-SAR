#!/bin/sh
# What scatterer density makes the simulated null resemble the real desert?
# Same scene, same chain, 3 trials each; only --null-scatterers varies.
C="/Volumes/ZX20 II/sar-data/capella/CAPELLA_C13_SP_CPHD_HH_20241004001939_20241004002012/CAPELLA_C13_SP_CPHD_HH_20241004001939_20241004002012.cphd"
for N in 400 4000 20000; do
  echo "########## --null-scatterers $N"
  ./build/micromotion mmotion --cphd "$C" --at 29.979175,31.134186 \
      --estimator phase --n 128 --overlap 0.90 \
      --size 96 --cell 1.0 --win 32 --rbins 4096 --coherence 0 \
      --null-static 3 --null-scatterers $N \
      --out runs/giza/2026-08-03-null-density/d$N 2>&1 \
    | grep -E 'null scene|static trial|mean |detection|reached it'
done
