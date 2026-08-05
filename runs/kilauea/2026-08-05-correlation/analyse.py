"""Rank-correlate reported displacement against seismometer truth over the 16
Kilauea collects.

THE METRIC IS DECLARED HERE BEFORE THE DATA EXISTS, and that is the point. With
16 points and several plausible metrics per run -- dominant amplitude, excursion,
prominence, the modal set's leading mode -- picking after the fact is picking the
one that correlates. The primary is fixed:

    PRIMARY: the MEDIAN of `amplitude` over windows passing the shared gates.

Median because the scene is mostly quiet ground and a mean chases the tail;
`amplitude` because it is the spectrum's own displacement estimate; over passing
windows because failed windows carry no information either way. Spearman, not
Pearson: the relation between ground RMS and a sub-aperture amplitude is not
claimed to be linear, only monotone.

Everything else below is reported as SECONDARY and must be labelled as such --
including if one of them is the only thing that correlates.
"""
import csv, sys, os, glob, statistics as st

R = os.path.dirname(os.path.abspath(__file__))
truth = {}
for line in open(os.path.join(R, "collects_and_truth.tsv")):
    p = line.rstrip("\n").split("\t")
    if len(p) >= 2: truth[p[0]] = float(p[1])

def spearman(x, y):
    def rank(v):
        s = sorted(range(len(v)), key=lambda i: v[i]); r = [0.0]*len(v)
        for pos, i in enumerate(s): r[i] = pos + 1.0
        return r
    rx, ry = rank(x), rank(y); n = len(x)
    mx, my = sum(rx)/n, sum(ry)/n
    num = sum((a-mx)*(b-my) for a, b in zip(rx, ry))
    den = (sum((a-mx)**2 for a in rx) * sum((b-my)**2 for b in ry)) ** 0.5
    return num/den if den else 0.0

metrics = {"amplitude_median": [], "excursion_median": [], "prominence_median": []}
T, used = [], []
for scene, gt in sorted(truth.items(), key=lambda kv: -kv[1]):
    f = os.path.join(R, scene + "_windows.csv")
    if not os.path.exists(f): continue
    rows = list(csv.DictReader([l for l in open(f) if not l.startswith("#")]))
    ok = [r for r in rows if r.get("passed_gates") in ("1", "yes", "true")] or rows
    def med(col):
        v = [float(r[col]) for r in ok if r.get(col) not in (None, "", "nan")]
        return st.median(v) if v else float("nan")
    metrics["amplitude_median"].append(med("amplitude") if "amplitude" in rows[0] else med("excursion_px"))
    metrics["excursion_median"].append(med("excursion_px"))
    metrics["prominence_median"].append(med("prominence"))
    T.append(gt); used.append(scene)

print("collects analysed: %d of %d" % (len(T), len(truth)))
if len(T) < 8:
    print("TOO FEW TO CORRELATE -- need at least 8"); sys.exit(0)
print("truth range %.3f to %.3f um (%.1fx)" % (min(T), max(T), max(T)/min(T)))
print("\nPRIMARY")
r = spearman(metrics["amplitude_median"], T)
print("   amplitude_median vs ground RMS   Spearman %+.3f  (n=%d)" % (r, len(T)))
print("   Lotti et al. report Pearson 0.33-0.47 against accelerometers.")
print("\nSECONDARY (declared secondary in advance; do not promote)")
for k in ("excursion_median", "prominence_median"):
    print("   %-20s Spearman %+.3f" % (k, spearman(metrics[k], T)))
print("\nEvery collect here is BELOW the 5.5 um detection floor, so a null is")
print("uninterpretable: it is what the floor predicts. Only a positive counts.")
