#!/bin/sh
# The paired increment item 38 could not compute: both runs probed at the SAME
# frequency, so their prominence there can be differenced window by window.
C="/Volumes/ZX20 II/sar-data/capella/CAPELLA_C13_SP_CPHD_HH_20241004001939_20241004002012/CAPELLA_C13_SP_CPHD_HH_20241004001939_20241004002012.cphd"
R=runs/giza/2026-08-04-scene-null
COMMON="--at 29.979175,31.134186 --estimator phase --n 128 --overlap 0.90 \
        --size 96 --cell 1.0 --win 32 --rbins 4096 --coherence 0 --probe-hz 0.163"
for A in 0.0 0.5 2.0; do
  echo "########## amplitude ${A} mm"
  ./build/micromotion mmotion --cphd "$C" $COMMON \
      --inject-vib "0.163,${A},20" --out $R/probe_a${A} 2>&1 \
    | grep -E 'probe:|strongest|backed by'
done
echo "########## no injection at all"
./build/micromotion mmotion --cphd "$C" $COMMON --out $R/probe_none 2>&1 \
  | grep -E 'probe:|strongest|backed by'
