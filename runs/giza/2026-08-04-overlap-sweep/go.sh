#!/bin/sh
# The overlap trade is three-sided and has never been measured as a whole:
#   item 13  higher overlap raises the sampling ceiling, spent on frequencies
#            already averaged away
#   item 14  higher overlap buys sub-look coherence -- recommends 0.90-0.95
#   item 47  higher overlap manufactures the red noise that beats the signal
# Injection and control at each overlap, scored on the local peak, which is the
# only statistic here that is comparable across a coloured floor.
C="/Volumes/ZX20 II/sar-data/capella/CAPELLA_C13_SP_CPHD_HH_20241004001939_20241004002012/CAPELLA_C13_SP_CPHD_HH_20241004001939_20241004002012.cphd"
R=runs/giza/2026-08-04-overlap-sweep
BASE="--at 29.979175,31.134186 --estimator phase --n 128 --size 96 --cell 1.0 \
      --win 32 --rbins 4096 --coherence 0"
for OV in 0.0 0.5 0.75 0.90 0.95; do
  TAG=$(echo "$OV" | tr '.' 'p')
  echo "########## overlap ${OV} -- injected 0.163 Hz at 2 mm"
  ./build/micromotion mmotion --cphd "$C" $BASE --overlap "$OV" \
      --inject-vib "0.163,2.0,20" --out $R/inj_$TAG 2>&1 \
    | grep -E 'sub-apertures|observable band|response|local peak|strongest'
  echo "########## overlap ${OV} -- control"
  ./build/micromotion mmotion --cphd "$C" $BASE --overlap "$OV" \
      --out $R/ctl_$TAG 2>&1 | grep -E 'local peak|strongest'
done
