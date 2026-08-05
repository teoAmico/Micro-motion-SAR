#!/bin/zsh
# Item 95: replication of item 91 on an INDEPENDENT building (Naples).
# Hypotheses pre-registered at commit cb47b06, runs/synthetic/2026-08-05-naples-ambient/PREREG.md
# Segments 02 (setup 1) and 13 (setup 3) -- different setups, so independent in
# time AND in sensor mounting. Rate is per segment from its own dominant.
D=/private/tmp/claude-501/-Users-matteoamico-Workspace-micromotion/f78ece78-0dbf-4132-82ce-528a122eb41c/scratchpad
cd /Users/matteoamico/Workspace/micromotion
O=$D/sweep_naples_results.txt
echo "target_hz seg seed rate_hz reported_hz subbin block p support crit" > $O
emit() { L=$1; T=$2; S=$3; SD=$4; R=$5
  C=$(grep 'chance alone reaches' $L | awk '{print $5}')
  M=$(grep -E '[0-9]+\.[0-9]+ Hz \(sub-bin' $L | head -1 | awk '{gsub(/[()]/,"");print $1, $4, $8, $10, $12}')
  if [ -z "$M" ]; then echo "$T $S $SD $R REFUSED - - - - ${C:--}" >> $O
  else echo "$T $S $SD $R $M ${C:--}" >> $O; fi
}
for seg in 02 13; do
  TRUTH=$(awk -v s=$((10#$seg)) '$1==s{print $2}' $D/naples_wave/truth.txt)
  for seed in 7 11; do
    for target in 0.30 0.40 0.50 0.60 0.75 0.90; do
      RATE=$(python3 -c "print(200.0*$target/$TRUTH)")
      ./build/sim_cphd $D/np.cphd 0.5 0.002 --clutter 400 --clutter-vib --seed $seed \
          --wave "$D/naples_wave/seg$seg.txt,$RATE" >/dev/null 2>&1
      ./build/micromotion mmotion --cphd $D/np.cphd --estimator phase \
          --n 128 --overlap 0 --size 128 --cell 0.5 --win 32 --upsample 200 \
          --coherence 0 --out $D/np > $D/np_${seg}_${seed}_${target}.log 2>&1
      emit $D/np_${seg}_${seed}_${target}.log $target $seg $seed $RATE
    done
  done
done
for seed in 7 11; do
  ./build/sim_cphd $D/np.cphd 0.5 0.0 --clutter 400 --clutter-vib --seed $seed >/dev/null 2>&1
  ./build/micromotion mmotion --cphd $D/np.cphd --estimator phase \
      --n 128 --overlap 0 --size 128 --cell 0.5 --win 32 --upsample 200 \
      --coherence 0 --out $D/np > $D/np_static_$seed.log 2>&1
  emit $D/np_static_$seed.log STATIC - $seed -
done
echo SWEEP_NAPLES_DONE >> $O
