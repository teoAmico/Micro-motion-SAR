#!/bin/zsh
# Item 117: the stabilization LADDER on REAL Kilauea clutter.
# Pre-registered at d949eb1. Eight configurations, six rungs each; the same
# flags as ../2026-08-07-bracket/rerun.sh, only --n varying between rungs.
D=/private/tmp/claude-501/-Users-matteoamico-Workspace-micromotion/cdb256dd-5a5b-4955-9cf1-adfbd7d0321e/scratchpad
DEST="/Volumes/ZX20 II/micromotion/datasets/kilauea"
cd /Users/matteoamico/Workspace/micromotion
O=$D/ladderreal_results.txt
RUNGS=(96 128 160 192 224 256)
RATE=$(python3 -c "print(200.0*1.00/1.7578)")
echo "collect amp chain verdict" > $O
for S in CAPELLA_C10_SP_CPHD_HH_20240609091921_20240609091951 \
         CAPELLA_C14_SP_CPHD_HH_20240610013244_20240610013313; do
  MP=$(awk -F'\t' -v s=$S '$1==s{print $6}' runs/kilauea/2026-08-05-correlation/kilauea_trunc.tsv)
  for amp in 0.00 0.13 0.26 0.53; do
    if [ "$amp" = "0.00" ]; then INJ=(); else
      INJ=(--inject-wave "$D/naples_wave/seg02.txt,$RATE,$amp,20" --inject-at 24,24); fi
    others=""
    for N in $RUNGS; do
      ./build/micromotion mmotion --cphd "$DEST/$S.cphd" --estimator phase \
          --n $N --overlap 0 --size 256 --cell 0.5 --win 32 --upsample 200 \
          --coherence 0 --max-pulses $MP "${INJ[@]}" \
          --out $D/lr_${S}_${amp}_$N > $D/lr_${S}_${amp}_$N.log 2>&1
      [ $N -ne 128 ] && others="${others:+$others,}$D/lr_${S}_${amp}_${N}_windows.csv"
    done
    ./build/micromotion mmotion --cphd "$DEST/$S.cphd" --estimator phase \
        --n 128 --overlap 0 --size 256 --cell 0.5 --win 32 --upsample 200 \
        --coherence 0 --max-pulses $MP "${INJ[@]}" --stable "$others" \
        --out $D/lr_${S}_${amp}_v > $D/lr_${S}_${amp}_v.log 2>&1
    python3 - "$D/lr_${S}_${amp}_v.log" "$S" "$amp" >> $O <<'PY'
import re,sys
t=open(sys.argv[1]).read()
m=re.search(r"longest chain of CONSECUTIVE agreeing rungs: (\d+) of \d+ needed -> (.+)",t)
print("%s %s %s %s"%(sys.argv[2][8:11],sys.argv[3],
  m.group(1) if m else '-',
  m.group(2).strip().replace(' ','_') if m else 'NO_VERDICT'))
PY
  done
done
echo LADDERREAL_DONE >> $O
