#!/bin/zsh
# Item 80's sweep: the same 12 injected points plus 2 static controls as items
# 74/77, at BOTH look counts, recording the chance model's threshold and each
# leading mode's p. Question: does pricing the block for chance change any
# verdict that the fixed floor of 4 got wrong?
D=/private/tmp/claude-501/-Users-matteoamico-Workspace-micromotion/f78ece78-0dbf-4132-82ce-528a122eb41c/scratchpad
cd /Users/matteoamico/Workspace/micromotion
O=$D/sweep80_results.txt
echo "looks true_hz seed reported_hz subbin sd block p support crit" > $O
emit() { # log true seed
  L=$1; T=$2; S=$3; N=$4
  C=$(grep 'chance alone reaches' $L | awk '{print $5}')
  M=$(grep -E '[0-9]+\.[0-9]+ Hz \(sub-bin' $L | head -1 | awk '{gsub(/[()]/,"");print $1, $4, $6, $8, $10, $12}')
  if [ -z "$M" ]; then echo "$N $T $S REFUSED - - - - - ${C:--}" >> $O
  else echo "$N $T $S $M ${C:--}" >> $O; fi
}
for N in 128 48; do
  for seed in 7 11; do
    for pair in "41.6667 0.300" "50 0.400" "58 0.450" "66 0.550" "75 0.750" "85 0.850"; do
      R=${pair%% *}; TRUE=${pair##* }
      ./build/sim_cphd $D/s80.cphd 0.5 0.002 --clutter 400 --clutter-vib --seed $seed \
          --wave "$D/burst_20s.txt,$R" >/dev/null 2>&1
      ./build/micromotion mmotion --cphd $D/s80.cphd --estimator phase \
          --n $N --overlap 0 --size 128 --cell 0.5 --win 32 --upsample 200 \
          --coherence 0 --out $D/s80 > $D/s80_${N}_${seed}_${R}.log 2>&1
      emit $D/s80_${N}_${seed}_${R}.log $TRUE $seed $N
    done
  done
  for seed in 7 11; do
    ./build/sim_cphd $D/s80.cphd 0.5 0.0 --clutter 400 --clutter-vib --seed $seed >/dev/null 2>&1
    ./build/micromotion mmotion --cphd $D/s80.cphd --estimator phase \
        --n $N --overlap 0 --size 128 --cell 0.5 --win 32 --upsample 200 \
        --coherence 0 --out $D/s80 > $D/s80_${N}_static_${seed}.log 2>&1
    emit $D/s80_${N}_static_${seed}.log STATIC $seed $N
  done
done
echo SWEEP80_DONE >> $O
