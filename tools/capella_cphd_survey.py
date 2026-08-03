"""Survey Capella's open CPHD collection: mode, dwell, geometry, local time.

Answers "what is actually available and which collects are worth screening",
from the provider's own STAC catalogue rather than from its marketing pages.
Writes a JSON list; docs/DATASETS.md carries the summary it produced.

WHAT IT READS THAT THE DELIVERED FILES DO NOT CARRY. The catalogue holds
view:incidence_angle, capella:squint_angle and locale:time -- the last being the
LOCAL acquisition time, which decides whether a bridge was under traffic. None of
those is in the delivery directory beside the .cphd, and none is in the CPHD XML,
so this is the cheapest way to rank candidates before downloading anything.

Selection here is by incidence and daylight, NOT by dwell: FOLLOW-UPS.md item 32
records the published validation operating at 5.2-6.1 s of observation, so a long
dwell is not the scarce resource it was once taken to be.
"""
import json, re, datetime, urllib.request, concurrent.futures as cf, sys
from urllib.parse import urljoin
BASE="https://capella-open-data.s3.us-west-2.amazonaws.com/stac/capella-open-data-by-product-type/capella-open-data-cphd/collection.json"
d=json.load(open('cap_cphd.json'))
items=[l['href'] for l in d['links'] if l.get('rel')=='item']
def dwell(i):
    m=re.search(r'_(\d{14})_(\d{14})', i)
    f=lambda t: datetime.datetime.strptime(t,'%Y%m%d%H%M%S')
    return (f(m.group(2))-f(m.group(1))).total_seconds()
sel=[h for h in items if '_SP_CPHD_' in h and dwell(h)>=6]
print(f"spotlight CPHD with dwell >= 30 s: {len(sel)}", file=sys.stderr)
def get(h):
    u=urljoin(BASE, h)
    try:
        with urllib.request.urlopen(u, timeout=40) as r: j=json.load(r)
    except Exception: return None
    p=j.get('properties',{}); b=j.get('bbox',[0,0,0,0])
    return dict(id=h.rsplit('/',1)[-1][:-5],
                dwell=dwell(h),
                lat=(b[1]+b[3])/2, lon=(b[0]+b[2])/2,
                tz=p.get('locale:timezone','?'),
                ltime=(p.get('locale:time','?') or '?')[:5],
                inc=p.get('view:incidence_angle'),
                squint=p.get('capella:squint_angle'))
out=[]
with cf.ThreadPoolExecutor(24) as ex:
    for r in ex.map(get, sel):
        if r: out.append(r)
json.dump(out, open('cap_survey_all.json','w'))
print(f"fetched {len(out)}", file=sys.stderr)
