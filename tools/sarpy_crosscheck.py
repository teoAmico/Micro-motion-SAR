#!/usr/bin/env python3
"""Check this project's CPHD reader against SARPy, on a real vendor product.

WHY THIS EXISTS
---------------
Every stage of this pipeline downstream of the reader is tested against fixtures
the project generates itself.  That leaves one hole, and it is the widest one:
a reader that misparses a real vendor file *consistently* is invisible to the
whole suite.  Nothing fails.  The pipeline simply measures a different collect
from the one on disk, and reports a confident spectrum for it -- which is the
failure mode CLAUDE.md and `validate.h` are otherwise written around.

`tools/sim_cphd.c` cannot close this hole.  It writes the project's own private
"RSCH" container, not a conformant CPHD 1.x file -- SARPy refuses to open its
output, correctly -- so the synthetic fixtures exercise the focusing and
tracking arithmetic and never touch the CPHD parse at all.

The evidence that this matters is already in the tree.  `src/readers/cphd.c`
carries an override for Capella products whose signal does not follow their
declared `SGN`, and the note beside it cites SARPy, NGA's reference
implementation, as the authority for the convention.  That defect was found by
comparison against another reader.  Nothing in this repository would have found
it, and nothing in this repository would notice if a second one appeared.

WHAT IS COMPARED, AND WHY THESE FIELDS
--------------------------------------
Derived quantities, not raw metadata.  Reading `SC0` out of a file and printing
it proves only that two programs can find the same eight bytes.  Each field
below is something focusing actually depends on, and each exercises a different
part of the parse:

    n_pulse        the output of the validity screening -- the SIGNAL flag and
                   the finite-geometry test, replicated here
    n_rbin         sample count and the --rbins window
    fc_hz          the SC0 PVP offset, and the byte order
    lambda_m       fc and the constant derived from it
    prf_hz         TxTime, over the pulses actually retained
    dwell_s        the same times, as a span
    dr_m           SCSS and the sample count -- the range scale
    r_near_m       dr and the reference range together
    r_ref_*_m      TxPos, RcvPos and SRPPos, i.e. the bistatic geometry

A disagreement in `r_ref` with agreement in `fc` says the PVP field offsets are
wrong for the position blocks specifically.  A disagreement in `n_pulse` alone
says the two disagree about which vectors are usable.  The fields are chosen so
that the pattern of failures localises the fault.

WHAT IT DOES NOT CHECK
----------------------
The signal samples.  This compares the geometry and timing the pipeline reads,
not the phase history it reads them for -- so the SGN convention, which affects
only the sample values, is OUTSIDE what this can see.  A cross-check of the
samples would have to compress a pulse both ways and decide which lands inside
the declared TOA support, which is `FOLLOW-UPS.md` item 3's open suggestion and
a different piece of work.  Two readers agreeing here does not mean the image
will be right.

USAGE
-----
    pip install sarpy
    python3 tools/sarpy_crosscheck.py /path/to/product.cphd

By default it reads the first 2000 valid vectors, which is enough to exercise
every parse path and takes seconds instead of reading tens of gigabytes; pass
`--all-pulses` for the whole collect.  Exits non-zero on any disagreement.
"""

import argparse
import json
import math
import subprocess
import sys

try:
    import numpy as np
except ImportError:
    sys.exit("sarpy_crosscheck: numpy is required (pip install numpy)")

# The value src/readers/cphd.c uses.  Stated here rather than imported from
# scipy so that a disagreement is a disagreement about the file and never about
# which CODATA revision each side happened to pick up.
C_LIGHT = 299792458.0

# Per-field tolerance, as (absolute, relative).  A field matches if it is within
# EITHER.  These are floating-point-agreement tolerances, not physical ones: two
# correct readers doing the same double-precision arithmetic in a different
# order should agree far inside them, so anything outside is a parse difference
# and not rounding.
TOLERANCE = {
    "n_pulse":       (0, 0),          # counts must be identical
    "n_rbin":        (0, 0),
    "fc_hz":         (0.0, 1e-12),
    "lambda_m":      (0.0, 1e-12),
    "prf_hz":        (0.0, 1e-9),
    "dwell_s":       (1e-9, 0.0),
    "t_first_s":     (1e-9, 0.0),
    "t_last_s":      (1e-9, 0.0),
    "dr_m":          (0.0, 1e-12),
    "r_near_m":      (1e-6, 0.0),     # micrometres on a ~600 km range
    "r_ref_first_m": (1e-6, 0.0),
    "r_ref_last_m":  (1e-6, 0.0),
}


def read_with_micromotion(binary, path, rbins, max_pulses):
    """Run `micromotion info --cphd ... --json` and parse what it prints.

    The binary is invoked rather than linked because that is what a user runs;
    a cross-check against a library entry point could pass while the tool the
    project ships reads something else.  stderr is passed through -- the reader
    reports its skipped-vector counts and the Capella SGN override there, and
    both are worth seeing beside the comparison.
    """
    cmd = [binary, "info", "--cphd", path, "--json"]
    if rbins:
        cmd += ["--rbins", str(rbins)]
    if max_pulses:
        cmd += ["--max-pulses", str(max_pulses)]
    proc = subprocess.run(cmd, capture_output=True, text=True)
    if proc.stderr.strip():
        for line in proc.stderr.strip().splitlines():
            print(f"    [micromotion] {line}")
    if proc.returncode != 0:
        raise SystemExit(f"micromotion exited {proc.returncode}: {proc.stderr.strip()}")
    try:
        return json.loads(proc.stdout)
    except json.JSONDecodeError as exc:
        raise SystemExit(f"could not parse --json output: {exc}\n{proc.stdout[:400]}")


def valid_vector_mask(pvp):
    """Which vectors src/readers/cphd.c would keep.

    Replicated rather than trusted, because the screening is itself a thing that
    can be wrong: a vector is usable when the file does not flag it invalid and
    when every quantity focusing needs is finite.  Both halves matter -- the
    SIGNAL array is optional and some products omit it, while a vector flagged
    invalid carries NaN rather than stale numbers, so neither test subsumes the
    other.

    Returns a boolean array over all vectors.
    """
    n = len(pvp["TxTime"])
    ok = np.ones(n, dtype=bool)

    if "SIGNAL" in pvp.dtype.names:
        ok &= np.asarray(pvp["SIGNAL"]).reshape(n) != 0

    ok &= np.isfinite(np.asarray(pvp["TxTime"]).reshape(n))
    for field in ("TxPos", "RcvPos", "SRPPos"):
        ok &= np.isfinite(np.asarray(pvp[field]).reshape(n, 3)).all(axis=1)
    return ok


def read_with_sarpy(path, rbins, max_pulses):
    """Derive the same quantities from SARPy's parse of the same file."""
    try:
        from sarpy.io.phase_history.converter import open_phase_history
    except ImportError:
        raise SystemExit("sarpy_crosscheck: sarpy is required (pip install sarpy)")

    reader = open_phase_history(path)
    channel = reader.cphd_meta.Data.Channels[0]
    n_samp = int(channel.NumSamples)

    pvp = reader.read_pvp_array(index=0)
    keep = np.flatnonzero(valid_vector_mask(pvp))
    if len(keep) < 2:
        raise SystemExit(f"sarpy: only {len(keep)} usable vector(s)")
    if max_pulses:
        keep = keep[:max_pulses]

    t = np.asarray(pvp["TxTime"]).reshape(-1)[keep]
    tx = np.asarray(pvp["TxPos"]).reshape(-1, 3)[keep]
    rcv = np.asarray(pvp["RcvPos"]).reshape(-1, 3)[keep]
    srp = np.asarray(pvp["SRPPos"]).reshape(-1, 3)[keep]

    # Bistatic reference range, the mean of the two one-way distances.  Written
    # exactly as cphd.c writes it so the comparison tests the parsed positions
    # and not two different conventions for the same word.
    r_ref = 0.5 * (np.linalg.norm(tx - srp, axis=1) + np.linalg.norm(rcv - srp, axis=1))

    # SC0 and SCSS are per-vector.  The reader takes the FIRST retained vector's
    # values and checks the rest for spread, so the first is what to compare.
    fc = float(np.asarray(pvp["SC0"]).reshape(-1)[keep[0]])
    scss0 = float(np.asarray(pvp["SCSS"]).reshape(-1)[keep[0]])

    # The bin spacing comes from the FULL sample count, not the --rbins window:
    # the window crops the compressed profile, it does not change the transform
    # that produced it.  Getting this backwards is an easy and silent error, so
    # it is spelt out.
    dr = C_LIGHT / (2.0 * n_samp * scss0)

    n_rbin = min(n_samp, rbins) if rbins else n_samp
    n_pulse = len(keep)

    return {
        "n_pulse": n_pulse,
        "n_rbin": n_rbin,
        "fc_hz": fc,
        "lambda_m": C_LIGHT / fc,
        "prf_hz": (n_pulse - 1) / (t[-1] - t[0]),
        "dwell_s": float(t[-1] - t[0]),
        "t_first_s": float(t[0]),
        "t_last_s": float(t[-1]),
        "dr_m": dr,
        "r_near_m": float(r_ref[0]) - (n_rbin // 2) * dr,
        "r_ref_first_m": float(r_ref[0]),
        "r_ref_last_m": float(r_ref[-1]),
    }


def agrees(name, a, b):
    """True when two readings of one field are the same measurement."""
    abs_tol, rel_tol = TOLERANCE[name]
    if abs_tol == 0 and rel_tol == 0:
        return a == b
    if not (math.isfinite(a) and math.isfinite(b)):
        return False
    delta = abs(a - b)
    return delta <= abs_tol or (b != 0 and delta / abs(b) <= rel_tol)


def main():
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("cphd", help="path to a CPHD 1.x product")
    ap.add_argument("--binary", default="./build/micromotion",
                    help="the micromotion binary (default ./build/micromotion)")
    ap.add_argument("--rbins", type=int, default=512,
                    help="range-bin window, passed to both sides (default 512)")
    ap.add_argument("--max-pulses", type=int, default=2000,
                    help="valid vectors to read (default 2000)")
    ap.add_argument("--all-pulses", action="store_true",
                    help="read the whole collect; slow on a large product")
    args = ap.parse_args()

    max_pulses = 0 if args.all_pulses else args.max_pulses

    print(f"cross-checking {args.cphd}")
    print(f"  rbins={args.rbins or 'all'} max_pulses={max_pulses or 'all'}\n")

    mine = read_with_micromotion(args.binary, args.cphd, args.rbins, max_pulses)
    theirs = read_with_sarpy(args.cphd, args.rbins, max_pulses)

    print(f"\n  {'field':<16} {'micromotion':>22} {'sarpy':>22}   verdict")
    print(f"  {'-' * 16} {'-' * 22} {'-' * 22}   -------")

    failures = []
    for name in TOLERANCE:
        a, b = mine.get(name), theirs.get(name)
        if a is None or b is None:
            failures.append((name, a, b))
            print(f"  {name:<16} {str(a):>22} {str(b):>22}   MISSING")
            continue
        ok = agrees(name, a, b)
        fmt = (lambda v: f"{v:d}") if isinstance(b, int) else (lambda v: f"{v:.12g}")
        print(f"  {name:<16} {fmt(a):>22} {fmt(b):>22}   {'ok' if ok else 'DIFFERS'}")
        if not ok:
            failures.append((name, a, b))

    print()
    if failures:
        print(f"{len(failures)} field(s) disagree.  This is a reader defect on one")
        print("side or the other, not a tolerance to widen -- both are doing the")
        print("same double-precision arithmetic on the same bytes.")
        return 1

    print("every field agrees.  Note what this does NOT establish: the signal")
    print("samples were not compared, so the SGN convention -- the one vendor")
    print("defect this project has actually hit -- is outside what was checked.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
