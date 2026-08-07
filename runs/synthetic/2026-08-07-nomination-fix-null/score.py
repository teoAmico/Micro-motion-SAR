#!/usr/bin/env python3
"""Score item 110's H3b arm: how many of the twelve motionless scenes survive the
128-vs-256 stability test, and how many of the six injected ones do.

The rule is item 107's, unchanged: a scene reports only if its modal leading
frequency at the two look counts agrees within half a bin of the LOWER count, and
a frequency above the lower count's Nyquist is NOT COMPARABLE rather than
unstable.  Reads the `kind seed n_looks modal_hz prominence` table the sweep
writes."""
import sys
from collections import defaultdict

DT = 0.046959429882 * 0          # unused; df comes from the look count below


def rows(path):
    out = defaultdict(dict)
    for line in open(path):
        f = line.split()
        if len(f) != 5 or f[0] not in ('static', 'injected'):
            continue
        kind, seed, n, hz, prom = f
        out[(kind, seed)][int(n)] = (hz, prom)
    return out


def main(path, dwell_s):
    """dwell_s is the fixture's dwell, so df = 1/T at either look count and the
    Nyquist is N/(2T).  Both counts share the dwell here, which is why item 107's
    `df` warning does not fire."""
    df = 1.0 / dwell_s
    tally = defaultdict(lambda: [0, 0, 0])       # report / reject / abstain
    for (kind, seed), r in sorted(rows(path).items()):
        if 128 not in r or 256 not in r:
            continue
        lo, hi = r[128][0], r[256][0]
        if lo == 'REFUSED' or hi == 'REFUSED':
            verdict = 'REFUSED'
        else:
            flo, fhi = float(lo), float(hi)
            if fhi > 128 / (2.0 * dwell_s):
                verdict = 'NOT COMPARABLE'
            elif abs(fhi - flo) <= 0.5 * df:
                verdict = 'STABLE -> report'
            else:
                verdict = 'MOVED -> reject'
        t = tally[kind]
        t[0 if verdict.startswith('STABLE') else
          1 if verdict.startswith('MOVED') else 2] += 1
        print(f"{kind:9s} seed {seed:>2s}  {lo:>7s} @128  {hi:>7s} @256   {verdict}")
    print()
    for kind, (rep, rej, absta) in sorted(tally.items()):
        print(f"{kind:9s}: {rep} report, {rej} reject, {absta} abstain")


if __name__ == '__main__':
    main(sys.argv[1], float(sys.argv[2]) if len(sys.argv) > 2 else 6.6132)
