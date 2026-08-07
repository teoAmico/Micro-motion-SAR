#!/bin/zsh
# Item 104: does TARGET BRIGHTNESS govern recovery against real clutter?
# Pre-registered at c0de7f5. Only REL varies; amplitude fixed at 0.26 mm, the
# threshold item 103 measured at REL 20.
D=/private/tmp/claude-501/-Users-matteoamico-Workspace-micromotion/f78ece78-0dbf-4132-82ce-528a122eb41c/scratchpad
DEST="/Volumes/ZX20 II/micromotion/datasets/kilauea"
cd /Users/matteoamico/Workspace/micromotion
O=$D/relsweep_results.txt
echo "collect rel reported_hz prominence target_floor_mm scene_median_mm" > $O
RATE=$(python3 -c "print(200.0*1.00/1.7578)")
for S in CAPELLA_C10_SP_CPHD_HH_20240609091921_20240609091951 \
         CAPELLA_C14_SP_CPHD_HH_20240610013244_20240610013313; do
  MP=$(awk -F'\t' -v s=$S '$1==s{print $6}' runs/kilauea/2026-08-05-correlation/kilauea_trunc.tsv)
  for rel in 20 10 5 2 1; do
    ./build/micromotion mmotion --cphd "$DEST/$S.cphd" --estimator phase \
        --n 128 --overlap 0 --size 256 --cell 0.5 --win 32 --upsample 200 \
        --coherence 0 --max-pulses $MP --probe-hz 1.00 \
        --inject-wave "$D/naples_wave/seg02.txt,$RATE,0.26,$rel" \
        --out $D/rl_${S}_$rel > $D/rl_${S}_$rel.log 2>&1
    python3 - "$D/rl_${S}_$rel.log" "$S" "$rel" >> $O <<'PY'
import re,sys
t=open(sys.argv[1]).read()
m=re.search(r'strongest peak in window \d+: ([0-9.]+) Hz, prominence ([0-9.]+)',t)
f=re.search(r'strongest-prominence window \d+: ([0-9.]+) mm',t)
s=re.search(r'scene median ([0-9.]+) mm',t)
print("%s %s %s %s %s %s"%(sys.argv[2],sys.argv[3],
    m.group(1) if m else '-', m.group(2) if m else '-',
    f.group(1) if f else '-', s.group(1) if s else '-'))
PY
  done
done
echo RELSWEEP_DONE >> $O
