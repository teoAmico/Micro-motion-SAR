import re, json, math, sys, datetime, urllib.request, concurrent.futures as cf
B="https://capella-open-data.s3.us-west-2.amazonaws.com/"
keys=[k for k in open(sys.argv[1]).read().split() if re.search(r'CAPELLA_C\d+_(SP|SS)_', k)]
print(f"{len(keys)} spotlight / sliding-spotlight CPHD", file=sys.stderr)

def head(key):
    req=urllib.request.Request(B+urllib.parse.quote(key), headers={"Range":"bytes=0-262143"})
    try:
        blob=urllib.request.urlopen(req,timeout=45).read().decode('utf-8','replace')
    except Exception:
        return None
    pts=[(float(a),float(b)) for a,b in re.findall(r'<Lat>([-\d.eE+]+)</Lat>\s*<Lon>([-\d.eE+]+)</Lon>', blob)]
    cs=re.findall(r'<CollectionStart>\s*([^<\s]+)\s*</CollectionStart>', blob)
    t1=re.findall(r'<TxTime1>([^<]+)</TxTime1>', blob); t2=re.findall(r'<TxTime2>([^<]+)</TxTime2>', blob)
    if len(pts)<5 or not cs or not t1 or not t2: return None
    dur=float(t2[0])-float(t1[0])
    corners=pts[1:5]
    cy=sum(p[0] for p in corners)/4; cx=sum(p[1] for p in corners)/4
    ring=sorted(corners,key=lambda p: math.atan2(p[0]-cy,p[1]-cx))
    st=datetime.datetime.fromisoformat(cs[0].replace('Z','+00:00'))
    return {"id":key.split('/')[-1].replace('.cphd',''),
            "dur":dur,"start":st.strftime('%Y-%m-%dT%H:%M:%S')+'Z',
            "end":(st+datetime.timedelta(seconds=dur)).strftime('%Y-%m-%dT%H:%M:%S')+'Z',
            "ring":ring,"centre":[cy,cx]}
import urllib.parse
out=[]
with cf.ThreadPoolExecutor(max_workers=16) as ex:
    for i,r in enumerate(ex.map(head, keys)):
        if r: out.append(r)
        if (i+1)%100==0: print(f"  {i+1}/{len(keys)} ...", file=sys.stderr)
json.dump(out, open(sys.argv[2],"w"))
print(f"harvested {len(out)} headers", file=sys.stderr)
