#!/usr/bin/env python3
"""Stream an ADSB.lol daily tar and find traces inside a CPHD footprint."""

import argparse
import csv
import gzip
import io
import json
import sys
import tarfile
from datetime import datetime, timezone


class ConcatenatedFiles(io.RawIOBase):
    def __init__(self, paths):
        self.paths = iter(paths)
        self.current = None

    def readable(self):
        return True

    def readinto(self, buffer):
        view = memoryview(buffer)
        total = 0
        while total < len(view):
            if self.current is None:
                try:
                    self.current = open(next(self.paths), "rb")
                except StopIteration:
                    break
            count = self.current.readinto(view[total:])
            if count:
                total += count
            else:
                self.current.close()
                self.current = None
        return total


def parse_utc(value):
    return datetime.fromisoformat(value.replace("Z", "+00:00")).timestamp()


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
    parser = argparse.ArgumentParser()
    parser.add_argument("scene", help="Capella STAC item JSON")
    parser.add_argument("parts", nargs="+", help="ordered ADSB.lol tar parts")
    parser.add_argument("--margin", type=float, default=30.0, help="seconds around aperture")
    args = parser.parse_args()

    with open(args.scene, encoding="utf-8") as source:
        scene = json.load(source)
    start = parse_utc(scene["properties"]["start_datetime"])
    end = parse_utc(scene["properties"]["end_datetime"])
    geometry = scene["geometry"]
    polygons = [geometry["coordinates"]] if geometry["type"] == "Polygon" else geometry["coordinates"]

    rows = []
    stream = io.BufferedReader(ConcatenatedFiles(args.parts), buffer_size=1024 * 1024)
    with tarfile.open(fileobj=stream, mode="r|") as archive:
        for member in archive:
            if not member.isfile() or "/trace_full_" not in member.name:
                continue
            extracted = archive.extractfile(member)
            if extracted is None:
                continue
            payload = extracted.read()
            if payload[:2] == b"\x1f\x8b":
                payload = gzip.decompress(payload)
            trace = json.loads(payload)
            base = float(trace.get("timestamp", 0))
            for point in trace.get("trace", []):
                if len(point) < 3 or point[1] is None or point[2] is None:
                    continue
                timestamp = base + float(point[0])
                if timestamp < start - args.margin or timestamp > end + args.margin:
                    continue
                lat, lon = float(point[1]), float(point[2])
                if not any(point_in_ring(lon, lat, polygon[0]) for polygon in polygons):
                    continue
                rows.append({
                    "scene": scene["id"],
                    "icao": trace.get("icao", ""),
                    "registration": trace.get("r", ""),
                    "timestamp": datetime.fromtimestamp(timestamp, timezone.utc).isoformat(),
                    "unix_time": f"{timestamp:.3f}",
                    "in_aperture": int(start <= timestamp <= end),
                    "latitude": lat,
                    "longitude": lon,
                    "altitude_ft": point[3] if len(point) > 3 else "",
                    "ground_speed_kt": point[4] if len(point) > 4 else "",
                    "track_deg": point[5] if len(point) > 5 else "",
                    # readsb puts the STRING "ground" in the altitude field for
                    # an aircraft on the surface. Index 6 is the flags word, and
                    # reading it as a boolean reported every aircraft as
                    # airborne -- which looks like a scan finding no ground
                    # traffic rather than like a bug. FOLLOW-UPS item 57.
                    "on_ground": int(point[3] == "ground") if len(point) > 3 else "",
                })

    fields = [
        "scene", "icao", "registration", "timestamp", "unix_time",
        "in_aperture", "latitude", "longitude", "altitude_ft",
        "ground_speed_kt", "track_deg", "on_ground",
    ]
    writer = csv.DictWriter(sys.stdout, fieldnames=fields)
    writer.writeheader()
    writer.writerows(sorted(rows, key=lambda row: (row["unix_time"], row["icao"])))


if __name__ == "__main__":
    main()
