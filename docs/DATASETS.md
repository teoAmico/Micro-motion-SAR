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

Before downloading a large scene, check its mode, dwell, incidence angle,
polarization and complex product type. Run `micromotion validate` before the
full processing chain: a file being open and readable does not mean its
geometry can support the intended measurement.
