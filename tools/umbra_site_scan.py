"""Scan Umbra's open STAC catalogue for collects near instrumented ground sites.

Written to answer one question: is the corner-reflector campaign behind Suppi et
al. (IWSHM 2025) -- Villa Gherta in Trento, and Glasgow Green -- in Umbra's open
data programme? It is not: 1558 items across 2024 and 2025, none within 0.3
degrees of either site. Those were ESA-funded tasked acquisitions, so obtaining
them is a request rather than a download. See docs/DATASETS.md.

THE CONTROL MATTERS MORE THAN THE ANSWER. A zero from a broken scan looks exactly
like a zero from a clean one, so TARGETS also carries two sites the catalogue is
known to hold -- Rotterdam and the Panama Canal -- and a run is only trustworthy
if it finds them. Keep them there.

No credentials needed; the bucket is public. The catalogue nests
year -> month -> task -> item, so the walk is depth-capped.
"""
import json, urllib.request, concurrent.futures as cf, sys
B="https://umbra-open-data-catalog.s3.us-west-2.amazonaws.com/stac/"
def get(u):
    try:
        with urllib.request.urlopen(u, timeout=30) as r: return json.load(r)
    except Exception: return None
# Catalogue controls, published shaker sites, and public sensor archives.
TARGETS = {
    # Catalogue controls and the two published shaker sites.
    "Rotterdam": (51.95, 4.14),
    "Panama Canal": (9.08, -79.68),
    "Glasgow Green": (55.8475, -4.2400),
    "Trento": (46.07, 11.12),
    # Independently hosted, timestamped structural-monitoring archives.
    "Hardanger Bridge": (60.4808, 6.8296),
    "Werrington Bridge": (-33.764025, 150.741983),
    "Vanersborg Bridge": (58.3808, 12.3233),
    "openLAB Bautzen": (51.1816, 14.4277),
    "KW51 Leuven": (50.9004, 4.7000),
    "NREL Flatirons": (39.9106, -105.2347),
    "Punta Langosteira Port": (43.3577, -8.3871),
    "TigerRAY mooring": (47.679517, -122.230500),
    "D1244 Stuttgart": (48.74871, 9.11152),
    "Xiaolangdi Dam": (35.154, 112.390),
}
months=[]
for y in ("2024","2025"):
    c=get(B+y+"/catalog.json")
    if c: months += [B+y+"/"+l["href"][2:] for l in c["links"] if l.get("rel")=="child"]
print(f"months: {len(months)}", file=sys.stderr)
items=[]
def kids(u):
    c=get(u)
    if not c: return []
    base=u.rsplit("/",1)[0]+"/"
    return [base+l["href"][2:] for l in c["links"] if l.get("rel") in ("child","item")]
with cf.ThreadPoolExecutor(16) as ex:
    for r in ex.map(kids, months): items += r
print(f"level-3 entries: {len(items)}", file=sys.stderr)
hits=[]
def check(u):
    d=get(u)
    if not d: return None
    if d.get("type")=="Catalog":
        return ("cat", u)
    bb=d.get("bbox")
    if not bb or len(bb)<4: return None
    for name,(lat,lon) in TARGETS.items():
        inside = bb[0] <= lon <= bb[2] and bb[1] <= lat <= bb[3]
        near = bb[0]-0.3<=lon<=bb[2]+0.3 and bb[1]-0.3<=lat<=bb[3]+0.3
        if near:
            return ("hit", "inside" if inside else "near", name, d.get("id"),
                    bb, d.get("properties",{}).get("datetime"))
    return None
todo=items; depth=0
while todo and depth<4:
    nxt=[]
    with cf.ThreadPoolExecutor(24) as ex:
        for r in ex.map(check, todo):
            if r and r[0]=="cat": nxt += kids(r[1])
            elif r and r[0]=="hit": hits.append(r)
    print(f"depth {depth}: {len(todo)} checked, {len(nxt)} deeper, {len(hits)} hits", file=sys.stderr)
    todo=nxt; depth+=1
for h in hits: print(h)
if not hits: print("NO ITEM within 0.3 deg of any target")
