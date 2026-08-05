#!/bin/zsh
# Item 97: paired zero-amplitude twin on the SAME scene, differenced at the
# injected frequency. Pre-registered at commit 8543974.
D=/private/tmp/claude-501/-Users-matteoamico-Workspace-micromotion/f78ece78-0dbf-4132-82ce-528a122eb41c/scratchpad
cd /Users/matteoamico/Workspace/micromotion
O=$D/paired_results.txt
echo "kind target seg seed centre_prom median_prom" > $O
read_csv() { python3 -c "
import sys
L=[l.rstrip() for l in open(sys.argv[1])]
i=next(k for k,l in enumerate(L) if not l.startswith('#'))
c=L[i].split(','); ia,ir,pp=c.index('iaz'),c.index('irg'),c.index('probe_prominence')
rows=[l.split(',') for l in L[i+1:] if l.strip()]
v=[float(r[pp]) for r in rows]
cen=[float(r[pp]) for r in rows if int(r[ia])==3 and int(r[ir])==3]
v.sort()
print('%.6f %.6f'%(cen[0] if cen else float('nan'), v[len(v)//2]))
" "$1"; }
for seed in 7 11; do
  for target in 0.30 0.40 0.50 0.60 0.75 0.90; do
    # the TWIN: same seed, same clutter, nothing moving
    ./build/sim_cphd $D/tw.cphd 0.5 0.0 --clutter 400 --clutter-vib --seed $seed >/dev/null 2>&1
    ./build/micromotion mmotion --cphd $D/tw.cphd --estimator phase --n 128 --overlap 0 \
        --size 128 --cell 0.5 --win 32 --upsample 200 --coherence 0 --probe-hz $target \
        --out $D/tw > /dev/null 2>&1
    echo "twin $target - $seed $(read_csv $D/tw_windows.csv)" >> $O
    for seg in 02 13; do
      TRUTH=$(awk -v s=$((10#$seg)) '$1==s{print $2}' $D/naples_wave/truth.txt)
      RATE=$(python3 -c "print(200.0*$target/$TRUTH)")
      ./build/sim_cphd $D/inj.cphd 0.5 0.002 --clutter 400 --clutter-vib --seed $seed \
          --wave "$D/naples_wave/seg$seg.txt,$RATE" >/dev/null 2>&1
      ./build/micromotion mmotion --cphd $D/inj.cphd --estimator phase --n 128 --overlap 0 \
          --size 128 --cell 0.5 --win 32 --upsample 200 --coherence 0 --probe-hz $target \
          --out $D/inj > /dev/null 2>&1
      echo "inj $target $seg $seed $(read_csv $D/inj_windows.csv)" >> $O
    done
  done
done
echo PAIRED_DONE >> $O
