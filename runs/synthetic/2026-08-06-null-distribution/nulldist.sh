#!/bin/zsh
# Item 99: the EMPIRICAL null distribution of per-window prominence on
# motionless clutter -- the self-calibrating alternative to a modelled trials
# factor (item 98). Twelve scenes x 49 windows = 588 samples, each CSV kept.
D=/private/tmp/claude-501/-Users-matteoamico-Workspace-micromotion/f78ece78-0dbf-4132-82ce-528a122eb41c/scratchpad
cd /Users/matteoamico/Workspace/micromotion
for seed in 3 5 7 11 13 17 19 23 29 31 37 41; do
  ./build/sim_cphd $D/nd.cphd 0.5 0.0 --clutter 400 --clutter-vib --seed $seed >/dev/null 2>&1
  ./build/micromotion mmotion --cphd $D/nd.cphd --estimator phase \
      --n 128 --overlap 0 --size 128 --cell 0.5 --win 32 --upsample 200 \
      --coherence 0 --out $D/nd_$seed > $D/nd_$seed.log 2>&1
done
echo NULLDIST_DONE > $D/nulldist.done
