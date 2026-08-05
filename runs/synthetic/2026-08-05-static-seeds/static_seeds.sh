#!/bin/zsh
# Is 1.512 Hz "seed 7's fault", or does EVERY motionless realisation invent its
# own confident frequency? Twelve static scenes, identical processing.
D=/private/tmp/claude-501/-Users-matteoamico-Workspace-micromotion/f78ece78-0dbf-4132-82ce-528a122eb41c/scratchpad
cd /Users/matteoamico/Workspace/micromotion
O=$D/static_seeds_results.txt
echo "seed modal_hz block p support prom_hz prominence" > $O
for seed in 3 5 7 11 13 17 19 23 29 31 37 41; do
  ./build/sim_cphd $D/ss.cphd 0.5 0.0 --clutter 400 --clutter-vib --seed $seed >/dev/null 2>&1
  ./build/micromotion mmotion --cphd $D/ss.cphd --estimator phase \
      --n 128 --overlap 0 --size 128 --cell 0.5 --win 32 --upsample 200 \
      --coherence 0 --out $D/ss > $D/ss_$seed.log 2>&1
  M=$(grep -E '[0-9]+\.[0-9]+ Hz \(sub-bin' $D/ss_$seed.log | head -1 | awk '{gsub(/[()]/,"");print $1, $8, $10, $12}')
  P=$(grep 'strongest peak' $D/ss_$seed.log | sed -E 's/.*: ([0-9.]+) Hz, prominence ([0-9.]+).*/\1 \2/')
  echo "$seed ${M:-REFUSED - - -} ${P:--  -}" >> $O
done
echo STATIC_SEEDS_DONE >> $O
