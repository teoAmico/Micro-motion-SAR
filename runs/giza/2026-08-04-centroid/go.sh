#!/bin/sh
# Item 41 bounded localisation at one window and blamed the window geometry: at
# 50% overlap the target sits in four windows and nothing prefers the centred
# one. This tests the centroid over the agreeing cluster instead.
#
# INTERIOR placements only. Item 40's +-32 m runs put the truth at a grid edge,
# where the cluster is clipped and any centroid is pulled inward by half a
# window -- which is exactly the residual error those runs showed.
C="/Volumes/ZX20 II/sar-data/capella/CAPELLA_C13_SP_CPHD_HH_20241004001939_20241004002012/CAPELLA_C13_SP_CPHD_HH_20241004001939_20241004002012.cphd"
R=runs/giza/2026-08-04-centroid
COMMON="--at 29.979175,31.134186 --estimator phase --n 128 --overlap 0.90 \
        --size 96 --cell 1.0 --win 32 --rbins 4096 --coherence 0 --probe-hz 0.163"
for AT in "0,0" "-16,-16" "-16,0" "0,16" "16,-16"; do
  TAG=$(echo "$AT" | tr ',-' 'p_')
  echo "########## injected at ${AT} m"
  ./build/micromotion mmotion --cphd "$C" $COMMON \
      --inject-vib "0.163,2.0,20" --inject-at "$AT" --out $R/at${TAG} 2>&1 \
    | grep -E 'injected a|strongest|backed by'
done
