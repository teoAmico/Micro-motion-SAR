#!/bin/sh
# The cubic term took the static-scatterer artefact from 317.7x to 70.7x, a
# further 4.5x on top of the quadratic's 38x. Where does the floor land now?
C="/Volumes/ZX20 II/sar-data/capella/CAPELLA_C13_SP_CPHD_HH_20241004001939_20241004002012/CAPELLA_C13_SP_CPHD_HH_20241004001939_20241004002012.cphd"
R=runs/giza/2026-08-04-cubic-floor
for A in 0.015625 0.0078125 0.00390625 0.001953125 0.0009765625; do
  echo "########## ${A} mm"
  ./build/micromotion mmotion --cphd "$C" --at 29.979175,31.134186 \
    --estimator phase --n 128 --overlap 0.5 --size 96 --cell 1.0 --win 32 \
    --rbins 4096 --coherence 0 --inject-vib "0.163,${A},20" --out $R/a${A} 2>&1 \
    | grep -E 'local peak|strongest peak'
done
