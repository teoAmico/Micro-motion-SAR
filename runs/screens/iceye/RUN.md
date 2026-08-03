# ICEYE: six CPHD in the open archive, and they include dwell modes

Of 374 items in ICEYE's open STAC collection, **six carry a `cphd` asset**:

```
ICEYE-X47  dwell-precise  inc 25.2  Houston      2026-05-06  19.4 GB
ICEYE-X38  dwell-precise  inc 26.2  Vandenberg   2026-03-14
ICEYE-X38  dwell-precise  inc 27.6  Vienna       2026-04-02
ICEYE-X49  dwell-fine     inc 36.9  Mexico City  2026-03-17
ICEYE-X56  spot-fine      inc 30.3  Paris        2025-10-28
ICEYE-X50  spot-fine      inc 40.9  Bratislava   2025-10-27  16.5 GB
```

**`dwell-precise` and `dwell-fine` are long-stare modes** — the acquisition this
method wants, and the only open provider that labels them as such. Bratislava is
the site `DATASETS.md` already listed without knowing a CPHD existed for it.

## The Houston dwell-precise, screened

```
CollectorName ICEYE-X47   SGN -1   CI4   AmpSF PRESENT   ModeType EXPERIMENTAL
dwell 15.345 s, 100802 pulses x 48000 samples, incidence 25.2 deg, 580.1 km

PASS  metadata, frequency resolution, grid width, ambiguity, memory
PASS  phase floor   0.2017 mm per look  <- the best of any collect screened here
FAIL  observable band at the published aperture fractions
```

`SGN = -1`, honestly declared, so the same reasoning as Umbra applies and no
override is needed. `AmpSF` is present where Umbra omits it; item 21's handling
covers both.

**The FAIL is the dwell, not the collect.** At 3.6 percent aperture fraction the
band reaches 0.905 Hz and at 1.8 percent 1.810 Hz, both short of a 2 Hz target,
because 15.3 seconds makes each sub-look long. The observation ratio at 1.8
percent is eta 0.552 — squarely inside the published 0.39–0.69. Truncating to
about six seconds with `--max-pulses` puts a 2 Hz target back inside the band;
that is item 4, now reachable.

## Not read

Only the 11 KB metadata block was fetched. No signal sample has been read from an
ICEYE product, so the PVP layout and the CI4 decode are unverified on this vendor
even though the reader handles both formats for others.
