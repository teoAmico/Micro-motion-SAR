"""Cross-match Umbra open-data dwells with plausible earthquake arrivals.

The first-stage result is intentionally permissive.  It reports acquisitions
whose scene-centre P/S arrival interval overlaps the SAR dwell; an independent
station inside the exact footprint must then be found and its waveform screened.
"""
import concurrent.futures as cf
import datetime as dt
import json
import math
import urllib.parse
import urllib.request

STAC = "https://umbra-open-data-catalog.s3.us-west-2.amazonaws.com/stac/"
USGS = "https://earthquake.usgs.gov/fdsnws/event/1/query"


def get(url):
    try:
        with urllib.request.urlopen(url, timeout=45) as response:
            return json.load(response)
    except Exception:
        return None


def children(url, relations=("child", "item")):
    data = get(url)
    if not data:
        return []
    base = url.rsplit("/", 1)[0] + "/"
    return [urllib.parse.urljoin(base, link["href"])
            for link in data.get("links", []) if link.get("rel") in relations]


def parse_time(value):
    return dt.datetime.fromisoformat(value.replace("Z", "+00:00"))


def item_summary(url):
    item = get(url)
    if not item or item.get("type") != "Feature":
        return None
    bbox = item.get("bbox")
    props = item.get("properties", {})
    stamp = props.get("datetime")
    if not bbox or not stamp:
        return None
    start = props.get("start_datetime", stamp)
    end = props.get("end_datetime", stamp)
    return {
        "id": item.get("id"), "bbox": bbox,
        "lat": (bbox[1] + bbox[3]) / 2, "lon": (bbox[0] + bbox[2]) / 2,
        "start": start, "end": end, "datetime": stamp, "item": url,
    }


def distance_km(lat1, lon1, lat2, lon2):
    radius = 6371.0
    p1, p2 = math.radians(lat1), math.radians(lat2)
    dp = math.radians(lat2 - lat1)
    dl = math.radians(lon2 - lon1)
    value = math.sin(dp / 2) ** 2 + math.cos(p1) * math.cos(p2) * math.sin(dl / 2) ** 2
    return 2 * radius * math.asin(math.sqrt(value))


months = []
for year in ("2024", "2025"):
    months.extend(children(STAC + year + "/catalog.json", ("child",)))
with cf.ThreadPoolExecutor(20) as executor:
    tasks = [url for group in executor.map(children, months) for url in group]
with cf.ThreadPoolExecutor(28) as executor:
    item_urls = [url for group in executor.map(children, tasks) for url in group]
with cf.ThreadPoolExecutor(28) as executor:
    items = [item for item in executor.map(item_summary, item_urls) if item]

events = []
for year in (2024, 2025):
    query = urllib.parse.urlencode({
        "format": "geojson", "starttime": f"{year}-01-01",
        "endtime": f"{year + 1}-01-01", "minmagnitude": 3.0,
        "orderby": "time-asc", "limit": 20000,
    })
    data = get(USGS + "?" + query)
    for feature in (data or {}).get("features", []):
        lon, lat, depth = feature["geometry"]["coordinates"][:3]
        events.append({
            "id": feature["id"], "lat": lat, "lon": lon,
            "depth": max(0.0, depth), "mag": feature["properties"]["mag"],
            "time": dt.datetime.fromtimestamp(feature["properties"]["time"] / 1000,
                                               tz=dt.timezone.utc),
        })

hits = []
for item in items:
    start, end = parse_time(item["start"]), parse_time(item["end"])
    for event in events:
        horizontal = distance_km(item["lat"], item["lon"], event["lat"], event["lon"])
        distance = math.hypot(horizontal, event["depth"])
        detectable_regime = (
            (distance <= 500 and event["mag"] >= 3.0)
            or (distance <= 2000 and event["mag"] >= 5.0)
            or event["mag"] >= 6.5
        )
        if not detectable_regime:
            continue
        # Broad body/surface-wave envelope: fastest P to slow regional surface waves.
        first = event["time"] + dt.timedelta(seconds=distance / 8.5)
        last = event["time"] + dt.timedelta(seconds=distance / 2.0 + 20)
        if first <= end and last >= start:
            hits.append({**item, "event": event, "distance_km": round(distance, 1),
                         "arrival_first": first.isoformat(), "arrival_last": last.isoformat()})

print(json.dumps({"items": len(items), "events": len(events), "hits": hits}, indent=2, default=str))
