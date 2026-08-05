"""Harvest Umbra footprints and dwells from the open-data STAC sidecars.

Item 63 disqualified Umbra on DWELL from a 250-product sample and did not join
its footprints to anything. Item 83 then found that no instrumented bridge or dam
has ever been inside a Capella footprint, which makes the spatial question worth
asking of a second archive even though the dwell answer is already known: if
Umbra covers structures Capella does not, the blocker is dwell alone and that is
a statement about what to ask a vendor for, rather than about the sky.

Every collect folder under sar-data/ carries one '.stac.v2.json' beside its
products, holding the footprint polygon, start_datetime, end_datetime and
instrument mode. That is one small fetch per collect instead of parsing 13,564
CPHD headers.

Resumable: results are appended as JSON lines, and a re-run skips ids already
present. FAILURES ARE COUNTED BY REASON -- an audit that cannot say what it did
not measure is not an audit, and a fetch that fails looks exactly like a collect
with no footprint.
"""
import json, os, re, sys, urllib.request, urllib.parse, collections
from concurrent.futures import ThreadPoolExecutor

B = "https://umbra-open-data-catalog.s3.amazonaws.com/"
KEYS = sys.argv[1]
OUT = sys.argv[2]
N_THREAD = 12


def listing(prefix):
    """Every key under 'prefix', following continuation tokens to the end."""
    keys, tok = [], None
    while True:
        q = {"list-type": "2", "max-keys": "1000", "prefix": prefix}
        if tok:
            q["continuation-token"] = tok
        with urllib.request.urlopen(B + "?" + urllib.parse.urlencode(q), timeout=120) as r:
            x = r.read().decode()
        keys += re.findall(r"<Key>([^<]+)</Key>", x)
        m = re.search(r"<NextContinuationToken>([^<]+)</", x)
        if not m:
            break
        tok = m.group(1)
        if len(keys) % 20000 < 1000:
            print("  listed %d keys" % len(keys), file=sys.stderr, flush=True)
    return keys


def one(key):
    """Fetch one STAC sidecar and reduce it to the fields the join needs.

    Refuses anything that is not a .stac.v2.json outright: these are kilobyte
    sidecars and the products beside them are tens of gigabytes.

    Returns (record, None) or (None, reason) so the caller can count why a
    collect is missing rather than silently dropping it."""
    if not key.endswith(".stac.v2.json"):
        return None, "not-a-sidecar"
    try:
        with urllib.request.urlopen(B + urllib.parse.quote(key), timeout=90) as r:
            raw = r.read(4 << 20)          # a sidecar is kilobytes; cap it
        d = json.loads(raw.decode())
    except Exception as e:                                    # noqa: BLE001
        return None, "fetch:%s" % type(e).__name__
    p = d.get("properties") or {}
    g = d.get("geometry") or {}
    if g.get("type") != "Polygon":
        return None, "geometry:%s" % g.get("type")
    s, e = p.get("start_datetime"), p.get("end_datetime")
    if not s or not e:
        return None, "no-time"
    import datetime
    def ts(v):
        v = v.replace("Z", "")
        for f in ("%Y-%m-%dT%H:%M:%S.%f", "%Y-%m-%dT%H:%M:%S"):
            try:
                return datetime.datetime.strptime(v, f)
            except ValueError:
                pass
        return None
    t0, t1 = ts(s), ts(e)
    if t0 is None or t1 is None:
        return None, "bad-time"
    # STAC polygons are [lon, lat, (height)]; the join wants [lat, lon] to match
    # the Capella harvest, and mixing the two is exactly the bug item 83 hit.
    ring = [[c[1], c[0]] for c in g["coordinates"][0]]
    lat = sum(c[0] for c in ring) / len(ring)
    lon = sum(c[1] for c in ring) / len(ring)
    return {"id": d.get("id") or key, "dur": (t1 - t0).total_seconds(),
            "start": s, "end": e, "ring": ring, "centre": [lat, lon],
            "mode": p.get("sar:instrument_mode"), "platform": p.get("platform"),
            "has_cphd": any(k.lower().endswith(".cphd")
                            for k in (d.get("assets") or {}))}, None


def main():
    """List the sidecars, fetch what is missing, and report the funnel."""
    # VALIDATE THE CACHE, never just its existence. The first run of this
    # script was handed a keys file left by item 63's dwell survey which held
    # .cphd keys, and it began fetching 47 GB products on twelve threads. A
    # cached artefact whose CONTENT differs from what the code assumes is the
    # same failure mode as a remote query that fails looking like one that
    # found nothing.
    keys = []
    if os.path.exists(KEYS):
        cached = [k for k in open(KEYS).read().split("\n") if k]
        if cached and all(k.endswith(".stac.v2.json") for k in cached):
            keys = cached
        else:
            bad = sum(1 for k in cached if not k.endswith(".stac.v2.json"))
            print("ignoring %s: %d of %d keys are not STAC sidecars"
                  % (KEYS, bad, len(cached)), file=sys.stderr, flush=True)
    if not keys:
        keys = [k for k in listing("sar-data/") if k.endswith(".stac.v2.json")]
        open(KEYS, "w").write("\n".join(keys))
    print("STAC sidecars: %d" % len(keys), file=sys.stderr, flush=True)

    done = set()
    if os.path.exists(OUT):
        for ln in open(OUT):
            try:
                done.add(json.loads(ln)["_key"])
            except Exception:                                 # noqa: BLE001
                pass
    todo = [k for k in keys if k not in done]
    print("already harvested %d, fetching %d" % (len(done), len(todo)),
          file=sys.stderr, flush=True)

    fh = open(OUT, "a")
    why = collections.Counter()
    n_ok = 0
    with ThreadPoolExecutor(max_workers=N_THREAD) as ex:
        for i, (rec, reason) in enumerate(ex.map(one, todo)):
            if rec is None:
                why[reason] += 1
            else:
                rec["_key"] = todo[i]
                fh.write(json.dumps(rec) + "\n")
                n_ok += 1
            if (i + 1) % 500 == 0:
                fh.flush()
                print("  %d/%d  ok %d  failed %d"
                      % (i + 1, len(todo), n_ok, sum(why.values())),
                      file=sys.stderr, flush=True)
    fh.close()
    print("\nHARVESTED %d of %d" % (n_ok + len(done), len(keys)))
    print("NOT harvested, by reason: %s" % (dict(why) or "none"))


main()
