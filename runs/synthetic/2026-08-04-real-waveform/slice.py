import numpy as np
for tag in ("almanor_hnz","ambient_hhz"):
    d = np.loadtxt(tag+".txt"); fs=100.0; n=2000
    e = np.convolve(d**2, np.ones(n), 'valid'); i0 = int(np.argmax(e))
    s = d[i0:i0+n]; s = s - s.mean()
    np.savetxt(tag+"_20s.txt", s, fmt="%.6e")
    f = np.fft.rfftfreq(n,1/fs); P=np.abs(np.fft.rfft(s*np.hanning(n)))**2
    m=(f>0.15)&(f<10); idx=np.argsort(P[m])[-6:][::-1]
    print(f"{tag}_20s  start {i0/fs:.1f}s  peak {np.abs(s).max()*1e6:.2f} um  rms {s.std()*1e6:.2f} um")
    print("   lines:", "  ".join(f"{f[m][i]:.3f}Hz({P[m][i]/P[m].max():.2f})" for i in idx))
