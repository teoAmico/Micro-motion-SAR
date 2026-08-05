#!/bin/zsh
# Create a run directory and seed its RUN.md. See runs/README.md for the
# layout, what belongs in a run directory, and what is committed.
#
# Exists so that recording provenance is easier than not recording it. The
# alternative -- remembering to write it afterwards -- is what produced cubes
# labelled "model unknown" and figures whose processing route had to be
# reconstructed from shell history.
#
# usage: tools/new-run.sh <scene> <suffix> "<what question this answers>"
set -e
[ $# -ge 2 ] || { echo "usage: $0 <scene> <suffix> [question]" >&2; exit 1; }
scene=$1; suffix=$2; question=${3:-"(not stated)"}
dir="runs/$scene/$(date +%Y-%m-%d)-$suffix"
mkdir -p "$dir"
{
  echo "# Run: $(date +%Y-%m-%d) $scene / $suffix"
  echo
  echo "**Question this run is meant to answer:** $question"
  echo
  echo "- git commit: \`$(git rev-parse --short HEAD 2>/dev/null || echo unknown)\`"
  echo "- started:    $(date -u +%Y-%m-%dT%H:%M:%SZ)"
  echo "- host:       $(uname -sm)"
  echo
  echo "## Collect"
  echo
  echo "\`\`\`"
  echo "(fill in: file, size, dwell, pulses, range bins)"
  echo "\`\`\`"
  echo
  echo "## Commands"
  echo
  echo "\`\`\`sh"
  echo "(paste each command verbatim, including every option)"
  echo "\`\`\`"
  echo
  echo "## Result"
  echo
  echo "*To be completed. A null result stays here rather than being deleted.*"
} > "$dir/RUN.md"

# The pre-registration is seeded BESIDE the run record and is meant to be
# committed BEFORE the first processing command, so the git history shows the
# hypotheses predate the data. See docs/PREREGISTRATION.md for why: everything in
# FOLLOW-UPS.md was scored after the numbers were in, and items 2, 38, 40, 69-74,
# 77, 84 and 86 are what that cost.
# Only the FORM is copied, from the "# PREREG" heading down; the rationale stays
# in docs/PREREGISTRATION.md so it is maintained in one place.
{
  echo "<!-- Why this form exists, and the failures it is aimed at:"
  echo "     docs/PREREGISTRATION.md -->"
  awk '/^# PREREG/,0' docs/PREREGISTRATION.md
} | sed -e "s|<scene>|$scene|" -e "s|<suffix>|$suffix|" \
      -e "s|\`<date>\`|\`$(date -u +%Y-%m-%d)\`|" \
      -e "s|\`<sha>\`|\`$(git rev-parse --short HEAD 2>/dev/null || echo unknown)\`|" \
  > "$dir/PREREG.md"
echo "$dir"
echo "seeded $dir/PREREG.md -- fill it in and COMMIT IT BEFORE running anything" >&2
