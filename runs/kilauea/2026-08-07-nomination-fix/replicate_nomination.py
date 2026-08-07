#!/usr/bin/env python3
"""Replicate rs_spectrum_compute_opts + rs_local_ratio + rs_spectrum_modal_set
offline from a `--shifts` dump, so the selection can be swept without rebuilding.

This is the exploratory diagnostic that refuted item 109's guard-band
explanation.  It reproduces the binary exactly at the shipped settings (0.499 Hz,
block 14, support 42 on C10 at 0.53 mm, 128 looks), which is what makes a sweep
over it worth reading.

ONE THING IT DOES NOT MODEL, AND THAT OMISSION IS ITSELF PART OF ITEM 110:
`modal()` applies NO support threshold, where the binary derived one of 34 of
225 from a binomial null.  The injected bin's support is 28, so the replica
ranked a candidate the binary had already refused -- which is how the support
gate came to be identified as the first of the two losses.  `modal2()` takes a
ratio function so the neighbourhood can be varied; neither applies the gate.

    python3 replicate_nomination.py run_shifts.csv <injected_bin>
"""
import sys, numpy as np
from collections import deque

LEAK = 3          # RS_SPECTRUM_LEAKAGE_BINS
HALF = 12         # RS_LOCAL_HALF_BINS
PER_WIN = 6       # RS_MODAL_PER_WINDOW

def load(path):
    hdr = {}
    with open(path) as f:
        for line in f:
            if line.startswith('#') and 'dt_s' in line:
                for tok in line[1:].split():
                    if '=' in tok:
                        k, v = tok.split('=', 1)
                        hdr[k] = v
            if not line.startswith('#'):
                break
    d = np.loadtxt(path, delimiter=',', skiprows=3)
    n = int(hdr['looks'])
    dt = float(hdr['dt_s'])
    nwin = int(d[:, 0].max()) + 1
    los = d[:, 6].reshape(nwin, n)      # disp_los_m
    return los, n, dt

def psd_matrix(los, n, dt):
    """Exactly rs_spectrum_compute_opts: linear detrend, Hann /(n-1), one-sided."""
    fs = 1.0 / dt
    i = np.arange(n, dtype=float)
    win = 0.5 * (1.0 - np.cos(2.0 * np.pi * i / (n - 1)))
    wp = (win * win).sum()
    # least-squares line per window
    sx = i.sum(); sxx = (i * i).sum(); det = n * sxx - sx * sx
    sy = los.sum(1); sxy = (los * i).sum(1)
    b = (n * sxy - sx * sy) / det
    a = (sy - b * sx) / n
    y = (los - (a[:, None] + b[:, None] * i[None, :])) * win[None, :]
    # the C code casts to float before the FFT; harmless at this dynamic range
    F = np.fft.rfft(y.astype(np.float32).astype(np.float64), axis=1)
    P = np.abs(F) ** 2
    scale = np.full(P.shape[1], 2.0); scale[0] = 1.0
    if n % 2 == 0:
        scale[-1] = 1.0
    P = P * scale[None, :] / (wp * fs)
    return P, fs / n

def local_ratio(P, guard, half=HALF, k_lo=LEAK):
    """Vectorised rs_local_ratio over one window's spectrum. -1 where undefined."""
    nf = P.shape[0]
    out = np.full(nf, -1.0)
    for k in range(k_lo, nf):
        lo = k - half if k > k_lo + half else k_lo
        hi = min(k + half + 1, nf)
        j = np.arange(lo, hi)
        ref = P[j[np.abs(j - k) > guard]]
        if ref.size < 4:
            continue
        med = np.median(ref)
        if med > 0:
            out[k] = P[k] / med
    return out

def nominate(R, k_lo=LEAK, per_win=PER_WIN):
    """Greedy nomination with the LEAK-bin block, as in rs_spectrum_modal_set."""
    picks = []
    r = R.copy()
    while len(picks) < per_win:
        best, best_r = -1, 0.0
        for k in range(k_lo, len(r)):
            if not r[k] > best_r:
                continue
            if any(abs(k - p) < LEAK for p in picks):
                continue
            best, best_r = k, r[k]
        if best < 0:
            break
        picks.append(best)
    return picks

def largest_block(mask, naz, nrg):
    seen = np.zeros_like(mask, dtype=bool)
    best = 0
    for s in range(mask.size):
        if not mask[s] or seen[s]:
            continue
        q = deque([s]); seen[s] = True; sz = 0
        while q:
            cur = q.popleft(); sz += 1
            ia, ir = cur % naz, cur // naz
            for da, dr in ((-1, 0), (1, 0), (0, -1), (0, 1)):
                a2, r2 = ia + da, ir + dr
                if 0 <= a2 < naz and 0 <= r2 < nrg:
                    nb = r2 * naz + a2
                    if nb < mask.size and mask[nb] and not seen[nb]:
                        seen[nb] = True; q.append(nb)
        best = max(best, sz)
    return best

def modal(P, df, guard, naz, nrg, top=6):
    nwin, nf = P.shape
    support = np.zeros(nf, int)
    voted = np.zeros((nf, nwin), bool)
    for w in range(nwin):
        R = local_ratio(P[w], guard)
        if not (R > 0).any():
            continue
        for k in nominate(R):
            support[k] += 1
            voted[k, w] = True
    rows = []
    for k in range(LEAK, nf):
        if support[k] == 0:
            continue
        blk = largest_block(voted[k], naz, nrg)
        rows.append((blk, support[k], k, k * df))
    rows.sort(key=lambda r: (-r[0], -r[1], r[2]))
    return rows[:top], support, voted

if __name__ == '__main__':
    los, n, dt = load(sys.argv[1])
    P, df = psd_matrix(los, n, dt)
    naz = nrg = int(round(np.sqrt(P.shape[0])))
    print(f"windows {P.shape[0]} ({naz}x{nrg})  bins {P.shape[1]}  df {df:.6f} Hz")
    tgt = int(sys.argv[2]) if len(sys.argv) > 2 else 6
    print(f"injected bin {tgt} = {tgt*df:.3f} Hz;  band floor bin {LEAK} = {LEAK*df:.3f} Hz")
    for guard in (2, 3, 4, 5, 6, 8):
        rows, support, voted = modal(P, df, guard, naz, nrg)
        lead = rows[0]
        print(f"\nguard={guard}: reports {lead[3]:.3f} Hz (bin {lead[2]}) "
              f"block {lead[0]} support {lead[1]}")
        for blk, sup, k, f in rows:
            mark = ' <== INJECTED' if k == tgt else (' <-- band floor' if k == LEAK else '')
            print(f"    {f:7.3f} Hz  bin {k:3d}  block {blk:3d}  support {sup:3d}{mark}")


def local_ratio_ext(P, guard, half=HALF, k_lo=LEAK, n_want=None):
    """Same, but the neighbourhood is EXTENDED on whichever side is available so
    every bin is scored against the same number of reference bins."""
    nf = P.shape[0]
    if n_want is None:
        n_want = 2 * (half - guard)
    out = np.full(nf, -1.0)
    for k in range(k_lo, nf):
        lo, hi = k, k + 1
        ref = []
        # grow outward alternately, skipping the guard, until n_want collected
        d = guard + 1
        while len(ref) < n_want and (k - d >= k_lo or k + d < nf):
            if k - d >= k_lo:
                ref.append(P[k - d])
            if len(ref) < n_want and k + d < nf:
                ref.append(P[k + d])
            d += 1
        if len(ref) < 4:
            continue
        med = np.median(ref)
        if med > 0:
            out[k] = P[k] / med
    return out


def modal2(P, df, guard, naz, nrg, ratio_fn, top=6):
    nwin, nf = P.shape
    support = np.zeros(nf, int)
    voted = np.zeros((nf, nwin), bool)
    for w in range(nwin):
        R = ratio_fn(P[w], guard)
        if not (R > 0).any():
            continue
        for k in nominate(R):
            support[k] += 1
            voted[k, w] = True
    rows = []
    for k in range(LEAK, nf):
        if support[k] == 0:
            continue
        rows.append((largest_block(voted[k], naz, nrg), support[k], k, k * df))
    rows.sort(key=lambda r: (-r[0], -r[1], r[2]))
    return rows[:top]
