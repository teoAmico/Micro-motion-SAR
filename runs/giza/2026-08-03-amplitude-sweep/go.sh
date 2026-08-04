#!/bin/sh
# How small an injected vibration still clears the null?
#
# The null trials synthesise their own static scenes and never see the injection,
# so item 35's 19-trial distribution applies unchanged to every amplitude here.
# Running it once and reusing it turns a 5 x 90 min sweep into 5 x 5 min.
# Threshold: a measurement must exceed all 19 (max 23.8) for p = 0.05.
C="/Volumes/ZX20 II/sar-data/capella/CAPELLA_C13_SP_CPHD_HH_20241004001939_20241004002012/CAPELLA_C13_SP_CPHD_HH_20241004001939_20241004002012.cphd"
for A in 2.0 1.0 0.5 0.25 0.125; do
  echo "########## amplitude ${A} mm"
  ./build/micromotion mmotion --cphd "$C" --at 29.979175,31.134186 \
      --estimator phase --n 128 --overlap 0.90 \
      --size 96 --cell 1.0 --win 32 --rbins 4096 --coherence 0 \
      --inject-vib "0.163,${A},20" \
      --out runs/giza/2026-08-03-amplitude-sweep/a${A} 2>&1 \
    | grep -E 'injected a|WARNING: [0-9]|strongest|backed by|NOT ADJUD'
done
