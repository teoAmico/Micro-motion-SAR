#!/usr/bin/env python3
"""Find aircraft ON THE GROUND inside a footprint during a radar aperture.

The readsb trace format puts the STRING "ground" in the altitude field when an
aircraft reports itself on the surface, which is the reliable marker -- the
flags word at index 6 that adsblol_cphd_crossmatch.py reads as `on_ground` is
not that field.
"""
import gzip, io, json, sys, tarfile
from datetime import datetime, timezone

START, END = 1742411714.551489, 1742411731.554762     # the CPHD aperture
LAT0, LAT1 = 38.4712, 39.1639
LON0, LON1 = -90.7401, -89.8345
MARGIN = 120.0

class Cat(io.RawIOBase):
    def __init__(self, paths):
        self.paths = iter(paths); self.cur = None
    def readable(self): return True
    def readinto(self, b):
        v = memoryview(b); tot = 0
        while tot < len(v):
            if self.cur is None:
                try: self.cur = open(next(self.paths), "rb")
                except StopIteration: break
            n = self.cur.readinto(v[tot:])
            if n: tot += n
            else: self.cur.close(); self.cur = None
        return tot

rows = []
stream = io.BufferedReader(Cat(sys.argv[1:]), buffer_size=1 << 20)
with tarfile.open(fileobj=stream, mode="r|") as arc:
    for m in arc:
        if not m.isfile() or "/trace_full_" not in m.name: continue
        f = arc.extractfile(m)
        if f is None: continue
        p = f.read()
        if p[:2] == b"\x1f\x8b": p = gzip.decompress(p)
        try: tr = json.loads(p)
        except Exception: continue
        base = float(tr.get("timestamp", 0))
        for pt in tr.get("trace", []):
            if len(pt) < 4 or pt[1] is None or pt[2] is None: continue
            t = base + float(pt[0])
            if t < START - MARGIN or t > END + MARGIN: continue
            lat, lon = float(pt[1]), float(pt[2])
            if not (LAT0 <= lat <= LAT1 and LON0 <= lon <= LON1): continue
            on_ground = (pt[3] == "ground")
            gs = pt[4] if len(pt) > 4 else None
            rows.append((t, tr.get("icao",""), tr.get("r",""), lat, lon,
                         "GROUND" if on_ground else pt[3], gs,
                         START <= t <= END))

rows.sort()
ground = [r for r in rows if r[5] == "GROUND"]
print(f"{len(rows)} reports inside the footprint +/- {MARGIN:.0f}s; {len(ground)} ON GROUND")
print(f"{len([r for r in rows if r[7]])} in aperture; "
      f"{len([r for r in ground if r[7]])} on ground AND in aperture\n")
seen = {}
for t,icao,reg,lat,lon,alt,gs,ina in ground:
    seen.setdefault(icao, []).append((t,reg,lat,lon,gs,ina))
for icao, pts in sorted(seen.items(), key=lambda kv: -len(kv[1])):
    ina = sum(1 for p in pts if p[5])
    speeds = [p[4] for p in pts if p[4] is not None]
    print(f"  {icao} {pts[0][1]:>8}  {len(pts):>3} ground reports ({ina} in aperture)"
          f"  lat {pts[0][2]:.5f} lon {pts[0][3]:.5f}"
          f"  gs {min(speeds) if speeds else '?'}-{max(speeds) if speeds else '?'} kt")
