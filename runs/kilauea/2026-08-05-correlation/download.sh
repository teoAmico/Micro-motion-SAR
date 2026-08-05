#!/bin/zsh
# Download the 16 Kilauea collects, verifying SIZE against Content-Length.
# The first version accepted any non-empty file as complete, so a connection
# reset left a truncated CPHD that looked exactly like a finished one -- and
# mmotion on a truncated dwell produces a well-formed spectrum from a shortened
# aperture, which is the failure this project warns about everywhere else.
D=/private/tmp/claude-501/-Users-matteoamico-Workspace-micromotion/f78ece78-0dbf-4132-82ce-528a122eb41c/scratchpad
B="https://capella-open-data.s3.us-west-2.amazonaws.com/"
DEST="/Volumes/ZX20 II/micromotion/datasets/kilauea"
while IFS=$'\t' read -r s m k; do
  f="$DEST/$(basename $k)"
  want=$(curl -sSI --max-time 40 "$B$k" | awk 'tolower($1)=="content-length:"{print $2}' | tr -d '\r')
  [ -z "$want" ] && { echo "NO SIZE for $s"; continue; }
  for attempt in 1 2 3 4 5; do
    have=$([ -f "$f" ] && stat -f%z "$f" || echo 0)
    [ "$have" = "$want" ] && break
    echo "GET $(basename $k) attempt $attempt  ($have/$want bytes, truth $m um)"
    curl -sSL --retry 5 --retry-delay 10 -C - "$B$k" -o "$f"
  done
  have=$([ -f "$f" ] && stat -f%z "$f" || echo 0)
  if [ "$have" = "$want" ]; then echo "OK   $(basename $k) $want bytes"
  else echo "FAIL $(basename $k) $have of $want bytes -- NOT usable"; fi
done < $D/dl_list.txt
echo DOWNLOAD_DONE
