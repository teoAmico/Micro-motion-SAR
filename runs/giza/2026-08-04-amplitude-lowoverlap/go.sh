#!/bin/sh
# Every sensitivity figure in items 37 and 43 was measured at --overlap 0.90,
# which item 48 has just shown to be the worst setting tested -- 11x separation
# against 95,838x at 0.5. Re-run the sweep at 0.5 and go further down, since the
# 2 mm point now sits five orders of magnitude above the control.
#
# The control distribution at these settings is already measured: nine disjoint
# grids of real desert gave 15.1-34.4 (item 49). A local peak under ~35x is
# inside what empty desert produces.
C="/Volumes/ZX20 II/sar-data/capella/CAPELLA_C13_SP_CPHD_HH_20241004001939_20241004002012/CAPELLA_C13_SP_CPHD_HH_20241004001939_20241004002012.cphd"
R=runs/giza/2026-08-04-amplitude-lowoverlap
BASE="--at 29.979175,31.134186 --estimator phase --n 128 --overlap 0.5 \
      --size 96 --cell 1.0 --win 32 --rbins 4096 --coherence 0"
for A in 2.0 1.0 0.5 0.25 0.125 0.0625 0.03125 0.015625 0.0078125; do
  echo "########## ${A} mm"
  ./build/micromotion mmotion --cphd "$C" $BASE --inject-vib "0.163,${A},20" \
      --out $R/a${A} 2>&1 | grep -E 'local peak|strongest|located at'
done
