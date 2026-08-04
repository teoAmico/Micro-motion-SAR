#!/bin/sh
# BLIND LOCALISATION: does anything find the target without being told where?
#
# Every injection before this landed on the grid origin -- the centre window --
# so "which window" was never a question. --inject-at moves it. The grid is 96 m
# at 1.0 m cells with 32-pixel windows on a 16-pixel stride, so a 16 m step is
# one window. Five placements, none of them the centre.
#
# Scored on WHERE the answer lands, not whether 0.163 Hz comes back.
C="/Volumes/ZX20 II/sar-data/capella/CAPELLA_C13_SP_CPHD_HH_20241004001939_20241004002012/CAPELLA_C13_SP_CPHD_HH_20241004001939_20241004002012.cphd"
R=runs/giza/2026-08-04-blind-localisation
COMMON="--at 29.979175,31.134186 --estimator phase --n 128 --overlap 0.90 \
        --size 96 --cell 1.0 --win 32 --rbins 4096 --coherence 0 --probe-hz 0.163"
for AT in "-32,-32" "-16,16" "0,-32" "16,32" "32,0"; do
  TAG=$(echo "$AT" | tr ',-' 'p_')
  echo "########## injected at ${AT} m from the grid origin"
  ./build/micromotion mmotion --cphd "$C" $COMMON \
      --inject-vib "0.163,2.0,20" --inject-at "$AT" --out $R/at${TAG} 2>&1 \
    | grep -E 'injected a|strongest|scene-derived|backed by'
done
