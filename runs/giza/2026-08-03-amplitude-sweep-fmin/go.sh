#!/bin/sh
# The same sweep with the lowest bins excluded.
#
# df = 1/30.71 s = 0.0326 Hz. A Hann window's main lobe is +/-2 bins, so bins 1
# and 2 lie inside the leakage skirt of any residual DC or trend and cannot be
# separated from it. --fmin 0.098 = 3*df is the first bin that can.
C="/Volumes/ZX20 II/sar-data/capella/CAPELLA_C13_SP_CPHD_HH_20241004001939_20241004002012/CAPELLA_C13_SP_CPHD_HH_20241004001939_20241004002012.cphd"
R=runs/giza/2026-08-03-amplitude-sweep-fmin
COMMON="--at 29.979175,31.134186 --estimator phase --n 128 --overlap 0.90 \
        --size 96 --cell 1.0 --win 32 --rbins 4096 --coherence 0 --fmin 0.098"
for A in 2.0 1.0 0.5 0.25 0.125 0.0625; do
  echo "########## amplitude ${A} mm"
  ./build/micromotion mmotion --cphd "$C" $COMMON \
      --inject-vib "0.163,${A},20" --out $R/a${A} 2>&1 \
    | grep -E 'strongest|backed by|NOT ADJUD|ADJUDICATED'
done
echo "########## null distribution under the same band floor"
./build/micromotion mmotion --cphd "$C" $COMMON \
    --inject-vib "0.163,2.0,20" --null-static 19 --out $R/null 2>&1 \
  | grep -E 'static trial|empirical|ADJUD|strongest'
