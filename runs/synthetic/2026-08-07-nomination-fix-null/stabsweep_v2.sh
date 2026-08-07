#!/bin/zsh
# Item 110 H3b: item 107 re-run verbatim against the fixed admission and ranking.
# Pre-registered at fce5869. Twelve motionless scenes and six injected ones, each
# processed at 128 and 256 looks. Only --n differs between the paired runs.
D=/private/tmp/claude-501/-Users-matteoamico-Workspace-micromotion/cdb256dd-5a5b-4955-9cf1-adfbd7d0321e/scratchpad
cd /Users/matteoamico/Workspace/micromotion
O=/private/tmp/claude-501/-Users-matteoamico-Workspace-micromotion/cdb256dd-5a5b-4955-9cf1-adfbd7d0321e/scratchpad/stab_results_v2.txt
echo "kind seed n_looks modal_hz prominence" > $O
grab() { python3 - "$1" "$2" "$3" "$4" >> $O <<'PY'
import re,sys
t=open(sys.argv[1]).read()
m=re.search(r'([0-9]+\.[0-9]+) Hz \(sub-bin',t)
p=re.search(r'strongest peak in window \d+: [0-9.]+ Hz, prominence ([0-9.]+)',t)
print("%s %s %s %s %s"%(sys.argv[2],sys.argv[3],sys.argv[4],
    m.group(1) if m else 'REFUSED', p.group(1) if p else '-'))
PY
}
for seed in 3 5 7 11 13 17 19 23 29 31 37 41; do
  ./build/sim_cphd $D/st.cphd 0.5 0.0 --clutter 400 --clutter-vib --seed $seed >/dev/null 2>&1
  for N in 128 256; do
    ./build/micromotion mmotion --cphd $D/st.cphd --estimator phase --n $N --overlap 0 \
       --size 128 --cell 0.5 --win 32 --upsample 200 --coherence 0 \
       --out $D/st_${seed}_$N > $D/st_${seed}_$N.log 2>&1
    grab $D/st_${seed}_$N.log static $seed $N
  done
done
for seed in 3 5 7 11 13 17; do
  ./build/sim_cphd $D/st.cphd 0.5 0.002 --clutter 400 --clutter-vib --seed $seed >/dev/null 2>&1
  for N in 128 256; do
    ./build/micromotion mmotion --cphd $D/st.cphd --estimator phase --n $N --overlap 0 \
       --size 128 --cell 0.5 --win 32 --upsample 200 --coherence 0 \
       --out $D/si_${seed}_$N > $D/si_${seed}_$N.log 2>&1
    grab $D/si_${seed}_$N.log injected $seed $N
  done
done
echo STAB_DONE >> $O
