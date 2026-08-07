#!/bin/zsh
# Item 115: the stabilization test as a LADDER rather than a pair.
# Pre-registered at HEAD. Twelve motionless scenes and six injected, each
# processed at SIX look counts; only --n differs between a scene's rungs.
D=/private/tmp/claude-501/-Users-matteoamico-Workspace-micromotion/cdb256dd-5a5b-4955-9cf1-adfbd7d0321e/scratchpad
cd /Users/matteoamico/Workspace/micromotion
O=$D/ladder_results.txt
RUNGS=(96 128 160 192 224 256)
echo "kind seed chain p verdict" > $O
run_one() {   # $1 kind, $2 seed, $3 amplitude
  local kind=$1 seed=$2 amp=$3
  ./build/sim_cphd $D/ld.cphd 0.5 $amp --clutter 400 --clutter-vib --seed $seed >/dev/null 2>&1
  local others=""
  for N in $RUNGS; do
    ./build/micromotion mmotion --cphd $D/ld.cphd --estimator phase --n $N --overlap 0 \
       --size 128 --cell 0.5 --win 32 --upsample 200 --coherence 0 \
       --out $D/ld_${kind}_${seed}_$N > $D/ld_${kind}_${seed}_$N.log 2>&1
    [ $N -ne 128 ] && others="${others:+$others,}$D/ld_${kind}_${seed}_${N}_windows.csv"
  done
  ./build/micromotion mmotion --cphd $D/ld.cphd --estimator phase --n 128 --overlap 0 \
     --size 128 --cell 0.5 --win 32 --upsample 200 --coherence 0 \
     --stable "$others" --out $D/ld_${kind}_${seed}_v > $D/ld_${kind}_${seed}_v.log 2>&1
  python3 - "$D/ld_${kind}_${seed}_v.log" "$kind" "$seed" >> $O <<'PY'
import re,sys
t=open(sys.argv[1]).read()
m=re.search(r'longest chain of CONSECUTIVE agreeing rungs: (\d+) \(p ([0-9.]+)\) -> (.+)',t)
print("%s %s %s %s %s"%(sys.argv[2],sys.argv[3],
  m.group(1) if m else '-', m.group(2) if m else '-',
  m.group(3).strip().replace(' ','_') if m else 'NO_VERDICT'))
PY
}
for seed in 3 5 7 11 13 17 19 23 29 31 37 41; do run_one static $seed 0.0; done
for seed in 3 5 7 11 13 17;                    do run_one injected $seed 0.002; done
echo LADDER_DONE >> $O
