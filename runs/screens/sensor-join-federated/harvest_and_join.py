"""Re-run the footprint/sensor join over the whole Capella spotlight archive,
federated. Writes runs/screens/sensor-join/measurement_hits_federated.csv
incrementally so a kill at any point leaves usable partial results."""
import sys, json, csv, urllib.request, urllib.parse, re, math, datetime, os
sys.path.insert(0, "tools")
import importlib.util
spec = importlib.util.spec_from_file_location("fsj", "tools/footprint_sensor_join.py")
fsj = importlib.util.module_from_spec(spec); spec.loader.exec_module(fsj)

B = "https://capella-open-data.s3.us-west-2.amazonaws.com/"
D = os.environ["SCRATCH"]
KEYS, FOOT = D + "/keys.txt", D + "/footprints.json"

# 1. list the bucket
if not os.path.exists(KEYS):
    keys, token = [], None
    while True:
        q = {"list-type": "2", "max-keys": "1000", "prefix": "data/"}
        if token: q["continuation-token"] = token
        with urllib.request.urlopen(B + "?" + urllib.parse.urlencode(q), timeout=90) as r:
            xml = r.read().decode()
        keys += re.findall(r"<Key>([^<]+\.cphd)</Key>", xml)
        m = re.search(r"<NextContinuationToken>([^<]+)</", xml)
        if not m: break
        token = m.group(1)
        print("  listed %d keys" % len(keys), file=sys.stderr, flush=True)
    open(KEYS, "w").write("\n".join(keys))
    print("bucket: %d CPHD keys" % len(keys), file=sys.stderr, flush=True)

# 2. harvest footprints
if not os.path.exists(FOOT):
    os.system("python3 runs/screens/sensor-join/harvest.py %s %s" % (KEYS, FOOT))
foot = json.load(open(FOOT))
foot = [f for f in foot if f["dur"] >= 15.0]
print("footprints with dwell >= 15 s: %d" % len(foot), file=sys.stderr, flush=True)

# 3. the federated join
out = "runs/screens/sensor-join/measurement_hits_federated.csv"
w = csv.writer(open(out, "w", newline=""))
w.writerow(["scene","dwell_s","net","sta","lat","lon","name","datacentre","start","end","waveform_bytes"])
nhit = 0
for i, f in enumerate(foot):
    ring = f["ring"]
    found, _ = fsj.stations_in(ring, f["start"], f["end"])
    for net, sta, lat, lon, name, centre in found:
        b = fsj.has_waveform(net, sta, f["start"], f["end"])
        if b > 0:
            nhit += 1
            w.writerow([f["id"], round(f["dur"],1), net, sta, lat, lon, name, centre,
                        f["start"], f["end"], b])
            print("  HIT %s %s.%s %s (%d bytes)" % (f["id"][:40], net, sta, name[:30], b),
                  file=sys.stderr, flush=True)
    if (i+1) % 25 == 0:
        print("  %d/%d footprints, %d hits" % (i+1, len(foot), nhit), file=sys.stderr, flush=True)
        sys.stdout.flush()
print("DONE: %d hits over %d footprints" % (nhit, len(foot)), file=sys.stderr, flush=True)
