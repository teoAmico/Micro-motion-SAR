#!/bin/zsh
# Item 105: does the SUB-LOOK COUNT set the target's SCR, and therefore the floor?
#
# The literature's mechanism: SCR rises as the resolution cell shrinks, because a
# smaller cell holds less clutter. Sub-aperture decomposition runs that backwards
# -- N sub-looks each carry 1/N of the azimuth bandwidth, so each cell is ~N times
# larger. PREDICTION: the target-window floor scales as sqrt(N) from the
# averaging gain TIMES N from the SCR loss, i.e. the per-look phase sd rises as N
# and the floor as sqrt(N).
#
# A fixed target (REL 20, 0.26 mm, 1.00 Hz) on one real collect, look count swept.
# Everything else held. --size is scaled so the grid covers the same ground.
D=/private/tmp/claude-501/-Users-matteoamico-Workspace-micromotion/f78ece78-0dbf-4132-82ce-528a122eb41c/scratchpad
DEST="/Volumes/ZX20 II/micromotion/datasets/kilauea"
cd /Users/matteoamico/Workspace/micromotion
O=$D/looksweep_results.txt
echo "n_looks df_hz reported_hz prominence target_floor_mm scene_median_mm phase_sd" > $O
S=CAPELLA_C10_SP_CPHD_HH_20240609091921_20240609091951
MP=$(awk -F'\t' -v s=$S '$1==s{print $6}' runs/kilauea/2026-08-05-correlation/kilauea_trunc.tsv)
RATE=$(python3 -c "print(200.0*1.00/1.7578)")
for N in 16 32 64 128 256; do
  ./build/micromotion mmotion --cphd "$DEST/$S.cphd" --estimator phase \
      --n $N --overlap 0 --size 256 --cell 0.5 --win 32 --upsample 200 \
      --coherence 0 --max-pulses $MP --probe-hz 1.00 \
      --inject-wave "$D/naples_wave/seg02.txt,$RATE,0.26,20" \
      --shifts $D/lk_$N.csv --out $D/lk_$N > $D/lk_$N.log 2>&1
  python3 - "$D/lk_$N.log" "$N" >> $O <<'PY'
import re,sys
t=open(sys.argv[1]).read()
m=re.search(r'strongest peak in window \d+: ([0-9.]+) Hz, prominence ([0-9.]+)',t)
f=re.search(r'strongest-prominence window \d+: ([0-9.]+) mm \(phase sd ([0-9.]+) rad\)',t)
s=re.search(r'scene median ([0-9.]+) mm',t)
d=re.search(r'spectra: \d+ bins, ([0-9.]+) Hz',t)
print("%s %s %s %s %s %s %s"%(sys.argv[2], d.group(1) if d else '-',
  m.group(1) if m else '-', m.group(2) if m else '-',
  f.group(1) if f else '-', s.group(1) if s else '-', f.group(2) if f else '-'))
PY
done
echo LOOKSWEEP_DONE >> $O
