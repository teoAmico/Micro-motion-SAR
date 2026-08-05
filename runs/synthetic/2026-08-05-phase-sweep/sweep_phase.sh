#!/bin/zsh
# Item 74's sweep, re-run on --estimator phase at 2 mm (the phase observable
# wraps beyond lambda/4 ~ 7.8 mm, so item 74's 20 mm is not usable here).
# Records the modal set's LEADING mode, its BLOCK, and prominence's answer.
D=/private/tmp/claude-501/-Users-matteoamico-Workspace-micromotion/f78ece78-0dbf-4132-82ce-528a122eb41c/scratchpad
cd /Users/matteoamico/Workspace/micromotion
O=$D/sweep_phase_results.txt
echo "rate true_hz seed reported_hz block support policy" > $O
for seed in 7 11; do
  for pair in "41.6667 0.300" "50 0.400" "58 0.450" "66 0.550" "75 0.750" "85 0.850"; do
    R=${pair%% *}; TRUE=${pair##* }
    ./build/sim_cphd $D/swp.cphd 0.5 0.002 --clutter 400 --clutter-vib --seed $seed \
        --wave "$D/burst_20s.txt,$R" >/dev/null 2>&1
    ./build/micromotion mmotion --cphd $D/swp.cphd --estimator phase \
        --n 128 --overlap 0 --size 128 --cell 0.5 --win 32 --upsample 200 \
        --coherence 0 --out $D/swp > $D/swp_${seed}_${R}.log 2>&1
    M=$(grep -A8 "modal set:" $D/swp_${seed}_${R}.log | grep -E '[0-9]+\.[0-9]+ Hz +block' | head -1)
    P=$(grep 'strongest peak' $D/swp_${seed}_${R}.log | sed -E 's/.*: ([0-9.]+) Hz.*/\1/')
    if [ -z "$M" ]; then echo "$R $TRUE $seed REFUSED - - modal" >> $O
    else echo "$R $TRUE $seed $(echo $M|awk '{print $1, $4, $6}') modal" >> $O; fi
    echo "$R $TRUE $seed $P - - prominence" >> $O
  done
done
for seed in 7 11; do
  ./build/sim_cphd $D/swp.cphd 0.5 0.0 --clutter 400 --clutter-vib --seed $seed >/dev/null 2>&1
  ./build/micromotion mmotion --cphd $D/swp.cphd --estimator phase \
      --n 128 --overlap 0 --size 128 --cell 0.5 --win 32 --upsample 200 \
      --coherence 0 --out $D/swp > $D/swp_static_${seed}.log 2>&1
  M=$(grep -A8 "modal set:" $D/swp_static_${seed}.log | grep -E '[0-9]+\.[0-9]+ Hz +block' | head -1)
  P=$(grep 'strongest peak' $D/swp_static_${seed}.log | sed -E 's/.*: ([0-9.]+) Hz.*/\1/')
  if [ -z "$M" ]; then echo "- STATIC $seed REFUSED - - modal" >> $O
  else echo "- STATIC $seed $(echo $M|awk '{print $1, $4, $6}') modal" >> $O; fi
  echo "- STATIC $seed $P - - prominence" >> $O
done
echo SWEEP_PHASE_DONE >> $O
