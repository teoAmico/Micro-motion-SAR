from obspy.clients.fdsn import Client
from obspy import UTCDateTime
import numpy as np
c = Client("NCEDC")
for tag, cha, t0, dur in [("almanor_hnz","HNZ",UTCDateTime("2023-05-11T23:19:55"),180),
                          ("ambient_hhz","HHZ",UTCDateTime("2025-06-01T12:00:00"),180)]:
    try:
        st = c.get_waveforms("BK","ORV","00",cha,t0,t0+dur,attach_response=True)
        st.merge(fill_value='interpolate'); tr = st[0]
        tr.detrend("linear"); tr.taper(0.05)
        tr.remove_response(output="DISP", pre_filt=(0.05,0.1,40,45))
        d = tr.data.astype(float); d -= d.mean()
        np.savetxt(tag+".txt", d, fmt="%.6e")
        print(f"{tag}: {cha} {tr.stats.sampling_rate:g} Hz  n={len(d)}  "
              f"rms={d.std()*1e6:.3f} um  peak={np.abs(d).max()*1e6:.3f} um")
        # dominant lines
        f = np.fft.rfftfreq(len(d), tr.stats.delta); P = np.abs(np.fft.rfft(d*np.hanning(len(d))))**2
        m = (f>0.1)&(f<25); idx = np.argsort(P[m])[-5:][::-1]
        print("   top lines Hz:", " ".join(f"{f[m][i]:.2f}" for i in idx))
    except Exception as e:
        print(tag, "FAILED", type(e).__name__, e)
