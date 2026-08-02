"""Offline selection-policy test against item 30's committed evidence.

Ground truth: window 112 of a 15x15 grid holds the injected scatterer; the
injected frequency is in the filename; CONTROL had nothing injected. A candidate
policy must NAME the injected frequency in the five injected runs and REFUSE on
the control. Nothing here reprocesses anything -- it reads the CSVs the runs left.
"""
import glob, os, re, sys
from collections import defaultdict

NAZ = NRG = 15
TRUTH_WIN = 112

def load(path):
    hdr = {}
    rows = []
    head = None
    for line in open(path):
        if line.startswith('#'):
            for k, v in re.findall(r'(\w+)=([-\d.eE]+)', line):
                hdr[k] = float(v)
            continue
        parts = line.rstrip('\n').split(',')
        if head is None:
            head = parts
            continue
        rows.append(dict(zip(head, parts)))
    return hdr, rows

def largest_block(wins):
    wins = set(wins); seen = set(); best = 0
    for w in wins:
        if w in seen: continue
        st = [w]; seen.add(w); size = 0
        while st:
            c = st.pop(); size += 1
            a, r = divmod(c, NRG)
            for na, nr in ((a-1,r),(a+1,r),(a,r-1),(a,r+1)):
                if 0 <= na < NAZ and 0 <= nr < NRG:
                    nb = na*NRG + nr
                    if nb in wins and nb not in seen:
                        seen.add(nb); st.append(nb)
        best = max(best, size)
    return best

runs = {}
for f in sorted(glob.glob('runs/giza/2026-08-02-inject-sweep/sweep_*_windows.csv')):
    tag = os.path.basename(f).split('_')[1]
    hdr, rows = load(f)
    df = hdr['df_hz']
    gated = [r for r in rows if r['passed_gates'] == '1']
    by = defaultdict(list)
    for r in gated:
        by[round(float(r['dominant_hz']) / df)].append(r)
    runs[tag] = dict(df=df, rows=rows, gated=gated, by=by, hdr=hdr)

def bin_of(tag):
    return None if tag == 'CONTROL' else round(float(tag) / runs[tag]['df'])

TAGS = [t for t in runs if t != 'CONTROL']
TAGS.sort()

print(f"{'run':9s} {'gated':>5s}  {'inj bin':>7s}  "
      f"{'best(prom)':>16s} {'consensus':>12s} {'block-rank':>12s} {'block-diff':>12s}")

ctrl = runs['CONTROL']
ctrl_block = {b: largest_block(int(r['window']) for r in rs)
              for b, rs in ctrl['by'].items()}

results = {}
for tag in TAGS + ['CONTROL']:
    R = runs[tag]; df = R['df']; inj = bin_of(tag)

    # policy 1: argmax prominence over gated windows  (what mmotion reports)
    bw = max(R['gated'], key=lambda r: float(r['prominence']))
    p_best = round(float(bw['dominant_hz']) / df)

    # policy 2: plurality of dominant bin
    p_cons = max(R['by'], key=lambda b: len(R['by'][b]))

    # policy 3: rank candidate bins by largest contiguous block
    blocks = {b: largest_block(int(r['window']) for r in rs) for b, rs in R['by'].items()}
    p_blk = max(blocks, key=lambda b: (blocks[b], -b))

    # policy 4: same, differenced against the control's block at the same bin
    diff = {b: blocks[b] - ctrl_block.get(b, 0) for b in blocks}
    p_dif = max(diff, key=lambda b: (diff[b], -b))

    def mark(p):
        if inj is None:
            return f"{p*df:.3f}"
        return f"{p*df:.3f}{'  HIT' if p == inj else '  --'}"

    print(f"{tag:9s} {len(R['gated']):5d}  {('-' if inj is None else f'{inj}'):>7s}  "
          f"{mark(p_best):>16s} {mark(p_cons):>12s} {mark(p_blk):>12s} {mark(p_dif):>12s}")
    results[tag] = dict(best=p_best, cons=p_cons, blk=p_blk, dif=p_dif,
                        blocks=blocks, diff=diff, inj=inj)

print()
for name, key in (('best (prominence)', 'best'), ('consensus (plurality)', 'cons'),
                  ('block-rank', 'blk'), ('block-rank minus control', 'dif')):
    hits = sum(results[t][key] == results[t]['inj'] for t in TAGS)
    cfreq = results['CONTROL'][key] * runs['CONTROL']['df']
    print(f"  {name:26s} {hits}/5 injected runs correct;  control answers {cfreq:.3f} Hz")

print("\n--- Is there a REFUSAL criterion? The frequency is found; the null is not. ---")
print(f"{'run':9s} {'max prom':>9s} {'2nd':>7s} {'ratio':>6s} {'blk@winner':>11s} "
      f"{'blk margin vs ctrl':>19s} {'n@winner':>9s}")
for tag in TAGS + ['CONTROL']:
    R, res = runs[tag], results[tag]
    proms = sorted((float(r['prominence']) for r in R['gated']), reverse=True)
    b = res['best']
    blk = res['blocks'].get(b, 0)
    marg = blk - ctrl_block.get(b, 0)
    n = len(R['by'][b])
    print(f"{tag:9s} {proms[0]:9.2f} {proms[1]:7.2f} {proms[0]/proms[1]:6.3f} "
          f"{blk:11d} {marg:19d} {n:9d}")

print("\n--- separation of the winning window's prominence ---")
inj_max = [max(float(r['prominence']) for r in runs[t]['gated']) for t in TAGS]
ctl_max = max(float(r['prominence']) for r in runs['CONTROL']['gated'])
print(f"  injected runs: {min(inj_max):.2f} .. {max(inj_max):.2f}")
print(f"  control:       {ctl_max:.2f}")
print(f"  gap:           {min(inj_max)/ctl_max:.2f}x  "
      f"({'SEPARATES' if min(inj_max) > ctl_max else 'overlaps'})")
