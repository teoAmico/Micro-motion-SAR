# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this is

Single-pass micro-motion SAR: one radar dwell is cut into sub-apertures, what moves
between them is tracked, and the tracked series becomes a per-window vibration
spectrum. No repeat pass, no interferometric stack. C11, CMake, no external
dependencies — the FFT (pocketfft) is vendored.

Read `README.md` and `docs/FOLLOW-UPS.md` before changing anything in the tracking or
spectrum stages. `FOLLOW-UPS.md` is the record of what has been tried and disproven,
including entries that withdraw earlier entries; none of it is recoverable from the
code, and several conclusions in it reversed after further measurement. It opens with
an index of all 68 items and their status.

`docs/CODE-REVIEW.md` is the companion: defects found by READING the code against its
own documentation rather than by measuring it, each with file:line and what a fix has
to do. It also logs which reviews have been run, at which commit, and what they did
not cover. Entries are removed when fixed; add a row to its "Reviews performed" table
when you run one.

## Build, test, run

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
ctest --test-dir build --output-on-failure          # 21 tests, ~3 min
```

```sh
ctest --test-dir build -R test_tracking --output-on-failure   # one test
./build/test_tracking                                          # or run it directly
cmake -S . -B build-asan -DCMAKE_BUILD_TYPE=ASAN && cmake --build build-asan
```

`ASAN` is a project-defined build type (`-fsanitize=address,undefined`). Changes to
the numerical core should pass under it as well as in Release.

Tests build their own fixtures, so `ctest` passes on a fresh clone with no data files.
`tests/rs_sim.h` is the shared synthetic-scene generator; `tests/rs_test.h` is a
20-line assertion harness (`RS_CASE`, `RS_CHECK_NEAR`, `RS_CHECK_OK`) plus
`rs_track_fit()`, which scores a frequency sweep by slope and rms rather than
comparing a single point.

Two build facts that bite: OpenMP is optional and silently absent on macOS unless
Homebrew's keg-only `libomp` is found (read the configure line, don't assume), and
`-ffast-math` is deliberately absent because reassociation and denormal flushing
perturb the sub-pixel correlation peaks and interferometric phase this project
measures. Do not add it.

## Naming

The tool and the library are named differently. The binary and CMake project are
`micromotion`, headers live under `include/resonarsat/`, every symbol is `rs_`-prefixed,
and some usage strings still print `resonarsat`. Treat `rs_`/`resonarsat` as the
library's identity and `micromotion` as the tool's.

## Layout and pipeline

`mm_core` is one static library; `src/main.c` (~2000 lines) is the whole CLI, four
subcommands dispatched at the bottom of the file:

- `info` — a product's geometry and timing
- `validate` — fourteen checks on whether a collect can support the measurement
  asked for, before any processing (`src/core/validate.c`, `validate.h`). Thirteen
  are arithmetic; `RS_VALIDATE_GROUND_TRUTH` always reports unknown and says why.
  Takes `--estimator`: four of the checks answer differently for the phase route and
  answering them for the correlator refused a valid Giza configuration (items 16, 17)
- `focus` — time-domain backprojection to an image (`src/core/focus.c`)
- `mmotion` — the measurement chain, and where nearly all the complexity is

The `mmotion` chain, in order:

1. **Read** — `src/readers/` handles CPHD (32-bit float and 16-bit int samples), SICD
   and UAVSAR into `rs_cphd_t` (phase history) or `rs_slc_t` (focused image).
2. **Sub-aperture** — `src/core/subaperture.c`. Two distinct routes, selected by
   `--subap` and dispatched in `rs_build_subaps()` in `main.c`:
   - `pulse` (default) — backproject each pulse window separately from phase history.
   - `paper` / `uniform` — focus the **full** aperture first, then split the image
     spectrally. `paper` is Biondi & Malanga's deliberately-band-limited sweep;
     `uniform` is a plain filter bank.
3. **Track** — `rs_microm_track()` in `src/core/microm.c`, per-window sub-pixel offset
   tracking over the stack. Two orthogonal choices: `rs_microm_ref_t` (which look each
   correlation is taken against: `first`, `adjacent`, `pair`, `lag`) and
   `rs_microm_estimator_t` (`correlation`, `phase`, `splitband`). The correlator itself
   is `src/core/coreg.c`; split-band phase linking is `src/core/phaselink.c`.
4. **Spectrum** — `src/core/spectrum.c`. `rs_spectrum_compute_opts()` per window, then
   three selection statistics side by side: `rs_spectrum_best_window()` (prominence, the
   reported answer), `rs_spectrum_consensus()` (which frequency the most windows
   agree on, how many distinct answers there are, and the largest 4-connected block of
   agreeing windows), and `rs_spectrum_ampcor_window()` (an ampcor-style cull on the
   correlation surfaces' own SNR and curvature plus a neighbourhood test, which is the
   only one reading evidence from the tracker rather than from the spectrum).

`src/core/ccd.c` is a separate scale-invariant change-detection locator over the same
stack — "where is something moving", not "at what frequency". `src/io/` writes PNG and
raster maps; `src/util/` holds geometry, geocoding, orbit and the thread-local error
buffer.

`tools/sarpy_crosscheck.py` compares this project's CPHD reader against SARPy on a real
vendor product, via `info --cphd --json`. It is the only check anywhere here that
exercises the CPHD parse — `sim_cphd` writes the project's own container, so the test
suite never touches it. Needs `pip install sarpy` and a real product; see `FOLLOW-UPS.md`
item 12b for what it does and does not cover.

`tools/sim_cphd.c` generates synthetic phase history and is how nearly every claim in
`FOLLOW-UPS.md` was measured: `--clutter N` for distributed texture, `--clutter-vib` to
make the patch move coherently, `--seed`/`--offset-x` for independent realisations.

## Conventions this codebase actually enforces

**Every function definition needs a preceding block comment.** `scripts/check_docs.py`
runs as the `docs_coverage` ctest and fails the suite otherwise. The rule exists
because this code is full of dimensionally interchangeable quantities that are
physically different — Hz against rad/s, slant against ground range, azimuth line rate
against transmit PRF — and the comment must state units and conventions.

**The shell here is zsh, which does NOT word-split unquoted parameters.** A loop
of the form `for a in "" "--max-pulses 60000"; do prog $a; done` passes the whole
string as ONE argument in zsh where bash would split it into two, so the flag is
silently not recognised and the run looks exactly like a flag that does not work.
That cost a wrong diagnosis here: `--max-pulses` was reported as unimplemented in
`mmotion`, and a reader defect was hunted for, when the option worked and the
test harness did not. Write the arguments out in full, or use `${=a}` to force
splitting. The general form of the lesson is the one this codebase repeats: a
negative result from an unverified harness is not a negative result.

**EVERY INJECTION HERE HAS BEEN A PURE SINUSOID** (item 68), and real structures
are multi-modal, non-stationary and amplitude-modulated — the regime items 25, 55
and 56 found the selection policies failing in. `GROUND_TRUTH_DATASETS.md` lists
public SHM waveforms (Aventa turbine 200 Hz, Hell Bridge 100 Hz, Route 345) whose
sites are in NO Capella footprint — checked against the harvested 939, all zero —
but which can be INJECTED in place of a sine. That needs no collect pairing and is
the cheapest untried experiment left.

**PUBLIC SOURCES LEAVE EXACTLY ONE PAIRING: KILAUEA** (item 67). Hardanger fails
item 58's dwell test before its missing Sentinel-1 reader matters — IW lights a
point for 0.16-0.63 s, `df` ~1-6 Hz. Oroville and Mexico City are an order of
magnitude below the floor. Kilauea is public, native, and dwell-adequate, but 95%
of its station readings are below the floor — so it supports a CORRELATION test
across 51 collects (does reported displacement track the seismometer's RMS?),
not a detection test. That needs no collect above the floor and is the first test
here whose truth VARIES.

**TWO FLOORS, DO NOT CONFUSE THEM** (item 66): `validate` reports a PER-LOOK CRLB
(~0.2 mm/look); item 53's END-TO-END detection floor after 128 looks and a
periodogram is 0.0055 mm RMS, **37x lower**. `docs/GROUND_TRUTH_DATASETS.md` uses
the per-look figure, which makes Kilauea look 700x below the floor when it is
really 3x below at the median and one station 2.7x ABOVE. **The best entry there,
South Portland Bridge, is above the floor by ~300x and blocked by ACCESS, not
sensitivity** — an author request, not another factor of two.

**MULTI-PIXEL COMBINATION IS NOT A LEVER, IN EITHER FORM** (items 64-65). SqueeSAR
done properly — KS-test homogeneous pixel selection plus `phaselink.c`'s ML solver
— is WORSE than the naive version and eightfold worse than one pixel: artefact
70.7 to 1326 at K=9. **Statistically homogeneous pixels are homogeneous in the
ARTEFACT too**, so the ML estimator sharpens the shared carrier residual because
it is doing its job. Same shape as items 47 and 55: an average, a null or a
normalisation only helps against variation ACROSS the things combined. That closes
the factor-of-four search — finer cells 1.4x and a quartic ~1.4x are what remain,
so the floor is within about 2x of what this approach can reach.

**NAIVE MULTI-PIXEL COMBINATION FAILS** (item 64): combining the K brightest
pixels makes the artefact GROW faster than the signal — 70.7 to 182 at K=4 — where
sqrt(K) predicted a 2x gain. The literature explains it: SqueeSAR selects
STATISTICALLY HOMOGENEOUS pixels by a two-sample Kolmogorov-Smirnov test, not the
brightest, because the second-brightest pixel is usually a DIFFERENT scatterer with
its own carrier and its own residual; and phase linking combines via the sample
COVARIANCE MATRIX by maximum likelihood, not by averaging independently-estimated
phases. **`src/core/phaselink.c` already implements that solver** — written for
split-band, never applied to the sub-aperture stack. That is the experiment worth
running.

**UMBRA IS DISQUALIFIED ON DWELL** (item 63) despite holding 4048 CPHD against
Capella's 707 spotlight: median dwell 3.50 s, 0.4% at >=15 s, NONE at >=30 s,
against Capella's median 22.5 s. `df` = 0.29 Hz is coarser than most of the target
band. Same arithmetic as item 58 — Umbra is built for resolution and revisit, not
staring. **Finer grid cells are worth 1.4x and are NON-MONOTONE**: the carrier is
`(4*pi/lambda)*dX*dx/R` with `dx` the offset from the pixel centre, so 1.0 m cells
on 0.051 m resolution are the carrier's own cause — 0.25 m halves the artefact,
0.125 m makes it four times worse.

**THE DATA PROBLEM IS A FLOOR PROBLEM, NOT A SEARCH PROBLEM** (item 62). The
earthquake route fails on DUTY CYCLE: 20,013 s of aperture over 2,036 days is
1.14e-4, so with 1,242 M>=5 events within 200 km of a collect site the expected
number coinciding with an aperture is **0.14**. Finding zero is what the arithmetic
predicts, and an expectation of one needs seven times this archive. Quiet sites sit
at 1-2 um ambient against a 5.5 um floor — **another factor of ~4 in sensitivity
would turn all 315 synchronised hits into candidates instead of none**. That is the
only route this project can pursue alone, and item 53 shows the returns falling
fast.

**ALL 315 SYNCHRONISED HITS SCREENED: NONE has motion above the floor that
survives auditing** (item 61). 52 were above 5.5 um raw, 0 after three checks —
non-seismic channels (`LWD` is WIND DIRECTION, `LCE` is CLOCK ERROR; read the SEED
middle letter), chronically-high stations (`HV.UWB` reads 1-1.9 mm on every collect
for six weeks), and the neighbour test (UWB reads 400x its neighbours on the same
ground at the same instant). **Invert the search**: start from the earthquake
catalogue and look for a collect over the epicentre, not from collects hoping for
motion.

**OROVILLE DAM IS A PROVEN-STATIC SCENE** (item 60): its seismometer records
0.5-0.8 um RMS during the aperture, 7-11x BELOW item 53's 5.5 um floor. That makes
it useless as a positive control and valuable as the first real-data scene where a
null is INTERPRETABLE — USER_GUIDE section 7 item 0 says a real-data null means
nothing because "nothing moved" and "the chain cannot see" look identical, and here
an instrument separates them. A reported frequency there is a PROVEN false
positive. **Screen the 315 hits for an instrument showing motion ABOVE 5.5 um**
before downloading any more of them; that is one waveform request each.

**315 SYNCHRONISED MEASUREMENTS EXIST IN THE CAPELLA ARCHIVE** (item 59) —
station-collect pairs where an FDSN instrument was recording inside the footprint
during the aperture. `runs/screens/sensor-join/measurement_hits.csv`. The best is
**Oroville Dam** (`BK.ORV`, 30.4 s spotlight, 32 GB); Kilauea gives forty repeats
with six stations each. Nothing downloaded or run yet, and a waveform existing is
not a waveform showing anything — ambient ground motion is ~50x below item 53's
5.5 um floor, so a dam or building responding to wind or traffic is the case worth
pursuing, not quiet ground.

**SPOTLIGHT OR A DWELL MODE IS A HARD REQUIREMENT** (item 58), and it is the
cheapest thing to check about any candidate collect. What matters is PER-TARGET
observation time, which in stripmap is `lambda*R/(2*rho*V)` and not the collect
duration: KSTL's 17 s stripmap gives each point 0.71 s, so `df` is 1.4 Hz —
coarser than the entire 0.3-3 Hz band — and 128 sub-looks of 5.5 ms have no
azimuth resolution. No processing fixes it. The same arithmetic disqualifies
Umbra's 2 s spotlight at 0.50 Hz. Check the mode before the footprint and before
the sensors.

**A PROXY is not a MEASUREMENT** (item 57). ADS-B, AIS and METAR establish that
something was present and moving; they supply no displacement waveform, so they
cannot validate a frequency or an amplitude. Only an instrument recording a time
series — FDSN seismic and strong-motion, GNSS, SHM archives — is truth here.
`tools/footprint_sensor_join.py` does that classification, and **intersect the
POLYGON, never the bounding box**: at KSTL eleven FDSN stations fall in the box
and none in the 10 km strip, three of them building-mounted. Capella footprint
corners in the CPHD header are the scene reference followed by four corners NOT
in ring order — sort them by angle about the centroid or the scene excludes
itself.

**Search the literature online before concluding something is unsolved, and
before designing a method this field already has.** This project reasons from a
small set of papers it has read, and that set goes stale. Two of its standing
beliefs were wrong for want of one search: `RS_MICROM_EST_ARGMAX` -- brightest-pixel
tracking, validated in the literature against a shaker-driven corner reflector --
was absent for years while `FOLLOW-UPS.md` item 6 recorded this project measuring
the same estimator outperforming the correlator on its own data; and item 13's
"the published campaigns use ~99% overlap" is contradicted by a campaign using
0-65%. Prefer the primary source over a summary, quote figures with their
operating point attached, and record what the paper does NOT establish -- the
detection threshold is usually the part nobody publishes, because the published
experiments know where the target is.

**The headers are the documentation of record.** `microm.h` is 1343 lines for 17
declarations; `subaperture.h` and `validate.h` are similar. They carry derivations,
measured tables, and the reasoning behind each constant, and resolved `FOLLOW-UPS`
entries are retired *into* them. When a measurement settles something, it belongs
beside the code it constrains, not in a new document.

**Keep `docs/USER_GUIDE.md` current — it is not enforced by any test.** Update it in the
same change that alters a subcommand's flags, its defaults, what it prints, or which
estimator and reference modes are known to work. Its transcripts are **real captured
output**, not illustrations: if you change what a command prints, re-run the command and
paste what it actually says rather than editing the quoted text by hand. Section 9 and
the Gotchas list are claims about the current state of the code — when a defect there is
fixed or a mode starts working, that section is wrong until it is changed.

**Errors: set then return.** Every fallible function returns `resonarsat_status_t` and
calls `rs_set_error("...")` immediately before returning non-OK, at the point where the
specific detail is still in scope. Library code never prints; the CLI calls
`rs_report_error()`. A malformed or hostile input file must produce a status and a
message — never a crash, a partial write into a caller's buffer, or a silently
zero-filled result. The error buffer is thread-local, so this is safe inside OpenMP
regions.

**The verdict comes from the null control, not from cross-window agreement.**
`mmotion` refuses only when `--null-static` shows a motionless realisation reached
the measurement; without one it reports and says `NOT ADJUDICATED`. The old
scene-wide agreement gate rejected all five true positives on real data
(`FOLLOW-UPS.md` items 30-31) because a localised target cannot reach a fraction
calibrated on fixtures where the whole scene vibrates. Agreement and contiguity
are printed and gate nothing.

**Every positive control must ship its zero-amplitude twin** (item 38). Running
`--inject-vib` at amplitude 0 -- a bright scatterer added to the phase history that
does not move -- produces a LARGER peak than any real injection (prominence 56.3
against 38-47) and clears the same 19 motionless controls. Prominence,
`--null-static` and `rs_spectrum_scene_null()` all rank the motionless target
first. Only the reported FREQUENCY separates them: zero amplitude answers at the
band floor, a real injection at the injected value. So item 35's `p = 0.05`
measured that a scatterer was ADDED, not that it moved. Keep RECOVERABILITY (a
frequency put in comes back out -- established) apart from DETECTABILITY (deciding
something moved without knowing the answer -- not established), and never quote a
positive control that has no zero-amplitude run beside it.

**Injections land on the grid origin unless `--inject-at DX,DY` says otherwise**
(item 40). Every experiment before that put the target at the exact centre of the
analysis grid, so localisation scored perfectly without being tested. Off-centre,
the reported window is within ONE window of the target in 5 of 5 placements and
exactly right in 0 of 5 (item 41). Localisation is good to 16 m at these settings
and systematically no better: at 50% overlap the target sits in four windows at
once, they track the same dominant scatterer and return BIT-IDENTICAL series, and
the window it is centred in scores LOWEST of the cluster.

`rs_spectrum_centroid()` fixes that (item 42): the centre of mass of the agreeing
4-connected cluster, weighted by prominence ABOVE THE SCENE MEDIAN, locates the
target to 0.008 windows — 0.13 m, finer than the grid cell — where argmax is wrong
by exactly one window every time. **Quote that with its amplitude** (item 43): it
is 0.13 m at 2 mm and 2.24 m at 0.125 mm, linear in 1/amplitude, because the error
is set by the target's excess over the background gradient. argmax stays at
exactly one window at every amplitude, being quantised to an integer index. Item 41's bound was a property of reporting an
integer index, not of the measurement.

Score localisation on the INJECTION GEOMETRY — measure where the target lands by
differencing two `focus` runs that differ only in `--inject-vib`. Do not score on
"the window with the most energy at the injected frequency" (the aliasing ghosts
carry it too) and do not score against the window a zero-offset run reports (it is
itself one off). Both gave wrong answers here, and the second produced a published
"4 of 5 exact" that had to be withdrawn.

`--probe-hz F` is how that twin is compared (item 39): it adds `probe_psd` and
`probe_prominence` at ONE nominated frequency to `PREFIX_windows.csv`, so two runs
can be differenced there. Differencing the dominant-peak columns is meaningless
when the two runs peak at different frequencies, which is exactly how item 38's
motionless target won. The median window's increment is +0.00 and the injected
window's is positive — but the pairing must differ in NOTHING BUT amplitude:
paired against the uninjected run instead of the zero-amplitude one, the increment
is LARGER than a real 0.5 mm signal, because it is then measuring the scatterer.

**On the phase route `quality` is SPATIAL DOMINANCE, `1 - mean/peak`, and no longer
`1 - D_A`** (items 45-46). It was amplitude stability, which rejected vibrating
targets because a scatterer vibrating at 2 mm is not amplitude-stable — six ICEYE
windows carried the injected frequency at the scene's highest prominence and all
six failed the gate. The precondition being proxied is item 15's, one dominant per
sub-look resolution cell, which is spatial; a vibrating dominant is still dominant.
Consequences: `quality` and `d_a` are no longer complements, `--coherence F` is no
longer `D_A <= 1-F`, and any phase-route `quality` quoted from before item 46 is a
different quantity. The shared `quality >= 0.5*q_max` gate is now INERT on real
scenes — speckle alone scores 0.67 and real imagery 0.81-0.94 — which is a better
failure than removing the signal but means a pass is not evidence.

**The carrier removal fits linear, QUADRATIC and CUBIC rates on an ORTHOGONAL
basis** (items 52-53): `kc = k-(N-1)/2`, `q = kc^2-(N^2-1)/12`,
`c = kc^3-kc*(3N^2-7)/20`. The constants are not decoration — `c` and `kc` are both
odd, so parity does not separate them, and an uncentred staged search returns zero
for the higher terms every time (measured: 1.13x reduction against 38x). Artefact
down 171x in total, floor from 0.125 mm to 0.0039-0.0078 mm. Returns are falling
fast — 38x then 4.5x — so a quartic is not worth a fourth O(N^2) pass. Centring is not cosmetic: `k` and `k*k` correlate over a finite record,
so an uncentred staged search shifts the mean rate as it scans curvature and
returns `mu = 0` every time — measured, 1.13x artefact reduction against 38x
centred. The floor fell from 0.125 mm to 0.0156 mm, eightfold. That is 0.011 mm
RMS, NINE TIMES BELOW the published 0.10 mm, which is a reason for suspicion
rather than celebration: the injected target is a perfectly coherent point source,
cleaner than any real structure.

**The floor was a QUADRATIC PHASE RESIDUAL, not noise** (item 51). The carrier
removal fits only a linear ramp `exp(-i*v*k)`, but a scatterer's range history is
quadratic in time, so the curvature survives and lands at the band floor. Fitting
and removing a quadratic drops the worst window's artefact 2000x, from 21,602 to
10.4. It scales as `REL^2` and so does the signal, so the ratio is
brightness-independent — a brighter target does NOT make smaller motion detectable.
The fix is a second search dimension over `exp(-i(v*k + w*k^2))`; NOT implemented,
and predicted to drop the floor toward 0.003 mm.

**The sensitivity floor is 0.0625-0.125 mm** at overlap 0.5 with a zero-amplitude
twin (item 50) — 0.088 mm RMS, which finally agrees with the published 0.10 mm
rather than beating it 2x. Below 0.125 mm the answer becomes the twin's artefact,
whose ratio plateaus and stops responding to amplitude. **Use the twin, not item
49's empty-desert range, to judge an injected run**: desert has no bright scatterer,
so 34.4 against an injected run's 12,060 would have called 0.0078 mm a detection.

**Do not raise `--overlap` for the phase route** (item 48): separation collapses
four orders of magnitude between 0.75 and 0.90 and fails at 0.95, on real data,
withdrawing item 14's advice. **The local peak's control range on real Giza desert
is 15.1-34.4 over nine disjoint grids** (item 49), so anything under ~35x there is
inside what empty desert produces; nine controls give p_min = 0.10, not 0.05.

**Two ideas have now failed against item 25's unsafe failure, for the same reason**
(items 55-56): `rs_spectrum_local_window()` and `rs_spectrum_am_check()` both test
the SPECTRUM, and the failure is not in the spectrum. The aspect lobe makes the
tracked pixel FADE, and the pixel is chosen once from the reference look, so its
phase is noise-dominated during the fade — the series is NON-STATIONARY, and the
spectrum of that has structure at no particular frequency. A test for
non-stationarity is what this needs and none exists here. Amplitude dispersion is
the closest thing, which is why `rs_spectrum_ps_window()` is still the only safe
policy.

**`rs_spectrum_local_window()` addresses COLOURED NOISE, not spurious peaks
generally** (item 55). It was a large win on real Giza data — separation 3.3x to
37x — and is a LOSS on the aspect fixture, recovering at 1 of 4 lobe widths where
prominence gets 3 of 4, with the same 3-of-12 static false positives. Aspect
dependence produces genuine sidebands rather than a coloured floor, and a local
background cannot tell a real sideband from a real tone. **No policy here is fit
for an aspect-dependent scene** — prominence is useful and unsafe, the local peak
is worse, the PS selector refuses everything.

**The noise floor is RED, so `prominence` is biased toward low frequencies** (item
47). At `--overlap 0.90` adjacent sub-looks share nine tenths of their pulses, so
their errors are correlated: bins 1-4 carry 24x the power of the band above
Nyquist/2 on real data. `prominence` divides by the mean of all other bins, which
is right only for a white floor, so an uninjected scene reliably reports the lowest
admissible frequency — and it is NOT a trend, since 83% of it survives a cubic fit.
`rs_spectrum_local_window()` scores each bin against the median of its own
neighbourhood instead and widens the injected-versus-control separation from 3.3x
to 37x. This is a third cost of overlap that items 13-14 did not account for: the
setting item 14 recommends for the phase route manufactures the noise that beats
the signal. **Prominence is also not comparable across `--fmin` settings** — it is
measured against the admissible band, so changing the floor changes the number
while the reported window and frequency stay put.

**But a null only calibrates against noise its own model can produce** (item 37).
Below 2 mm the Giza sweep reported bin 1 instead of the injected frequency, with
prominence *rising* from 32.0 to 56.0 as the target weakened — and prominence,
quality, `D_A` and the null control all endorsed it, because the null's trials are
synthetically static and contain no dominant scatterer to impose a trend. Item 11's
rule that only a null catches common-mode artefacts is true only of artefacts the
null can generate. A null built from the real scene rather than simulated beside it
is the open half.

**The first `RS_SPECTRUM_LEAKAGE_BINS` (3) frequency bins are never reportable, and
that is not a tunable.** A Hann main lobe is ±2 bins, so bins 1-2 are the skirt of
any residual trend; bin k is k cycles across the dwell, so those are one and two
cycles of a record one dwell long. `--fmin` raises the floor and cannot lower it.
Do not "fix" a fixture that stops reporting by lowering it — move the fixture's
frequencies up, as `test_tracking.c` had to in two places.

**The floor relocates a trend; it does not remove one.** The real uninjected Giza
scene under the floor answers at 0.130 Hz — bin 4, the first admissible bin — at
prominence 17.9 against the recalibrated null's worst of 22.9. It is refused by
1.28x, and by the NULL rather than by the floor. A cluster of windows reporting
the first admissible frequency is a trend field, not a detection.

**Warn rather than silently degrade.** A wrong setting here does not fail loudly: ask
for a measurement a collect cannot support and you get a complete, well-formed spectrum
with a confident peak produced by the processing rather than the ground, and a
motionless scene produces one too. That shapes the design — `validate` before
processing, `--null-static` beside every result, the consensus statistic that can
report disagreement, and the `*_windows.csv` evidence file written beside every run so
a later question about the selection policy can be asked without reprocessing.

**No result is a measurement until it survives a sweep.** The bar is
`rs_track_fit()`: slope near 1 and rms under half a bin across injected frequencies,
pooled over independent clutter realisations, with a static control through identical
processing. A single frequency matched once is a coincidence — `FOLLOW-UPS.md` item 2
records several. Do not describe a per-point match as a recovery.

**The consensus gate is blind to common-mode artefacts** (`FOLLOW-UPS.md` item 11).
Agreement catches noise that is independent across windows; an artefact produced by the
processing appears identically in every window and scores 100%. Only a null control
catches that. Neither substitutes for the other.

## State of the work

**`--estimator phase` now recovers** on synthetic fixtures: slope 1.008, rms 0.0070 Hz
against a half-bin bound of 0.0252, pooled over three clutter seeds with static controls
outside the swept band (`FOLLOW-UPS.md` item 14). It had returned a fixed 0.407 Hz for
every injection and for motionless scenes alike, because it wrapped a geometric phase ramp
of 1.1-1.9 rad per look instead of removing it first. Read item 14's caveats before quoting
this: it is synthetic, `rs_sim_scene()` has no sub-look decorrelation to speak of, the
observable wraps beyond ~lambda/4 so it needs a much smaller injected amplitude than the
correlation fixtures use, and its precondition is one dominant scatterer per SUB-LOOK
RESOLUTION CELL (item 15 — a lattice finer than that puts several equal scatterers in
one cell and satisfies nothing, which is why it first appeared to fail on the
dominant-scatterer fixture).

That sweep also concluded **phase wants high `--overlap`** — recovery holding to 95%,
high overlap buying sub-look coherence. **Item 48 withdraws it on real data.** The
injection-versus-control separation collapses four orders of magnitude between 0.75
and 0.90 and fails entirely at 0.95, because overlap correlates the noise between
adjacent looks and correlated noise is red (item 47). Overlap does buy coherence; it
spends it manufacturing the floor the coherence was for. Where the optimum sits is
not established — only that 0.90–0.95 is far worse than anything below it.

**Run once on the real Giza collect** (item 17): `--estimator phase` at 0.90 overlap,
45 minutes, and it returned a NULL at 16% window agreement — the project's own
motionless signature — with the frequency histogram flat across the low bins. The
common-mode sawtooth artefact item 11 recorded at *100%* agreement is gone on real
data, which is the first evidence outside simulation that item 14's carrier removal
holds. Nothing in that scene is known to move, so the null says nothing about
sensitivity and is not a negative result about the method.

Item 19 sharpens that null: `mmotion` now reports **amplitude dispersion** (`D_A`,
Ferretti et al.'s persistent-scatterer statistic) per window, and at Giza 0 of 225 windows
met `D_A <= 0.25` with the best at 0.381. The phase route's precondition was unmet across
the whole scene, so that run could not have succeeded whatever the pyramid was doing. `D_A`
separates the synthetic fixtures whose recovery and failure item 15 established, with no
overlap, which is what makes it worth reporting.

**The tracker meets the bar on real data; nothing that reports does** (item 30). Five
frequencies injected into the real Giza phase history recover in the injected window at
slope 0.999, rms 0.00033 Hz against a 0.0163 bound, with an uninjected control below the
swept band -- while every run printed `NO FREQUENCY REPORTED` because the scene-wide
policies pool 136 windows of desert against one of signal. That bounds items 19-20 too:
`D_A` at the recovering window ran 0.376-0.645 and never met 0.25, so the
persistent-scatterer criterion would have rejected all five recoveries.

Nothing has been shown to DETECT REAL MOTION -- the injected window was known because the
target was put there -- and no collect available to this project
contains motion known to be there. Every
estimator and reference mode has failed at operating points this project's own
arithmetic calls admissible; the most recent finding (`FOLLOW-UPS.md` items 7-9) is that
the tracker does recover the injected carrier in most windows and the *selection policy*
discards it. `rs_spectrum_best_window()` still selects by prominence and is still what
the tool reports; ranking on consensus and contiguity instead is the open half.
`rs_spectrum_ampcor_window()` is a third policy added since (`FOLLOW-UPS.md` item 12)
that culls on what the correlator knew rather than on the spectrum. It is reported
beside the other two and gates nothing. Swept in `tests/test_cullsweep.c`: it answers
rarely and, so far, always correctly, and refuses every static control where the other
two report a confident frequency — but its answers cluster at two distinct injections,
so its recall, not its correctness, was the open question (items 12c-e). All three gates
are now swept. The answer: gate 3 holds the recall down, its derived threshold of two is
pinned from both sides — below it a static scene gets an answer, above it nothing does —
and the coverage a fit needs exists in the tracking only together with false positives.
No gate setting gives both, which is a statement about the operating point rather than
about any constant.

`tests/test_cullsweep.c` also measures what this fixture family can reach: coherence tops
out at 0.323 even at 95% overlap, against a `--coherence` default of 0.4 and a real
collect's 0.85. **The coherence gate has never been tested by anything**, and item 12f shows why no
fixture built on `rs_sim_scene()` can test it: every scatterer carries analytically
exact phase, so coherence there is set by sub-look separation alone and is invariant
to scene content — a dominant-scatterer fixture was built and moves it by 0.022 where
overlap moves it by 0.267. Testing that gate needs a change to the propagation model
itself, not to the target list.

**Do not raise `--overlap` to chase the literature's operating point** (item 13). The
sampling ceiling and the sub-aperture response ceiling have a ratio of
`0.829/(1-overlap)`, independent of look count and dwell, so zero overlap is the
balanced setting and anything above it spends sampling rate on frequencies already
averaged away. Measured across 25 sweep points, recovery requires a response above
~0.5 with no exceptions. The published campaigns use ~99% overlap because they read
pixel phase, which the averaging does not attenuate; entering their regime means making
the PHASE estimator work, not raising the look count.

`rs_spectrum_ps_window()` is a fourth selection policy (item 20): it SELECTS on amplitude
dispersion rather than gating on it, as the source literature does, at a threshold measured
here rather than borrowed — `D_A <= 0.25` lifts a 33% window-level hit rate to 95% over 1800
windows. `rs_read_cphd()` now applies the per-vector `AmpSF` the CPHD standard requires (item 21);
it cannot affect phase or any reported frequency. It does NOT explain the real-vs-synthetic
`D_A` gap — applying it to real data changed nothing, and the synthetic experiment that
appeared to prove it did was an artefact of an unbounded injected gain. **Scatterer density
does not explain it either** (item 23): swept on real data across a 340× change in cell area,
`D_A` does not follow. Nor does the brightest-pixel selection bias — 4× the candidate pixels
moves the synthetic median by 0.025, where it moves the real median by 0.95. That last
asymmetry is the lead: real scenes respond ~40× more strongly to window size than the
fixtures do, which points at ASPECT-DEPENDENT SCATTERING, the same modelling gap item 12f
found from the coherence side. `rs_sim_scene()` gives every scatterer an isotropic,
analytically exact response, so a simulated dominant is dominant in every look by
construction. **`rs_sim_scene_aspect()` now provides that response** (item 24): a facet with a
sinc lobe over part of the aperture, opt-in, with `rs_sim_scene()` unchanged so every earlier
measurement stands. It reproduces the level real collects occupy (median `D_A` 0.79-0.87
against a real 0.58-0.89) and the sign and monotonicity of the window-size response, at about
a third of the real slope. The larger finding is a side effect: the window-level hit rate
collapses from 45-49% to 3-10%, the first time a fixture here has behaved like a real collect
in refusing to yield a frequency.

**The carrier fix of items 52-53 rescues item 25's RECOVERY and not its SAFETY**
(item 54): 3 of 4 lobe widths now clear slope-and-rms where none did, but 3 of 12
static controls still return an in-band frequency on scenes where nothing moves.
The slope failure was a carrier residual; the false positives are a POLICY failure,
which is what item 25 said at the time. Read the rest of item 25 with that split
in mind.

**Item 14's recovery does NOT survive it** (item 25). Item 14's own sweep against the aspect
fixture: the reported policy, `rs_spectrum_best_window()`, fails at every lobe width with rms
0.59-1.78 Hz against a 0.0252 Hz bound and a negative slope at three of four — and returns an
in-band frequency for two of twelve MOTIONLESS controls. `rs_spectrum_ps_window()` is the only
policy that behaves: it refuses where it cannot tell and, where it answers, returns slope 1.008
and rms 0.0070, item 14's figures to four decimals, with no in-band static false positive
anywhere. Precision without recall — 6 of 18 points. This bounds item 14 rather than
withdrawing it: the estimator does what it claims on the scene it was measured on, and there is
now no basis for expecting that to transfer to a real collect.

`tests/rs_sim.h` now carries two fixture families: `make_clutter`-style uniform speckle
and `rs_sim_dominant_patch()`, dominants on a lattice over a diffuse background. The
second is harder for the spectrum-only policies and leaves the cull's profile unchanged,
which is the only cross-fixture evidence any of this has.

`--reference pair` and `--reference adjacent` do not recover a frequency on the
synthetic fixture and are exposed because the sources describe them, not because they
work. `--reference lag` is documented and harmless but does not beat `first` on a
consensus measure.

`build/` and `data/` are gitignored; run outputs are NOT -- `.gitignore` opts `runs/` back in, because a run directory IS the record. Raw `.f32` cubes and `.cphd` stay ignored. `tools/new-run.sh <scene> <suffix>
"<question>"` seeds a run directory with a `RUN.md` recording commit, host, commands and
result — a null result stays there rather than being deleted.
