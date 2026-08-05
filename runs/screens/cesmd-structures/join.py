"""Join CESMD structural records against the Capella spotlight footprints.

Two stages, deliberately in this order. The SPATIAL stage asks whether any
instrumented building, bridge or dam has ever been inside a Capella footprint at
all; if that is zero the temporal question cannot arise and nothing more needs
running. The SPATIOTEMPORAL stage then asks whether the shaking of a recorded
event overlapped the aperture.

The shaking window is derived, not guessed: strong motion reaches a station at
roughly the S-wave speed and the coda outlasts it, so the window used is
[t0 + R/8.0, t0 + R/2.5 + 120 s] for an epicentral distance R in km -- generous
at both ends on purpose, because a window too narrow would manufacture the
negative this search exists to test.

Intersect the POLYGON, never the bounding box (item 57): at KSTL eleven FDSN
stations fell in the box and none in the strip.
"""
import json, sys, math, csv, datetime

FOOT, CESMD, OUT = sys.argv[1], sys.argv[2], sys.argv[3]
FLOOR_UM = 5.5                      # item 53's end-to-end detection floor


def inside(ring, lat, lon):
    """Ray-casting point-in-polygon. The harvested ring holds [LAT, LON] pairs,
    not [lon, lat] -- reading them the other way round put every scene centre
    outside its own scene, which is the control that caught it."""
    x, y, c = lon, lat, False
    n = len(ring)
    for i in range(n):
        y0, x0 = ring[i][0], ring[i][1]
        y1, x1 = ring[(i + 1) % n][0], ring[(i + 1) % n][1]
        if ((y0 > y) != (y1 > y)) and \
           (x < (x1 - x0) * (y - y0) / (y1 - y0 + 1e-300) + x0):
            c = not c
    return c


def km(lat0, lon0, lat1, lon1):
    """Great-circle distance in km, adequate at these scales."""
    p = math.pi / 180.0
    a = 0.5 - math.cos((lat1 - lat0) * p) / 2 + math.cos(lat0 * p) * \
        math.cos(lat1 * p) * (1 - math.cos((lon1 - lon0) * p)) / 2
    return 12742 * math.asin(math.sqrt(max(a, 0.0)))


def ts(s):
    """Parse either the CESMD or the footprint time format to a datetime."""
    s = s.replace("Z", "").replace("T", " ").strip()
    return datetime.datetime.strptime(s, "%Y-%m-%d %H:%M:%S")


def main():
    """Run both stages and print the funnel with every count that was dropped."""
    foot = json.load(open(FOOT))
    stas = json.load(open(CESMD))
    long_dwell = [f for f in foot if f["dur"] >= 15.0]

    # --- stage 1: spatial ---
    spatial = []
    best = []
    for s in stas:
        lat, lon = s["latitude"], s["longitude"]
        if lat is None or lon is None:
            continue
        for f in foot:
            if inside(f["ring"], lat, lon):
                spatial.append((s, f))
            else:
                d = km(lat, lon, f["centre"][0], f["centre"][1])
                best.append((d, s["name"], f["id"]))
    best.sort()
    # The control that makes the negative readable: a scene centre must fall
    # inside its own scene. It did not, the first time -- the ring holds
    # [lat, lon] and was being read as [lon, lat], so the whole join was void.
    ctrl = sum(1 for f in foot if inside(f["ring"], f["centre"][0], f["centre"][1]))
    print("CONTROL -- scene centre inside its own ring: %d of %d" % (ctrl, len(foot)))
    if ctrl != len(foot):
        print("  containment test is broken; every count below is meaningless")
        return
    print("STAGE 1 -- spatial")
    print("  %d structures x %d footprints" % (len(stas), len(foot)))
    print("  structures inside a footprint: %d" % len(spatial))
    if not spatial:
        print("  closest approaches (structure centre to scene centre):")
        for d, n, i in best[:8]:
            print("    %7.1f km  %-42s %s" % (d, n[:42], i[:44]))
        print("\nSTAGE 2 not run: nothing is inside a footprint, so no record "
              "can overlap an aperture.")
        return

    # --- stage 2: spatiotemporal ---
    w = csv.writer(open(OUT, "w", newline=""))
    w.writerow(["scene", "dwell_s", "type", "net", "sta", "name", "lat", "lon",
                "event", "mag", "epidist_km", "event_time", "aperture_start",
                "aperture_end", "pgd_cm", "pgv_cms", "pgd_um", "above_floor"])
    hits = 0
    for s, f in spatial:
        a0, a1 = ts(f["start"]), ts(f["end"])
        for e in s["events"]:
            rec = e.get("record") or {}
            R = rec.get("epidist")
            if R is None or not e.get("time"):
                continue
            t0 = ts(e["time"])
            s0 = t0 + datetime.timedelta(seconds=R / 8.0)
            s1 = t0 + datetime.timedelta(seconds=R / 2.5 + 120.0)
            if s1 < a0 or s0 > a1:
                continue
            pgd = rec.get("pgd")
            pgd_um = pgd * 10000.0 if pgd is not None else None
            hits += 1
            w.writerow([f["id"], round(f["dur"], 1), s["type"], s["network"],
                        s["code"], s["name"], s["latitude"], s["longitude"],
                        e["id"], e.get("mag"), R, e["time"], f["start"],
                        f["end"], pgd, rec.get("pgv"),
                        None if pgd_um is None else round(pgd_um, 1),
                        None if pgd_um is None else pgd_um >= FLOOR_UM])
    print("\nSTAGE 2 -- spatiotemporal")
    print("  records whose shaking overlapped an aperture: %d" % hits)


main()
