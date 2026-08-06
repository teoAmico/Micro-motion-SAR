#!/bin/zsh
# Item 100, corrected: vary ONLY the analysis grid, holding the simulated scene
# fixed, so the scatterers keep their positions and only their offset from a
# pixel centre changes. The first attempt varied sim_cphd's cell too, which
# re-placed every scatterer and confounded the thing being tested.
D=/private/tmp/claude-501/-Users-matteoamico-Workspace-micromotion/f78ece78-0dbf-4132-82ce-528a122eb41c/scratchpad
cd /Users/matteoamico/Workspace/micromotion
for seed in 7 11; do
  ./build/sim_cphd $D/cs2_$seed.cphd 0.5 0.0 --clutter 400 --clutter-vib --seed $seed >/dev/null 2>&1
  for cell in 1.0 0.5 0.25 0.125; do
    # grid extent held constant in METRES: size scales inversely with cell
    case $cell in
      1.0)   SZ=64  ;;
      0.5)   SZ=128 ;;
      0.25)  SZ=256 ;;
      0.125) SZ=512 ;;
    esac
    ./build/micromotion mmotion --cphd $D/cs2_$seed.cphd --estimator phase \
        --n 128 --overlap 0 --size $SZ --cell $cell --win 32 --upsample 200 \
        --coherence 0 --out $D/c2_${seed}_${cell} > $D/c2_${seed}_${cell}.log 2>&1
  done
done
echo DONE > $D/cellsweep2.done
