# User guide

How to build the tool, run it, and read what it tells you.

**Read this first:** nothing in this project has yet been shown to recover a
vibration frequency it was not told. The chain runs end to end and every stage is
tested, but no configuration has passed the bar in `README.md`. This guide
therefore spends as much space on how the tool refuses, and how to tell a
measurement from an artefact, as on how to make it produce a number. That is the
correct ratio for this instrument.

---

## 1. Build

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

19 tests, about a minute. No external dependencies and no data needed — every
test builds its own fixture, so a fresh clone should be green.

Watch the configure line for one message:

```
-- OpenMP enabled                                    good
CMake Warning: OpenMP NOT found -- backprojection will run on one core
```

On macOS OpenMP comes from Homebrew's keg-only `libomp` (`brew install libomp`);
CMake looks in `/opt/homebrew/opt/libomp` and `/usr/local/opt/libomp`. Without it
everything still works, roughly 7x slower.

For memory errors and undefined behaviour there is a dedicated build type:

```sh
cmake -S . -B build-asan -DCMAKE_BUILD_TYPE=ASAN && cmake --build build-asan
ctest --test-dir build-asan --output-on-failure
```

---

## 2. The commands

```
micromotion info       print a product's geometry and timing
            validate   can this collect support the measurement you want?
            focus      form an image from phase history (backprojection)
            mmotion    track sub-looks and extract vibration spectra
```

Run any of them with no arguments for its full help, which is long and worth
reading — the options carry warnings that are not repeated here.

Readers accept CPHD (`--cphd`), SICD (`--sicd`) and UAVSAR (`--uavsar` with
`--ann`).

The intended order is **`info` → `validate` → `focus` → `mmotion`**. Skipping
`validate` is the most common way to spend an hour producing a confident number
that the geometry could never have supported.

---

## 3. Quick start, on synthetic data

The fastest way to see the whole chain work. `sim_cphd` writes phase history
containing a target vibrating at a frequency you choose, so the answer is known
in advance — the only data here where that is true.

```sh
./build/sim_cphd scene.cphd 0.5 0.02 --clutter 400 --clutter-vib --seed 7
```

`0.5` Hz, `0.02` m amplitude, 400 Rayleigh clutter scatterers, all vibrating
coherently so the patch moves as a whole — which is what a structure's surface
does. Writes `scene.cphd` (16 MB) and `scene.cphd.truth`.

Then track it:

```sh
./build/micromotion mmotion --cphd scene.cphd \
    --n 128 --overlap 0 --size 128 --cell 0.5 --win 32 --upsample 200 \
    --out run1
```

About 9 seconds with OpenMP. Abridged output:

```
sub-apertures: 128 looks, dt 0.1550 s
  observable band  f_max 3.23 Hz   AT sub-look resolution 8.26 m
sub-pixel refinement: 1/200 px
tracked 49 windows (7 x 7); 49 pass the 0.00 coherence mask
spectra: 65 bins, 0.0504 Hz resolution
NO FREQUENCY REPORTED: only 4 of 49 windows agree (8%), which is what a
  MOTIONLESS scene produces. Diagnostics only, NOT a measurement -- strongest
  window 48: 0.504 Hz, prominence 8.3, quality 0.060
  consensus: 0.504 Hz, agreed by 4 of 49 voting windows (8%), 36 distinct
  answers, largest contiguous block 2
  WARNING: the agreeing windows are SCATTERED (largest block 2 < 4).
```

**This is the tool working correctly.** The strongest window says 0.504 Hz
against an injected 0.500 — which looks like a recovery and is not one. Only 4 of
49 windows agree, in a scattered pattern, which is the signature of coincidence
rather than a spatially resolved mode. `docs/FOLLOW-UPS.md` item 2 records
several such near-misses across a frequency sweep, where they are visibly
uncorrelated with what was injected. The consensus gate refuses it, and it is
right to.

Vary the injection and watch what the report does — that is the exercise this
tool is for. One frequency matched once means nothing.

---

## 4. Working with a real collect

Real spotlight collects are tens of gigabytes and will not fit in memory whole.
Using the Capella Giza collect as the example:

```sh
C=/path/to/CAPELLA_C13_SP_CPHD_HH_20241004001939_20241004002012.cphd
./build/micromotion info --cphd "$C" --rbins 512
```

```
note: Capella product -- its signal does not follow its declared SGN=+1, so the
      FX-to-delay transform is inverted to match Capella's own reference reader.
      Without this the image is mirrored in range.
cphd: skipped 8 vector(s) flagged invalid and 0 with non-finite geometry, of 335149
pulses              335141
carrier             9.3000 GHz
transmit PRF        10196.35 Hz    (NOT the vibration sampling rate)
dwell               32.869 s
range bin spacing   0.2498 m
```

Both notes matter. The Capella SGN override is a vendor metadata defect the
reader compensates for — without it every image is mirrored in range and still
looks perfectly focused. And the transmit PRF is **not** the rate at which
vibration is sampled; that is set by the sub-aperture step, which is thousands of
times slower.

### Three flags that control memory

Omit them and you get an honest refusal rather than a swap death:

```
info: out of memory (cphd: 335141 pulses x 29160 range bins needs 78.2 GB, and
this machine has 25.8 GB. Pass --rbins 4805 or fewer to read a range window, or
--pulse-stride to thin the pulses.)
```

| flag | what it does | what it costs |
|---|---|---|
| `--rbins N` | reads a **window** of N range bins, N/2 either side of the scene reference point | a target far out in Y falls outside it — 550 m out needs ≈4096 bins at 0.2498 m/bin |
| `--max-pulses N` | caps pulses **read**, shortening the aperture | coarsens azimuth resolution — which is the *honest* way to match a large cell |
| `--pulse-stride N` | keeps every nth pulse | lowers effective PRF and the observable band. **Never use it for a measurement run** — registration images only |

`--pulse-count` limits focusing but loads the whole collect first, so it does not
help memory. Use `--max-pulses`.

### Match the aperture to the cell

This collect focuses to 0.051 m. Backproject that onto a 2 m grid and it aliases
into fully developed speckle with no structure — which reads as "the target is
not there". The tool warns and names the cell that would be safe:

```
warning: grid cell 0.500 m is coarser than the 0.064 m azimuth resolution this
         collect supports. The image will alias: each scatterer gains ghost
         peaks that resemble real targets. Use --cell 0.064 or finer, or treat
         the result as deliberately multi-looked.
```

Act on it. Shorten the aperture with `--max-pulses` so resolution matches the
cell: `L = lambda*R/(2d)`, `pulses = L/V * PRF`. For this collect, cell 2.0 m →
8,348 pulses; cell 1.0 m → 16,696. It is also about 10x faster.

### Place the grid with `--at`, not `--offset`

```sh
./build/micromotion focus --cphd "$C" --at 29.979175,31.134186 \
    --size 2048 --cell 1.0 --rbins 4096 --max-pulses 16696 --out khufu.png
```

`--offset X,Y` speaks the *file's* image-plane axes (CPHD `uIAX`/`uIAY`), which
point where the collector chose — not north and east, and not azimuth and ground
range. Converting coordinates into them by hand produces an error the output
cannot reveal, because **a misplaced grid still produces a complete, well-focused
image of the wrong ground.** `--at LAT,LON` makes the product do the conversion
and prints the offset it resolved to.

---

## 5. `validate` before you process

Every check is arithmetic on the geometry and costs milliseconds; only the file
read is slow, and it keeps 8 range bins. Run it before committing to a job that
takes an hour.

```sh
./build/micromotion validate --cphd "$C" --frequency 3.0
```

Real output on the Giza collect at default settings, abridged:

```
  PASS    frequency resolution   0.0304 Hz from a 32.869 s dwell. Resolvable.
  FAIL    observable band        alpha 5.000% and overlap 0.40 give 33 sub-apertures.
                                 Each averages over 1.6434 s, so the band reaches
                                 0.304 Hz. The target is ABOVE the band.
  WARN    observation ratio      eta 4.930. The sub-look RESOLVES the target's own
                                 paired echoes, so the tracker follows a train
                                 rather than a point.
  FAIL    averaging nulls        the response vanishes at integer eta. The target
                                 sits 0.070 from the nearest. That is on a null.
  FAIL    ambiguity              wrap ceiling 1.5 px p2p against a 7.0 px artefact
                                 floor. The ceiling is BELOW the floor, so no
                                 target amplitude satisfies both.
  UNKNOWN ground truth           Whether anything in this scene moves is not a
                                 property of the collect.

  VERDICT: FAIL
```

Read the failures as instructions. Here they say: 3 Hz is far outside what a
1.64 s sub-aperture can observe, and the sub-aperture must be shortened — more
looks, less overlap — before this question is even askable.

Three properties of this command worth internalising:

- **Every check is necessary and none is sufficient.** Passing all of them means
  the configuration is *capable*, not that the measurement is real.
- **`ground truth` always reports UNKNOWN.** Whether the ground moves is not a
  property of the file. Every validated result in this literature used a corner
  reflector on a shaker with a synchronous displacement sensor, and no such
  collect is in any open archive.
- **A FAIL does not stop you.** Processing a failing configuration still produces
  a complete, well-formed spectrum with a confident peak in it. That is the
  failure mode the command exists to catch.

---

## 6. Reading `mmotion` output

Line by line, in the order it prints:

| line | what to check |
|---|---|
| `sub-apertures: N looks, dt X s` | `1/dt` is your sampling rate; the band is half that. `dt` is often not what `t_sap*(1-overlap)` predicts — read it, don't derive it |
| `observable band f_max ... AT sub-look resolution ...` | the frequency ceiling, and the resolution you paid for it |
| `sub-pixel refinement: 1/N px` | the **quantisation** of the reported series. An excursion under one step comes back as a two-level staircase whose energy sits at low frequency regardless of cause |
| `tracked W windows; K pass the coherence mask` | K of 0 means nothing downstream is a measurement |
| `spectra: B bins, df Hz` | `df` sets what "within half a bin" means |
| `consensus: f, agreed by A of V windows, D distinct, largest block C` | the detection statistic — see below |
| `sub-aperture response ... at an observation ratio of eta` | near-integer eta means the observable's response is near zero there |

### The consensus line is the important one

`rs_spectrum_best_window()` reports the most prominent single window's argmax.
That statistic is equally confident whether or not there is any signal, which is
why prominence turned out to be anti-correlated with correctness. Beside it,
`rs_spectrum_consensus()` reports what the windows collectively say:

| | agreement | distinct answers | largest contiguous block |
|---|---|---|---|
| injected 3.000 Hz | 47% | 18 | **15** |
| static, no motion | 16% | 33 | **3** |

Contiguity is the sharper of the two. Windows overlap at half their width, so a
resolvable target occupies a 2×2 block at minimum — a largest block under four
cannot be a spatially resolved mode, and that bound comes from geometry rather
than tuning.

**But the consensus gate is blind to one class of artefact.** It catches noise
that is *independent* across windows. An artefact produced by the processing
rather than the scene appears identically in every window, so the windows agree
about it unanimously — measured at 100% agreement on a motionless scene. No
threshold helps; 100% is the ceiling. Only a null control catches that.

### When it refuses

Two refusal paths, both correct behaviour:

```
mmotion: value out of range (spectrum: no window resolved motion above the
0.061225 px quantisation floor (3 sigma of 1/40 px) ...)
  No frequency is reported because none is supported by the data, which is a
  different statement from finding none.
```

Nothing moved by more than the sub-pixel step could resolve. Raise `--upsample`,
or check with `--shifts` whether the series moves at all.

```
NO FREQUENCY REPORTED: only 4 of 49 windows agree (8%), which is what a
MOTIONLESS scene produces. Diagnostics only, NOT a measurement
```

Something was found, and the windows do not agree it is real. The number printed
after this is a diagnostic. Do not quote it.

---

## 7. Making a result credible

Four things, in increasing cost. A result without at least the first two is not
worth reporting.

**1. A null control.** `--null-static N` runs the identical processing over a
scene known to be motionless. This is the only check that catches common-mode
artefacts, and it is what `README.md` names as the credibility check that
matters.

```sh
./build/micromotion mmotion --cphd scene.cphd ... --null-static 20
```

`--shuffle-looks SEED` is a cheaper cousin that shuffles sub-look time order,
answering "is there temporal structure here at all". It is **not** a bound for
the phase estimator — a drifting series beats its own shuffles by construction,
and the tool warns when you try.

**2. A sweep, not a point.** Inject several frequencies and fit. `rs_track_fit()`
(`tests/rs_test.h`) returns slope and rms, and the bar is **slope near 1 and rms
under half a bin**, pooled over independent clutter realisations. A chain that
emits one fixed spurious frequency passes a per-point test wherever that value
lands near the injection.

**3. Independent realisations.** `--seed`, and `--offset-x`/`--offset-y` to
translate the scene relative to the processing grid. Geometry and motion stay
identical; only the speckle changes.

**4. The raw series.** `--shifts FILE.csv` writes the per-look shift series
before detrending or any spectral estimation. The spectrum cannot separate a real
low-frequency motion from correlator bias; the series can. Several conclusions in
`docs/FOLLOW-UPS.md` were overturned by dumping this and looking.

---

## 8. What gets written

With `--out PREFIX`:

| file | contents |
|---|---|
| `PREFIX_freq.png` | dominant frequency per window |
| `PREFIX_quality.png` | tracking quality per window, 0–1 |
| `PREFIX_windows.csv` | **per-window evidence behind the selection** — every window's frequency, prominence, quality, excursion, whether it passed the gates, and whether it agrees with the consensus |

The CSV is the one to keep. Every earlier result in this project kept the answer
and discarded what produced it, so later questions about the selection policy
could not be asked without reprocessing the collect. It costs one small text file
and changes no reported number.

`--ccd-out PREFIX` additionally runs a scale-invariant change-detection locator
over the same stack. It answers *where* something is moving, not at what
frequency, and does not need the tracker to succeed. Its no-change value is 1.0,
a bright but stationary target also scores near 1.0, and it has **no detection
threshold** — compare against a map from a motionless scene before reading
structure into it.

Use `tools/new-run.sh <scene> <suffix> "<question>"` to seed a run directory with
a `RUN.md` recording the commit, host, commands and result. A null result stays
there rather than being deleted.

---

## 9. Choosing estimator and reference

Two orthogonal choices. Neither has a setting known to work; the honest summary
is below.

`--estimator correlation|phase|splitband`

- `correlation` (default) — cross-correlation peak, sub-pixel refined.
- `phase` — phase of a single dominant pixel. Closest to the published work that
  reports success, but re-scoring found it returning one fixed frequency at 100%
  window agreement for every injection *and* for a static scene.
- `splitband` — split-band phase linking over all N² interferograms. Returned one
  fixed frequency at every configuration swept.

`--reference first|adjacent|pair|lag`

- `first` (default) — every look against look 0. Decorrelates badly at low
  overlap, and is nonetheless the best performer measured on a consensus vote.
- `pair` — what WO2024008365A1 and the Giza paper describe. **Does not recover a
  frequency** on the synthetic fixture; exposed because the sources describe it
  and it should be testable.
- `adjacent` — accumulates consecutive differences. Also does not recover.
- `lag` — each look against the one `--lag N` before it. Fixes the coherence
  problem and introduces a low-frequency floor near 1.5 Hz on this geometry. Does
  not beat `first` on a consensus measure.

`--coherence F` masks windows whose sub-looks do not correlate, default 0.40.
Isolated point targets on an empty scene score below that even when tracking
perfectly; pass `0` to inspect an unmasked result.

`--no-optimize` is an audit baseline, not a better measurement: it searches the
whole upsampled correlation surface rather than the neighbourhood of the integer
peak, and runs serially. Backprojection is bitwise identical either way.

---

## 10. Gotchas

- **`validate` cannot read `sim_cphd` output.** `info`, `focus` and `mmotion`
  sniff the file magic and accept both real CPHD and the simulator's internal
  format; `validate` accepts real CPHD only and fails with *"does not begin with
  a CPHD version line"*. Known gap, not a decision.
- **`rs_slc_t.r0` means different things in different readers.** UAVSAR fills it
  as the slant range of the first range sample, as documented; the SICD reader
  sets it from `SCPCOA/SlantRange`, the range to scene centre. On a wide swath
  those differ by half a swath. See `docs/FOLLOW-UPS.md` item 5.
- **The amplitude field is qualitative.** Frequencies recover where relative
  amplitudes do not. Label it as such wherever it is presented.
- **`-ffast-math` must stay out of the build.** It permits reassociation and
  flushes denormals, which perturbs exactly the sub-pixel correlation peaks and
  interferometric phase this project measures.
- **Every function needs a preceding block comment** stating units and
  conventions. `scripts/check_docs.py` runs as the `docs_coverage` ctest and
  fails the suite otherwise.

---

## 11. Where to read next

- `README.md` — what the project is and what the bar for a result is.
- **`docs/FOLLOW-UPS.md`** — the map of dead ends, with measurements. It records
  that sub-look images can be correct while the tracker fails to read them; that
  the reference scheme looks like the binding constraint and is not; and that a
  gate derived from window geometry refused correct measurements. It includes
  entries that withdraw earlier entries. Each cost hours to establish and none is
  recoverable from the code.
- The headers. `include/resonarsat/microm.h` is 944 lines for about ten
  declarations, and carries the derivations, measured tables and reasoning behind
  every constant. `subaperture.h` and `validate.h` are the same. When a
  measurement settles something in this project it is retired *into* a header,
  beside the code it constrains.
