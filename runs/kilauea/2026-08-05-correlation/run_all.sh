#!/bin/zsh
# Wait for the download, then run mmotion on all 16 with IDENTICAL settings.
# The measurement is a CORRELATION across collects, so any setting that differs
# between them is a confound -- which is why the whole command line is literal
# here and nothing is derived per scene.
D=/private/tmp/claude-501/-Users-matteoamico-Workspace-micromotion/f78ece78-0dbf-4132-82ce-528a122eb41c/scratchpad
DEST="/Volumes/ZX20 II/micromotion/datasets/kilauea"
R=runs/kilauea/2026-08-05-correlation
cd /Users/matteoamico/Workspace/micromotion
while pgrep -f dl.sh >/dev/null; do sleep 120; done
sleep 60   # settle, so a download RESTART is not mistaken for completion
echo "DOWNLOAD PHASE OVER; $(ls "$DEST" | wc -l) files, $(du -sh "$DEST" | cut -f1)"
while IFS=$'\t' read -r s m k; do
  f="$DEST/$(basename $k)"
  # SIZE-VERIFIED, not merely non-empty. A reset connection left a 69 KB file
  # where 25 GB belonged, and mmotion on a truncated dwell yields a well-formed
  # spectrum from a shortened aperture -- a confident answer from nothing.
  want=$(curl -sSI --max-time 40 "https://capella-open-data.s3.us-west-2.amazonaws.com/$k" | awk 'tolower($1)=="content-length:"{print $2}' | tr -d '\r')
  have=$([ -f "$f" ] && stat -f%z "$f" || echo 0)
  if [ -z "$want" ] || [ "$have" != "$want" ]; then
     echo "SKIP $s -- $have of ${want:-?} bytes, incomplete"; continue
  fi
  [ -s "$R/${s}_windows.csv" ] && { echo "have $s"; continue; }
  echo "=== $s  truth $m um"
  ./build/micromotion mmotion --cphd "$f" \
      --n 128 --overlap 0 --size 256 --cell 0.5 --win 32 \
      --upsample 200 --coherence 0 --max-pulses 60000 \
      --out "$R/$s" > "$R/$s.log" 2>&1
  grep -E 'strongest peak|modal set:|consensus:' "$R/$s.log" | head -3
done < $D/dl_list.txt
echo KILAUEA_DONE
