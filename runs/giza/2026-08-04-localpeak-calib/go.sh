#!/bin/sh
# The local peak is reported and gates nothing because its control maximum was
# known from two scenes. This takes nine DISJOINT 96 m grids across the same real
# uninjected collect -- 150 m apart, so no two share a window -- and reads the
# distribution of the control maximum off real clutter, with no simulator.
#
# At --overlap 0.5, which the overlap sweep beside this makes the setting worth
# calibrating rather than 0.90.
C="/Volumes/ZX20 II/sar-data/capella/CAPELLA_C13_SP_CPHD_HH_20241004001939_20241004002012/CAPELLA_C13_SP_CPHD_HH_20241004001939_20241004002012.cphd"
R=runs/giza/2026-08-04-localpeak-calib
BASE="--estimator phase --n 128 --overlap 0.5 --size 96 --cell 1.0 --win 32 \
      --rbins 4096 --coherence 0"
for OFF in "-150,-150" "0,-150" "150,-150" "-150,0" "0,0" "150,0" "-150,150" "0,150" "150,150"; do
  TAG=$(echo "$OFF" | tr ',-' 'p_')
  echo "########## grid at ${OFF} m"
  ./build/micromotion mmotion --cphd "$C" $BASE --offset "$OFF" \
      --out $R/g$TAG 2>&1 | grep -E 'local peak|strongest'
done
