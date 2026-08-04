#!/bin/sh
# Item 42 claimed 0.1 m localisation from five placements ALL AT 2 mm, the strong
# end. Item 37 showed the reported window moving with amplitude at a fixed
# position, so this is where that claim is most likely to break. Same five
# interior placements at 0.125 mm -- sixteen times weaker.
C="/Volumes/ZX20 II/sar-data/capella/CAPELLA_C13_SP_CPHD_HH_20241004001939_20241004002012/CAPELLA_C13_SP_CPHD_HH_20241004001939_20241004002012.cphd"
R=runs/giza/2026-08-04-centroid-lowamp
COMMON="--at 29.979175,31.134186 --estimator phase --n 128 --overlap 0.90 \
        --size 96 --cell 1.0 --win 32 --rbins 4096 --coherence 0 --probe-hz 0.163"
for AT in "0,0" "-16,-16" "-16,0" "0,16" "16,-16"; do
  TAG=$(echo "$AT" | tr ',-' 'p_')
  echo "########## 0.125 mm at ${AT} m"
  ./build/micromotion mmotion --cphd "$C" $COMMON \
      --inject-vib "0.163,0.125,20" --inject-at "$AT" --out $R/at${TAG} 2>&1 \
    | grep -E 'strongest|located at'
done
