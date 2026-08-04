#!/bin/sh
# Item 50's floor at 0.125 mm is not set by noise. It is set by the
# zero-amplitude twin: a bright STATIC scatterer produces 12,060x at the band
# floor, and below 0.125 mm the real signal simply loses to it.
#
# That generalises past injection experiments. A real scene's bright dominant
# scatterers -- a building corner, a tower, a ship -- should produce the same
# thing, and would swamp a genuine weak vibration anywhere in the scene.
#
# So: zero amplitude at a range of brightness. If the artefact scales with
# brightness it is a leakage or carrier-removal residual and may be removable.
# If it saturates it is something else.
C="/Volumes/ZX20 II/sar-data/capella/CAPELLA_C13_SP_CPHD_HH_20241004001939_20241004002012/CAPELLA_C13_SP_CPHD_HH_20241004001939_20241004002012.cphd"
R=runs/giza/2026-08-04-static-artefact
BASE="--at 29.979175,31.134186 --estimator phase --n 128 --overlap 0.5 \
      --size 96 --cell 1.0 --win 32 --rbins 4096 --coherence 0"
for REL in 1 5 20 50 100; do
  echo "########## static scatterer at ${REL}x the median sample magnitude"
  ./build/micromotion mmotion --cphd "$C" $BASE --inject-vib "0.163,0.0,${REL}" \
      --out $R/rel${REL} 2>&1 | grep -E 'local peak|strongest|amplitude dispersion'
done
