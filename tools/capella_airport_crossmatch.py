#!/usr/bin/env python3
"""Cross-match downloaded Capella STAC item metadata with OurAirports."""

import csv
import glob
import json
import sys


def point_in_ring(lon, lat, ring):
    inside = False
    previous = ring[-1]
    for current in ring:
        x1, y1 = previous[:2]
        x2, y2 = current[:2]
        if (y1 > lat) != (y2 > lat):
            crossing = (x2 - x1) * (lat - y1) / (y2 - y1) + x1
            if lon < crossing:
                inside = not inside
        previous = current
    return inside


def main():
    if len(sys.argv) != 3:
        raise SystemExit(f"usage: {sys.argv[0]} ITEM_DIRECTORY AIRPORTS.csv")

    airports = []
    with open(sys.argv[2], newline="", encoding="utf-8") as source:
        for row in csv.DictReader(source):
            if row["type"] not in {"large_airport", "medium_airport"}:
                continue
            airports.append((float(row["longitude_deg"]), float(row["latitude_deg"]), row))

    matches = []
    for path in glob.glob(f"{sys.argv[1]}/*.json"):
        with open(path, encoding="utf-8") as source:
            item = json.load(source)
        geometry = item["geometry"]
        polygons = [geometry["coordinates"]] if geometry["type"] == "Polygon" else geometry["coordinates"]
        xmin, ymin, xmax, ymax = item["bbox"][:4]
        for lon, lat, airport in airports:
            if not (xmin <= lon <= xmax and ymin <= lat <= ymax):
                continue
            if any(point_in_ring(lon, lat, polygon[0]) for polygon in polygons):
                matches.append({
                    "scene": item["id"],
                    "start": item["properties"].get("start_datetime"),
                    "end": item["properties"].get("end_datetime"),
                    "ident": airport["ident"],
                    "iata": airport["iata_code"],
                    "name": airport["name"],
                    "municipality": airport["municipality"],
                    "country": airport["iso_country"],
                    "latitude": lat,
                    "longitude": lon,
                    "type": airport["type"],
                })

    matches.sort(key=lambda row: (row["ident"], row["start"] or "", row["scene"]))
    writer = csv.DictWriter(sys.stdout, fieldnames=matches[0].keys() if matches else ["scene"])
    writer.writeheader()
    writer.writerows(matches)


if __name__ == "__main__":
    main()
