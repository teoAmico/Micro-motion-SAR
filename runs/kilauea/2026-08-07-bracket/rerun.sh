#!/bin/zsh
# Item 114: items 110-113 re-run verbatim against the bracketed null.
# Pre-registered at 6fb1cb4; re-run at de65c45 after the per-scene gate fix. Identical to ../2026-08-07-clustermass/
# rerun.sh in every flag; only the binary differs.
D=/private/tmp/claude-501/-Users-matteoamico-Workspace-micromotion/cdb256dd-5a5b-4955-9cf1-adfbd7d0321e/scratchpad
DEST="/Volumes/ZX20 II/micromotion/datasets/kilauea"
cd /Users/matteoamico/Workspace/micromotion
O=/private/tmp/claude-501/-Users-matteoamico-Workspace-micromotion/cdb256dd-5a5b-4955-9cf1-adfbd7d0321e/scratchpad/bracket_results.txt
echo "collect amp_mm modal128 modal256 verdict" > $O
RATE=$(python3 -c "print(200.0*1.00/1.7578)")
for S in CAPELLA_C10_SP_CPHD_HH_20240609091921_20240609091951 \
         CAPELLA_C14_SP_CPHD_HH_20240610013244_20240610013313; do
  MP=$(awk -F'\t' -v s=$S '$1==s{print $6}' runs/kilauea/2026-08-05-correlation/kilauea_trunc.tsv)
  for amp in 0.00 0.13 0.26 0.53; do
    if [ "$amp" = "0.00" ]; then INJ=(); else
      INJ=(--inject-wave "$D/naples_wave/seg02.txt,$RATE,$amp,20" --inject-at 24,24); fi
    ./build/micromotion mmotion --cphd "$DEST/$S.cphd" --estimator phase \
        --n 256 --overlap 0 --size 256 --cell 0.5 --win 32 --upsample 200 \
        --coherence 0 --max-pulses $MP "${INJ[@]}" \
        --out $D/sc_${S}_${amp}_256 > $D/sc_${S}_${amp}_256.log 2>&1
    ./build/micromotion mmotion --cphd "$DEST/$S.cphd" --estimator phase \
        --n 128 --overlap 0 --size 256 --cell 0.5 --win 32 --upsample 200 \
        --coherence 0 --max-pulses $MP "${INJ[@]}" \
        --stable $D/sc_${S}_${amp}_256_windows.csv \
        --out $D/sc_${S}_${amp}_128 > $D/sc_${S}_${amp}_128.log 2>&1
    python3 - "$D/sc_${S}_${amp}_128.log" "$S" "$amp" >> $O <<'PY'
import re,sys
t=open(sys.argv[1]).read()
m=re.search(r'modal set: ([0-9.]+) Hz here, ([0-9.]+) Hz there -> (.+)',t)
print("%s %s %s %s %s"%(sys.argv[2],sys.argv[3],
  m.group(1) if m else '-', m.group(2) if m else '-',
  m.group(3).strip().replace(' ','_') if m else 'NO_COMPARISON'))
PY
  done
done
echo STABLECENTRED_DONE >> $O
