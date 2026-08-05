"""Neighbour check for the La Palma survivors: does ES.CJED read high because
the ground moved, or because the instrument did? The audit could not say -- no
second station sat within 30 km IN THE SAME SCENE -- so this asks the archive
directly for every station around it during the same aperture.

Clients are built from the federator's service URLs. Hardcoding ODC here
returned 204 for every request and looked exactly like 'no neighbours exist'.
"""
import csv, warnings, sys, urllib.parse, importlib.util
warnings.filterwarnings("ignore")
from obspy.clients.fdsn import Client
from obspy import UTCDateTime
import numpy as np
sys.path.insert(0, "tools")
spec = importlib.util.spec_from_file_location("fsj", "tools/footprint_sensor_join.py")
fsj = importlib.util.module_from_spec(spec); spec.loader.exec_module(fsj)

rows = [r for r in csv.DictReader(open("runs/screens/sensor-join/measurement_hits_federated.csv"))
        if r["net"] == "ES" and r["sta"] in ("CJED","CENR")]
seen = set()
for r in rows:
    k = (r["sta"], r["start"])
    if k in seen: continue
    seen.add(k)
    t0, t1 = UTCDateTime(r["start"]), UTCDateTime(r["end"])
    svc = fsj.waveform_services("ES", r["sta"], r["start"], r["end"])
    if not svc:
        print("%-7s %s  no service URL" % (r["sta"], r["start"][:10]), flush=True); continue
    base = urllib.parse.urlparse(svc[0][1])._replace(
        path="", params="", query="", fragment="").geturl()
    try:
        c = Client(base_url=base, timeout=60)
        inv = c.get_stations(latitude=float(r["lat"]), longitude=float(r["lon"]),
                             maxradius=0.45, level="station", starttime=t0, endtime=t1)
    except Exception as e:
        print("%-7s %s  station query %s" % (r["sta"], r["start"][:10], type(e).__name__), flush=True)
        continue
    cand = [(n.code, s.code) for n in inv for s in n]
    res = []
    for net, sta in cand[:25]:
        try:
            st = c.get_waveforms(net, sta, "*", "*", t0 - 5, t1 + 5)
            iv = c.get_stations(network=net, station=sta, level="response",
                                starttime=t0, endtime=t1)
        except Exception:
            continue
        for tr in st:
            ch = tr.stats.channel
            if tr.stats.sampling_rate < 6 or len(ch) < 2 or ch[1].upper() not in "HNL":
                continue
            try:
                tr.detrend("linear"); tr.taper(0.05)
                tr.remove_response(inventory=iv, output="DISP", pre_filt=(0.05,0.1,40,45))
            except Exception:
                continue
            d = tr.data.astype(float); d -= d.mean()
            res.append((net + "." + sta, d.std() * 1e6))
    tgt = [v for n, v in res if n.endswith("." + r["sta"])]
    oth = [v for n, v in res if not n.endswith("." + r["sta"])]
    med = sorted(oth)[len(oth)//2] if oth else 0
    print("%-7s %s  %2d nearby | target %8.2f um | nbr median %8.2f um (%d ch) | ratio %s" % (
        r["sta"], r["start"][:10], len(cand), max(tgt) if tgt else -1, med, len(oth),
        "%.1f" % (max(tgt)/med) if tgt and med else "UNTESTABLE"), flush=True)
print("NBR_DONE", flush=True)
