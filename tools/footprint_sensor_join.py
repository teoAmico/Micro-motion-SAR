#!/usr/bin/env python3
"""Find open SAR collects whose footprint contains a sensor recording during the aperture.

WHY THIS EXISTS. Every sensitivity figure this project has is measured against a
signal it injected itself (FOLLOW-UPS items 38-53). Detection -- deciding
something moved without already knowing the answer -- is unestablished, and
docs/DATASETS.md records that no available collect has independently confirmed
motion. That is a DATA problem, and this is the search for the data.

WHAT COUNTS. Three classes, and only the first is worth anything as truth:

  MEASUREMENT   an instrument recording a displacement or acceleration time
                series, inside the footprint, with data covering the aperture --
                FDSN seismic and strong-motion stations, GNSS, structural health
                monitoring archives.
  OBJECT        a structure known to be there from a map -- OpenStreetMap towers,
                bridges, dams -- with no measurement attached.
  PROXY         evidence that something was happening: ADS-B, AIS, METAR. Not
                structural truth, and item 57 records ADS-B being exactly this.

THE POLYGON INTERSECTION IS THE POINT, not a detail. On the KSTL scene ELEVEN
FDSN stations fall inside the footprint's BOUNDING BOX and NONE inside the actual
10 x 100 km strip -- including three building-mounted strong-motion instruments.
Comparing scene centres, or boxes, would have reported a match that does not
exist.
"""
import argparse, json, math, sys, urllib.parse, urllib.request

FDSN_STATION = "https://service.iris.edu/fdsnws/station/1/query"
FDSN_AVAIL = "https://service.iris.edu/fdsnws/availability/1/query"


def get(url, params, timeout=60):
    """One FDSN GET. Returns text, or None for 204 No Content and any error --
    FDSN answers an empty result with 204, which urllib raises on."""
    try:
        with urllib.request.urlopen(url + "?" + urllib.parse.urlencode(params),
                                    timeout=timeout) as response:
            return response.read().decode()
    except Exception:
        return None


def ring_of(geometry):
    """Outer ring of a GeoJSON Polygon or the first ring of a MultiPolygon."""
    if geometry["type"] == "Polygon":
        return [(point[1], point[0]) for point in geometry["coordinates"][0]]
    return [(point[1], point[0]) for point in geometry["coordinates"][0][0]]


def inside(lat, lon, ring):
    """Crossing-number point-in-polygon on (lat, lon) pairs."""
    result = False
    previous = ring[-1]
    for current in ring:
        y1, x1 = previous
        y2, x2 = current
        if (y1 > lat) != (y2 > lat):
            crossing = (x2 - x1) * (lat - y1) / (y2 - y1) + x1
            if lon < crossing:
                result = not result
        previous = current
    return result


def stations_in(ring, start, end):
    """FDSN stations whose coordinates fall inside 'ring' and that were open
    across the aperture. Queried by bounding box because FDSN takes no polygon,
    then filtered -- which is where almost everything is eliminated."""
    lats = [p[0] for p in ring]
    lons = [p[1] for p in ring]
    text = get(FDSN_STATION, {
        "minlatitude": min(lats), "maxlatitude": max(lats),
        "minlongitude": min(lons), "maxlongitude": max(lons),
        "starttime": start, "endtime": end,
        "level": "station", "format": "text",
    })
    if not text:
        return [], 0
    found, n_box = [], 0
    for line in text.splitlines():
        if not line or line.startswith("#"):
            continue
        parts = line.split("|")
        if len(parts) < 6:
            continue
        n_box += 1
        lat, lon = float(parts[2]), float(parts[3])
        if inside(lat, lon, ring):
            found.append((parts[0], parts[1], lat, lon, parts[5]))
    return found, n_box


def has_waveform(net, sta, start, end):
    """Whether any channel actually has samples across the aperture. A station
    being OPEN is not the same as it having recorded -- strong-motion
    instruments in buildings are usually triggered, so they are open for
    decades and hold data for minutes of it."""
    text = get(FDSN_AVAIL, {
        "net": net, "sta": sta, "starttime": start, "endtime": end,
        "format": "text", "merge": "samplerate",
    })
    if not text:
        return 0
    return sum(1 for line in text.splitlines()
               if line and not line.startswith("#"))


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("items", nargs="+",
                        help="STAC item JSON files, or '-' to read one from stdin")
    parser.add_argument("--verbose", action="store_true")
    args = parser.parse_args()

    print(f"{'scene':<52} {'box':>4} {'in':>3} {'rec':>4}  verdict")
    print("-" * 92)
    for path in args.items:
        item = json.load(sys.stdin if path == "-" else open(path, encoding="utf-8"))
        start = item["properties"]["start_datetime"]
        end = item["properties"]["end_datetime"]
        ring = ring_of(item["geometry"])
        found, n_box = stations_in(ring, start, end)
        recording = []
        for net, sta, lat, lon, name in found:
            if has_waveform(net, sta, start, end):
                recording.append((net, sta, lat, lon, name))
            if args.verbose:
                print(f"      {net}.{sta:<6} {lat:8.4f},{lon:9.4f}  {name}")
        verdict = ("MEASUREMENT" if recording else
                   "in footprint, no data" if found else
                   "nearby but outside" if n_box else "nothing in the box")
        print(f"{item['id'][:52]:<52} {n_box:>4} {len(found):>3} {len(recording):>4}  {verdict}")
        for net, sta, lat, lon, name in recording:
            print(f"      -> {net}.{sta} {lat:.4f},{lon:.4f}  {name}")


if __name__ == "__main__":
    main()
