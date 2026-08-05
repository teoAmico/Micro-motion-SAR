"""Audit federated hits for motion above item 53's 5.5 um floor.

TWO THINGS THE FIRST VERSION GOT WRONG, both of which made failure look like
absence -- the failure mode this project has now hit four times.

1. It mapped datacentre NAMES to obspy client shortcuts by hand. Names outside
   that map fell back to IRIS, which does not hold the station, and answered
   nothing. 59 of 76 stations were never measured and the funnel looked clean.
   The federator already reports each archive's service URLs; build the client
   from those and the map disappears.
2. It swallowed every exception. A failed lookup, an unmapped centre and a
   genuinely silent station were indistinguishable. Failures are now COUNTED
   and REPORTED BY REASON, because an audit that cannot say what it did not
   measure is not an audit.
"""
import csv, sys, os, collections, warnings, urllib.parse
warnings.filterwarnings("ignore")
from obspy.clients.fdsn import Client
from obspy import UTCDateTime
import numpy as np
sys.path.insert(0, "tools")
import importlib.util
spec = importlib.util.spec_from_file_location("fsj", "tools/footprint_sensor_join.py")
fsj = importlib.util.module_from_spec(spec); spec.loader.exec_module(fsj)

FLOOR_UM, BAND_HI = 5.5, 3.0
D = os.environ["SCRATCH"]
rows = list(csv.DictReader(open("runs/screens/sensor-join/measurement_hits_federated.csv")))
print("auditing %d hits, %d distinct stations" %
      (len(rows), len({(r["net"], r["sta"]) for r in rows})), flush=True)

w = csv.writer(open(D + "/audit2_results.csv", "w", newline=""))
w.writerow(["scene","net","sta","cha","sps","lat","lon","name","rms_um","peak_um","above_floor"])
fail = collections.Counter()
measured_sta, clients, base_cache = set(), {}, {}

for i, r in enumerate(rows):
    key = (r["net"], r["sta"])
    t0, t1 = UTCDateTime(r["start"]), UTCDateTime(r["end"])
    if key not in base_cache:
        svc = fsj.waveform_services(r["net"], r["sta"], r["start"], r["end"])
        base_cache[key] = [urllib.parse.urlparse(u)._replace(
            path="", params="", query="", fragment="").geturl() for _, u in svc]
    bases = base_cache[key]
    if not bases:
        fail["no service URL from federator"] += 1; continue
    got = False
    for base in bases:
        try:
            if base not in clients: clients[base] = Client(base_url=base, timeout=90)
            c = clients[base]
        except Exception as e:
            fail["client init: %s" % type(e).__name__] += 1; continue
        try:
            st = c.get_waveforms(r["net"], r["sta"], "*", "*", t0 - 5, t1 + 5)
        except Exception as e:
            fail["get_waveforms: %s" % type(e).__name__] += 1; continue
        try:
            inv = c.get_stations(network=r["net"], station=r["sta"],
                                 level="response", starttime=t0, endtime=t1)
        except Exception as e:
            fail["get_stations: %s" % type(e).__name__] += 1; continue
        for tr in st:
            ch = tr.stats.channel
            if len(ch) < 2 or ch[1].upper() not in ("H", "N", "L"):
                fail["not a motion channel"] += 1; continue
            if tr.stats.sampling_rate < 2.0 * BAND_HI:
                fail["sample rate below 2x band"] += 1; continue
            try:
                tr.detrend("linear"); tr.taper(0.05)
                tr.remove_response(inventory=inv, output="DISP",
                                   pre_filt=(0.05, 0.1, 40, 45))
            except Exception as e:
                fail["remove_response: %s" % type(e).__name__] += 1; continue
            d = tr.data.astype(float); d -= d.mean()
            rms, pk = d.std() * 1e6, np.abs(d).max() * 1e6
            w.writerow([r["scene"], r["net"], r["sta"], ch, tr.stats.sampling_rate,
                        r["lat"], r["lon"], r["name"], round(rms, 4), round(pk, 4),
                        int(rms > FLOOR_UM)])
            got = True
            if rms > FLOOR_UM:
                print("  ABOVE FLOOR %s.%s.%s %.2f um  %s" %
                      (r["net"], r["sta"], ch, rms, r["name"][:34]), flush=True)
        if got: break
    if got: measured_sta.add(key)
    if (i + 1) % 25 == 0:
        print("  %d/%d  stations measured %d" % (i + 1, len(rows), len(measured_sta)), flush=True)

print("\nCOVERAGE: %d of %d distinct stations measured" %
      (len(measured_sta), len({(r["net"], r["sta"]) for r in rows})), flush=True)
print("FAILURES BY REASON:", flush=True)
for k, v in fail.most_common():
    print("   %6d  %s" % (v, k), flush=True)
print("AUDIT2_DONE", flush=True)
