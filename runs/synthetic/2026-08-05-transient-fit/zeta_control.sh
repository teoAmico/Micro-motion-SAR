#!/bin/zsh
# Does the fitted DAMPING measure transience, or just the presence of signal?
# Three arms at the same amplitude and processing:
#   burst   -- the real accelerometer arrival, a decaying transient
#   sine    -- SUSTAINED, the case that is not a transient at all
#   static  -- motionless, ten realisations so the null has more than n=2
# If zeta tracks transience, sine sits with static. If it tracks signal
# presence, sine sits with burst.
D=/private/tmp/claude-501/-Users-matteoamico-Workspace-micromotion/f78ece78-0dbf-4132-82ce-528a122eb41c/scratchpad
cd /Users/matteoamico/Workspace/micromotion
O=$D/zeta_control_results.txt
echo "arm seed zeta onset resid lead_hz block" > $O
emit() { L=$1; A=$2; S=$3
  Z=$(grep 'median damping' $L | sed -E 's/.*median damping ([0-9.]+),.*/\1/')
  ON=$(grep 'median onset' $L | sed -E 's/.*median onset ([0-9.]+) s.*/\1/')
  RS=$(grep 'median onset' $L | sed -E 's/.*median residual ([0-9.]+) of.*/\1/')
  M=$(grep -E '[0-9]+\.[0-9]+ Hz \(sub-bin' $L | head -1 | awk '{gsub(/[()]/,"");print $1, $8}')
  echo "$A $S ${Z:--} ${ON:--} ${RS:--} ${M:--REFUSED -}" >> $O
}
go() { ./build/micromotion mmotion --cphd $D/zc.cphd --estimator phase \
        --n 128 --overlap 0 --size 128 --cell 0.5 --win 32 --upsample 200 \
        --coherence 0 --tfit 3 --out $D/zc > $1 2>&1; }
for seed in 3 5 7 11 13 17 19 23 29 31; do
  ./build/sim_cphd $D/zc.cphd 0.5 0.0 --clutter 400 --clutter-vib --seed $seed >/dev/null 2>&1
  go $D/zc_static_$seed.log; emit $D/zc_static_$seed.log static $seed
done
for seed in 3 5 7 11 13 17; do
  ./build/sim_cphd $D/zc.cphd 0.5 0.002 --clutter 400 --clutter-vib --seed $seed >/dev/null 2>&1
  go $D/zc_sine_$seed.log; emit $D/zc_sine_$seed.log sine $seed
done
for seed in 3 5 7 11 13 17; do
  ./build/sim_cphd $D/zc.cphd 0.5 0.002 --clutter 400 --clutter-vib --seed $seed \
      --wave "$D/burst_20s.txt,41.6667" >/dev/null 2>&1
  go $D/zc_burst_$seed.log; emit $D/zc_burst_$seed.log burst $seed
done
echo ZETA_CONTROL_DONE >> $O
