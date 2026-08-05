#!/bin/zsh
# Item 90: inject a REAL BUILDING'S AMBIENT displacement and sweep it.
#
# Source: Hospital Real, Granada (Zenodo 17358241), sensor 6, acceleration at
# 200 Hz double-integrated to displacement with a 2.5 Hz high-pass. The corner
# was CHOSEN BY MEASUREMENT: below 2 Hz the dominant displacement frequency
# tracks the corner rather than the building, because 1/omega^2 amplifies the
# broadband floor; at 2.0, 2.5 and 3.0 Hz it is stable at 3.91 Hz.
#
# Each segment's own dominant frequency is the TRUTH for that segment (item 69's
# rule: score against the record, not the instrument), so the playback rate is
# computed PER SEGMENT to place that dominant at the target frequency.
#
# 128 looks, overlap 0, --estimator phase, 2 mm -- identical to items 74/77/80/81
# so the answer is comparable with the burst and the sine.
D=/private/tmp/claude-501/-Users-matteoamico-Workspace-micromotion/f78ece78-0dbf-4132-82ce-528a122eb41c/scratchpad
cd /Users/matteoamico/Workspace/micromotion
O=$D/sweep_granada_results.txt
echo "target_hz seg seed rate_hz reported_hz subbin block p support crit" > $O
emit() { L=$1; T=$2; S=$3; SD=$4; R=$5
  C=$(grep 'chance alone reaches' $L | awk '{print $5}')
  M=$(grep -E '[0-9]+\.[0-9]+ Hz \(sub-bin' $L | head -1 | awk '{gsub(/[()]/,"");print $1, $4, $8, $10, $12}')
  if [ -z "$M" ]; then echo "$T $S $SD $R REFUSED - - - - ${C:--}" >> $O
  else echo "$T $S $SD $R $M ${C:--}" >> $O; fi
}
for seg in 00 05; do
  TRUTH=$(awk -v s=$((10#$seg)) '$1==s{print $2}' $D/granada/truth.txt)
  for seed in 7 11; do
    for target in 0.30 0.40 0.50 0.60 0.75 0.90; do
      RATE=$(python3 -c "print(200.0*$target/$TRUTH)")
      ./build/sim_cphd $D/gr.cphd 0.5 0.002 --clutter 400 --clutter-vib --seed $seed \
          --wave "$D/granada/seg$seg.txt,$RATE" >/dev/null 2>&1
      ./build/micromotion mmotion --cphd $D/gr.cphd --estimator phase \
          --n 128 --overlap 0 --size 128 --cell 0.5 --win 32 --upsample 200 \
          --coherence 0 --out $D/gr > $D/gr_${seg}_${seed}_${target}.log 2>&1
      emit $D/gr_${seg}_${seed}_${target}.log $target $seg $seed $RATE
    done
  done
done
for seed in 7 11; do
  ./build/sim_cphd $D/gr.cphd 0.5 0.0 --clutter 400 --clutter-vib --seed $seed >/dev/null 2>&1
  ./build/micromotion mmotion --cphd $D/gr.cphd --estimator phase \
      --n 128 --overlap 0 --size 128 --cell 0.5 --win 32 --upsample 200 \
      --coherence 0 --out $D/gr > $D/gr_static_$seed.log 2>&1
  emit $D/gr_static_$seed.log STATIC - $seed -
done
echo SWEEP_GRANADA_DONE >> $O
