#!/bin/zsh
# Item 103: does item 102's PREDICTED floor separate recoverable from
# unrecoverable injections on real clutter? Pre-registered at commit 201d897.
#
# Naples segment 02 (a real building's ambient displacement, Zenodo 20667124)
# injected into real Capella phase history at 0.25x, 0.5x, 1x, 2x and 4x the
# predicted 0.53 mm floor, on two independent collects. Each collect's own
# UNINJECTED run is its twin.
D=/private/tmp/claude-501/-Users-matteoamico-Workspace-micromotion/f78ece78-0dbf-4132-82ce-528a122eb41c/scratchpad
DEST="/Volumes/ZX20 II/micromotion/datasets/kilauea"
cd /Users/matteoamico/Workspace/micromotion
O=$D/injfloor_results.txt
echo "collect amp_mm reported_hz prominence twin_llr twin_p" > $O
# Naples seg02's own dominant is 1.7578 Hz; place it at 1.00 Hz in the dwell
RATE=$(python3 -c "print(200.0*1.00/1.7578)")
for S in CAPELLA_C10_SP_CPHD_HH_20240609091921_20240609091951 \
         CAPELLA_C14_SP_CPHD_HH_20240610013244_20240610013313; do
  MP=$(awk -F'\t' -v s=$S '$1==s{print $6}' runs/kilauea/2026-08-05-correlation/kilauea_trunc.tsv)
  # the control: uninjected, and the twin every injected run is differenced against
  ./build/micromotion mmotion --cphd "$DEST/$S.cphd" --estimator phase \
      --n 128 --overlap 0 --size 256 --cell 0.5 --win 32 --upsample 200 \
      --coherence 0 --max-pulses $MP --probe-hz 1.00 \
      --out $D/if_${S}_ctl > $D/if_${S}_ctl.log 2>&1
  F=$(python3 -c "
import sys,re
t=open('$D/if_${S}_ctl.log').read()
m=re.search(r'strongest peak in window \d+: ([0-9.]+) Hz, prominence ([0-9.]+)',t)
print(m.group(1)+' '+m.group(2) if m else '- -')")
  echo "$S 0.00 $F - -" >> $O
  for amp in 0.13 0.26 0.53 1.06 2.12; do
    ./build/micromotion mmotion --cphd "$DEST/$S.cphd" --estimator phase \
        --n 128 --overlap 0 --size 256 --cell 0.5 --win 32 --upsample 200 \
        --coherence 0 --max-pulses $MP --probe-hz 1.00 \
        --inject-wave "$D/naples_wave/seg02.txt,$RATE,$amp" \
        --twin $D/if_${S}_ctl_windows.csv \
        --out $D/if_${S}_$amp > $D/if_${S}_$amp.log 2>&1
    R=$(python3 -c "
import re
t=open('$D/if_${S}_$amp.log').read()
m=re.search(r'strongest peak in window \d+: ([0-9.]+) Hz, prominence ([0-9.]+)',t)
l=re.search(r'twin LLR at [0-9.]+ Hz: best ([0-9.]+) .*?exact p = ([0-9.]+)',t,re.S)
print((m.group(1)+' '+m.group(2) if m else '- -')+' '+(l.group(1)+' '+l.group(2) if l else '- -'))")
    echo "$S $amp $R" >> $O
  done
done
echo INJFLOOR_DONE >> $O
