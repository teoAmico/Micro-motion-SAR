"""List public ICEYE STAC items that expose a CPHD asset."""
import concurrent.futures as cf
import json
import urllib.request

COLLECTION = "https://iceye-open-data-catalog.s3.amazonaws.com/collections/iceye-sar.json"


def get(url):
    try:
        with urllib.request.urlopen(url, timeout=30) as response:
            return json.load(response)
    except Exception:
        return None


collection = get(COLLECTION)
urls = [link["href"] for link in collection["links"] if link.get("rel") == "item"]


def inspect(url):
    item = get(url)
    if not item:
        return None
    assets = item.get("assets", {})
    cphd = []
    for name, asset in assets.items():
        href = asset.get("href", "")
        media = asset.get("type", "")
        if "cphd" in (name + " " + href + " " + media).lower():
            cphd.append(href)
    if not cphd:
        return None
    return {
        "id": item.get("id"),
        "bbox": item.get("bbox"),
        "datetime": item.get("properties", {}).get("datetime"),
        "cphd": cphd,
        "item": url,
    }


with cf.ThreadPoolExecutor(24) as executor:
    results = [result for result in executor.map(inspect, urls) if result]
print(json.dumps(results, indent=2))
