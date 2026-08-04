#!/bin/sh
# First measurement run on ICEYE. Three runs, because item 38 made the
# zero-amplitude twin mandatory beside any positive control.
#
# The question Giza could not answer: item 19 found 0 of 225 desert windows
# meeting D_A <= 0.25, so the phase route's precondition was unmet across the
# whole scene and the null said nothing. Houston is urban. If an urban scene
# meets it, that is the first collect here where the phase route is admissible.
I="/Volumes/ZX20 II/sar-data/iceye/ICEYE_X47_Houston.cphd"
R=runs/iceye/2026-08-04-houston-first
COMMON="--estimator phase --n 128 --overlap 0.90 --size 96 --cell 1.0 --win 32 \
        --rbins 4096 --coherence 0 --max-pulses 40320 --probe-hz 1.000"
echo "########## no injection -- the screen"
./build/micromotion mmotion --cphd "$I" $COMMON --out $R/plain 2>&1 \
  | grep -E 'sub-apertures|observable|amplitude dispersion|persistent|strongest|located|scene-derived|backed by|NOT ADJUD|NO FREQ|warning'
echo "########## positive control, 1.0 Hz at 2 mm"
./build/micromotion mmotion --cphd "$I" $COMMON --inject-vib "1.0,2.0,20" --out $R/inj 2>&1 \
  | grep -E 'amplitude dispersion|persistent|strongest|located|scene-derived|backed by'
echo "########## its zero-amplitude twin"
./build/micromotion mmotion --cphd "$I" $COMMON --inject-vib "1.0,0.0,20" --out $R/zero 2>&1 \
  | grep -E 'amplitude dispersion|persistent|strongest|located|scene-derived|backed by'
