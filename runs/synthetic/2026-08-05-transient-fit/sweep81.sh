#!/bin/zsh
# Item 81: the same 12 injected points + 2 static controls as items 74/77/80,
# through the JOINT TRANSIENT-AND-MODE FIT instead of the periodogram. Captures
# the transient parameters too -- median damping and onset -- because those are
# quantities a periodogram cannot report at all, and the question is whether
# they separate a driven scene from a motionless one.
D=/private/tmp/claude-501/-Users-matteoamico-Workspace-micromotion/f78ece78-0dbf-4132-82ce-528a122eb41c/scratchpad
cd /Users/matteoamico/Workspace/micromotion
O=$D/sweep81_results.txt
echo "true_hz seed reported_hz subbin sd block p support zeta onset resid" > $O
emit() { L=$1; T=$2; S=$3
  Z=$(grep 'median damping' $L | sed -E 's/.*median damping ([0-9.]+),.*/\1/')
  ON=$(grep 'median onset' $L | sed -E 's/.*median onset ([0-9.]+) s.*/\1/')
  RS=$(grep 'median onset' $L | sed -E 's/.*median residual ([0-9.]+) of.*/\1/')
  M=$(grep -E '[0-9]+\.[0-9]+ Hz \(sub-bin' $L | head -1 | awk '{gsub(/[()]/,"");print $1, $4, $6, $8, $10, $12}')
  if [ -z "$M" ]; then echo "$T $S REFUSED - - - - - ${Z:--} ${ON:--} ${RS:--}" >> $O
  else echo "$T $S $M ${Z:--} ${ON:--} ${RS:--}" >> $O; fi
}
for seed in 7 11; do
  for pair in "41.6667 0.300" "50 0.400" "58 0.450" "66 0.550" "75 0.750" "85 0.850"; do
    R=${pair%% *}; TRUE=${pair##* }
    ./build/sim_cphd $D/s81.cphd 0.5 0.002 --clutter 400 --clutter-vib --seed $seed \
        --wave "$D/burst_20s.txt,$R" >/dev/null 2>&1
    ./build/micromotion mmotion --cphd $D/s81.cphd --estimator phase \
        --n 128 --overlap 0 --size 128 --cell 0.5 --win 32 --upsample 200 \
        --coherence 0 --tfit 3 --out $D/s81 > $D/s81_${seed}_${R}.log 2>&1
    emit $D/s81_${seed}_${R}.log $TRUE $seed
  done
done
for seed in 7 11; do
  ./build/sim_cphd $D/s81.cphd 0.5 0.0 --clutter 400 --clutter-vib --seed $seed >/dev/null 2>&1
  ./build/micromotion mmotion --cphd $D/s81.cphd --estimator phase \
      --n 128 --overlap 0 --size 128 --cell 0.5 --win 32 --upsample 200 \
      --coherence 0 --tfit 3 --out $D/s81 > $D/s81_static_${seed}.log 2>&1
  emit $D/s81_static_${seed}.log STATIC $seed
done
echo SWEEP81_DONE >> $O
