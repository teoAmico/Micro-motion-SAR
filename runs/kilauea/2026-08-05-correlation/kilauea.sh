#!/bin/zsh
# Wait for the download, then run mmotion on all 16 with a MATCHED DWELL.
#
# The measurement is a CORRELATION across collects, so anything that differs
# between them is a confound. The previous version enforced that by making the
# whole command line literal, including `--max-pulses 60000` -- and that flag is
# exactly where the confound entered. PRF here runs 7,945 to 10,327 Hz, a 1.30x
# spread, so a fixed pulse COUNT gives dwells of 5.81 to 7.55 s: a 30% spread in
# aperture, hence in frequency resolution and in what each sub-look averages.
# Matching in seconds requires a DIFFERENT pulse count per collect, which is the
# opposite of holding the command line literal.
#
# max_pulses = round(PRF * T_TARGET), precomputed per scene in kilauea_trunc.tsv
# and VERIFIED against each file's own header before use.
#
# WHY TRUNCATE AT ALL: the full products do not fit. 282,972 pulses x 27,650
# samples is 62.6 GB decoded against 24 GB of RAM, and `info` fails on them
# outright. The alternatives were rejected: --rbins cuts BANDWIDTH on an
# FX-domain product rather than range (see docs/CODE-REVIEW.md), and
# --pulse-stride lowers the effective PRF, which no measurement run may use.
#
# T_TARGET = 6.0 s costs frequency resolution: df 0.1667 Hz against 0.0389 Hz at
# full dwell, so the first admissible bin is 0.500 Hz. That is acceptable ONLY
# because this test scores AMPLITUDE against a seismometer's RMS, not frequency.
#
# MEASURED, not assumed. Full dwell is REFUSED by the reader's own check before
# it allocates -- "282972 pulses x 27650 range bins needs 62.6 GB, and this
# machine has 25.8 GB" -- so the truncation is required and not a precaution.
# At T = 6.0 s the first collect runs in 3.84 GB peak RSS and reports 0.1664 Hz,
# which is the predicted 0.1667 to four figures. Note the gate is the READER'S
# CHECK (n_pulse * n_rbin * 8 against free memory), which is far more
# conservative than the resident set actually reached; T must satisfy the check,
# not the RSS. Raising T to the check's limit would buy 7.1 s and df 0.143 Hz --
# not worth an OOM refusal partway through a 16-collect run when free memory
# differs.
T_TARGET=6.0

# ESTIMATOR: item 76 found that the default `correlation` route's precision
# scales with the sub-look RESOLUTION CELL and measured sigma_px at 46.7 m per
# look -- so a tracked series from it carries no micro-motion at all, and every
# conclusion in items 69-74 had to be withdrawn. The previous version of this
# script did not pass --estimator, so it would have run the route that cannot
# see. Change this only with that item read.
ESTIMATOR=phase

D=/private/tmp/claude-501/-Users-matteoamico-Workspace-micromotion/f78ece78-0dbf-4132-82ce-528a122eb41c/scratchpad
DEST="/Volumes/ZX20 II/micromotion/datasets/kilauea"
R=runs/kilauea/2026-08-05-correlation
TSV=$R/kilauea_trunc.tsv
cd /Users/matteoamico/Workspace/micromotion
while pgrep -f dl.sh >/dev/null; do sleep 120; done
sleep 60   # settle, so a download RESTART is not mistaken for completion
echo "DOWNLOAD PHASE OVER; $(ls "$DEST" | wc -l) files, $(du -sh "$DEST" | cut -f1)"
echo "matched dwell T = ${T_TARGET} s, estimator ${ESTIMATOR}"

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

  # The truncation is per collect, so it MUST be checked against the collect.
  # A table row silently paired with the wrong file would reintroduce exactly
  # the mismatched dwell this script exists to remove.
  chk=$(python3 - "$f" "$TSV" "$s" <<'PY'
import re,sys
f,tsv,scene=sys.argv[1],sys.argv[2],sys.argv[3]
x=open(f,'rb').read(700000).decode('utf-8','replace')
nv=re.findall(r"<NumVectors>([^<]+)</NumVectors>",x)
ns=re.findall(r"<NumSamples>([^<]+)</NumSamples>",x)
row=[l.split("\t") for l in open(tsv).read().splitlines()[1:] if l.startswith(scene)]
if not row:            print("ERR no row for %s"%scene); sys.exit()
if not nv or len(ns)<2: print("ERR header unreadable"); sys.exit()
r=row[0]
if int(nv[0])!=int(r[1]) or int(ns[-1])!=int(r[2]):
    print("ERR header %s/%s != table %s/%s"%(nv[0],ns[-1],r[1],r[2])); sys.exit()
print("OK %s %s"%(r[5],r[6]))
PY
)
  if [[ "$chk" != OK* ]]; then echo "SKIP $s -- $chk"; continue; fi
  MP=$(echo $chk | awk '{print $2}'); SEC=$(echo $chk | awk '{print $3}')

  echo "=== $s  truth $m um   max-pulses $MP  = $SEC s"
  ./build/micromotion mmotion --cphd "$f" --estimator $ESTIMATOR \
      --n 128 --overlap 0 --size 256 --cell 0.5 --win 32 \
      --upsample 200 --coherence 0 --max-pulses $MP \
      --out "$R/$s" > "$R/$s.log" 2>&1
  grep -E 'strongest peak|modal set:|consensus:|spectra:' "$R/$s.log" | head -4
done < $D/dl_list.txt
echo KILAUEA_DONE
