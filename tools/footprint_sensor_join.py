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

# THE FEDERATOR, NOT ONE ARCHIVE. service.iris.edu/fdsnws answers only for what
# IRIS holds, and that is not the world: the first screen run with it returned
# 289 of its 315 hits from IRISDMC, five from INGV and none from Japan or from
# most of EIDA. The federator resolves a query across EVERY FDSN data centre and
# reports which archive holds each channel, so Europe, Japan and the national
# networks are reachable in the same call rather than one endpoint at a time.
FDSN_FEDCATALOG = "https://service.iris.edu/irisws/fedcatalog/1/query"
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
    text = get(FDSN_FEDCATALOG, {
        # The federator refuses a purely spatial query -- it needs a network or
        # station argument -- and get() returns None on the 400, which is
        # indistinguishable from an empty result. Both cost a wrong answer once.
        "net": "*",
        "minlatitude": min(lats), "maxlatitude": max(lats),
        "minlongitude": min(lons), "maxlongitude": max(lons),
        "starttime": start, "endtime": end,
        "level": "station", "format": "text",
    })
    if not text:
        return [], 0
    # The federator's text format is the station format in blocks, each preceded
    # by DATACENTER= and the archive's own service URLs. Those lines carry no
    # pipes and are skipped by the field count, but the datacentre is worth
    # keeping: it is what says WHERE a waveform must then be asked for.
    found, n_box, centre = [], 0, "?"
    for line in text.splitlines():
        line = line.strip()
        if line.startswith("DATACENTER="):
            centre = line.split("=", 1)[1].split(",")[0]
            continue
        if not line or line.startswith("#") or "=" in line.split("|")[0]:
            continue
        parts = line.split("|")
        if len(parts) < 6:
            continue
        n_box += 1
        try:
            lat, lon = float(parts[2]), float(parts[3])
        except ValueError:
            continue
        if inside(lat, lon, ring):
            found.append((parts[0], parts[1], lat, lon, parts[5], centre))
    return found, n_box


def waveform_services(net, sta, start, end):
    """Which archive holds this station, and the URL to ask it for samples.

    The federator's REQUEST format -- its default, not format=text -- emits one
    block per data centre carrying DATACENTER, DATASELECTSERVICE, STATIONSERVICE
    and AVAILABILITYSERVICE. That is the routing table, and it is why the
    federated query is worth making: a station at EIDA or a national archive is
    unanswerable at service.iris.edu, and asking there returns "no data" rather
    than an error.

    Note what this does NOT establish. The federator routes on METADATA, so it
    lists channels for a station that has never recorded a sample -- NP.2030
    comes back with nine channels and holds nothing across the aperture. Sample
    presence still has to be asked of the archive itself, which is the whole
    point of has_waveform() below."""
    text = get(FDSN_FEDCATALOG, {
        "net": net, "sta": sta, "starttime": start, "endtime": end,
    })
    if not text:
        return []
    services, centre, avail = [], None, None
    for line in text.splitlines():
        line = line.strip()
        if line.startswith("DATACENTER="):
            if centre and avail:
                services.append((centre, avail))
            centre = line.split("=", 1)[1].split(",")[0]
            avail = None
        elif line.startswith("DATASELECTSERVICE="):
            # DATASELECT, NOT AVAILABILITY. The availability service is RETIRED:
            # https://service.earthscope.org/fdsnws/availability/1/ answers 410
            # with an HTML page, and get() turns that into None, which reads as
            # "this station has no data". This project has already published one
            # wrong screen that way -- "0 of 553 with data" -- and this is the
            # same endpoint. Dataselect costs the samples and cannot lie about
            # whether they exist.
            avail = line.split("=", 1)[1]
    if centre and avail:
        services.append((centre, avail))
    return services


def has_waveform(net, sta, start, end):
    """Whether any channel actually has samples across the aperture, asked of
    the archive that HOLDS the station rather than of IRIS.

    A station being OPEN is not the same as it having recorded -- strong-motion
    instruments in buildings are usually triggered, so they are open for decades
    and hold data for minutes of it. NP.2030, an instrumented building 200 m
    from a confirmed hit, is open since 2012 and returns nothing at all.

    Asking one archive about a station held at another returned the same empty
    answer as a station with no data, which is how a screen reports a confident
    zero it has not earned."""
    total = 0
    for centre, service in waveform_services(net, sta, start, end):
        url = service.rstrip("/") + "/query?" + urllib.parse.urlencode({
            "net": net, "sta": sta, "starttime": start, "endtime": end,
        })
        try:
            with urllib.request.urlopen(url, timeout=90) as response:
                total += len(response.read())
        except Exception:
            continue
    return total


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
        for net, sta, lat, lon, name, centre in found:
            if has_waveform(net, sta, start, end):
                recording.append((net, sta, lat, lon, name, centre))
            if args.verbose:
                print(f"      {net}.{sta:<6} {lat:8.4f},{lon:9.4f}  {name}")
        verdict = ("MEASUREMENT" if recording else
                   "in footprint, no data" if found else
                   "nearby but outside" if n_box else "nothing in the box")
        print(f"{item['id'][:52]:<52} {n_box:>4} {len(found):>3} {len(recording):>4}  {verdict}")
        for net, sta, lat, lon, name, centre in recording:
            print(f"      -> {net}.{sta} {lat:.4f},{lon:.4f}  {name}")


if __name__ == "__main__":
    main()
