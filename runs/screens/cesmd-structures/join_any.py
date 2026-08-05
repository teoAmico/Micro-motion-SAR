"""Join CESMD structural records against ANY harvested SAR footprint catalogue.

Same two-stage logic and the same mandatory control as join.py -- a scene's own
centre must fall inside its own ring, because reading a [lat, lon] ring as
[lon, lat] silently voids the whole join and returns a clean-looking zero.

Takes footprints as either a JSON list or JSON lines, so the Capella harvest and
the Umbra STAC harvest go through identical code and the two answers are
comparable.
"""
import json, sys, math, datetime, collections

FLOOR_UM = 5.5
DWELL_MIN = 15.0


def load(path):
    """Read a footprint catalogue as either a JSON list or JSON lines."""
    txt = open(path).read().strip()
    if txt.startswith("["):
        return json.loads(txt)
    return [json.loads(l) for l in txt.split("\n") if l.strip()]


def inside(ring, lat, lon):
    """Ray casting on a ring of [LAT, LON] pairs."""
    x, y, c = lon, lat, False
    n = len(ring)
    for i in range(n):
        y0, x0 = ring[i][0], ring[i][1]
        y1, x1 = ring[(i + 1) % n][0], ring[(i + 1) % n][1]
        if ((y0 > y) != (y1 > y)) and \
           (x < (x1 - x0) * (y - y0) / (y1 - y0 + 1e-300) + x0):
            c = not c
    return c


def ts(s):
    """Parse the CESMD or either footprint time format."""
    s = s.replace("Z", "").replace("T", " ").strip()
    for f in ("%Y-%m-%d %H:%M:%S.%f", "%Y-%m-%d %H:%M:%S"):
        try:
            return datetime.datetime.strptime(s, f)
        except ValueError:
            pass
    return None


def main():
    """Report the funnel for one catalogue against the CESMD structures."""
    foot, stas, label = load(sys.argv[1]), json.load(open(sys.argv[2])), sys.argv[3]
    ctrl = sum(1 for f in foot if inside(f["ring"], f["centre"][0], f["centre"][1]))
    print("== %s ==" % label)
    print("  CONTROL scene centre inside own ring: %d of %d" % (ctrl, len(foot)))
    if ctrl != len(foot):
        print("  containment broken; every count below would be meaningless")
        return
    dw = [f for f in foot if f.get("dur", 0) >= DWELL_MIN]
    print("  collects %d ; dwell >= %.0f s: %d (%.1f%%)"
          % (len(foot), DWELL_MIN, len(dw), 100.0 * len(dw) / max(len(foot), 1)))

    covered = collections.defaultdict(list)
    for s in stas:
        if s["latitude"] is None:
            continue
        for f in foot:
            if inside(f["ring"], s["latitude"], s["longitude"]):
                covered[(s["network"], s["code"])].append(f)
    typ = {}
    for k in covered:
        typ[k] = next(x["type"] for x in stas
                      if (x["network"], x["code"]) == k)
    print("  structures inside a footprint: %d  %s"
          % (len(covered), dict(collections.Counter(typ.values())) or "{}"))
    dwok = {k: [f for f in v if f.get("dur", 0) >= DWELL_MIN]
            for k, v in covered.items()}
    dwok = {k: v for k, v in dwok.items() if v}
    print("  ... of those, covered by a DWELL-ADEQUATE collect: %d  %s"
          % (len(dwok), dict(collections.Counter(typ[k] for k in dwok)) or "{}"))

    hits = 0
    for k, fs in covered.items():
        s = next(x for x in stas if (x["network"], x["code"]) == k)
        for f in fs:
            a0, a1 = ts(f["start"]), ts(f["end"])
            for e in s["events"]:
                rec = e.get("record") or {}
                R = rec.get("epidist")
                if R is None or not e.get("time"):
                    continue
                t0 = ts(e["time"])
                s0 = t0 + datetime.timedelta(seconds=R / 8.0)
                s1 = t0 + datetime.timedelta(seconds=R / 2.5 + 120.0)
                if not (s1 < a0 or s0 > a1):
                    hits += 1
                    print("    HIT %s %s %s M%s pgd %s cm"
                          % (f["id"][:38], s["name"][:30], e["time"],
                             e.get("mag"), rec.get("pgd")))
    print("  records whose shaking overlapped an aperture: %d" % hits)


main()
