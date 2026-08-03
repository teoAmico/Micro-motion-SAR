#!/bin/sh
# 19 null trials: the smallest M that can reach alpha = 0.05, since p_min = 1/(M+1).
# Injected 0.163 Hz -- item 29's case, where all three spectrum policies converged.
# The null trials are synthesised from the geometry and never see the injection,
# so this one distribution adjudicates both this measurement and the uninjected
# 16.6 already measured at the same settings.
C="/Volumes/ZX20 II/sar-data/capella/CAPELLA_C13_SP_CPHD_HH_20241004001939_20241004002012/CAPELLA_C13_SP_CPHD_HH_20241004001939_20241004002012.cphd"
exec ./build/micromotion mmotion --cphd "$C" --at 29.979175,31.134186 \
    --estimator phase --n 128 --overlap 0.90 \
    --size 96 --cell 1.0 --win 32 --rbins 4096 --coherence 0 \
    --inject-vib "0.163,2.0,20" --null-static 19 \
    --out runs/giza/2026-08-03-alpha-verdict/inj0163
