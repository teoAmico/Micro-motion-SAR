# SAR Micro-Motion Ground-Truth Data

> **Cross-checked against the measured floor, 2026-08-04 (`FOLLOW-UPS.md` item 66).**
> The comparisons below use the PER-LOOK phase floor that `validate` reports
> (0.2017 mm/look on ICEYE). The end-to-end detection floor measured after 128
> looks and a periodogram is **0.0055 mm RMS** (item 53) — **37x lower**. Against
> that, Kilauea is not "700 times below": its stations sit at 1.7 um median, 3x
> below, with `HV.RIMD` reaching 15 um on 2024-06-09, which is **2.7x ABOVE**.
> Item 61 rejected that reading on ATTRIBUTION — no catalogued event, and the
> station is only intermittently high — not on amplitude. Oroville (0.78 um) and
> Mexico City (0.6 um) remain genuinely below. The conclusion of this page stands;
> the margin on Kilauea is much narrower than stated.


This page tracks phase-preserving spaceborne SAR collections acquired at the
same time as an independent vibration measurement. The goal is to find data that
can test both questions the project cares about:

1. Can the software recover a vibration known to be present?
2. Does it stay silent on bright objects and scene areas that did not vibrate?

Two exact openly catalogued pairings have now been found. A Capella spotlight
CPHD over Kilauea overlaps a calibrated 100 Hz HVO seismometer trace and is
anonymous on both sides. It is native to the present `mmotion --cphd` route,
but the recorded displacement is hundreds of times below the estimated
single-look phase floor, so it is a synchronized negative/null candidate rather
than a credible positive recovery test. At the Hardanger Bridge, public
Sentinel-1 IW Level-0 RAW and SLC acquisitions overlap GPS-timed accelerometer
records, including records during Storm Tor and Storm Urd. Those ground-data
downloads are anonymous and the SAR products are public but require a free
Earthdata Login or Copernicus Data Space account; Sentinel-1 is not presently a
native `mmotion` input. No single repository packages either pair together, and
the stronger spotlight/dwell collections below still require requests to their
authors, project teams or commercial providers.

## Strict public, pipeline-compatible detectable benchmark result

Applying all four requirements at once gives no confirmed result: the data must
be downloadable from a public URL (an unofficial mirror is acceptable),
phase-preserving, directly usable by the current pipeline, independently
synchronized and demonstrably above the practical motion floor.

| Candidate | Native `mmotion --cphd` input | Independent synchronized truth | Motion demonstrated above practical floor | Access |
|---|---:|---:|---:|---|
| South Portland Bridge, 16 May 2024 | Yes | Yes, ten 2048 Hz accelerometers | Yes, 1.5--2 mm/s peak LOS velocity and matched modal peaks | **Not public:** author request |
| Trento Capella shaker, 18 Dec 2023 | Yes | **No**, LVDT was manually shifted 0.1 s | Yes, 15 mm at 2 Hz | Author/Capella or NASA CSDA request |
| Kilauea CPHD, 14 Jul 2024 | Yes | Yes, UTC 100 Hz seismometer | **No**, about 700 times below estimated phase floor | Anonymous |
| Mexico City ICEYE CPHD, 17 Mar 2026 | Yes | Yes, 100 Hz strong-motion and 20 Hz broadband station | **No**, about 0.0012 mm peak-to-peak derived displacement | Anonymous |
| SA Agulhas II, 21--24 Jul 2022 | **No**, public KMZ contains detected TSX/TDX/CSK quicklooks | Yes, UTC 9.99 Hz onboard accelerometer array | To measure from the released batches | Anonymous ground data and quicklooks; complex SAR must be obtained |
| Galway Girl vessel, 25 Sep 2024 | No, focused Umbra/ICEYE products reported | Yes, eight accelerometers and hydrophone | Yes, matched 36 Hz engine component | Project request |
| Glasgow idling van | No, focused SLC | Yes, bonnet accelerometer | Yes, matched 87 Hz component | Project request |

Thus there is still **no publicly downloadable positive benchmark** that is
both genuinely synchronized and directly accepted by the current executable.
This conclusion includes searches for unofficial copies and does not require an
official repository or an anonymous download. The South Portland CPHD plus its
uncropped accelerometer records remains the dataset to obtain first if the
public-download constraint can be relaxed. The Trento Capella experiment is the
best controlled detectability test, but it can validate frequency and amplitude
recovery only; it must not be represented as an absolute-time synchronization
test.

### Composing a benchmark from separate public sources

The SAR and ground truth do **not** need to be deposited together. A valid
benchmark can be assembled from independently hosted files when the resulting
manifest establishes all of the following:

1. the sensor or moving structure lies inside the SAR collection polygon, not
   merely near its centroid;
2. the calibrated sensor samples bracket the SAR `start_datetime` and
   `end_datetime` in a documented UTC time base;
3. the SAR asset is phase preserving and accepted by the executable (currently
   CPHD for `mmotion`);
4. the sensor measures, or can defensibly project to, the radar line of sight;
5. the motion amplitude and frequency lie above the estimator's practical floor
   and inside its usable band; and
6. every constituent file is downloadable, even if it comes from a different
   repository or an unofficial mirror.

This compositional search is now being applied as a spatiotemporal join between
public Capella and Umbra phase-history catalogues and independently published
bridge, turbine, dam, machinery and seismic archives. Catalogue proximity alone
is insufficient. For example, Capella CPHD
`CAPELLA_C03_SP_CPHD_HH_20211231164052_20211231164115` is centred only about
8.4 km from NREL's Flatirons Campus, where public CART2 records contain UTC and
100 Hz structural channels. Exact STAC geometry shows the SAR footprint ends at
39.94019263 N, while the instrumented turbine site is farther south; moreover,
the public SUMR-D experiment ended in 2020. This is therefore a reproducible
near-match, not a synchronized pair.

The same join found one additional exact pair. ICEYE dwell-fine CPHD
`ICEYE_X49_CPHD_SLEDF_9307864_20260317T141841.cphd` covers EarthScope station
`G.UNM.00` in Mexico City from 2026-03-17 14:18:41.531 through 14:18:58.976
UTC. The station exposes three 100 Hz strong-motion acceleration channels and
three 20 Hz broadband velocity channels. Both sources are anonymously
downloadable and independently timestamped. The vertical velocity record during
the dwell has about 0.76 micrometres/s RMS and 2.94 micrometres/s peak after
linear detrending. Direct integration gives only about 0.0012 mm peak-to-peak
vertical displacement; the largest periodic displacement components are below
0.0003 mm. This is far below the approximately 0.202 mm/look phase floor already
measured for the screened ICEYE CPHD, so retain the pair as another genuine
synchronized null rather than a positive recovery benchmark.

Downloads:

- [ICEYE STAC item](https://iceye-open-data-catalog.s3.amazonaws.com/stac-items/2026/03/ICEYE_9G3MZC_20260317T141841Z_9307864_X49_SLEDF.json)
- [ICEYE CPHD](https://iceye-open-data-catalog.s3.amazonaws.com/data/dwell-fine/ICEYE_9G3MZC_20260317T141841Z_9307864_X49_SLEDF/ICEYE_X49_CPHD_SLEDF_9307864_20260317T141841.cphd)
- [EarthScope vertical strong-motion waveform](https://service.earthscope.org/fdsnws/dataselect/1/query?net=G&sta=UNM&loc=00&cha=HNZ&start=2026-03-17T14:18:30&end=2026-03-17T14:19:10&format=geocsv&scale=AUTO&nodata=404)
- [EarthScope vertical broadband waveform](https://service.earthscope.org/fdsnws/dataselect/1/query?net=G&sta=UNM&loc=00&cha=BHZ&start=2026-03-17T14:18:30&end=2026-03-17T14:19:10&format=geocsv&scale=AUTO&nodata=404)

The provider-wide structural-site screen currently gives:

- **Capella:** all 707 public spotlight CPHD collections checked. Kilauea/RIMD
  is the only exact public sensor pairing retained; the Flatirons result is an
  exact-footprint rejection.
- **Umbra:** all 1,558 open STAC items from 2024--2025 checked against Hardanger,
  Werrington, Vänersborg, openLAB, KW51 and Flatirons. None contains a target.
  Several Sydney scenes are within the deliberately loose 0.3-degree screening
  margin of Werrington, but their northern boundary is around 33.86 degrees S
  while the bridge is at 33.764 degrees S; all are exact-footprint rejections.
  A reverse join against all USGS M3+ events found a particularly close proxy:
  Umbra-05 imaged Kourou, French Guiana, from 2025-07-30 00:45:53 to
  00:45:56.2 UTC while long-period motion from the M8.8 Kamchatka earthquake was
  present at station `G.MPG.00`, 19 km from the scene.  The calibrated BHZ trace
  changes by about 0.24 mm during the dwell.  This still cannot be a pipeline
  benchmark: the 3.2-second aperture is too short for the dominant teleseismic
  period, the station is outside the footprint, and the anonymous task folder
  contains only GEC, SICD and SIDD.  The STAC item advertises an `MM.cphd` asset
  with an empty `href`, but no CPHD object exists in the public S3 delivery.
- **ICEYE:** all 374 open items inspected and the six items with CPHD assets
  extracted. None contains the known bridge/turbine archives. A separate search
  for public high-rate stations inside those six footprints found the exact
  Mexico City `G.UNM.00` pair above. The Vandenberg and Houston footprints have
  no EarthScope high-rate channel inside them at their dwell times.

### Lattice and telecommunications tower archives screened

Metal lattice masts are attractive micro-motion targets, but the public records
found so far do not provide a satellite-synchronizable field stream:

- [ERIES-BOLT](https://doi.org/10.5281/zenodo.15839050) describes a 50 m
  telecommunications tower at Sânnicolau Mare, Romania, and is a public CC BY
  4.0 download.  A range request against the 4.22 GB ZIP shows that the archive
  itself contains only the WindEEE laboratory programme (`S1` wind profiles,
  1:100 tower models and the 1:10 sectional model).  It does not contain the
  full-scale Romanian monitoring time series mentioned in the description, so
  do not download it as SAR ground truth.
- The Welsh telecommunications-mast installation described by Full Scale
  Dynamics is close to ideal technically: ten accelerometer axes, an anemometer
  and GPS synchronization.  No anonymous waveform download is published.
- [LUMO](https://doi.org/10.25835/0027803) provides dated long-term lattice
  structure measurements from 2020--2021, but its README drawings confirm that
  the test structure is only about 3 m high.  It is not a resolvable phone tower
  or useful orbital-SAR target.
- The Belgian Smart Tower archive has 250 Hz timestamped TDMS acceleration from
  a real approximately 40 m electrical transmission tower, but the tower's
  coordinates are withheld.  It cannot presently be spatially cross-matched.

### Additional public structural-motion datasets found

These repositories are public and contain genuine structural response data.
They are useful inputs to the CPHD footprint join, but none is currently a
confirmed synchronized positive CPHD pair.

- **ETH Zurich Aventa AV-7 research wind turbine** ([Zenodo
  8229750](https://zenodo.org/records/8229750)) is the strongest new spatially
  joinable candidate.  The site is at 47.520056 N, 8.682139 E (Taggenberg,
  Winterthur, Switzerland).  The tower and nacelle carry eleven accelerometers
  plus two tower strain gauges, sampled at 200 Hz; wind, yaw, RPM, power and
  turbine status are available at 10 Hz.  Public HDF5 packages cover normal
  operation, blade imbalance, rotor icing and a pitch-drive failure.  Search
  Capella, Umbra and ICEYE CPHD polygons at this coordinate before downloading
  the multi-gigabyte packages.
- **Smart Tower transmission-tower data** ([Zenodo
  8405286](https://zenodo.org/records/8405286)) provide 250 Hz TDMS
  accelerometers during high-wind excitation and modal parameters.  The target
  tower's coordinates are withheld, so it remains a structural-motion benchmark
  rather than a spatially joinable SAR candidate until the maintainer supplies
  a location.
- **Hell Bridge Test Arena** ([Zenodo
  10507957](https://zenodo.org/records/10507957)) is a full-scale steel bridge
  with 100 Hz acceleration and strain data, a modal vibration shaker and known
  damage states.  It is the clearest public controlled positive/damage dataset,
  but the completed CPHD screen found no simultaneous Capella, Umbra or ICEYE
  pair.
- **Route 345 Bridge, New York** ([Zenodo
  15497673](https://zenodo.org/records/15497673)) contains dual-axis
  accelerometers and strain transducers under vehicle loading and imposed
  damage in a public HDF5 file.
- **Concrete box-girder bridge, Germany** ([Zenodo
  20522191](https://zenodo.org/records/20522191)) contains acceleration,
  inclination, temperature and strain during vehicle crossings and static loads,
  plus tracked modal frequencies.  The full experiment file is about 9.7 GB.
- **FLOOD-SHAB cable-stayed bridge** ([Zenodo
  20443860](https://zenodo.org/records/20443860)) provides a compact feature
  dataset and sensor-channel map for a documented 2026 hydrometric event.  It
  is useful for event screening, but is not a replacement for raw high-rate
  waveforms.
- The **Aventa, Hell Bridge, Route 345, box-girder and FLOOD-SHAB records**
  should be treated as standalone positive SHM fixtures unless an exact CPHD
  footprint and UTC dwell are found.  A positive SAR benchmark still requires
  polygon containment, temporal overlap, line-of-sight projection and motion
  above the measured CPHD phase floor.

### Rail and meteorological targets

- The **openLAB research bridge in Bautzen** is an excellent public terrestrial
  benchmark but has no public complex-SAR overlap in the completed provider
  screen. OPARA publishes 523 event-triggered, UTC ISO-8601 CSV records from
  May--October 2024: six vertical accelerometers at 500 Hz, 70 seconds per
  trigger, plus environmental, tilt, simulated-traffic and construction-model
  material. The trigger threshold is 2e-4 m/s2 and acceleration is band-passed
  0.5--100 Hz. This is immediately useful for positive/negative detector tests
  after converting acceleration to displacement, but cannot validate SAR until
  phase history over 51.1816 N, 14.4277 E at an exact trigger time is obtained.
  Dataset: <https://doi.org/10.25532/OPARA-660>; data paper:
  <https://doi.org/10.1016/j.dib.2025.111624>.
- Public rail-bridge monitoring is comparatively rich.  KW51 in Leuven provides
  two train passages plus six ambient records per day over 15 months, and the
  Vänersborg bascule railway bridge provides measurements around verified
  damage.  Both have known positions and absolute acquisition dates, but the
  Capella, Umbra and ICEYE footprint screens above found no native-CPHD overlap.
  They remain useful if a Sentinel-1 Level-0 ingest path is added.
- The supplied Vänersborg record is Zenodo `8300495`: a 551.7 MB ZIP containing
  64 raw UTC CSV bridge-opening events sampled at 200 Hz, with 16 strain gauges,
  five accelerometers, inclination and weather. It includes the *actual verified
  fracture event* beginning `2023-03-09T23:45:25Z`, plus before/after cases.
  This is one of the best downloadable structural-damage benchmarks found, but
  it remains a pipeline-side algorithm fixture rather than synchronized SAR.
  <https://doi.org/10.5281/zenodo.8300495>
- The 2025 Norwegian inventory confirms four complementary open collections:
  Hardanger (long-term wind/acceleration, 2 and 200 Hz), Gjemnessund (wind and
  acceleration), Bergsoysund (wind, waves, acceleration and GNSS displacement),
  and Hell Bridge Test Arena (100 Hz shaker tests across imposed damage states).
  The first three are real operational motion; Hell is the clearest controlled
  positive/damage benchmark. None supplies native complex SAR, and completed
  Capella/Umbra/ICEYE footprint screening found no simultaneous native-CPHD
  pair. Catalogue: <https://www.ntnu.edu/kt/open-data>.
- On-board train archives can include synchronized GNSS, IMU and axle-box
  acceleration.  They are poor validation targets for the current estimator:
  the whole scatterer translates rapidly through many resolution cells rather
  than oscillating about a fixed range bin.  A stopped, idling train with a
  coincident SAR dwell would be different, but no public instance was found.
- Tall meteorological masts such as the 213 m Cabauw mast have known coordinates
  and extensive public UTC wind/meteorology records, but those channels measure
  the atmosphere rather than structural displacement.  Wind speed alone is not
  admissible motion truth.
- A weather-radar station is a more interesting active target class.  Its large
  metal antenna rotates, and public Level-II radar volumes timestamp individual
  scans.  A CPHD footprint containing such a station could therefore supply an
  independently timed rotation/micro-Doppler event, although this is rotational
  motion rather than the small sinusoidal translation assumed by `mmotion`.

### High-rate GNSS sources

High-rate GNSS is preferable to ordinary daily coordinate products because it
provides absolute UTC displacement without integrating an accelerometer.  The
most relevant public holdings are EarthScope/NOTA raw 1 Hz and selected 5 Hz
RINEX, NASA CDDIS 1 Hz high-rate RINEX, GeoNet New Zealand 1 Hz plus event-only
10 Hz data, and the SOPAC earthquake archive with processed 1--5 Hz GNSS and
100 Hz seismogeodetic waveforms.  The USGS also publishes processed displacement
waveforms for M6--M9 events from 2003--2019.

Sampling rate matters for this pipeline.  A 1 Hz stream has a 0.5 Hz Nyquist
limit and cannot validate the roughly 1.5--3.8 Hz band used by the short-aperture
examples.  EarthScope 5 Hz data reaches 2.5 Hz and is the best large-network
cross-match source; 10--20 Hz bridge GNSS campaigns are better dynamically but
are sparse in time.  The Severn Suspension Bridge campaigns used nine receivers
at 10 or 20 Hz and observed centimetre-scale deck/tower response, but they were
short campaigns rather than a continuously public archive.

## Best candidates

### Kilauea earthquake, Capella CPHD plus HVO station RIMD

**Status:** Exactly synchronized, anonymously downloadable and native CPHD, but
the measured motion is too small to be a useful positive-recovery benchmark.
Retain it as a real-data null/negative case and as a reproducible integration
fixture.

- Capella acquisition
  `CAPELLA_C09_SP_CPHD_HH_20240714174801_20240714174831`, from
  2024-07-14 17:48:01.678623 through 17:48:31.142103 UTC: X-band HH,
  right-looking descending spotlight, 46.3 degree incidence, 29.463-second
  CPHD dwell, 277,337 pulses and nominal 9,412.87 Hz PRF.
- The CPHD is about 30.75 GB (`SIGNAL_BLOCK_SIZE` 30,673,472,200 bytes), so run
  the 11,191-byte metadata screen before downloading it. The repository's
  reader accepts the file directly; a full in-memory complex-float load would
  require about 56.7 GB, so a real run should restrict `--rbins`.
- Reviewed USGS event `hv74334241`: M1.79, origin
  2024-07-14 17:48:09.850 UTC, 7 km SSW of Volcano, Hawaii. The origin is 8.17 s
  after radar start and the epicentre lies near the eastern scene boundary.
- HVO station `HV.RIMD..HHZ` is inside the CPHD polygon at
  19.395368 N, 155.273643 W. It is a UTC 100 Hz vertical velocity channel. The
  EarthScope GeoCSV request below returns 4,501 calibrated samples in m/s from
  17:47:55 through 17:48:40 UTC, fully bracketing the dwell.
- In that trace, velocity RMS during the dwell is approximately
  3.40 micrometres/s versus 1.34 micrometres/s immediately before it, with a
  peak absolute velocity of 24.2 micrometres/s. Detrending and integrating the
  trace gives about 3.07 micrometres peak-to-peak displacement, but its largest
  periodic component inside the pipeline's band is only approximately
  0.00053 mm at 0.272 Hz. This derived displacement is useful for screening,
  not a substitute for a fully response-deconvolved displacement product.
- `validate --xml` reports WARN (no arithmetic failure) at 0.272 Hz with the
  phase estimator, but estimates 0.373 mm line-of-sight noise per look. The
  measured spectral amplitude is therefore roughly 700 times smaller. The
  stronger approximately 2.9 Hz component is also above the 2.533 Hz band for
  the documented `--alpha 0.0067 --overlap 0` configuration. Do not cite this
  pair as a successful SAR micro-motion detection opportunity.
- A global cross-match of all 707 Capella open spotlight CPHD dwell times with
  the complete 2021--2026 USGS M>=3 catalogue produced one other seismic
  travel-time match: M3.66 event `pr2024326000` during a Saint Kitts scene on
  2024-11-21. The nearby `TR.SKI` station has no EarthScope waveform for the
  interval, so that timing-only coincidence was rejected as ground truth.

Downloads and primary records:

- [CPHD data object](https://capella-open-data.s3.amazonaws.com/data/2024/7/14/CAPELLA_C09_SP_CPHD_HH_20240714174801_20240714174831/CAPELLA_C09_SP_CPHD_HH_20240714174801_20240714174831.cphd)
- [Capella STAC item](https://capella-open-data.s3.us-west-2.amazonaws.com/stac/capella-open-data-by-datetime/capella-open-data-2024/capella-open-data-2024-07/capella-open-data-2024-07-14/CAPELLA_C09_SP_CPHD_HH_20240714174801_20240714174831/CAPELLA_C09_SP_CPHD_HH_20240714174801_20240714174831.json)
- [USGS event record](https://earthquake.usgs.gov/earthquakes/eventpage/hv74334241)
- [EarthScope calibrated GeoCSV waveform](https://service.earthscope.org/fdsnws/dataselect/1/query?net=HV&sta=RIMD&loc=--&cha=HHZ&start=2024-07-14T17:47:55&end=2024-07-14T17:48:40&format=geocsv&scale=AUTO&nodata=404)
- [EarthScope station-service query](https://service.earthscope.org/fdsnws/station/1/query?format=text&level=channel&net=HV&sta=RIMD&loc=--&cha=HHZ&starttime=2024-07-14T17:47:55&endtime=2024-07-14T17:48:40)

Metadata-only reproduction:

```sh
P=CAPELLA_C09_SP_CPHD_HH_20240714174801_20240714174831
U=https://capella-open-data.s3.amazonaws.com/data/2024/7/14/$P/$P.cphd
curl -sS -r 1024-12214 "$U" > "$P.xml"
./build/micromotion validate --xml "$P.xml" --frequency 0.27155 \
  --amplitude 0.000528 --estimator phase --alpha 0.0067 --overlap 0
```

### South Portland Street Suspension Bridge, Glasgow

**Status:** Available from the corresponding author on request; not publicly
available because of privacy or ethical restrictions.

- Real suspension bridge under normal pedestrian use.
- Exact bridge centre: 55°51′17.79″ N, 4°15′20.11″ W.
- Two Umbra high-resolution spotlight SLC acquisitions.
- Exact passes: 20 May 2024 at 21:40:40 UTC and 16 May 2024 at
  20:21:23 UTC.
- The 16 May acquisition was delivered as CPHD: 22.663-second dwell, 6.06 kHz
  pulse repetition frequency (242.4 Hz effective), 9.6 GHz centre frequency,
  0.63 m range resolution and 0.074 m azimuth resolution. This is the closest
  known match to this project's present phase-history route.
- The later journal work used two focused SLC products. Request the CPHD and
  both SLCs rather than assuming the papers all used the same source product.
- Ten bridge-mounted accelerometers sampled at 2048 Hz for about 20 minutes
  around each satellite pass.
- Vertical and horizontal sensor layouts, with measurements projected into the
  radar line of sight.
- The same collections have been analysed using pixel tracking, modified
  backprojection and sub-aperture phase analysis.
- Best current candidate for testing natural structural vibration and spatial
  mode agreement.

Contact route: request the data from the corresponding author through the
publication or institutional repository linked below.

Access routes worth naming in a request: ESA contracts `4000142119` (Bridge
Monitoring Based on Single Pass SAR Images) and `4000142272/23/I-DT`
(EO4Security Topic B2: Micro-Doppler Processing). The work also cites NASA
commercial-data programmes `NNH22ZDA001N-CSDSA`, `NNH23ZDA001N-CNVOE` and
`NNH24ZDA001N-CESRA`. These are funding/procurement routes, not public dataset
records; repository and exact-contract searches found publications only. The
journal's explicit availability statement remains that the data can be obtained
from the corresponding author on request and are not public for privacy or
ethical reasons.

Sources:

- A. Lotti et al., "Monitoring Bridge Vibrations via Spaceborne SAR
  Micro-Doppler," *Structural Control and Health Monitoring*, 2026.
  <https://doi.org/10.1155/stc/3858095>
- ESA contract 4000142119, "Bridge monitoring based on single pass SAR
  images." <https://activities.esa.int/4000142119>
- CPHD acquisition and sensor details:
  <https://strathprints.strath.ac.uk/90733/1/Bridge_vibration_measurements_from_very_high-resolution_spaceborne_SAR.pdf>
- A. Lotti, *Advancements in Satellite SAR Methods for Monitoring Civil
  Infrastructure*, doctoral thesis, University of Trento, defended 29 June
  2026: <https://hdl.handle.net/11572/491650>. The repository record is public
  but current searches expose only the thesis metadata/abstract, not a data
  supplement. It independently describes successful identification down to
  roughly 1 mm/s and expected 0.03–0.06 Hz resolution for 20–30 s collects;
  neither number is a false-alarm-controlled sensitivity bound.

### Korean bridge, ICEYE Dwell

**Status:** No public data repository or availability statement found.

- The target is Galmae Bridge in Asan, Chungcheongnam-do, South Korea.
- ICEYE Dwell Fine X-band CPHD: 27-second dwell, 9.6 GHz, VV polarization
  and 6,991 Hz PRF.
- Six deck accelerometers sampled at 100 Hz during the same period.
- The accelerometers give a dominant natural frequency of 2.80 Hz; the SAR
  estimate is 2.76 Hz.
- Registration between sub-aperture images and low-order polynomial detrending
  of several scatterer phase series.
- Particularly relevant to this project's low-frequency trend problem because
  the authors explicitly remove residual trends before estimating frequency.
- The article does not state the acquisition date, sensor model, UTC
  synchronization method, raw-data availability or a static control. Those are
  the first details to request from the corresponding author.
- The experiment belongs to KICT project `2025-0285`, "Development of
  infrastructure disaster prevention technology based on satellite SAR," which
  runs from 2025 through 2027. Its first-year report is a 680-page KICT report
  (`KICT 2025-059`), catalogued as a physical library holding with no public PDF.
  This makes KICT, particularly coauthor Taejeong Lim in its Structural
  Engineering Research Department, a second data-request route independent of
  Korea Aerospace University. Because the programme is still active, a formal
  dataset may appear with later project outputs even though none is registered
  now.

Contact route: use the corresponding-author details on the article page linked
below.

Source: Y.-S. Yoo et al., "Estimating Bridge Natural Frequencies Using Phase
Time-Series Micro-Doppler Analysis with Dwell-Mode Satellite SAR," *The Journal
of Korean Institute of Electromagnetic Engineering and Science*, 37(3), 2026,
DOI `10.5515/KJKIEES.2026.37.3.241`.

- Direct open PDF:
  <https://www.jkiees.org/download/download_pdf?pid=jkiees-37-3-241>
- Article record:
  <https://www.jkiees.org/archive/view_article?pid=jkiees-37-3-241>
- KICT first-year report catalogue record:
  <https://library.kict.re.kr/search/catalog/view.do?bibctrlno=164335&se=b0&ty=B>

### MIDAS infrastructure campaign, Basilicata

**Status:** No public download found; request from the project team.

- ICEYE and COSMO-SkyMed fine-resolution SAR over bridges, water channels and
  water towers in Basilicata, Italy.
- Independent ground-based real-aperture radar vibration measurements.
- Includes a scene-wide frequency-map stage intended to locate vibrating
  targets before detailed analysis.
- The reported spaceborne and ground-radar frequencies agree.
- Not accelerometer ground truth, but potentially the best collection for
  testing blind target discovery across several real structures.

Contact route: contact the MIDAS project team through the conference record
linked below.

Source: "MIcro-Doppler InfrAstructure Stability Assessment using SAR (MIDAS),"
ESA Living Planet Symposium 2025.
<https://lps25.esa.int/lps25-presentations/poster/MIcro-Doppler%20InfrAstructure%20Stability%20Assessment%20using%20SAR%20%28MIDAS%29%20-.pdf>

## Controlled land targets

### Umbra controlled-target assessment, Trento and Glasgow

**Status:** No public repository found.

- Nine Umbra spotlight acquisitions of a corner reflector on a linear actuator.
- Test coordinates are Villa Gherta, Trento (46.06814, 11.14684) and Glasgow
  Green (55.84379, -4.23634). The LVDT sampled at 50 Hz for about 20 minutes
  around each scheduled pass.
- Synchronous displacement ground truth covering single tones, amplitude
  modulation, multiple tones and a frequency sweep from 1 to 4 Hz.
- Reported radial RMS displacements extend from 10.43 mm to 0.10 mm.
- The journal paper's complete acquisition table gives exact archive search
  keys for all nine tests:

  | Test | Spacecraft | Start (UTC) | Dwell | Input | Radial RMS |
  |---:|---|---|---:|---|---:|
  | 1 | Umbra-05 | 2023-11-18 20:46:27 | 6.04 s | 1 Hz tone | 10.43 mm |
  | 2 | Umbra-04 | 2024-03-14 20:35:48 | 5.41 s | 2 Hz tone | 1.65 mm |
  | 3 | Umbra-05 | 2024-03-14 21:29:22 | 5.95 s | 2 Hz tone | 0.52 mm |
  | 4 | Umbra-09 | 2025-02-24 10:53:09 | 16.01 s | 2 Hz, 0.2 Hz AM | 0.19 mm |
  | 5 | Umbra-10 | 2025-02-25 10:52:49 | 15.93 s | 2 Hz, 0.2 Hz AM | 0.10 mm |
  | 6 | Umbra-09 | 2025-01-30 10:46:27 | 14.70 s | 1 Hz, 0.2 Hz AM | 0.23 mm |
  | 7 | Umbra-09 | 2025-01-31 10:46:52 | 14.79 s | 1 Hz, 0.2 Hz AM | 0.10 mm |
  | 8 | Umbra-08 | 2024-03-29 20:57:41 | 7.98 s | 2 and 4 Hz | 0.66 mm |
  | 9 | Umbra-04 | 2024-03-29 21:29:38 | 6.38 s | 1–4 Hz sweep | 1.98 mm |

  The raw PRFs span 5.44–7.75 kHz and the stated azimuth resolutions span
  0.07–0.22 m. Exact-timestamp web searches only recover the publications, not
  catalogue records or downloadable products.
- Useful for frequency and amplitude sweeps, but every acquisition contains a
  known vibrating target and the published work provides no static-scene
  false-alarm assessment.
- This is not a fully blind benchmark. The authors report residual timing
  uncertainty and allow the recovered SAR series to shift by up to half a
  vibration period to maximise correlation with the LVDT. They also choose
  processing parameters by maximising correlation with that same ground truth.
  A reuse should lock timing and parameters on separate calibration tests before
  scoring held-out tests; otherwise the published errors can be optimistic.
- A separate open conference paper identifies the sites as Villa Gherta in
  Trento and Glasgow Green and describes the inputs for four of the trials:
  45 mm at 1 Hz, 24 mm at 1 Hz, 5 mm at 2 Hz and 3 mm at 2 Hz. It says the
  processing starts from Umbra SICD SLC products and the displacement reference
  is a real-time LVDT. It does not supply acquisition dates or data files.
- All nine images are now tied to exact spacecraft and UTC start times in the
  table above. Umbra's anonymous open-data listings for those dates do not
  expose the experiment products. Umbra's broader Archive Catalog API returns
  `Unauthorized` without a Canopy account, so it cannot provide task IDs
  anonymously. These remain precise commercial-archive requests, not public
  downloads.

Contact route: use the corresponding-author details in the publication linked
below.

Source: A. B. Vattulainen et al., "Assessment of Spaceborne SAR Micro-Motion
Measurement for Vibration-Based SHM," *IEEE Access*, vol. 14, 2026.
<https://doi.org/10.1109/ACCESS.2026.3652346>

Additional test table: C. Suppi et al., "Vibrational Monitoring of Isolated
Targets Using Single-Pass SAR Images," SHM 2025.
<https://www.dpi-proceedings.com/index.php/shm2025/article/download/37524/36097>

### Glasgow Green shaker

**Status:** Described in a public thesis; SAR and accelerometer files are not
attached.

- Umbra-04 acquisition over a corner reflector on a mechanical shaker.
- Accelerometer ground truth: 20 Hz and 2.21 mm amplitude at the satellite pass.
- The surrounding park supplies many real stationary windows.
- A detection chain using CFAR, time-frequency refinement and classification was
  tested on the scene.
- Strong candidate for false-alarm work because detection was not limited to a
  nominated target window.
- A separate paper identifies the relevant Umbra trial date as 14 March 2024.
  Umbra's public STAC bucket has no entry for that date, so the experiment is
  not anonymously downloadable there.

Source: L. Parra Garcia, "Advanced maritime target recognition from SAR images
exploiting target's micro-motions and AI," University of Strathclyde PhD thesis,
2026. <https://doi.org/10.48730/ab1d-jm97>

### Trento TerraSAR-X shaker experiment

**Status:** The acquisition should exist in DLR's archive, but it is not an
anonymous download. Scientific access requires an accepted TerraSAR-X proposal;
the LVDT file still has to be requested from the experiment team.

- TerraSAR-X spotlight acquisition in Trento on 18 December 2023 at 17:09:29
  UTC.
- Published radar parameters include 9.65 GHz centre frequency, 4.4 kHz PRF,
  0.584 m by 0.216 m slant resolution and a 7.63-second dwell.
- The controlled target was a corner reflector on a programmable linear servo,
  with displacement measured directly by a contacting LVDT.
- This is a separate data-access route from the Capella CPHD experiment below:
  it is a focused TerraSAR-X product held under DLR's science-data rules.
- Published descriptions of the imposed motion conflict. One paper labels the
  target 1 Hz and 10 mm, while the coherent-change paper reports 2 Hz and a
  16.13 mm ground-truth amplitude for its TerraSAR-X trial. The raw LVDT record
  and product metadata are therefore essential; the captions are not adequate
  ground truth.

Sources and access:

- Experiment details:
  <https://strathprints.strath.ac.uk/90731/1/Scale_Invariant_Coherent_Change_Detection_to_Locate_Micro-Motion_in_Single_Pass_SAR_Images.pdf>
- Independent figure caption with the exact timestamp:
  <https://strathprints.strath.ac.uk/95029/7/Lotti-etal-SCHM-2026-Monitoring-bridge-vibrations-via-spaceborne-SAR-micro_Doppler.pdf>
- DLR TerraSAR-X science access:
  <https://sss.terrasar-x.dlr.de/>
- DLR archive catalogue and access explanation:
  <https://www.dlr.de/en/eoc/research-transfer/topics/satellite-data/data-guide>

### Trento Capella CPHD experiment

**Status:** No public download or availability statement found.

- Capella collection on 18 December 2023 over Trento, Italy.
- Corner reflector driven with a 2 Hz sinusoid and 1.5 cm vertical amplitude.
- LVDT measured displacement during the pass. The 2024 modified-backprojection
  paper explicitly says the clocks were not synchronized and shifts the LVDT by
  0.1 s to improve alignment. A later 2025 paper calls the measurement
  synchronous without explaining that discrepancy. Treat frequency and
  amplitude as genuine ground truth, but not absolute phase or UTC alignment,
  unless the original files and clock logs resolve it.
- Processed from CPHD, making it a direct match for this project's CPHD route.

Source: F. Rollo et al., "Micro-motion extraction from spotlight SAR using a
modified backprojection approach," SPIE, 2024.
<https://doi.org/10.1117/12.3031678>

The date is present in Capella's public date index, but the only public SLC on
18 December 2023 covers Western Australia (centroid 23.184 S, 118.771 E), not
Trento. It is therefore a different acquisition and is not useful ground truth.

A 2025 EUSIPCO paper confirms that the Trento file is CPHD with per-pulse
transmit and receive timestamps, about 200,000 pulses over roughly 20 seconds,
and describes the LVDT displacement as synchronous. It reports a 2 Hz, 15 mm
test and explicitly thanks Capella for providing the data, but supplies neither
data availability statement nor attached files. Because the earlier analysis
discloses the 0.1 s manual alignment, “synchronous” here means overlapping
measurement unless the authors can supply evidence of a common timebase. This
is the same collection, not a new dataset:
<https://strathprints.strath.ac.uk/94400/>.

NASA CSDA says its controlled Capella archive has partial global coverage from
18 October 2022 through 14 May 2024, a range that includes this experiment.
Therefore the CPHD may be discoverable in the logged-in Satellite Data Explorer
even though it is absent from Capella's open bucket. Download requires Earthdata
Login, CSDA authorization and a qualifying licence; the public NASA CMR API does
not expose a Capella collection record. This is a credible restricted-access
route, not a public download.

- NASA Capella access: <https://science.nasa.gov/earth-science/csda/vendor-capella/>
- Satellite Data Explorer: <https://csdap.earthdata.nasa.gov/>

### ESA coherent-change detection trials

**Status:** Project data have not been published.

- TerraSAR-X data with a shaker moving at about 0.2 m/s.
- Umbra data with a shaking corner reflector around 0.01 m/s and a stationary
  fence in the same scene.
- The stationary bright target is unusually useful: it can test whether a
  detector mistakes stable dominant scatterers for vibration.

Source: ESA, "Innovative micro-Doppler processing and applications from SAR."
<https://eo4society.esa.int/2025/05/05/innovative-micro-doppler-processing-and-applications-from-sar/>

## Other real targets

### Celtic Park stadium, Glasgow

**Status:** No acquisition identifiers, ground-truth files or public download
found.

- Umbra data over a real stadium.
- Doppler processing and pixel tracking reportedly extracted consistent modes.
- ESA reports agreement with in-situ measurements, but the public description
  does not state which sensor supplied them.

Source: ESA, "Innovative micro-Doppler processing and applications from SAR."
<https://eo4society.esa.int/2025/05/05/innovative-micro-doppler-processing-and-applications-from-sar/>

### Galway Girl vessel

**Status:** Described in ESA material and a public thesis; source files are not
attached.

- Umbra-08 acquisition of a cooperative 24 m passenger vessel in Galway Bay.
- Eight onboard accelerometers plus GPS; a hydrophone recording is also
  mentioned by ESA.
- Dominant vibration near 36 Hz, tied to the engine firing rate.
- The acquisition occurred on 25 September 2024 at 11:28 UTC.
- Not a fixed structure, but the multiple sensors and surrounding water make it
  useful for spatial agreement and false-alarm experiments.

Sources:

- ESA, "Listening to motion from space."
  <https://eo4society.esa.int/2026/02/09/listening-to-motion-from-space/>
- L. Parra Garcia, University of Strathclyde PhD thesis, 2026.
  <https://doi.org/10.48730/ab1d-jm97>

### Idling van, Glasgow

**Status:** No public download found.

- Standard SLC acquisition over a stationary Volkswagen Transporter.
- Triaxial accelerometer mounted on the bonnet.
- ESA reports a strong 87 Hz SAR component consistent with the accelerometer.
- Nearby land clutter could provide genuine negative windows.

Source: ESA, "Listening to motion from space."
<https://eo4society.esa.int/2026/02/09/listening-to-motion-from-space/>

## Airborne fallback

An older APY-6 airborne SAR experiment observed a reflector vibrating at 2 Hz
with an 8 mm ground-truth displacement. The SAR estimate was about 1.5 mm. The
amplitude error is scientifically useful, but the collection is airborne rather
than spaceborne and no current public archive was found.

### GOTCHA SAR-GMTI challenge

**Status:** Public and downloadable from AFRL SDMS after creating a free public
account, with phase history and independent motion truth.

- Airborne X-band spotlight SAR rather than spaceborne SAR.
- Two 71-second, three-channel phase-history passes over an urban scene.
- A Dodge Durango carries GPS sampled at 1 Hz; trajectory and speed truth are
  supplied with the radar data.
- This is translation, not vibration, so it cannot validate millimetric
  structural frequency or amplitude. It can test phase-history ingestion,
  Doppler displacement, motion-versus-clutter separation and false alarms in a
  real cultural scene.

Download page:
<https://www.sdms.afrl.af.mil/index.php?collection=gmti>

### Capella public infrastructure CPHD controls

**Status:** Direct, anonymous downloads under CC BY 4.0; no accelerometer truth.

Capella's infrastructure collection contains many SLC, SICD and CPHD products.
These are useful as real-scene negative/control material, not positive vibration
ground truth. One practical candidate is a 10.01-second spotlight CPHD near
Barcelona acquired on 16 May 2024. It is 12.30 GB and includes dense built
infrastructure. A longer 60-second Valencia-area CPHD is also available but is
60.48 GB.

- 10-second CPHD metadata:
  <https://capella-open-data.s3.us-west-2.amazonaws.com/stac/capella-open-data-by-datetime/capella-open-data-2024/capella-open-data-2024-05/capella-open-data-2024-05-16/CAPELLA_C14_SP_CPHD_VV_20240516031533_20240516031543/CAPELLA_C14_SP_CPHD_VV_20240516031533_20240516031543.json>
- 10-second CPHD direct download:
  <https://capella-open-data.s3.amazonaws.com/data/2024/5/16/CAPELLA_C14_SP_CPHD_VV_20240516031533_20240516031543/CAPELLA_C14_SP_CPHD_VV_20240516031533_20240516031543.cphd>
- Public infrastructure collection:
  <https://capella-open-data.s3.us-west-2.amazonaws.com/stac/capella-open-data-by-use-case/capella-open-data-public-infrastructure/collection.json>

Because no ground sensor was synchronized to these scenes, a reported line
cannot be labelled false with certainty: machinery, traffic and other real
motion may be present. They are nevertheless much stronger stress tests for a
synthetic-null model than noise generated from that same model.

### NISAR Level 0 raw spaceborne data

**Status:** Public NASA archive; no vibration ground truth.

NISAR now archives Level 0B Radar Raw Signal Science Data containing unpacked
radar echoes and instrument telemetry. This is genuine spaceborne pulse data,
not an image-only benchmark. It could broaden null testing across ordinary
terrain and infrastructure and expose instrument effects absent from a Capella-
or Umbra-derived model. NISAR is L-band and its standard imaging geometry and
resolution are not designed for the millimetric spotlight-vibration problem, so
it is not a positive sensitivity dataset or a direct substitute for X-band
CPHD.

- Dataset record:
  <https://catalog.data.gov/dataset/nisar-permanent-archive-radar-raw-signal-science-data>
- NISAR product guide:
  <https://hyp3-docs.asf.alaska.edu/nisar-docs/products-overview/>

### Raw laboratory GBSAR collections

**Status:** Anonymous public downloads from Mendeley Data; static scenes only.

Two 24 GHz rail-scanner datasets publish raw complex I/Q measurements rather
than rendered images. They can verify reconstruction and complex-data handling
on independently produced radar data, but their stop-and-go indoor scenes do
not contain a synchronized vibrating target and cannot validate micro-motion.

- Multimodal raw GBSAR and optical scenes (2026), DOI
  `10.17632/y5gb5368xr.1`:
  <https://data.mendeley.com/datasets/y5gb5368xr/1>
- Raw GBSAR with horizontal and vertical polarization (2024), DOI
  `10.17632/nbc9xpwv96.1`:
  <https://data.mendeley.com/datasets/nbc9xpwv96/1>

## Open structural waveforms for injection tests

These releases contain measured structural motion but no simultaneous SAR.
Their raw time histories are better stress tests than ideal sine waves because
they include changing excitation, multiple modes, transients and real sensor
noise. They can test whether an injected waveform is recovered, but they cannot
establish the radar's sensitivity or false-alarm rate. Acceleration also should
not be converted to displacement without stating the integration and filtering
assumptions.

### openLAB bridge shaker tests, Bautzen

**Status:** Anonymous Zenodo download; CSV files, 4.6 GB total.

- Full-scale research prestressed-concrete bridge.
- Shaker sine sweeps and band-limited noise across several known damage states.
- Records both acceleration at the shaker and structural vibration velocity
  from geophones in two bridge fields.
- The companion benchmark paper says its example files were resampled to 500 Hz
  and high-pass filtered at 2 Hz. Use the repository's original channels and
  metadata, not the plotted example, for a low-frequency test.
- This is the strongest newly found source for testing peak tracking against a
  nonstationary, measured input and for checking whether a band-edge artefact
  follows a sweep.

- Dataset: <https://zenodo.org/records/18546379>
- Benchmark description:
  <https://fe.up.pt/evaces2025/ficheiros/papers_finais/SS34_paper_1038.pdf>
- A Copernicus catalogue check at the bridge found Sentinel-1 Level-0 IW RAW
  acquisitions at 05:16:27--05:17:00 UTC on 5 May 2025 and
  05:09:21--05:09:54 UTC on 6 May 2025. The published shaker files on those
  dates start during the afternoon/local working day (14:47 on 5 May and 09:27
  on 6 May onward), so neither satellite pass overlaps a released shaker
  record. This campaign is therefore **not** a synchronized SAR pair.

### Laboratory model bridge with displacement and acceleration

**Status:** Anonymous Mendeley download; one 158 MB MATLAB file plus a README.

- A 5.6 m steel model bridge crossed by a 28 kg model vehicle at three speeds.
- Synchronous vertical acceleration **and displacement** at five bridge
  positions, plus vehicle acceleration.
- The displacement channels avoid the uncertain double integration required by
  acceleration-only data and are therefore attractive for waveform injection.

Dataset: <https://data.mendeley.com/datasets/3srffc36dz/1>

### Hell Bridge Test Arena shaker campaign

**Status:** Anonymous Zenodo download.

- Full-scale steel bridge tested in undamaged and known-damage states.
- Bridge and shaker accelerometers under sine-sweep and white-noise excitation;
  strain and load channels are also included.
- The published benchmark applies a 2 Hz high-pass filter to its examples, so
  this dataset is more suitable for multi-hertz recovery tests than for the
  present 0.1--0.2 Hz trend question.

Dataset: <https://zenodo.org/records/10507957>

### Railway bridge KW51, Leuven

**Status:** Anonymous Zenodo download; 45.6 GB in month-sized archives.

- Fifteen months spanning a real structural retrofit.
- Acceleration, strain, bearing displacement and environment channels.
- Two train passages and six ambient-vibration periods per day.
- Useful for long, messy, operational waveforms and stationary/ambient controls;
  the records were not collected at a satellite pass.

Dataset: <https://zenodo.org/records/3745914>

### NSW cable-stayed bridge continuous archive

**Status:** Anonymous UTS download; raw and decimated ZIP archives.

- Independent authors: Hamed Kalhori, Mehrisadat Makki Alamdari, Bijan Samali,
  Ben Halkon and Maria Rashidi.
- Dense, time-synchronised uniaxial accelerometers on the deck and cables,
  continuous temperature, and bridge-end shear strain for traffic activity.
- The repository exposes a single-day set (1 GB decimated, 5 GB original), a
  constant-temperature set (510 MB decimated, 2 GB original), two damaged-case
  files (18 MB and 1 MB), and three seasonal files of about 11 GB each. It also
  supplies 23 structural drawings.
- The structure is the Western Sydney University/Werrington Bridge over the
  Great Western Highway. An ANSHM project report gives its WGS84 position as
  33°45′50.49″S, 150°44′31.14″E. This resolves the repository's erroneous
  Dubai coordinates.
- Sentinel-1A Level-0 IW RAW products cover the bridge at 19:15 UTC on both
  9 November (`S1A_IW_RAW__0SDV_20161109T191503_20161109T191536_013869_0164F4_DF32`)
  and 21 November (`S1A_IW_RAW__0SDV_20161121T191503_20161121T191536_014044_016A4A_BBDE`).
  Inspection of the public ZIP central directories,
  however, shows that the 5.74 GB single-day original archive contains 2
  November, while the selected constant-temperature archive exposes only
  three 21 November records beginning at 12:33, 12:43 and 12:53. Those records
  end many hours before the 19:15 satellite pass, and no released 9 November
  record is present. The catalogue/date-range coincidence is therefore a
  verified **non-match**, not a synchronized pair.
- The Taylor & Francis Figshare page contains only a 1.8 MB access-instructions
  document; the actual measurements are in the UTS portal linked below.

Dataset: <https://data.research.uts.edu.au/publication/0bacede4bd5a39993ec5088e6247cac8/>

Metadata/API record: DOI `10.26195/5e5302baf9221`.

### Istanbul Golden Horn Metro Bridge — exact private sensor counterpart

**Status:** Public Capella CPHD is already downloaded and pipeline-compatible;
the target bridge has an unusually complete permanent SHM system, but no public
waveform export for the acquisition interval was found.

Capella spotlight CPHD
`CAPELLA_C09_SP_CPHD_HH_20230321101754_20230321101819` covers the Golden Horn
Metro Crossing Bridge at 41.0227 N, 28.9667 E on 21 March 2023 from
10:17:54--10:18:19 UTC (13:17:54--13:18:19 local). It is a 17.3 GB, 25-second,
19.53-degree-incidence X-band HH collect and is stored under
`/Volumes/ZX20 II/sar-data/capella/`. The public phase history has already been
focused and measured by this repository.

The exact bridge—not merely another structure in Istanbul—has continuously
recorded structural monitoring operated through Metro Istanbul's control
centre. Published system descriptions enumerate:

- six 3-D, eight 2-D and one 1-D deck accelerometers;
- 3-D/2-D pylon, stay-cable, swing-deck, pier, abutment and seismic
  accelerometers;
- four GPS receivers, joint/ultrasonic displacement, deck/pylon tilt, wind and
  environmental channels; and
- source-position identifiers and timestamps attached to the raw data, with
  tabular export from the monitoring interface.

This makes the 25-second interval a potentially excellent positive benchmark:
traffic and M2 trains were operating, the radar footprint contains the monitored
deck/pylons, and the Valens Aqueduct supplies a nearby nominally static control.
The missing component is only access to the bridge-side export for
`2023-03-21 10:17:45--10:18:30 UTC`, ideally including a few minutes around it
for noise and modal estimation.

A second public Capella CPHD over the same site,
`CAPELLA_C11_SP_CPHD_HH_20230907223849_20230907223949`, provides a useful
same-bridge comparison: 60 seconds at approximately 01:39 local, when scheduled
M2 service was not operating. Its 38.5-degree incidence is worse and the file is
about 60.8 GB, but matching SHM exports around both acquisitions would form a
daytime traffic/train candidate plus a night ambient-control pair without
changing the structure or sensor system. Confirm maintenance movements and the
actual train log before labelling the night interval a null.

No anonymous endpoint, Zenodo/Figshare/Mendeley record, thesis supplement or
indexed web-interface export was found. The raw records are reported to reside
with Metro Istanbul; VCE Vienna Consulting Engineers designed/delivered the SHM
system. The most targeted request is therefore to Metro Istanbul and the paper
authors/VCE, quoting the Capella product ID and asking for the raw acceleration,
GPS/displacement, sensor map, sampling rates, axes, units and clock uncertainty
for that UTC window. A 45-second or five-minute extract is sufficient; the full
monitoring archive is unnecessary. Also request
`2023-09-07 22:38:35--22:40:05 UTC` for the night comparison.

Eray Temur's 2019 ITU master's thesis adds an important audit trail but not a
downloadable waveform. It states that the permanent system comprised 32
accelerometers, four meteorological, 12 temperature, five tilt, four
displacement and four GPS sensors, with raw data saved on a Metro Istanbul
server and sent to VCE. Appendix B identifies two actual comparison records:

- a VCE/permanent-system record measured **10 October 2018 at approximately
  17:00 local**, plotted for about 300 seconds; and
- a seven-channel ITU Advanced Structural Systems field measurement made
  **25 April 2019 at approximately 17:00 local**, sampled at **200 Hz** over
  roughly 200 seconds, with sensors 15 m apart on the main span/station deck.

The thesis contains plots only (`pdfdetach` reports no embedded files), and its
2018/2019 records cannot truth the 2023 Capella acquisition. They are nevertheless
specific legacy dates to mention in a request: obtaining either record would
provide an independent real-bridge algorithm fixture, while the requested 2023
export remains necessary for strict SAR/SHM synchronization.

The 2024 state-of-the-art review supplied separately does **not** describe this
Golden Horn bridge. Its Istanbul examples are the First Bosphorus/15 July
Martyrs and Second Bosphorus/Fatih Sultan Mehmet suspension bridges; its 2019
Silivri-earthquake data were supplied by the Turkish General Directorate of
Highways. The ETH repository exposes the paper but no related dataset, so it is
background evidence rather than a new Golden Horn ground-truth source.

Existing SAR-only runs do not replace that truth. The on-bridge searches report
consensus peaks near 0.086, 0.171 and 0.563 Hz depending on aperture overlap,
but every configuration fails the persistent-scatterer precondition
(`D_A` best 0.391--0.516 versus the required 0.25, zero qualifying windows).
Those frequencies must not be claimed as bridge motion until a valid scatterer
selection and the SHM trace agree.

- Permanent SHM paper:
  <https://doi.org/10.3233/BRS-180134>
- Public paper copy and sensor layout:
  <https://www.researchgate.net/publication/268693421_Structural_health_monitoring_of_Golden_Horn_Metro_Crossing_Bridge_in_Istanbul_-_initial_assessment_and_permanent_monitoring>
- ITU thesis confirming continuous records are held at the Metro Istanbul
  operating centre:
  <https://polen.itu.edu.tr/entities/publication/ef2681c9-2abb-405b-a448-a370309f5293>
- Direct public copy of Temur's 2019 thesis, including the dated Appendix B
  comparison records:
  <https://web.itu.edu.tr/bariserkus/pdf/Temur2019_MSThesis.pdf>
- 2024 Istanbul Bosphorus-bridge review (different bridges; no linked data):
  <https://doi.org/10.1007/s10518-023-01819-3>
- Direct Capella CPHD:
  <https://capella-open-data.s3.us-west-2.amazonaws.com/data/2023/3/21/CAPELLA_C09_SP_CPHD_HH_20230321101754_20230321101819/CAPELLA_C09_SP_CPHD_HH_20230321101754_20230321101819.cphd>

### Smaller field-bridge downloads

- Hardanger Bridge, Norway: NTNU's long-term archive is the strongest candidate
  found for systematic time matching. Twenty triaxial accelerometers and nine
  anemometers have monitored the 1310 m suspension span since 2013. Raw files
  are GPS-synchronised, named with their UTC start, and each covers 30 minutes;
  processed MATLAB/HDF5 versions are also published at 200 Hz and 2 Hz. The
  released archive spans roughly seven years and emphasizes high-wind events,
  including signals around the bridge's very low 0.05 Hz first mode. NTNU moved
  the files from the retired BIRD service to NVA/Sikt. The live NVA record has
  271 anonymous CC BY 4.0 downloads totalling about 1.82 TB: 58 monthly raw CSV
  archives (761.74 GB), and 58 monthly MATLAB/HDF5 archives at each of the
  original rate (200 Hz accelerometers and 32 Hz anemometers; 895.53 GB), 10 Hz
  (81.38 GB), and 2 Hz (16.77 GB). Another 34 archives isolate storms, low-wind
  intervals, maximum wind, and maximum drag, lift, and pitch response. The
  archive also contains a README and the data-paper preprint. NVA's public JSON
  record lists every filename, byte size, licence, and file identifier:
  <https://api.nva.unit.no/publication/01977e2c97f8-09ea4ff5-fd74-48c0-bd99-790e3d8c19ac>
  (request `Accept: application/json`). For file identifier `<file-id>`, obtain
  the temporary S3 URL and stable alias from
  `/publication/01977e2c97f8-09ea4ff5-fd74-48c0-bd99-790e3d8c19ac/filelink/<file-id>`.
  Useful small files are the
  [README](https://api.nva.unit.no/publication/file/019fcce5d8c9-691178a0-adf2-4454-9a86-fd0705d2a6cc)
  and [data-paper preprint](https://api.nva.unit.no/publication/file/019fcce5d774-69aa9b6a-bc79-4deb-aadf-68becb439251).
  Dataset landing pages:
  <https://www.ntnu.edu/kt/open-data> and
  <https://nva.sikt.no/registration/01977e2c97f8-09ea4ff5-fd74-48c0-bd99-790e3d8c19ac>.
  Indexing the ZIP central directories gives 7,877 distinct ground-record start
  times. Intersecting their approximately 31-minute intervals with every
  Copernicus Sentinel-1 IW footprint containing the bridge from November 2014
  through April 2020 finds 99 Level-0 RAW products overlapping 73 ground
  records. A separate SLC search finds 89 product matches, representing 87
  acquisition times and 70 ground records. Two particularly useful high-wind
  pairs are:

  - **Storm Tor, 29 January 2016.** Ground record
    `HB141M-2016-01-29_17-00-00` contains the ascending Sentinel-1A pass. The
    1.74 GB Level-0 product runs from 17:18:29 to 17:19:01 UTC:
    `S1A_IW_RAW__0SDV_20160129T171829_20160129T171901_009712_00E2DB_24DA`.
    Its 4.50 GB, VV+VH, relative-orbit-15, frame-192 SLC derivative runs from
    17:18:31 to 17:18:58 UTC:
    `S1A_IW_SLC__1SDV_20160129T171831_20160129T171858_009712_00E2DB_7036`.
    Downloads and records:
    [Copernicus RAW metadata](https://catalogue.dataspace.copernicus.eu/odata/v1/Products(1bcd6326-e451-57af-9800-80950749ad84)),
    [Copernicus RAW download](https://catalogue.dataspace.copernicus.eu/odata/v1/Products(1bcd6326-e451-57af-9800-80950749ad84)/$value),
    [ASF SLC ZIP](https://datapool.asf.alaska.edu/SLC/SA/S1A_IW_SLC__1SDV_20160129T171831_20160129T171858_009712_00E2DB_7036.zip),
    [ASF metadata/search response](https://api.daac.asf.alaska.edu/services/search/param?granule_list=S1A_IW_SLC__1SDV_20160129T171831_20160129T171858_009712_00E2DB_7036&output=geojson),
    [200 Hz event ground data, 8.63 GB](https://api.nva.unit.no/publication/file/019fccf2fa1e-985677d0-edec-4c41-9337-2c457c3c8518),
    and [2 Hz event ground data, 153 MB](https://api.nva.unit.no/publication/file/019fccf1d476-a92de00c-95e5-4f25-bfad-17ae203951d1).
  - **Storm Urd, 26 December 2016.** Ground record
    `HB141M-2016-12-26_16-34-38` contains the ascending Sentinel-1B pass. The
    1.63 GB Level-0 product runs from 17:01:42 to 17:02:14 UTC:
    `S1B_IW_RAW__0SDV_20161226T170142_20161226T170214_003570_0061B6_8C50`.
    Its 4.70 GB, VV+VH, relative-orbit-44, frame-195 SLC derivative runs from
    17:01:44 to 17:02:12 UTC:
    `S1B_IW_SLC__1SDV_20161226T170144_20161226T170212_003570_0061B6_9C2E`.
    Downloads and records:
    [Copernicus RAW metadata](https://catalogue.dataspace.copernicus.eu/odata/v1/Products(4693cd59-46aa-52bd-9c48-3f3f8d545a78)),
    [Copernicus RAW download](https://catalogue.dataspace.copernicus.eu/odata/v1/Products(4693cd59-46aa-52bd-9c48-3f3f8d545a78)/$value),
    [ASF SLC ZIP](https://datapool.asf.alaska.edu/SLC/SB/S1B_IW_SLC__1SDV_20161226T170144_20161226T170212_003570_0061B6_9C2E.zip),
    [ASF metadata/search response](https://api.daac.asf.alaska.edu/services/search/param?granule_list=S1B_IW_SLC__1SDV_20161226T170144_20161226T170212_003570_0061B6_9C2E&output=geojson),
    [200 Hz event ground data, 3.29 GB](https://api.nva.unit.no/publication/file/019fccf2f8e7-aa344cdf-95ff-437f-bc08-b9c8c3772e05),
    and [2 Hz event ground data, 49.5 MB](https://api.nva.unit.no/publication/file/019fccf1d2cd-9a308882-cfda-4fa5-b6d2-f34b3fe0d765).

  The Level-0 products make these real synchronized public phase-history pairs,
  but Sentinel-1 IW is a TOPS wide-swath mode rather than a long spotlight
  dwell. Its relatively short target illumination and coarse bridge sampling
  make it a lower-sensitivity micro-motion test than the Umbra and ICEYE
  candidates. It is nevertheless the first pair here that can be assembled
  without author or commercial-provider approval, and its storm records provide
  a known high-response condition. An ASF search found no ALOS-2 product at the
  bridge during the released ground-data period.
- Vänersborg railway bascule bridge, Sweden: a 551.7 MB ZIP with 64 raw bridge
  opening events before, during and after a verified truss fracture. The files
  contain five accelerometers together with strain, inclination and weather
  channels at 200 Hz. Crucially, every filename records the UTC time of its
  first sample. The bridge identity and location are public, making this a real
  candidate for an archived-satellite coincidence check rather than only a
  waveform benchmark. The records concern bridge openings, however, so a
  satellite pass must overlap one of those short events to form a pairing.
  An ASF catalogue check found Sentinel-1 IW SLC coverage at 05:24 UTC on
  20 February, 05:32 and 16:46 UTC on 25 February, 16:54 UTC on 2 March,
  05:24 UTC on 4 March, and 05:32 and 16:46 UTC on 9 March 2023. None matches
  a published event start. The closest is the 25 February 16:46:06 pass,
  9 min 59 s after the 16:36:07 bridge-opening record began and before the
  next record at 17:05:27. Direct extraction of the relevant CSV shows that the
  16:36:07 record ends at 16:42:33 UTC. It therefore misses the satellite pass
  by 3 min 33 s and is **not** a synchronous pairing. Sentinel-1 IW would in any
  event not be equivalent to a long spotlight/dwell collection.
  Authors: John Leander, Jacob Nyman, Raid Karoumi, Peter Rosengren and Gunnar
  Johansson: <https://zenodo.org/records/8300495>.
- openLAB continuous monitoring, Bautzen: a 1.01 GB ZIP covering 1 February to
  31 October 2024 on the full-scale research bridge. Its data paper explicitly
  defines the ISO 8601 timestamps as **UTC**. The archive contains 523 triggered
  acceleration files sampled at 500 Hz, structural and environmental channels,
  regular ten-minute records and non-damaging vehicle load tests. It is therefore
  substantially more matchable than elapsed-time-only archives:
  <https://opara.zih.tu-dresden.de/items/6653124a-8659-40b8-817e-51250639c95b>.
  A separate 37.4 GB construction-monitoring release provides 200 Hz daily CSV
  files through 11 March 2024. Its full-day 10 February file contains samples
  throughout the 05:17:39--05:18:12 UTC Sentinel-1A Level-0 RAW acquisition
  (`S1A_IW_RAW__0SDV_20240210T051739_20240210T051812_052492_06593E_8184`).
  This proves simultaneous public sensor and SAR coverage, but the documentation
  specifies neither the CSV time zone nor an NTP/GPS clock source. Treat it as a
  **candidate with uncertified clock alignment**, not genuine synchronized
  ground truth, until the authors confirm the time basis:
  <https://zenodo.org/records/12735389>.
- Mensa Bridge, Hannover: Ralf Herrmann's CC0 download contains seven raw CSV
  recordings (585.4 MB total) from 24 April 2018: one ambient record and six
  modal-hammer tests. The ambient file exposes ten simultaneous deck channels,
  seven vertical and three horizontal, in m/s². Its measured time increment is
  approximately 0.0005 s (about 2 kHz). The date is retained in every filename,
  but the CSV begins at elapsed time zero and the public metadata gives no
  acquisition clock time. A same-day satellite match may be investigated, but
  exact temporal coincidence cannot be claimed from these files alone:
  <https://data.uni-hannover.de/dataset/reference-vibration-measurements-of-mensa-bridge-hannover>.
- Edinburgh pedestrian/cycle bridge: two directly downloadable ZIP archives
  (about 10.6 and 3.6 MB compressed) with raw CSV acceleration responses.
  Phase 1 contains four bridge sensors under ambient excitation and controlled
  heel-drop impacts at several positions. Phase 2 contains simultaneous bridge
  and bicycle responses for freewheeling and two pedalling cadences, plus a
  three-axis ambient record. The samples are spaced by 0.001953125 s (512 Hz).
  The CSV time axes restart at zero and contain no calendar timestamp, so the
  archive cannot be matched to a satellite pass without additional information.
  Authors: Richard May, Hwa Kian Chai, Thomas Reynolds and Yong Lu:
  <https://datashare.ed.ac.uk/handle/10283/8908>.
- Munich full-scale test bridge: an unofficial GitHub repository contains
  directly downloadable ZIP-compressed CSV measurements, not just processing
  code. The included sample covers a 11 April 2022 load/settlement test at
  100 Hz, with acceleration, strain, force and inclination channels. It includes
  reference-state files, controlled support-lowering runs, support-load runs and
  two ambient-vibration files of about 10.8 and 12.0 MB. The public sample is
  only a small part of a stated 3 TB archive available from the authors on
  request. Authors: Yusuf Jaelani, Alexander Klemm, Johannes Wimmer and
  collaborators. This is measured structural ground truth with known loading,
  but has no satellite-pass pairing:
  <https://github.com/imcs-compsim/munich-bridge-data>.
- Japanese RC bridge traffic-vibration archive: 293 MB of directly
  downloadable Excel files, including raw triaxial acceleration for 17–18 May
  and 19 July 2023 plus labelled pedestrian, car and two railway classes.
  Authors are Kouichi Takeya, Yuichi Ito and Eiichi Sasaki. The README says the
  two sensors sit on ground covers north and south of the 10 m bridge, so this
  measures bridge/ground vibration propagation rather than a spatial array on
  the deck. Absolute event centre times are retained. The public record does
  not identify the bridge location, preventing a defensible satellite
  coincidence search without contacting the authors:
  <https://doi.org/10.50915/data.jsceiii.24603012.v1>.
- Route 345 Bridge: 906.5 MB HDF5 with dual-axis acceleration and strain before
  and after mechanically imposed damage:
  <https://zenodo.org/records/13737790>.
- Steel truss bridge under traffic: eight small text files, five uniaxial
  accelerometers at 200 Hz:
  <https://data.mendeley.com/datasets/d3by55pjh7/1>.
- Medium and small bridges: raw spreadsheet recordings at 50--200 Hz, including
  paired smartphone and permanent-monitoring-system measurements at some sites:
  <https://data.mendeley.com/datasets/y4jgkwgsyf/1>.
- Scour & Shake: 136.6 MB ZIP of controlled 0.01 g white-noise shake-table tests
  on bridge-pier models with fixed-base and several scour states:
  <https://zenodo.org/records/15229606>.

Of these, the model-bridge displacement channels and openLAB shaker sweep are
the first two worth adapting into regression fixtures. They exercise the
estimator with measured motion while retaining either direct displacement or a
known excitation channel. They must be labelled waveform-injection tests, not
SAR validation.

## Other independent spaceborne author lines screened

- Vera Costantini, Bernardino Chiaia, Marco Civera, Alberto Ciavattone,
  Davide Ambrosio, Carlo Ranalletta Felluga, Emanuele Del Monte, Roberta Marini
  and Paolo Mazzanti report the ESA-funded ISABHEL project on Turin's Amedeo
  VIII and Regina Margherita bridges. It combines COSMO-SkyMed InSAR,
  photomonitoring and contact sensors. The important qualification is temporal:
  the paper presents an earlier multitemporal InSAR analysis, while installation
  posts say contact-sensor collection was only beginning in 2025. Dynamic
  accelerometers are principally used on Regina Margherita; Amedeo VIII is
  principally a static-monitoring site. No public raw sensor or SAR files were
  located, and the material does not establish synchronous single-pass
  micro-motion ground truth:
  <https://iris.polito.it/handle/11583/3005856> and
  <https://business.esa.int/projects/isabhel>.
- Filippo Biondi, Pia Addabbo, Silvia Liberata Ullo and Danilo Orlando use
  COSMO-SkyMed for the Mosul Dam and Morandi Bridge. Public repositories expose
  the papers, not the SAR products or vibration waveforms. Validation is against
  PSI/GNSS trends, cracks or photographs rather than synchronous accelerometers,
  so these are not ground-truth vibration datasets.
- Adrian Focsa, Andrei Anghel, Giovanni Nico and collaborators compare
  spaceborne micro-Doppler bridge frequencies with ground-based radar. The
  measurements are asynchronous, and no downloadable paired files were found.
- Qian Wang, Michael Pepin, Robert Beach, Robert Dunkel, Timothy Atwood,
  Balu Santhanam, Walter Gerstle, Armin Doerry and Majeed Hayat publish SAR
  vibration-estimation experiments, but searches locate papers only. These are
  older airborne/experimental SAR studies rather than downloadable spaceborne
  structure-plus-accelerometer packages.
- Felice Carlo Ponzo, Chiara Iacovino, Rocco Ditommaso, Manuela Bonano,
  Riccardo Lanari and collaborators combine COSMO-SkyMed SBAS products with
  on-site vibration measurements at Rome's Ponte della Musica. The remote and
  ground measurements are not simultaneous: 236 Stripmap SLCs cover March
  2011–March 2019, while four 900 s velocimeter tests were collected in October
  2020; a later campaign used six synchronized triaxial accelerometers in
  November 2021. The article makes data available only on author request. This
  is useful structural context but cannot validate single-pass micro-motion.

## Adjacent micro-motion benchmarks

These datasets are downloadable and contain periodic motion, but they are not
measurements of vibrating structures by a SAR satellite.

### ISAR Micro-Motion Dataset (IMD)

**Status:** Anonymous Figshare downloads plus open construction code; entirely
simulated.

- Fully polarimetric 9--11 GHz electromagnetic scattering fields for four
  conical targets, sampled in 10 MHz frequency and 0.01-degree aspect steps.
- Two-second aspect-angle sequences with known precession, nutation and wobble
  parameters can be combined with the complex fields to generate radar echoes.
- Useful for checking spectral estimators against exact periodic-motion truth
  and for adversarial/simulation experiments.
- Not SAR platform data, not a structure, and not independent experimental
  ground truth. It must not support a real-scene sensitivity claim.

Data:

- Aspect-angle sequences: <https://doi.org/10.6084/m9.figshare.27247074.v2>
- Complex scattering fields: <https://doi.org/10.6084/m9.figshare.27266262.v1>
- Construction code: <https://github.com/NilasZ/MRM>
- Dataset paper: <https://doi.org/10.1038/s41597-025-05193-4>

### Open Radar Initiative outdoor-moving-object dataset

**Status:** Public download, CC BY-NC 4.0; stationary surveillance radar rather
than SAR.

The release contains full tracks of measured Doppler spectra for people,
bicycles, UAVs and vehicles, with timestamps, measured range, azimuth, radial
velocity, SNR and filtered position. It is useful for testing peak persistence,
background rejection and classification on measured micro-Doppler. Because the
sensor does not synthesize an aperture and the motions are not millimetric
structural vibration, it cannot validate the project's SAR measurement chain.

- Dataset and loader:
  <https://github.com/openradarinitiative/open_radar_datasets>

### OpenSARShip with AIS motion labels

**Status:** Public downloads; spaceborne Sentinel-1 SLC chips with AIS metadata,
but no aperture history.

OpenSARShip contains ship chips from Sentinel-1 IW SLC and GRD products. The
original SLC chips store real and imaginary values for VV and VH, while the
associated AIS metadata can include timestamp, position, speed over ground,
heading, length and breadth. This makes it a rare open combination of complex
spaceborne SAR and independently reported target motion.

It still cannot drive this project's estimator directly: the 2.7--3.5 m by
22 m SLC resolution is coarse, and the distributed files are cropped focused
chips rather than the pulse or full-aperture history required to construct a
sub-aperture time series. AIS also measures whole-vessel translation, not local
vibration. It is useful for studying whether motion-labelled ships exhibit
different focused-image artefacts, not for validating vibration frequency.

- Download page: <https://opensar.sjtu.edu.cn/DataAndCodes.html>
- Dataset paper: <https://doi.org/10.1109/JSTARS.2017.2755672>

### Ports and instrumented moored vessels

#### Panama Canal Capella stripmap CPHD

**Status:** Public genuine CPHD containing a lock-transiting vessel; operational
context is obtainable, but no synchronized onboard motion waveform is known.

Capella's Panama Canal release is
`CAPELLA_C02_SM_CPHD_HH_20210211192550_20210211192602`, acquired 11 February
2021 at 19:25:50--19:26:02 UTC.  The 3,120,732,928-byte HH file is real CPHD
1.0.1: 121808 vectors by 6341 samples, 9.65 GHz centre frequency, 181 MHz saved
bandwidth, 39.3-degree incidence, and a 71.8 by 5.2 km image grid.  The footprint
(79.959--79.488 W, 8.858--9.370 N) spans much of the canal, and Capella's
gallery explicitly describes a boat being guided through a lock.

This corrects an important search-scope trap: the earlier 707-scene survey only
ranked **spotlight** CPHD, so it could not find this `SM` stripmap acquisition.
The phase history is directly downloadable and the existing Capella reader can
parse its header, PVP layout and CI4 signal format.  Scientific compatibility is
not established merely by readability: in stripmap, each target's effective
illumination interval is shorter and location-dependent, while the method and
geometry implementation were validated for spotlight.  A ship moving through a
lock also adds bulk translation and rotation that can dominate vibration phase.

The Panama Canal Authority now exposes authenticated historical arrival, transit
count and estimated-transit-time endpoints.  Those can identify operational
context and possibly narrow the vessel, but they do not provide onboard heave,
roll or acceleration.  Fifteen-minute historical tide readings are likewise too
slow for the estimator.  Archived lock webcam frames or a vessel log at the
exact UTC interval would improve object identification, not provide millimetric
motion truth.

The delivered clocks are internally inconsistent at the precision this project
needs.  The STAC timestamp is 19:25:56.5278 UTC and the product identifier
implies 19:25:50--19:26:02, but the embedded XML declares a collection start of
19:26:02 followed by 11.9286 seconds of transmit time.  That approximately
one-dwell offset may be a producer convention or metadata defect; it must be
resolved before aligning an external sensor or webcam record to individual
pulses.

- Capella gallery: <https://www.capellaspace.com/gallery/panama-canal>
- Direct CPHD: <https://capella-open-data.s3.amazonaws.com/data/2021/2/11/CAPELLA_C02_SM_CPHD_HH_20210211192550_20210211192602/CAPELLA_C02_SM_CPHD_HH_20210211192550_20210211192602.cphd>
- Panama Canal developer portal: <https://developer.pancanal.com/>
- Historical tide record: <https://catalog.data.gov/dataset/tide-elevation-data-from-tide-gauges-in-the-panama-canal-from-1980-01-01-to-1998-12-31-ncei-acc>

#### TigerRAY 2024 moored deployment, Washington

**Status:** Public raw daily MATLAB files with UTC timestamps; no public CPHD
overlap found.

The University of Washington's TigerRAY wave-energy converter was moored at
47.679517 N, 122.230500 W from 10 January through 3 March 2024.  The release is
unusually useful for synchronization: it contains 100 Hz nacelle and heave-plate
IMU timestamps, 100 Hz load-cell/pressure/electronics channels, 40 Hz encoders,
and a 40 Hz-timestamped satellite compass.  IMU heave is 50 Hz.  The deployment
guide reports POSIX UTC timestamps and warns that the heave-plate IMU's internal
clock is not GPS-aided; use the acquisition-system timestamp rather than that
internal field.

The surface assembly is a real, radar-visible moored target, although much
smaller than a ship: two 2.8--3.0 m floats and a 2.6 m by 0.98 m nacelle/mast,
with a 1.43 tonne total surface body.  Measured waves occupy roughly 0.2--1 Hz,
below the current full-dwell estimator band but potentially usable with a longer
dwell or modified aperture partition.  The 707-product Capella spotlight CPHD
survey has no nearby scene (nearest centre about 209 km away).  The Umbra scan
finds three March 2025 footprints near the mooring, but each stops roughly 2--6
km west of TigerRAY and all were collected a year after the instrumented
deployment.  They are geographic controls, not synchronized pairs.

- Public record: <https://catalog.data.gov/dataset/tigerray-moored-deployment-data>
- Files and data guide: <https://mhkdr.openei.org/submissions/614>

#### Punta Langosteira instrumented cargo ships, A Coruna

**Status:** Public field measurements and exact dates, but only hourly movement
statistics are released; no Capella or Umbra CPHD coverage.

Two Zenodo releases describe 66 moored cargo ships measured at the Outer Port
of Punta Langosteira, Spain (approximately 43.3577 N, 8.3871 W), during
2015--2022.  The field system synchronized IMUs for roll/pitch, laser distance
meters for sway/yaw, and cameras for surge/heave.  This is genuine ship motion,
but the downloadable tables contain hourly mean, significant and maximum motion
parameters rather than the raw 3--30 second waveforms needed by the estimator.
The complete Capella spotlight survey has no centre within 0.2 degrees, and the
Umbra 2024--2025 site scan has no scene at the port.  It remains a plausible
Sentinel-1 Level-0 cross-match source if the archived dates are screened.

- 2015--2020 release: <https://zenodo.org/records/5113707>
- 2021--2022 release: <https://zenodo.org/records/10869119>

#### Historical AIS for port-scene screening

NOAA publishes downloadable annual AIS archives for US coastal waters.  AIS can
establish vessel presence, location, heading and speed during a SAR collect, so
it is useful for finding stationary or manoeuvring ships inside a public CPHD
footprint.  Its nominally minute-filtered reports do not measure heave, roll or
structural vibration and therefore are candidate-selection metadata, not motion
ground truth.

- NOAA historical AIS: <https://coast.noaa.gov/htdata/CMSP/AISDataHandler/>

#### Shipborne multi-antenna GNSS/IMU heave dataset

**Status:** Public CC BY 4.0 raw data; acquisition location and absolute dates
are not stated in the public record, preventing a SAR catalogue cross-match.

This 2026 Mendeley release is a strong standalone motion benchmark: four rover
GNSS antennas and a base station, synchronized IMU measurements, and POS MV
TrueHeave references cover calm water, artificial wakes, and open-sea swell.
The GNSS/IMU clocks are GPST while TrueHeave is UTC/UTCT, and the package
includes explicit time-system and lever-arm documentation.  Until coordinates
and acquisition dates are recovered from the archive or authors, it cannot be
paired honestly with spaceborne imagery.

- Dataset: <https://data.mendeley.com/datasets/wv5bwx694m/1>

## Historical experimental data located but not downloadable

The 2003--2006 APY-6 experiment is a particularly relevant predecessor. An
airborne X-band SAR at 9.6 GHz and 1 kHz PRF observed two mechanically vibrating
corner reflectors. One had 2 Hz single-tone ground truth; the radar estimate was
2.3 Hz. A later analysis estimated 1.5 mm amplitude against an 8 mm ground-truth
amplitude. The second reflector used multiple vibration rates. Many later papers
reuse figures and results from this phase history, but a deep search found no
public data record, supplementary file, acquisition identifier or download
endpoint. The radar was a US Navy/Northrop Grumman AN/APY-6, so this is unlikely
to appear in ordinary civilian SAR archives.

Likewise, the FGAN MEMPHIS 94 GHz airborne experiments contain real vehicle
vibration and rotating-dish examples, but only the article is public. No raw
MEMPHIS measurement package was found in the University of Zurich, ETH, DLR,
Zenodo or general research-data indexes.

## Public on-board ship-motion sources

These are genuine measurements made on radar-visible ships, rather than AIS,
weather or wave-height proxies. The SA Agulhas II case below is now confirmed
simultaneous onboard-motion plus SAR, but its anonymously downloadable radar
files are detected quicklooks rather than phase history. It is therefore a real
validation campaign lead, not yet a native `mmotion --cphd` benchmark.

### SA Agulhas II — exact onboard IMU/SAR overlap found

The SCALE Winter Cruise 2022 motion package is the strongest public on-board
candidate for the current estimator. It contains measured and rigid-body-derived
translational acceleration, angular velocity and angular acceleration from a
six-accelerometer array on the 134 m polar research and supply vessel *SA
Agulhas II*.

- Cruise: 11--31 July 2022, Cape Town to the Southern Ocean marginal ice zone
  along the Good Hope Line, returning to Cape Town.
- Public archive: one 1,173,659,641-byte ZIP plus a 1 MB metadata PDF, CC BY
  4.0. The ZIP is small enough for the external SSD.
- Files are consecutive five-minute MATLAB batches. Run directories are named
  `YYYYMMDD_HHmmss_Run`, explicitly in UTC.
- The released channels are strictly 9.99 Hz (decimated from 2048 Hz), 2,998
  samples per complete batch. The documented processing band is 0.08--0.63 Hz.
  This is a real observable band for `mmotion`, unlike ordinary 1 Hz ship logs.
- The motion ZIP does not contain position, but a separate anonymous Zenodo KMZ
  provides the full track, UTC ship positions at the SAR times, the exact
  TerraSAR-X/TanDEM-X and COSMO-SkyMed product identifiers and georeferenced
  quicklooks. Its checksum was verified as
  `md5:ac99ce6c215e41447174783d5a4df980`.

The track/footprint and ZIP-directory check confirms four useful overlaps:

| SAR interval (UTC) | mission/product | ship position | released IMU batch |
|---|---|---|---|
| 2022-07-21 19:16:28--19:16:50 | TerraSAR-X ScanSAR `TSX1_SAR__MGD_RE___SC_S_SRA_20220721T191628_20220721T191650` | 58.40127 S, 0.64935 W | `20220721_180725_Run_a0191231.mat.mat` |
| 2022-07-22 07:02:21--07:02:37 | COSMO-SkyMed `CSKS1_DGM_B_WR_01_VV_RA_SF_20220722070221_20220722070237` | 59.03135 S, 0.84183 W | straddles `...a1065731` and `...a1070231` |
| 2022-07-23 16:35:45--16:36:00 | COSMO Second Generation `CSG_SSAR1_DGM_B_0301_SC1_001_VV_RD_F_20220723163545_20220723163600` | 59.37862 S, 0.16197 E | `20220723_080426_Run_a0163432.mat.mat` |
| 2022-07-24 16:17:50--16:18:05 | COSMO-SkyMed `CSKS2_DGM_B_WR_02_VV_RD_SF_20220724161750_20220724161805` | 58.71725 S, 0.67825 E | `20220724_101217_Run_a0161723.mat.mat` |

The ship position is inside each corresponding quicklook footprint, and each
9.99 Hz IMU file brackets the stated SAR interval. This is stronger than an AIS
coincidence: it is a deliberately coordinated cruise with onboard motion
instrumentation and named radar acquisitions.

It is not immediately consumable by this repository. `MGD`/`DGM` and the PNG
quicklooks are detected products; the KMZ description says that the imagery was
formed from complex data, but that complex source is not included. The exact
identifiers should be submitted to DLR/ASI or searched in ESA's replacement TPM
service when TerraSAR-X and COSMO-SkyMed access returns. Request SSC/SLC/SICD or
raw/phase-history data, not another detected image. Stripmap/ScanSAR target
illumination will also be shorter than the full product intervals.

The clean 1.1 GB motion ZIP passed `unzip -tq` and is stored externally at
`/Volumes/ZX20 II/micromotion/datasets/sa-agulhas-ii-scale-win22/`. The five
small overlapping MATLAB files were extracted under `sar-overlap-mat/`; the
failed concurrent-resume duplicate was removed after verification.

Sources:

- Dataset and direct download metadata:
  <https://scholardata.sun.ac.za/articles/dataset/Measured_and_computed_IMU_readings_on_the_SA_Agulhas_II_-_SCALE_Winter_Cruise_2022/26369752>
- Cruise dates and route:
  <https://www.sanap.ac.za/scale-winter-cruise-2022>
- Public track, SAR positions, product identifiers and quicklooks:
  <https://zenodo.org/records/7902992>
- Cruise report DOI: <https://doi.org/10.15493/DEA.MIMS.09852023>

### NOAA Okeanos Explorer SCS — best scalable space/time join

NOAA/NCEI exposes public Shipboard Computing System NetCDF trajectories for the
*Okeanos Explorer*. These combine the necessary join keys and motion channels in
one file: UTC `time`, latitude/longitude and DGPS, plus POS MV pitch, roll,
heading and heave. For example, `EX1707_SCS.nc` has 1,967,915 observations from
2017-08-08 23:07:59 through 2017-08-31 19:04:53 UTC, with roll spanning
-8.9--8.58 degrees, pitch -3.92--5.15 degrees and heave -2.01--2.22 m.

The catalogue contains the same `*_SCS.nc` structure throughout modern SAR
years: seven cruises in 2021, seven in 2022 and nine in 2023 were directly
verified to contain `POSMV-SHR-HEAVE`; files continue through `EX2501`. This is
therefore a much better *catalogue-crossmatch source* than searching individual
papers. Use OPeNDAP to request only time, position and motion columns before
considering the full files: the example full NetCDF is 1,263,713,900 bytes.

The important limitation is cadence. The example is approximately one record
per second, so its 0.5 Hz Nyquist limit cannot validate the 1.5--3.8 Hz bands
used by the short-aperture examples. It can validate lower-frequency ship heave
only if the SAR dwell/aperture configuration admits that band. The SCS value at
SAR time is also a ship-wide attitude/heave measurement; projection to a chosen
scatterer needs the POS MV reference point, lever arm and radar line of sight.

- THREDDS catalogue:
  <https://www.ncei.noaa.gov/thredds-coastal/catalog/ocean_exploration_research/catalog.html>
- Example OPeNDAP metadata:
  <https://www.ncei.noaa.gov/thredds-coastal/dodsC/ocean_exploration_research/EX1707_SCS.nc.html>
- Example direct NetCDF:
  <https://www.ncei.noaa.gov/thredds-coastal/fileServer/ocean_exploration_research/EX1707_SCS.nc>

### Akademik Tryoshnikov ACE — global track plus IMU at 1 Hz

The Australian Antarctic Data Centre openly releases raw GPS, meteorology and
ship IMU measurements from the Antarctic Circumnavigation Expedition aboard
R/V *Akademik Tryoshnikov*. It is explicitly a 1 Hz time series covering
2017-01-22 through 2017-03-18, approximately 50--69 degrees south around the
full longitude range, under CC BY 4.0. Because GPS and motion are packaged
together, it is directly crossmatchable against historical SAR catalogues.
Like the NOAA stream, however, it cannot validate motion above 0.5 Hz.

- Metadata and download link:
  <https://data.aad.gov.au/metadata/records/fulldisplay/AAS_4434_ACE_GPS>
- Direct dataset download entry: <https://data.aad.gov.au/eds/4563/download>
- DOI: <https://doi.org/10.4225/15/5A178EF0E5156>

### Other vessel archives worth machine-screening

- The Irish Marine Institute research-vessel underway collection includes
  position, heave, pitch, roll, heading and speed, covers 2003 onward and is CC
  BY 4.0. A national inventory describes a ten-second interval, which makes it
  useful for finding the ship but not as vibration truth; a higher-rate native
  stream would need to be located separately.
- US hydrographic-survey raw Kongsberg/HYPACK packages frequently embed POS MV
  or Seapath attitude datagrams. These can supply higher-rate attitude plus
  GNSS and UTC, but require per-cruise parsing and often multi-gigabyte
  downloads. Screen cruise bounds and SAR timestamps before sending a package
  to the external SSD.
- Rolling Deck to Repository cruise holdings for large US research vessels
  (including *Sally Ride*) contain navigation and, on selected cruises, POS MV
  roll/pitch/heave. Availability and format vary by cruise; these should be
  catalogued as candidates, not assumed to contain motion.

The practical next join is therefore: enumerate NOAA `*_SCS.nc` and the SA
Agulhas II UTC run intervals; sample/obtain each vessel track; intersect each
track at acquisition time with Capella, Umbra and ICEYE footprints; then retain
only native CPHD (or explicitly plan a focused-complex reader). A spatial match
to a ship route without second-level temporal overlap is not a positive test.

## Airport CPHD and surface-motion cross-match

The complete Capella open-data CPHD collection was enumerated on 4 August 2026,
rather than searching gallery labels. Its **1,174 CPHD footprints** were
point-in-polygon intersected with all `large_airport` and `medium_airport`
records in OurAirports. This produced **111 scene-airport matches**. The local
reproducible scanner is `tools/capella_airport_crossmatch.py`; it consumes the
downloaded STAC item JSON and `airports.csv` and writes CSV.

The most useful exact pairing found is FAA SWIM/SMES airport-surface telemetry:
the open Amelia-42 project publishes 1 Hz UTC position reports for aircraft,
airport vehicles and unknown agents at 42 US airports, with latitude,
longitude, altitude, speed, heading and a geofence. Its documentation says raw
coverage runs from 2 December 2022 to the present and can be selected by exact
start/end time. Two Capella airports are in Amelia-42:

- **St Louis Lambert (KSTL), preferred:**
  `CAPELLA_C15_SS_CPHD_HH_20250316044755_20250316044811`, exact aperture
  2025-03-16 04:47:55.426--04:48:11.154 UTC, and
  `CAPELLA_C11_SM_CPHD_HH_20250319191515_20250319191532`,
  2025-03-19 19:15:14.551--19:15:31.555 UTC. The first 15.7 s sliding-spotlight
  preview visibly contains the complete airport; use it first. The second is a
  much larger strip-map footprint and is a useful independent repeat.
- **Boston Logan (KBOS):**
  `CAPELLA_C10_SS_CPHD_HH_20240927014213_20240927014227`, exact aperture
  2024-09-27 01:42:12.979--01:42:26.734 UTC. The preview visibly contains the
  airport, runways and terminal area.

These are genuine synchronized radar/ground-surveillance candidates, but they
validate bulk surface motion rather than millimetric vibration. Query the
trajectory source first and retain a scene only if a reported aircraft or
vehicle lies inside the radar footprint during the aperture. A moving aircraft
may migrate or smear beyond the present stationary-scatterer model; parked
aircraft and static apron/terminal pixels are required controls.

There is a current access caveat. Amelia's documented MinIO endpoint
`airlab-share-01.andrew.cmu.edu:9000`, bucket `ameliaswim`, timed out over both
HTTP and HTTPS during this check. The downloadable Amelia42-Mini mirror contains
only 15 sampled days per airport; its KSTL files are from April 2023 and do not
overlap the 2025 CPHD. Therefore the exact SWIM join is **promising but not yet
retrieved**. Try the server again or contact the Amelia maintainers before
downloading CPHD.

An independent fallback is the ODbL-licensed ADSB.lol daily historical archive.
It stores one gzip JSON trace per aircraft per day. Both KSTL dates were
downloaded as their original two-part tar archives to the external SSD and
streamed directly with `tools/adsblol_cphd_crossmatch.py` (no multi-gigabyte
extraction). The 16 March archive is **2,278,891,520 bytes**. It contains no
aircraft report in the CPHD footprint during the aperture or within a 60 s
margin, so that pairing is a negative/control candidate rather than proven
positive motion. A one-hour margin confirms that the daily archive is populated;
the nearest in-footprint report is N8930S at 04:49:53.480 UTC, about 102 s after
the aperture ended.

The 19 March archive is **2,652,497,920 bytes** and produces a genuine
synchronized positive result for
`CAPELLA_C11_SM_CPHD_HH_20250319191515_20250319191532`. Within its exact
2025-03-19 19:15:14.551--19:15:31.555 UTC aperture and polygon are **nine ADS-B
reports from two moving aircraft**: six from ICAO `a661ae`, registration N510CN
(about 4,375--4,600 ft and 135--141 kt), and three from ICAO `a97076`,
registration N707VM (about 2,875--3,000 ft and 242--246 kt). The first/last
in-aperture reports are 19:15:15.900 and 19:15:30.100 UTC. This is the best
immediate positive target found so far, although it validates detectable bulk
aircraft motion/smearing rather than structural micromotion. The reproducible
CSV is `/tmp/kstl-adsb-crossmatch-0319.csv`; copy it into a persistent run
directory before rebooting. ADS-B may omit airport ground vehicles and aircraft
whose transponders are disabled, whereas SWIM/SMES can label both aircraft and
vehicles.

Archive integrity recorded after download:

- 19 March `.tar.aa`: 2,000,000,000 bytes, SHA-256
  `e8f7f3f9e63df9719ed58edf3542cace49381c9b2128207842afa5ff8f9a809c`;
- 19 March `.tar.ab`: 652,497,920 bytes, SHA-256
  `f54f80c0ccb5315ee96ea57e47c00d98141b270d3e4c42e441b37ac2f143a9f2`.

Other notable true CPHD airport coverage includes 19 Van Nuys scenes, 13
Hamilton Island scenes, five Savannah scenes, three Tampa scenes, and scenes at
Santiago, Lima, Rio Santos Dumont, Kaohsiung, Salzburg, Juneau, Iqaluit and
Svalbard. Van Nuys is the richest SAR time series, but it is not in Amelia-42;
use ADSB.lol/OpenSky for it. The FAA Denver instrumented-runway database and
FAA pavement test-facility records provide real strain/displacement/pressure
waveforms, but their historical or test-track timing does not overlap the
Capella scenes and must not be presented as synchronized truth.

- Capella CPHD collection:
  <https://capella-open-data.s3.us-west-2.amazonaws.com/stac/capella-open-data-by-product-type/capella-open-data-cphd/collection.json>
- Amelia-42 coverage and schema: <https://ameliacmu.github.io/amelia-dataset/>
- Amelia downloader: <https://github.com/AmeliaCMU/AmeliaSWIM>
- ADSB.lol historical data: <https://www.adsb.lol/docs/open-data/historical/>
- OurAirports data: <https://ourairports.com/data/>

## What to request

A useful release needs more than a cropped image and a published spectrum. Ask
for:

- original SLC, SICD or CPHD data and all metadata;
- raw, unfiltered ground-sensor recordings;
- UTC timestamps, clock source and synchronization uncertainty;
- sensor locations, axes, calibration and units;
- target coordinates and radar line-of-sight vectors;
- processing masks or published pixel coordinates;
- uninjected or stationary targets in the same collect;
- permission to publish independent results and derived products.

For false-alarm work, also request every acquisition from the campaign, including
failed tests and scenes in which the ground sensors found no vibration. A release
containing only successful examples can validate frequency estimation but cannot
calibrate detection.

## Public-repository search

Searches of Zenodo, Figshare, Mendeley Data, the University of Strathclyde STAX
repository and general web data indexes found papers and theses but no paired
phase-preserving SAR and synchronized vibration package. Capella and Umbra have
open-data programmes, but none of the experimental collections above has been
identified in their public catalogues.

Direct catalogue checks made on 4 August 2026:

- Umbra's public catalogue has four scenes on 16 May 2024, but they cover
  Colombia, Panama, New Zealand and Indonesia—not Glasgow. It has no entry at
  all for 25 September 2024, the Galway Girl date.
- Public Umbra products indexed on 18 November 2023 include Umbra-05 scenes in
  North America, Colombia, Panama, Indonesia and Australia, but no product over
  Villa Gherta or Trento.
- Capella's public catalogue has one SLC/GEO pair on 18 December 2023, but its
  footprint is in Western Australia—not the Trento controlled target.
- Searches of Umbra's public object keys for South Portland, Glasgow, Celtic,
  Galway and Trento returned no matching task folders.
- ESA's Third Party Missions Online Dissemination Service exposes a searchable
  TerraSAR-X collection and requires Earth Online registration for downloads.
  The exact Trento product could not be resolved to a filename through its
  public web index, so the timestamp, location and published imaging parameters
  should be supplied to ESA or DLR in a product request.

The public-copy search was deliberately broader than official catalogues. Exact
acquisition timestamps, experiment coordinates, titles, author names and terms
such as `CPHD`, `LVDT`, `accelerometer`, `.mat`, `.csv` and `.zip` were checked
across GitHub and indexed source code, Zenodo, Figshare, Hugging Face,
Strathclyde's research-data catalogue, Google Drive/Dropbox web indexes and
general S3/object-key search. No downloadable mirror of the South Portland,
Trento, Glasgow Green, Galway Girl or Korean-bridge radar-plus-sensor files was
found. Public results labelled “micro-Doppler dataset” were ground-based CW/FMCW
radar, simulated signals, papers alone, or SAR imagery without synchronized
motion truth; none is a substitute for the required benchmark.

These checks rule out a coincidental match based only on date. They do not rule
out access through a commercial account, NASA CSDA, or a direct author request.

NASA's Commercial Satellite Data Acquisition programme is a second possible
route to the Umbra imagery. Its Satellite Data Explorer lets the public search
the catalogue, while downloads require an authorised, normally US-government-
funded user. Umbra holdings can include CPHD and SICD. This would recover only
the radar product; the accelerometer or actuator record would still have to come
from the experiment team.

- Umbra access through NASA CSDA:
  <https://science.nasa.gov/earth-science/csda/vendor-umbra/>
- Umbra public STAC catalogue:
  <https://s3.us-west-2.amazonaws.com/umbra-open-data-catalog/stac/catalog.json>
- Capella public STAC catalogue:
  <https://capella-open-data.s3.us-west-2.amazonaws.com/stac/catalog.json>
- ESA TerraSAR-X dissemination collection:
  <https://tpm-ds.eo.esa.int/oads/access/collection/TerraSAR-X/searchbyfilename>

## Additional project holdings

ESA's EO4Security programme says its teams also tested high-rise buildings and
stadiums and produced formal validation reports. Public pages name Celtic Park
but do not expose the remaining targets, acquisition identifiers or reports.
These should be requested as project deliverables under ESA contract
4000142272/23/I-DT, including unsuccessful and no-motion cases rather than only
the examples selected for publication.
