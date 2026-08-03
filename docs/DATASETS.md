# Open SAR datasets

Micro-Motion SAR needs phase-preserving data. Prefer **CPHD** (phase history)
for the full single-pass workflow; **SICD** and **SLC** retain complex phase and
are useful for focused-image workflows. Detected products such as GRD, GEC,
GEO, DGM and SIDD are not substitutes.

## Open X-band data

| Provider | Complex products | Access | Licence/terms | Notes |
|---|---|---|---|---|
| [Capella](https://registry.opendata.aws/capella_opendata/) | CPHD, SICD, SLC | Public AWS bucket; no account | CC BY 4.0 | Large spotlight archive; includes long-dwell scenes and public Giza CPHD |
| [Umbra](https://registry.opendata.aws/umbra-open-data/) | CPHD, SICD | Public AWS bucket; no account | Creative Commons | Large spotlight archive with repeat observations at several sites |
| [ICEYE](https://www.iceye.com/open-data-initiative) | Mostly SLC; selected CPHD and SICD | Public AWS bucket; no account | CC BY 4.0 | Selected phase-history scenes include Bratislava, Houston, Vienna, Paris, Mexico City and Vandenberg |
| [AFRL Gotcha](https://www.sdms.afrl.af.mil/index.php?collection=gotcha) | X-band phase history | Available by request | Check request terms | Airborne circular collection with calibration targets; useful for tomography and focusing tests |

Catalogue links:

- [Capella STAC catalogue](https://capella-open-data.s3.us-west-2.amazonaws.com/stac/catalog.json)
- [Umbra Open Data Program](https://umbra.space/open-data/)
- [Umbra STAC catalogue](https://stacindex.org/catalogs/umbra-open-sar-data)
- [ICEYE documentation](https://sar.iceye.com/6.0.6/opendata/opendata/)
- [ICEYE STAC collection](https://iceye-open-data-catalog.s3.amazonaws.com/collections/iceye-sar.json)

The Capella Giza spotlight collect is a known CPHD example:

```text
CAPELLA_C13_SP_CPHD_HH_20241004001939_20241004002012
```

It covers the Giza pyramid complex, has a dwell of about 33 seconds, and is
roughly 39 GB. Use resumable downloads for files of this size.

## Other open complex SAR data

| Provider | Band/product | Best use here |
|---|---|---|
| [Sentinel-1](https://search.asf.alaska.edu/) | C-band SLC | Multi-pass comparison; its short IW target illumination is unsuitable for this single-pass micro-motion method |
| [UAVSAR](https://uavsar.jpl.nasa.gov/) | L-band airborne SLC and repeat-pass stacks | Reader development, geometry checks and comparison experiments |

### "No collect with known motion" is about KNOWLEDGE, not about motion

This file's ground-truth note, and `validate`'s `RS_VALIDATE_GROUND_TRUTH`, say
that no open collect carries motion *known* to be there. That is a statement
about verification, and it must not be read as "these scenes are still". A bridge
under traffic vibrates whether or not anyone recorded it; the M2 deck at 13:18
local almost certainly does. What is missing is the synchronous displacement
sensor, not the displacement.

**The distinction has an operational edge: it decides what may be used as a
null.** Item 34 calibrates the simulated null against the uninjected Giza run,
which is defensible because desert and masonry are plausibly static over 33
seconds. Doing the same against an uninjected *Istanbul bridge* run would not be:
that scene may well contain exactly the signal the tool is trying to detect, and
using it as a null would calibrate the detector to ignore its own target. Pick a
null scene for a positive reason, not merely because nothing was injected into it.

**The campaign data behind the published validation is NOT in the open archive.**
Suppi et al. (IWSHM 2025) fly a corner reflector on an electromechanical shaker
with an LVDT, on Umbra data, at Villa Gherta (Trento) and Glasgow Green. Umbra's
open STAC catalogue was scanned in full for 2024 and 2025 — 1558 items — and
nothing falls within 0.3 degrees of either site; the scan was checked against
Rotterdam and the Panama Canal, which it finds. Those were ESA-funded tasked
acquisitions, not open-data releases. Obtaining them is a request to the authors
or to Umbra, not a download.

Before downloading a large scene, check its mode, dwell, incidence angle,
polarization and complex product type. Run `micromotion validate` before the
full processing chain: a file being open and readable does not mean its
geometry can support the intended measurement.

## What each provider actually has, surveyed

Counted from the providers' own STAC catalogues rather than from their marketing
pages, on 2026-08-03. `tools/umbra_site_scan.py` is the Umbra walker; the Capella
figures come from its `capella-open-data-cphd` collection.

| provider | phase history | what the survey found | usable here |
|---|---|---|---|
| **Capella** | **1174 CPHD** | 707 spotlight, 232 sliding spotlight, 235 stripmap. Spotlight dwell 6–60 s, median 25 s. **95 distinct timezones.** | **Yes, and it is the main source.** The reader is written and tested against it, including the SGN and AmpSF quirks. |
| **Umbra** | CPHD present per task | Task folders carry `*_CPHD.cphd` beside GEC/SICD/SIDD. 81 named sites plus 1314 ad-hoc collects. Declares `SGN = -1` honestly, ships `CF8`, omits `AmpSF`. | **Reads correctly, verified against the vendor's own GEC** (item 36). No override needed — its honest `SGN` produces the same transform Capella's override does. Focused a Panama Canal collect; not mirrored. Never run through the measurement chain. |
| **ICEYE** | **6 CPHD** of 374 open items | Includes `dwell-precise` and `dwell-fine` **long-stare modes** — Houston, Vandenberg, Vienna — plus spot-fine at Paris and **Bratislava**. Declares `SGN = -1`, ships `CI4`, carries `AmpSF`. | **Reads its metadata; screens clean on geometry.** Best phase floor of any collect screened here (0.2017 mm/look). Its 15.3 s dwell puts a 2 Hz target just above the band at published aperture fractions — truncate with `--max-pulses`. Signal never read. |
| **AFRL Gotcha** | X-band phase history | By request. | Airborne circular; useful for focusing and tomography checks, not for this measurement. |

### Capella spotlight CPHD, ranked by what matters here

**Long dwell is no longer the selection criterion.** `FOLLOW-UPS.md` item 32
records the published validation operating at **5.2–6.1 s** of observation, not
33, so a 20 s collect is already generous and the earlier hunt for 60 s scenes
was optimising the wrong axis. What matters is incidence angle — vertical modes
project onto the line of sight as `cos(incidence)` — and local time, because
traffic is the excitation.

Best geometry per site, over all 707 spotlight CPHD:

```
   n  best inc  max dwell  daylight       lat       lon   timezone
  23      5.7        34        19    20.819    92.989   Asia/Yangon
  75      6.3        38        53    16.769   -99.795   America/Mexico_City  (Acapulco)
 135      7.4        39        66    19.369  -155.196   Pacific/Honolulu     (Hawaii, volcanic)
  26     14.2        34        12    34.069  -118.560   America/Los_Angeles  (San Fernando Valley)
  23     14.3        35        15    -6.816   107.121   Asia/Jakarta
  66     17.2        33        37    28.789   -81.276   America/New_York     (central Florida)
  22     17.6        39        11    30.008   122.088   Asia/Shanghai        (Zhoushan)
  17     18.5        39         9    45.621     9.800   Europe/Rome          (Lombardy)
  14     19.5        60         9    41.005    28.977   Europe/Istanbul
```

`n` is collects at that site, `daylight` how many fall between 07:00 and 20:00
local. **Istanbul is no longer the obvious pick on geometry alone** — Acapulco,
Hawaii and Los Angeles all beat its 19.5 degrees, several with far more repeats.
What Istanbul still has is a bridge in the footprint with a masonry aqueduct
beside it at the same range, which is an in-scene static reference nothing else
on this list offers.

**The timezone grouping conflates sites, and that matters.**
`America/Los_Angeles` spans the whole US Pacific coast: of its 26 collects, one
is near Los Angeles and the rest are Eugene, Las Vegas, Fresno and the Mojave.
Read the column as "how many collects share a clock", not "how many cover a city".

**The best of them is screened and was not downloaded**
(`runs/screens/`). `CAPELLA_C11_SP_CPHD_HH_20250112202415_20250112202441` is the
finest geometry in the archive -- incidence 14.2 degrees, squint 0.1, 12:24 local,
25.4 s, 16.0 GB -- and returns WARN with no failures. Its footprint is a 7 km
square over the Santa Monica Mountains above Pacific Palisades, collected five
days into the January 2025 fire: a fire-response acquisition over terrain, with
nothing resembling a bridge deck in it. Excellent geometry over the wrong scene
is not a candidate.

That is the general lesson for this table: **geometry and clock are screenable
from metadata, scene content is not.** The footprint corners have to be checked
against a map before a collect is worth 16 GB, and the survey cannot do it.

## Long-dwell candidates over built infrastructure

`FOLLOW-UPS.md` items 4 and 18 refer to these. Dwell is taken from the two
timestamps in the product ID; footprint contents are verified against
OpenStreetMap rather than inferred from the scene centre.

| dwell | incidence | site | product ID | size |
|---:|---:|---|---|---:|
| 25.0 s | **19.5°** | **Istanbul** (Golden Horn) | `CAPELLA_C09_SP_CPHD_HH_20230321101754_20230321101819` | 17.3 GB |
| 60.0 s | 38.5° | Istanbul (night) | `CAPELLA_C11_SP_CPHD_HH_20230907223849_20230907223949` | 60.8 GB |
| 60.0 s | 36.1° | Valencia | `CAPELLA_C09_SP_CPHD_HH_20240227111009_20240227111109` | 60.5 GB |
| 40.0 s | 56.1° | Budapest (Danube) | `CAPELLA_C15_SP_CPHD_HH_20241115212743_20241115212823` | — |
| 34.0 s | **37.5°** | Rome (Tiber) | `CAPELLA_C13_SP_CPHD_HH_20240816102624_20240816102658` | — |
| 32.9 s | 38.6° | *Giza, for reference* | `CAPELLA_C13_SP_CPHD_HH_20241004001939_20241004002012` | 36 GB |

**The Istanbul 25 s daytime collect is the primary candidate**, despite half the
dwell of the 60 s one:

- **13:18 local.** Traffic, metro and trams running. Traffic is the principal
  excitation for these decks, and a 60 s dwell over an unexcited bridge measures
  nothing — the 60 s collect is 01:39 local with the metro not operating.
- **Incidence 19.5° against 38.5°**, so cos 0.943 against 0.782: about 20% better
  projection of vertical modes onto the line of sight.
- 17.3 GB against 60.8 GB.
- The extra dwell buys only `df`, and 0.040 Hz is ample to place a mode in a
  1–3 Hz band.

In the footprint: the **M2 cable-stayed metro bridge** over the Golden Horn
(41.0227 N, 28.9667 E), **Galata Köprüsü**, **Atatürk Köprüsü**, **Marmaray** rail
segments — and the **Valens Aqueduct** (41.0162 N, 28.9552 E), a Roman masonry
arch in the same scene at the same range through the same processing. That
aqueduct is an *in-scene static reference*, which no simulated null can be.

Screened with `validate --xml` it returns **WARN with no failures** for a 2 Hz
target of 3 mm on the phase route; item 18 has the full finding and the caveats.

### Screening before downloading

A CPHD begins with an ASCII header giving `XML_BLOCK_BYTE_OFFSET` and
`XML_BLOCK_SIZE`, and the XML block behind it carries the geometry. Two range
requests decide whether a collect is worth fetching:

```sh
P=CAPELLA_C09_SP_CPHD_HH_20230321101754_20230321101819
U=https://capella-open-data.s3.us-west-2.amazonaws.com/data/2023/3/21/$P/$P.cphd
curl -s -r 0-1200 "$U" | tr -d '\000' | head       # the ASCII header
curl -s -r 1024-12341 "$U" > meta.xml               # the XML block it points at
./build/micromotion validate --xml meta.xml --frequency 2.0 --amplitude 3.0 \
    --estimator phase --alpha 0.0067 --overlap 0.0
```

About 12 KB against a 17 GB download. `validate --xml` also accepts a whole CPHD
file and seeks the block itself, which screens the 36 GB Giza collect in 0.3 s.

### Keep the metadata block beside the collect

Once a collect is on disk, extract the header and the XML block **once** and keep
them next to it. Both local bundles are already arranged that way:

```text
CAPELLA_C09_SP_CPHD_HH_20230321101754_20230321101819/
  ....cphd                17,312,033,344 bytes    the collect
  ....cphd-header.txt                432 bytes    the plain-text header
  ....xml                         11,318 bytes    the XML block, extracted verbatim
  SHA256SUMS
```

Screening then costs nothing and needs no range requests:

```sh
./build/micromotion validate --xml "$D/$P.xml" --frequency 2.0 --amplitude 3.0 \
    --estimator phase --alpha 0.0067 --overlap 0.0
```

**0.006 seconds**, reproducing item 18's *WARN with no failures* exactly.

Pointing `--xml` at the `.cphd` works too and is also fast — the seek is two
reads, and it measured 0.005 s warm here against the 0.3 s item 18 recorded on a
cold read. The reason to keep the extracted block is not speed: it is that an
11 KB file can be committed, mailed, or read when the collect is on a drive that
is not plugged in, and that the screen then cannot be confused by a truncated
download. The header file is worth keeping for the same reason — it carries the
block offsets and sizes, so a short file can be diagnosed without opening it.

### There are no STAC or extended-JSON sidecars, and it was worth checking

Capella's *SAR Products Format Specification* documents a three-file TIFF+JSON
bundle — image, STAC catalog metadata, and an extended metadata JSON — carrying
`view:incidence_angle`, `capella:squint_angle`, `locale:datetime`,
`sar:resolution_azimuth`, `center_pixel`, `state.state_vectors` and more. That
bundle is the **imagery** product family: SLC, GEC, GEO, SICD and CSI. `"CPHD"`
appears in the spec only as an admissible value of `sar:product_type`, not as a
delivery that ships those files.

**Checked against both collects on disk: the open-data CPHD deliveries are the
`.cphd` and nothing else.** No `.json`, no `_extended.json`. The re-fetch URL in
each bundle's `README.md` is a single object. So the sidecar route does not
exist for phase history, and the extracted `.xml` above is what replaces it —
better for this purpose, since it is the same document the full reader parses
rather than a summary that could drift from it.

Three things the imagery metadata would have supplied and the CPHD XML does not,
recorded so nobody goes looking twice:

- **`locale:datetime` / `locale:time`** — local time of acquisition. *(Correction:
  these ARE available, in the STAC catalogue on the same bucket, just not in the
  delivery directory beside the `.cphd` and not in the CPHD XML. The survey above
  reads them straight out of the catalogue, so the by-hand derivation the
  Istanbul argument used was unnecessary.)*
- **`capella:squint_angle`**, stated directly rather than inferred.
- **`radar.time_varying_parameters`** — `{start_timestamps, prf, pulse_bandwidth,
  pulse_duration}`, which is Capella declaring the PRF as *piecewise constant
  with switch times*. That is exactly what `RS_VALIDATE_PRF_STABILITY` reports
  UNKNOWN for on a metadata-only screen, and it explains the shape of what
  `rs_subap_stack_t` records measuring on the Giza collect: a PRI spread of 1.1%
  that is systematic rather than jitter. For CPHD the per-pulse transmit times in
  the PVP block are strictly better than a step list — but they are 70 MB in, so
  the screen still cannot answer that check and the UNKNOWN stands.
