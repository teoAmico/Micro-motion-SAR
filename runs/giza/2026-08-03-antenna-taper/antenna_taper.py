"""Two-way antenna gain taper across a Capella spotlight dwell.

Reads TxPos/RcvPos/SRPPos/TxTime and the SIGNAL validity flag from the PVP block,
puts the target into the antenna coordinate frame per pulse via AntCoordFrame,
evaluates the Array GainPoly there, and reports the per-sub-look amplitude
dispersion the taper alone would produce.
"""
import re, sys, math, struct
import numpy as np

cphd, xml_path = sys.argv[1], sys.argv[2]
x = open(xml_path).read()

hdr = open(cphd, 'rb').read(2048).split(b'\f\n')[0].decode('ascii', 'replace')
def hv(k): return int(re.search(rf'{k}\s*:=\s*(\d+)', hdr).group(1))
pvp_off, pvp_size = hv('PVP_BLOCK_BYTE_OFFSET'), hv('PVP_BLOCK_SIZE')

nv  = int(re.search(r'<NumVectors>(\d+)</NumVectors>', x).group(1))
nbp = int(re.search(r'<NumBytesPVP>(\d+)</NumBytesPVP>', x).group(1))
assert pvp_size == nv * nbp, (pvp_size, nv*nbp)

pvp = np.memmap(cphd, dtype=np.uint8, mode='r', offset=pvp_off, shape=(nv, nbp))
def fld(word, n=1):
    b = pvp[:, word*8:(word+n)*8]
    return np.frombuffer(np.ascontiguousarray(b).tobytes(), dtype='>f8').reshape(nv, n)
def ifld(word):
    b = pvp[:, word*8:(word+1)*8]
    return np.frombuffer(np.ascontiguousarray(b).tobytes(), dtype='>i8')

t   = fld(0)[:, 0]
tx  = fld(1, 3); rcv = fld(8, 3); srp = fld(14, 3)
sig = ifld(31)

good = (sig == 1) & np.isfinite(t) & np.isfinite(tx).all(1) & np.isfinite(srp).all(1)
print(f"pulses {nv}, valid {good.sum()}  ({nv-good.sum()} rejected)")
t, tx, rcv, srp = t[good], tx[good], rcv[good], srp[good]
t0 = t - t[0]

def poly1(block, axis):
    m = re.search(rf'<{axis} order1="\d+">(.*?)</{axis}>', block, re.S)
    cs = sorted((int(e), float(v)) for e, v in
                re.findall(r'<Coef exponent1="(\d+)">([^<]+)</Coef>', m.group(1)))
    return np.array([v for _, v in cs])

acf = re.search(r'<AntCoordFrame>.*?</AntCoordFrame>', x, re.S).group(0)
AX = np.stack([np.polyval(poly1(acf, a)[::-1], t0) for a in 'XYZ'], axis=1)
apat = re.search(r'<AntPattern>.*?</AntPattern>', x, re.S).group(0)
acy  = re.search(r'<AntCoordFrame>.*?</AntCoordFrame>', x, re.S).group(0)

# The ACF gives X and Y axes; Z = X cross Y.
def axis_from(tag):
    m = re.search(rf'<{tag}>(.*?)</{tag}>', acy, re.S).group(1)
    return np.stack([np.polyval(
        np.array(sorted((int(e), float(v)) for e, v in
            re.findall(r'<Coef exponent1="(\d+)">([^<]+)</Coef>',
                       re.search(rf'<{a} order1="\d+">(.*?)</{a}>', m, re.S).group(1)))
        )[:, 1][::-1], t0) for a in 'XYZ'], axis=1)

Xa, Ya = axis_from('XAxisPoly'), axis_from('YAxisPoly')
Xa /= np.linalg.norm(Xa, axis=1, keepdims=True)
Ya /= np.linalg.norm(Ya, axis=1, keepdims=True)

# Line of sight from the antenna phase centre to the scene reference point.
apc = 0.5 * (tx + rcv)
u = srp - apc
u /= np.linalg.norm(u, axis=1, keepdims=True)
dcx = np.einsum('ij,ij->i', u, Xa)
dcy = np.einsum('ij,ij->i', u, Ya)

gp = re.search(r'<Array>\s*<GainPoly[^>]*>(.*?)</GainPoly>', apat, re.S).group(1)
C = np.zeros((3, 3))
for e1, e2, v in re.findall(r'<Coef exponent1="(\d+)" exponent2="(\d+)">([^<]+)</Coef>', gp):
    C[int(e1), int(e2)] = float(v)

g_db = np.polynomial.polynomial.polyval2d(dcx, dcy, C)     # one-way, dB
amp  = 10.0 ** (g_db / 10.0)   # two-way voltage amplitude ~ one-way linear power gain

print(f"off-boresight  |dcx| max {np.abs(dcx).max():.5f} ({math.degrees(math.asin(np.abs(dcx).max())):.3f} deg)"
      f"   |dcy| max {np.abs(dcy).max():.5f} ({math.degrees(math.asin(np.abs(dcy).max())):.3f} deg)")
print(f"one-way gain across dwell: {g_db.min():+.3f} .. {g_db.max():+.3f} dB   (span {g_db.max()-g_db.min():.3f} dB)")

print("\n  looks  overlap   t_sap    D_A from the antenna taper alone")
for n_looks, ov in ((128, 0.90), (128, 0.0), (64, 0.75), (32, 0.0)):
    npul = len(t)
    step = int(npul * (1 - ov) / n_looks) or 1
    win  = int(npul / (n_looks * (1 - ov) + ov)) if ov else npul // n_looks
    win  = max(win, 1)
    a = []
    for i in range(n_looks):
        s = i * step
        e = min(s + win, npul)
        if e - s < win // 2: break
        a.append(amp[s:e].mean())
    a = np.array(a)
    da = a.std() / a.mean()
    print(f"   {n_looks:4d}   {ov:4.2f}   {win/npul*32.869:6.3f}s   D_A = {da:.5f}")

# --- and at a pixel on the edge of the grid the Giza runs actually used ---
# D_A is measured at a window's brightest PIXEL, not at the SRP, so the taper
# that matters is the one a pixel offset from the reference point sees.
print("\n  offset from SRP    gain span across dwell    D_A (128 looks, 0.90 overlap)")
e1 = np.cross(srp[0], [0, 0, 1.0]); e1 /= np.linalg.norm(e1)
for off_m in (0.0, 128.0, 500.0, 2500.0):
    tgt = srp + off_m * e1
    v = tgt - apc
    v /= np.linalg.norm(v, axis=1, keepdims=True)
    gx = np.einsum('ij,ij->i', v, Xa); gy = np.einsum('ij,ij->i', v, Ya)
    gdb = np.polynomial.polynomial.polyval2d(gx, gy, C)
    am = 10.0 ** (gdb / 10.0)
    npul = len(t); n_looks, ov = 128, 0.90
    step = int(npul * (1 - ov) / n_looks) or 1
    win = int(npul / (n_looks * (1 - ov) + ov))
    a = np.array([am[i*step:min(i*step+win, npul)].mean()
                  for i in range(n_looks) if i*step + win//2 < npul])
    print(f"   {off_m:7.0f} m        {gdb.max()-gdb.min():.6f} dB              {a.std()/a.mean():.6f}")
