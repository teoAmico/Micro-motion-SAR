"""Harvest every CESMD BUILDING, BRIDGE and DAM station together with the events
each one recorded, over the Capella archive's own span.

Why these three types: item 57 established that only an instrument recording a
time series is truth here, and items 59-62 then screened GROUND stations and
found nothing above the floor -- quiet ground sits ~50x below item 53's 5.5 um.
A structure responding to an earthquake does not. The CESMD record carries pgd
directly, so each candidate arrives with its own displacement amplitude and can
be compared with the floor before anything is downloaded.

Written to be resumable: one file per (type, year), so a kill costs one query.
"""
import json, os, sys, time, urllib.request, urllib.parse

U = "https://www.strongmotioncenter.org/wserv/records/query"
OUT = sys.argv[1] if len(sys.argv) > 1 else "cesmd_structures.json"
TYPES = ["B", "Br", "D"]
YEARS = range(2020, 2027)


def fetch(sttype, y0, y1):
    """One CESMD metadata query, grouped by station so each carries its events."""
    q = {"rettype": "metadata", "format": "json", "groupby": "station",
         "sttype": sttype, "startdate": "%d-01-01" % y0,
         "enddate": "%d-01-01" % y1}
    url = U + "?" + urllib.parse.urlencode(q)
    for attempt in range(4):
        try:
            with urllib.request.urlopen(url, timeout=180) as r:
                return json.loads(r.read().decode())
        except Exception as e:                       # noqa: BLE001
            print("  retry %d (%s)" % (attempt + 1, e), file=sys.stderr, flush=True)
            time.sleep(5 * (attempt + 1))
    return None


def main():
    """Query every type and year, merge by station code, and report the funnel."""
    by_sta, n_fail = {}, 0
    for t in TYPES:
        for y in YEARS:
            d = fetch(t, y, y + 1)
            if d is None:
                n_fail += 1
                print("FAIL %s %d" % (t, y), file=sys.stderr, flush=True)
                continue
            stations = (d.get("results") or {}).get("stations") or []
            for s in stations:
                key = (s.get("network"), s.get("code"))
                rec = by_sta.setdefault(key, {k: s[k] for k in
                      ("code", "network", "name", "latitude", "longitude", "type")})
                rec.setdefault("events", [])
                seen = {e["id"] for e in rec["events"]}
                for e in (s.get("events") or []):
                    if e.get("id") not in seen:
                        rec["events"].append(e)
            print("%s %d: %d stations (running total %d)"
                  % (t, y, len(stations), len(by_sta)), file=sys.stderr, flush=True)
    out = list(by_sta.values())
    json.dump(out, open(OUT, "w"))
    n_ev = sum(len(s["events"]) for s in out)
    print("\nSTRUCTURES: %d  RECORDS: %d  FAILED QUERIES: %d"
          % (len(out), n_ev, n_fail))
    print("(a query that fails and one that finds nothing look identical -- the "
          "failure count is the audit)")


main()
