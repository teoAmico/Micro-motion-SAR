#!/bin/sh
# Item 46 changed the phase route's `quality`, which feeds the shared gate every
# selection policy applies. Item 37's floored amplitude sweep was measured with
# the old definition. Re-run it identically -- same --fmin 0.098, same grid --
# and compare against the recorded numbers.
C="/Volumes/ZX20 II/sar-data/capella/CAPELLA_C13_SP_CPHD_HH_20241004001939_20241004002012/CAPELLA_C13_SP_CPHD_HH_20241004001939_20241004002012.cphd"
R=runs/giza/2026-08-04-quality-remeasure
COMMON="--at 29.979175,31.134186 --estimator phase --n 128 --overlap 0.90 \
        --size 96 --cell 1.0 --win 32 --rbins 4096 --coherence 0 --fmin 0.098"
for A in 2.0 1.0 0.5 0.25 0.125 0.0625; do
  echo "########## ${A} mm"
  ./build/micromotion mmotion --cphd "$C" $COMMON --inject-vib "0.163,${A},20" \
      --out $R/a${A} 2>&1 | grep -E 'strongest|located at|scene-derived|DISAGREE'
done
echo "########## uninjected control"
./build/micromotion mmotion --cphd "$C" $COMMON --out $R/none 2>&1 \
  | grep -E 'strongest|located at|scene-derived|DISAGREE'
