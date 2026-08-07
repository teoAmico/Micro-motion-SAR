# User guide

How to build the tool, run it, and read what it tells you.

**Read this first:** one configuration has passed the bar in `README.md` —
`--estimator phase`, on synthetic fixtures, swept and pooled over seeds with a
static control (`FOLLOW-UPS.md` item 14), and that recovery does **not** survive
aspect-dependent scattering (item 25). On real data the *tracker* has met the bar
— five injected frequencies recovered from the Giza collect at slope 0.999 — but
every selection policy in the tool discarded the result and the tool printed *no
frequency* each time (item 30). So nothing that **reports** has passed it on real
data, and `--estimator correlation`, which is the default, has not passed it
anywhere. This
guide therefore spends as much space on how the tool refuses, and how to tell a
measurement from an artefact, as on how to make it produce a number. That is
still the correct ratio for this instrument.

---

## 1. Build

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

21 tests, about three minutes — `test_cullsweep` is two of those on its own,
running some sixty full chain passes across two fixture families. No external
dependencies and no data needed: every test builds its own fixture, so a fresh
clone should be green.

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
micromotion info       print a product's geometry and timing (--json for CPHD)
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
    --coherence 0 --out run1
```

About 9 seconds with OpenMP. `--coherence 0` opens the mask: an isolated
synthetic target on a clutter field scores far below the 0.4 default even when it
tracks perfectly, so without it every window is masked out and there is nothing
to look at. Output, with the aliasing warning and the file list cut:

```
sub-apertures: 128 looks, dt 0.1550 s
  observable band  f_max 3.23 Hz   AT sub-look resolution 8.26 m
sub-pixel refinement: 1/200 px (default 1/10 azimuth, 1/20 range)
tracked 49 windows (7 x 7); 49 pass the 0.00 coherence mask
spectra: 65 bins, 0.0504 Hz resolution
NO FREQUENCY REPORTED: only 4 of 49 windows agree (8%), which is what a
  MOTIONLESS scene produces. Diagnostics only, NOT a measurement -- strongest
  window 48: 0.504 Hz, prominence 8.3, quality 0.060, peak-to-peak velocity 188.7 mm/s
  49 of 49 windows were eligible for selection (coherence gate and quantisation floor)
  consensus: 0.504 Hz, agreed by 4 of 49 voting windows (8%), 36 distinct answers, largest contiguous block 2
  WARNING: the agreeing windows are SCATTERED (largest block 2 < 4).
           Windows overlap at the tracking stride, so a resolvable mode
           occupies a 2x2 block at minimum. This is the shape of coincidence.
  cull: no window survived (49 entered; SNR 49, sigma 0, neighbours 0 removed)
  sub-aperture response 0.9900 (-0.1 dB) at an observation ratio of 0.08
```

**This is the tool working correctly.** The strongest window says 0.504 Hz
against an injected 0.500 — which looks like a recovery and is not one. Only 4 of
49 windows agree, in a scattered pattern, which is the signature of coincidence
rather than a spatially resolved mode. `docs/FOLLOW-UPS.md` item 2 records
several such near-misses across a frequency sweep, where they are visibly
uncorrelated with what was injected. The consensus gate refuses it, and it is
right to.

The `cull` line refuses it a second time, on evidence the consensus never looks
at: all 49 windows fell at the **SNR** gate, meaning not one correlation surface
had a peak distinguishable from what an empty surface produces. Measured, the
SNRs run 5.9 to 8.3 against a noise-alone value of 7.5 for this window size. The
tracked series here are noise, and the 0.504 Hz is the periodogram of noise.

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

### Cross-check the reader against SARPy

The SGN note above is evidence that vendor metadata cannot be taken at face
value, and it was found by comparison with another reader — not by anything in
this repository. Every test here builds its own fixture, and `sim_cphd` writes
the project's private container rather than a conformant CPHD, so **nothing in
the test suite exercises the CPHD parse at all**. A reader that misparses a real
product consistently would be invisible: the pipeline would measure a different
collect and report a confident spectrum for it.

`info --cphd --json` prints the derived quantities the pipeline consumes, at full
double precision, for differencing against an independent parse:

```sh
pip install sarpy
python3 tools/sarpy_crosscheck.py "$C"
```

```
  field                       micromotion                  sarpy   verdict
  ---------------- ---------------------- ----------------------   -------
  n_pulse                          335141                 335141   ok
  n_rbin                              512                    512   ok
  fc_hz                        9300000000             9300000000   ok
  lambda_m                 0.032235748172         0.032235748172   ok
  prf_hz                    10196.3524146          10196.3524146   ok
  dwell_s                   32.8686167733          32.8686167733   ok
  dr_m                     0.249827048333         0.249827048333   ok
  r_near_m                  762749.526717          762749.526717   ok
  r_ref_first_m             762813.482441          762813.482441   ok
  r_ref_last_m              762812.628754          762812.628754   ok
```

That is the whole Giza collect, all 335,141 vectors after the validity screening,
which the script replicates rather than trusts. The fields are chosen so the
pattern of any failure localises the fault: a disagreement in `r_ref` alone points
at the position PVP offsets, in `n_pulse` alone at the validity screening.

By default the script reads 2,000 vectors, which takes seconds and exercises every
parse path; `--all-pulses` does the collect above and reads the full 36 GB.

**What it cannot see:** the signal samples. The SGN convention affects only sample
values, so the one vendor defect this project has actually hit is outside what is
compared. Two readers agreeing here does not mean the image will be right.

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

**`--stream N` removes the ceiling for `focus`.** It walks the collect in blocks
of N pulses and sums them, holding one block instead of the whole phase history.
Backprojection is a sum over pulses, so this is exact, not an approximation — the
streamed image is bit-identical to the monolithic one, verified at three block
sizes on a real collect.

```sh
./build/micromotion focus --cphd scene.cphd --at LAT,LON --stream 16384 --out q.png
```

At `--rbins 1024` on the Giza collect that is 0.12 GB resident against 2.56 GB,
for about 6% more runtime spent re-reading the header and PVP block per block.
16384 is a reasonable default; smaller blocks save more memory and cost more
re-parsing.

**`mmotion --stream N` does the same for the measurement**, which is where the
11–16 GB actually goes. On the Giza collect at `--rbins 4096`, 16 looks over a
64 m grid: **0.92 GB streamed against 6.07 GB resident, identical output** — same
peak, same prominence, same consensus, same window count.

```sh
./build/micromotion mmotion --cphd scene.cphd --at LAT,LON --stream 16384 ...
```

Two restrictions, both refusals rather than silent fallbacks. It needs `--subap
pulse` (the default); the spectral routes focus the full aperture first, which is
the thing that does not fit. And it cannot be combined with `--inject-vib`,
because the injection is written into the phase history in memory and a streamed
read would never see it — a positive control silently missing its injection is
the failure `FOLLOW-UPS.md` item 28 records.

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

**Screen before you download.** `validate --xml FILE` runs the same checks on a
CPHD's metadata block alone — either a whole collect, whose ASCII header says
where the block is, or the extracted block. Two HTTP range requests of about
12 KB decide whether a 17 GB download is worth making, and the 36 GB Giza file
screens in 0.3 s. It cannot measure PRF stability, which lives in the PVP block,
and reports that check as UNKNOWN rather than skipping it. `docs/DATASETS.md` has
the recipe.

**Pass the same `--estimator` you intend to run with.** Four checks answer
differently for different observables, and the default is `correlation`:

| check | correlation | phase |
|---|---|---|
| observable band | the sub-aperture **averaging** ceiling `1/(2·t_sap)` — its observable *is* the averaged position | the **sampling** ceiling `1/(2·dt)`, because it reads sidebands the averaging moves energy into |
| sensitivity | a tracking-pixel artefact floor | **unknown** — that quantity is never formed; defers to the phase floor |
| ambiguity | the pixel wrap ceiling against the artefact floor | the **λ/4 line-of-sight fold**, ~10 mm of vertical amplitude at 40° incidence |
| phase floor | bounds only the phase refinement | *the* sensitivity bound |

On the Giza collect at 90% overlap this is the difference between `VERDICT: FAIL`
and `VERDICT: WARN` for the same configuration — three of the four failures were
the correlator's limits in the correlator's units. `FOLLOW-UPS.md` items 16 and 17.

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
| `sub-apertures: N looks, dt X s` | `1/dt` is your sampling rate; the band is half that. `dt` is often not what `t_sap*(1-overlap)` predicts — read it, don't derive it. **`--overlap 0` is the right setting for the correlation estimator** — see below |
| `observable band f_max ... AT sub-look resolution ...` | the frequency ceiling, and the resolution you paid for it |
| `sub-pixel refinement: 1/N px` | the **quantisation** of the reported series. An excursion under one step comes back as a two-level staircase whose energy sits at low frequency regardless of cause |
| `tracked W windows; K pass the coherence mask` | K of 0 means nothing downstream is a measurement |
| `spectra: B bins, df Hz` | `df` sets what "within half a bin" means |
| `amplitude dispersion: best X, median Y; N of M windows meet D_A <= 0.25` | whether the scene can support `--estimator phase` at all — see below |
| `consensus: f, agreed by A of V windows, D distinct, largest block C` | the detection statistic — see below |
| `persistent scatterers: f from A of N candidates (D_A <= 0.25), best window W at D_A d` | the fourth policy: what only the low-dispersion windows say — see below |
| `cull: f from A of N windows surviving (SNR x, sigma y, neighbours z removed)` | a **second, independent** selection policy — see below |
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

### Leave `--overlap` at zero, unless you change observable

The sampling ceiling and the sub-aperture response ceiling move differently with
overlap, and their ratio depends on neither the look count nor the dwell:

```
Nyquist / response-0.5  =  0.829 / (1 - overlap)
```

At `--overlap 0` those two ceilings coincide to within 20% — the configuration is
balanced. Every overlap above zero raises the sampling rate by `1/(1-overlap)`
while leaving the response ceiling exactly where it is, so the extra rate is spent
on frequencies the sub-aperture has already averaged away. Measured across 25
sweep points: everything with a response of 0.608 or better recovered the
injection, everything at 0.481 or worse failed, no exceptions.

Concretely — 2048 looks at 0.98 overlap gives 52 Hz of Nyquist band of which
**1.27 Hz is usable**, against zero overlap's 3.20 Hz band with 3.86 Hz usable.
Forty times the compute for a narrower measurement.

The published campaigns *do* use ~99% overlap and are right to: they read pixel
**phase**, which is not subject to this attenuation, where `--estimator
correlation` tracks displacement, which is exactly what the averaging attenuates.
So high overlap is a reason to change `--estimator`, not a reason to raise `--n`.
`FOLLOW-UPS.md` item 13 has the measurements.

### The amplitude-dispersion line says whether a null means anything

`--estimator phase` needs **one dominant scatterer per sub-look resolution cell**
(`FOLLOW-UPS.md` item 15). Amplitude dispersion `D_A = σ_A/μ_A` of each window's
brightest pixel is the standard measure of that — Ferretti et al.'s
persistent-scatterer statistic, criterion `D_A ≤ 0.25`.

It separates the synthetic fixtures cleanly: those where phase recovers reach
0.079–0.084 and have windows meeting the criterion; those where it fails bottom
out at 0.381–0.397 with none. **On the real Giza collect the best window was
0.381 and none met the criterion** — so that null was what an unmet precondition
guarantees, not evidence about the pyramid.

So read this line first. `0 of M windows` under `--estimator phase` means a null
below tells you nothing about the ground. It is reported for every estimator,
because its most useful message is *switch estimator*.

The 0.25 criterion was calibrated over independent passes, not sub-looks of one
aperture, so read it as a scale rather than a bright line; the measured gap
(0.084 against 0.381) is wide enough that the distinction has not yet mattered.

### The `cull` line reads different evidence

The consensus reads the spectrum. The `cull` line reads what the **correlator**
knew, which is information neither of the other two policies looks at. It is
modelled on the culling that ISCE's `ampcor` and GMTSAR's `xcorr` have always
applied to offset fields, and it drops a window unless all three hold:

| gate | test | where the threshold comes from |
|---|---|---|
| surface SNR | peak power ÷ mean off-peak power ≥ 2× the noise-alone value | the noise-alone value is the harmonic number of the window's bin count — about 7.5 for a 32×32 window — so the gate is a multiple of what an *empty* surface produces. The ×2 is swept in `FOLLOW-UPS.md` item 12d: at or below ×1 the cull answers on a **static** scene, and 1.75–3.0 are indistinguishable, so 2 sits on a plateau rather than an edge |
| offset uncertainty | σ ≤ 2× the median σ of the windows entering the cull | relative, because σ is a *ranking* statistic with no absolute scale — an absolute form was tried and removed every window at every operating point (`FOLLOW-UPS.md` item 12c). This is ampcor's median-based rejection applied to the covariance |
| neighbourhood | ≥ 2 of the 4 lattice neighbours report the same bin, counting any window that entered the cull | each cell of a 2×2 block has exactly two in-block neighbours, so this is "belongs to a block or better". Neighbours vote whether or not they passed the first two gates — the bound describes a target's footprint, not the neighbours' reliability. Swept in item 12e: below 2 the cull answers on a **static** scene, above 2 it answers nothing at all — the derived value is the only usable one |

Two tuned factors of two, both written into `PREFIX_windows.csv` so a result never
depends on knowing them; the rest is derived. Both are reachable without editing
the source through `rs_spectrum_ampcor_window_opts()`, which is how the SNR one
was swept.

**Its measured profile is high precision and low recall.** On a swept fixture
(`tests/test_cullsweep.c`, 6 frequencies × 3 clutter seeds plus an isolated-point
control) it answered 7 times out of 24 and was **right every time**, to within
half a bin, while `best` and `consensus` answered everywhere and missed badly
(rms 0.24 and 0.37 Hz against a 0.025 Hz bar). It refused all three static
controls, where the other two each reported a confident frequency. But its
answers cluster at the bottom of the band — five clutter answers over **two**
distinct injections — so it has **not** been swept in the sense `README.md`
means, and none of this is yet a recovery.

**Use it as a disagreement detector, not as a better answer.** When the cull and
the consensus report the same frequency that is weak corroboration from two
different kinds of evidence. When they disagree the tool says so, and at most one
of them is measuring:

```
  NOTE: the cull and the consensus disagree (1.008 vs 0.504 Hz).
        They read different evidence, so this is not a tie to
        break by preference: at most one of them is measuring.
```

`cull: no window survived` is a result, not a failure — the per-gate counts say
which gate removed the population, and "every surface was indistinguishable from
noise" is a more informative null than a scattered consensus.

**It is not a null control either.** `FOLLOW-UPS.md` item 11 applies here
unchanged: an artefact produced by the processing has a genuine, sharp,
well-determined correlation peak behind it in every window, and passes all three
gates. Nothing but a motionless scene through identical processing catches that.
The cull narrows which windows are believed; it does not decide whether the
ground moved.

The `snr` and `sigma_px` columns of `PREFIX_windows.csv` carry the per-window
numbers, so an alternative policy can be scored against a finished run without
reprocessing the collect.

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
NO FREQUENCY REPORTED: 2 of 30 motionless realisations reached prominence 6.3
  through the identical chain, so the peak is not evidence of motion.
```

A scene known to hold nothing produced the same measurement. Only `--null-static`
can say this, and it is the only refusal in the tool that means "not motion"
rather than "not resolvable".

**The verdict is a p-value against an explicit alpha.** `p = (1 + #{null ≥
measurement}) / (M + 1)` — the finite-sample conformal form, no Gaussian
assumption, and calibrated for the whole search because each null trial
contributes its own best window, so both sides are a maximum over the same
window and frequency grid.

`--null-alpha F` sets the bar, default **0.05**. The smallest p that `M` trials
can produce is `1/(M+1)`, so alpha needs `M ≥ 1/alpha − 1` trials: **19 at 0.05,
99 at 0.01**. Below that the tool says so and reports nothing, rather than
stamping a verdict on evidence that cannot reach the threshold:

```
NOT ADJUDICATED -- 5 null trials give a smallest possible p of 0.167, and
--null-alpha 0.05 needs p <= 0.05. Run at least 19 trials, or raise --null-alpha.
```

That is deliberate. A single null answers "did this realisation reach it"; it
cannot establish a false-alarm probability, and the gate used to pass it anyway.

**This gate used to be a scene-wide agreement fraction and no longer is.** It
refused whenever fewer than a third of windows shared the consensus frequency,
which is what a *global* signal produces — the fixtures it was calibrated on make
the whole scene vibrate. A localised target cannot reach it: across the five
injected Giza runs, agreement ran 19–24% for **true positives** and all five were
refused with the words *"which is what a MOTIONLESS scene produces"* over a scene
that was not. `FOLLOW-UPS.md` items 30 and 31.

Without a null you now get the frequency and an explicit `NOT ADJUDICATED`:

### The joint transient-and-mode fit

`--tfit N` replaces the periodogram with a fit of N exponentially damped
sinusoids **with onsets**, to the **unwindowed** series:

```
JOINT TRANSIENT-AND-MODE FIT: up to 3 damped modes per window, no window
  applied. Fitted 49 of 49 windows, 147 modes total; median damping 0.0051,
  median onset 1.24 s, median residual 0.496 of the window's variance.
```

The point is that a Hann window applied to a burst removes the part of the
record that carries it, so for a short transient the field fits the transient
**together with** the modal parameters rather than tapering it off. Unlike a
periodogram this reports **damping**, which no spectrum can.

Two things to know before reading a number from it.

**The damping ceiling is frequency-dependent.** The grid runs to a decay of
`RS_TFIT_DECAY_MAX` across the record, so the largest representable damping is
`zeta_max = RS_TFIT_DECAY_MAX / (2*pi*f*T)` — 0.080 at 0.8 Hz over a 20 s dwell,
0.021 at 3 Hz over the same one. A mode damped harder comes back **at** the
ceiling, so a zeta sitting on it means *at least that*, not *that*.

**The spectrum it writes is the fit's, not the data's.** Its floor is the fit
residual and its lines are narrow because damping is a fitted parameter rather
than a smear left in the data. Do not compare its levels with a periodogram run.

`FOLLOW-UPS.md` item 81 is the caveat, and it is a large one: **at the chain
level this is not better than the periodogram** — 2 of 12 injected frequencies
recovered against the periodogram's 3 of 12 on the same sweep, with both static
controls still answering. It is opt-in and it is not a result. `--tfit` and
`--stft` both replace the spectrum and cannot be combined.

### The stabilization test

`--stable CSV[,CSV...]` compares this run against other runs of **the same scene
at different look counts** — a LADDER, one rung per file plus this run — and
reports the **longest chain of consecutive agreeing rungs**:

```
  stabilization ladder, 6 rungs over 62 admissible bins:
               96 looks:  0.502 Hz
              128 looks:  0.504 Hz   <- this run
              160 looks:  0.500 Hz
              192 looks:  0.508 Hz
              224 looks:  0.510 Hz
              256 looks:  0.504 Hz
            longest chain of CONSECUTIVE agreeing rungs: 6 (p 0.0000) -> STABLE -> report
            25 of 37 comparable windows agree within half a bin (0.0252 Hz) at
            96 looks; the verdict above is on the MODAL SET, not on windows.
```

`stable_hz` and `stable` join the per-window CSV (1 stable, 0 moved, -1 not
comparable).

**READ THE CHAIN LENGTH, NOT THE p** (item 115). The p is derived assuming each
rung is an independent draw over the band, and rungs are **not** independent —
every one re-divides the *same pulses over the same dwell*. Measured, a
motionless scene held ~0.95 Hz across four consecutive look counts, which the
model prices at 1.3e-5 and which happened on 1 of 12 scenes. The chain length is
sound; the probability attached to it is not.

**Why it works.** A real vibration sits at the same Hz however the aperture is
sliced; an artefact produced by the slicing need not. This is the
operational-modal-analysis *stabilization diagram* with the look count standing
in for model order. Measured, it takes a motionless fixture from **12 of 12 false
positives to 1 of 12** while keeping **6 of 6** injected scenes
(`FOLLOW-UPS.md` item 107). As a ladder it also gives a **definite verdict on
12 of 12** motionless scenes where the two-point form managed 1 (item 115).

**Why it matters more than the other controls here.** `--null-static` needs a
simulated motionless realisation; `--twin` needs a run differing in nothing but
the motion; the zero-amplitude control needs the injection machinery. **This needs
only the collect, processed twice** — so it is the only one available on a single
real acquisition.

**The verdict is on the MODAL SET's leading frequency**, not on any one window's
`dominant_hz`. Those differ: an injected scene whose modal answer is 0.504 Hz at
both look counts can have its strongest window report 0.504 at one and 2.571 at
the other, so comparing windows would reject a true recovery. That makes this
test downstream of the modal set's ranking — when item 110 changed the ranking,
these figures were re-measured on the same twelve scenes and came out identical,
1 of 12 and 6 of 6, with a *different* scene surviving. Item 111 re-measured them
again after fixing a band-edge bias in the nomination and got 1 of 12 and 6 of 6
once more — but the ABSTENTIONS fell from 5 of 12 to 3, because fewer answers
land on band-edge bins whose 256-look partner falls above the 128-look Nyquist.
The rate is what the test is quoted at; the abstention count is what changed.

**It refuses what it cannot test:** equal look counts (vacuous), a different
window grid, or a missing file. It warns when `df` differs, since that is a
comparison across *dwells* rather than look counts, which item 107 records as
untested. Frequencies above the lower run's Nyquist are marked not comparable
rather than unstable.

**It gates nothing**, like every other statistic here.

### The predicted floor

Every phase-route run now prints the smallest displacement each window could
detect, computed from that window's own phase noise:

```
  predicted floor, per window (phase route):
            strongest-prominence window 96: 0.0157 mm (phase sd 0.049 rad)
            scene median 0.5400 mm | quietest window 0.0157 mm | ratio 34.4x
```

`floor_mm` joins the per-window CSV.

**Why per window and not per scene.** A scene carries both bright scatterers and
diffuse clutter, and their phase noise differs by more than an order of
magnitude. On a real Kilauea collect the window holding a bright injected target
measured **34x quieter** than the scene median, so a scene-median floor
overstates that window's by the same factor — which is exactly the error
`FOLLOW-UPS.md` item 103 records.

**Three floors, and this is one of them.** The *target* floor (this number), the
*clutter* floor (the scene median), and the *competition* floor — the amplitude
at which a signal beats the scene's own strongest artefact, which ran 8–17×
higher on real clutter. **The competition floor is the operative one.** Say which
you mean.

**It is only a floor where nothing is injected.** A window containing real motion
has that motion in its circular sd, so the number returned is inflated —
conservative, but not the floor.

### The twin difference

`--twin PATH` differences this run against a previous one's `_windows.csv`, at
the frequency `--probe-hz` names. It requires `--probe-hz` and refuses without it.

```
  twin LLR at 0.7500 Hz: best 5.260 at window 35 (5,0), exact p = 0.0013
            Two degrees of freedom per bin, so the power RATIO must exceed 19
            before p < 0.05 -- a single-look pair cannot call a modest excess,
            whatever the difference below looks like. Multilooking is the remedy
            and this run has no independent looks to give it (item 98).
            10 of 49 windows reach p < 0.05, against 2.5 expected by chance:
            the per-window p is UNCORRECTED for testing 49 of them (item 1).
  twin difference at 0.7500 Hz, against tw2_windows.csv:
            35 of 49 windows gained; median +1.601 (scene-wide), best +21.025 at window 4 (0,4)
            excess of best over median +19.425 -- that is the LOCALISED part;
```

**Two statistics, and the LLR is the principled one.** The change-detection
literature ranks the log likelihood ratio above a difference (item 98). A
periodogram bin is exponentially distributed about the true PSD, so for the pair
`r = P_injected / P_twin` gives

```
    LLR = 2*log((1+r)/2) - log(r)        one-sided, zero when r <= 1
    p   = 1 / (1 + r)                    EXACT, the F(2,2) tail
```

The LLR is **scale-free** — only the ratio matters — where the difference is
not. `twin_llr` and `twin_p` join `twin_delta` in the CSV.

**The ceiling is the thing to read.** With two degrees of freedom per bin the
ratio must exceed **19** before `p < 0.05`. A single-look pair therefore cannot
call a modest excess, however large the raw difference looks. Multilooking is the
remedy and this chain has no independent looks to offer it.

**The p-value assumes the twin is an INDEPENDENT observation**, i.e. a second
acquisition whose noise realisation differs. A twin from a deterministic
simulator at the same seed is bit-identical — measured — so under the null the
ratio is exactly 1 on every window rather than F(2,2)-distributed, and the
p-value is then grossly conservative: there is no noise floor and *any* ratio
above 1 is the motion. `mmotion` detects that case (windows whose ratio is 1 to
within the evidence file's own precision) and says so.

**Do not read the per-window p as corrected.** It is not; the line reporting how
many windows clear 0.05 against how many are expected by chance is there because
testing 49 windows is the look-elsewhere effect (item 1), and this project has
had that open since the beginning.

**Why it exists.** `FOLLOW-UPS.md` item 96 measured a **100% false-positive rate
on motionless clutter** — twelve of twelve scenes returned a confident frequency
and nine were distinct, because every clutter realisation carries its own
residual carrier. Item 97 then measured that differencing against a twin of the
**same scene** recovers the injected frequency in **76% of the runs whose
reported frequency is wrong**. The measurement survives what the report loses.

**Read the excess, not the total.** The median is what the whole scene did; the
best window's excess over it is the localised part. On a whole-scene fixture
(`--clutter-vib`) a large median is expected behaviour, not a fault.

**The pairing must differ in nothing but amplitude.** Paired against an
*uninjected* run rather than a *zero-amplitude* one, the difference measures the
scatterer's presence and exceeds a real signal — item 39. `mmotion` refuses a
pairing probed at a different frequency or over a different grid, but it cannot
tell whether the twin is the right kind of control; that is on you.

**It gates nothing**, like consensus and contiguity. And on a real collect there
is no twin to pair against, which is why `--null-static` exists.

### The modal set

A structure does not have *a* frequency, so `mmotion` also prints a **modal
set** — every bin whose nominating windows form a block chance does not reach,
ranked by how much evidence that block carries:

```
  modal set: 1 mode recurring in >= 4 of 49 voting windows,
           admitted by the size of their largest contiguous block, ranked by
           that block times log of how far they stand above their own background
           (6 nominations each over 62 admissible bins. A scene-wide mode
            would need support 11 for 0.39 bins expected by chance -- that is
            REPORTED, not required: item 110's localised target reached 28 of
            225 against a threshold of 34 and its block said mode anyway)
           chance alone reaches evidence 54.0 here (1000 trials, worst 81.7), so what
           a mode must beat is derived from this configuration, not fixed. The null
           SHIFTS 2x2 TILES of windows, preserving the correlation that window
           overlap creates -- an independent draw understates it about tenfold
            0.504 Hz (sub-bin 0.502 +- 0.006)   block  30 (p 0.001)   support  35/49   ratio 126.0   ev 145.1
```

That is a 2 mm injection at a true 0.500 Hz, and **one mode is admitted.** Before
item 113 the same scene reported four, led by the same correct answer but with
the 1.512 Hz artefact — the sharpest line in the band at ratio 34.2 — third.
The gate now tests `ev` against a null that models the correlation window overlap
creates, and the competition does not clear it.

Nothing there is tuned. The block floor of 4 is the window geometry, since
windows are laid at half their width so a resolvable target falls in a 2×2 block
at minimum; the same 4 is the support floor, because four windows are what a
block of four needs; and the `p` beside each block is a Monte Carlo over the
band — nominations reshuffled a thousand times, the statistic being the largest
block *anywhere*, so the look-elsewhere cost is inside it.

**The support figure is reported and does not gate** (item 110). The binomial
threshold beside it — `support 11` in the run above — is a correct family-wise
budget and the wrong question for a *localised* target, because it is a fraction
of the whole window grid. On a real collect with a point target injected, the
true line reached 28 windows of 225 against a threshold of 34 and was refused
before anything looked at its shape, while its block of 13 stood against a chance
model reaching 9. Read `support` as a description of how much ground carries the
mode, never as a verdict.

**`ev` is the ranking key**: the block times the log of the ratio, which is what
you get by adding up each carrying window's evidence under the exponential model
`rs_twin_llr()` states exactly. Neither factor works alone, and both failures are
measured: the block alone loses the localised target above, and the ratio alone
hands this very fixture to the 1.512 Hz artefact — sharper than the true line, on
a third of the ground. It is an **ordering** statistic only; overlapping windows
are not independent looks, so do not exponentiate it or read it as a likelihood
ratio.

**`ratio` is the median over the windows of the mode's BLOCK, not over every
window that nominated the bin** (item 112), and the difference is large. Each
window nominates six bins wherever they fall, so on a 62-bin band about **22 of
225 windows nominate any given bin from noise alone** — more than half the
support of a typical reported mode. Summarising over all of them measured the
background: planting a line at five times the amplitude moved the old figure
from 5.97 to 6.39, **seven percent**. Over the block it moves from 14.48 to
67.74. On the real collect the injected line's `ratio` went from 9.0 to 40.3 at
the same amplitude while the competition barely moved, which is what took recall
from 5 of 6 to 6 of 6.

**The chance line is the one to read first.** That floor of 4 is a floor and
never a separator (item 77): the number of admissible bins falls with the look
count, so windows agree by accident more often, and what chance reaches moves
with the configuration. The chance model prices that automatically, which a fixed
constant cannot.

**And the null it prices against is a PERMUTATION, not an independent draw**
(item 113). Windows are laid at half their width, so neighbours share half their
pixels and their nominations are correlated — measured, an adjacent pair shares
2.37 of 6 nominations against 0.71 for a random pair. Drawing them independently
understated what chance reaches by about tenfold, which is
[the documented failure mode of cluster-extent inference in neuroimaging](https://www.pnas.org/doi/10.1073/pnas.1602413113).
The null now shifts 2×2 tiles of windows, keeping the correlation inside a tile
and destroying it across tiles.

**Read the block and the evidence, not the support.** A noise line can be
nominated by as many windows as a real mode — measured, 12 of 49 each — and loses
only on shape and strength. When nothing clears the gates the refusal says which
one spoke, and that distinction is the useful part: *short of the support needed*
now means fewer than four windows nominated it at all, so the tracker never
carried the frequency, while a block *chance reaches anyway* means it did and the
shape is not evidence.

**The leading frequency is the bin centre**, which is what every measurement in
`FOLLOW-UPS.md` is quoted at. The sub-bin figure interpolates the peak per window
and reports the **spread** across the nominating windows — dispersion, not a
posterior standard deviation. `FOLLOW-UPS.md` item 79 is the caveat: the modal
analysis literature fits a modal model and gets a genuine posterior covariance
including damping, and this does neither.

It gates nothing, and item 71 is the other caveat: on a real, non-stationary
record it correctly refuses rather than recovers, because the mode is not in the
per-window spectra to be selected.


```
strongest peak in window 112: 0.163 Hz, prominence 31.4, quality 0.444, ...
  backed by 32 windows, largest touching block 9
  NOT ADJUDICATED -- nothing here distinguishes this from an artefact of the
  processing. Run --null-static N, or --inject-vib to check the chain can see
  a known signal here at all.
```

That reports more often than the old gate did, deliberately. The old refusal was
not a safety property: it rejected every true positive this project has produced
on real data while still passing item 11's artefact at 100% agreement.

---

## 7. Making a result credible

Six things, in increasing cost. A result without at least the first three is not
worth reporting — step 0b was added after a zero-amplitude control showed that a
positive control alone can be passed by a target that never moves.

**0. A positive control, if the answer is a null.** `--inject-vib
FREQ_HZ[,AMP_MM[,REL]]` adds a scatterer of known frequency and amplitude to the
real phase history *before* sub-aperture formation, so the identical chain runs
over it with the real clutter, coherence, orbit and look geometry.

```sh
./build/micromotion mmotion --cphd scene.cphd ... --inject-vib 1.0,2.0,20
```

**This is the check that decides whether a null means anything.** A null cannot
distinguish "nothing in this scene moved" from "this chain cannot see motion in
this data" — they produce identical output, and every real collect this project
has processed returns it. If the injected frequency comes back, a null elsewhere
in that scene is evidence about the ground. If it does not, the null was only ever
evidence about the pipeline.

**A sine is the easiest possible motion, and the reported answer fails on
anything harder.** `--inject-wave FILE[,RATE_HZ[,AMP_MM[,REL]]]` drives the same
target from a measured displacement record instead — one sample per line, in
metres, centred and rescaled so `AMP_MM` still means peak displacement and the
two runs are comparable. `sim_cphd --wave FILE[,RATE_HZ]` does the same for a
whole `--clutter-vib` patch, which is the fixture that recovers.

```sh
./build/sim_cphd w.cphd 0.5 0.02 --clutter 400 --clutter-vib --seed 7 \
    --wave "almanor_hnz_20s.txt,41.6667"
```

On a real accelerometer record with six modes within a factor of two, the scene
and processing that report a sine correctly at 0.504 Hz against 0.500 report
1.966 Hz with no mode within 1.3 Hz (`FOLLOW-UPS.md` item 69). Prominence cannot
win when the energy is split across modes, so it goes to the tallest noise line.
**Read the windows CSV against the record's own spectrum, not the reported
frequency** — there is no single frequency for a structure to be right about.
If the record is shorter than the dwell the target is held still for the
remainder, which looks exactly like a null; `mmotion` warns when that happens.

Defaults are 2 mm of vertical displacement at 20× the scene's median non-zero
sample magnitude. Keep the *projected* amplitude below about λ/8 for
`--estimator phase`: the observable wraps beyond λ/4 and an injection that wraps
tests nothing. `REL` is deliberately bounded and relative — `FOLLOW-UPS.md` item
21 records a conclusion that had to be retracted because an unbounded injected
gain let outliers rather than the mechanism carry the result.

**A positive control on its own is not evidence, and this is measured rather
than cautionary.** Run the same command with `AMP_MM` set to `0.0` — a bright
scatterer added to the phase history that does not move — and on the Giza collect
it produces a *larger* peak than any real injection (prominence 56.3 against
38–47), clearing the same `--null-static` controls. Prominence, the null control
and the scene-derived null all rank the motionless target first. Only the
reported **frequency** separates them. See `0b` below for how to compare the two,
and `FOLLOW-UPS.md` items 38 and 39.

Run it as a **separate** run from the measurement. The injected scatterer is in
the data, so the reported answer is the injection — on the quick-start fixture,
which itself vibrates at 0.5 Hz, injecting 1.2 Hz gives:

```
POSITIVE CONTROL: injected a 1.2000 Hz scatterer at the grid origin,
  2.000 mm vertical displacement, 20.0x the scene's median non-zero
  sample magnitude (0.6925).
  deposited into 8000 of 8000 pulses, range bins 128.0 to 128.0 of 256
  THIS RUN IS ABOUT THE PIPELINE, NOT THE SCENE: if 1.2000 Hz does
  not come back, a null anywhere in this collect is not evidence
  that the ground is still.
  injected 1.2000 Hz sits at sub-aperture response 0.944 (observation ratio 0.19)
sub-apertures: 128 looks, dt 0.1550 s
  observable band  f_max 3.23 Hz   AT sub-look resolution 8.26 m
sub-pixel refinement: 1/200 px (default 1/10 azimuth, 1/20 range)
tracked 49 windows (7 x 7); 49 pass the 0.00 coherence mask
spectrum taken of line-of-sight DISPLACEMENT, which is what the phase estimator measures directly
spectra: 65 bins, 0.0504 Hz resolution
  amplitude dispersion: best 0.051, median 0.502; 9 of 49 windows meet D_A <= 0.25
  persistent scatterers: 1.210 Hz from 9 of 9 candidates (D_A <= 0.25), best window 30 at D_A 0.051
strongest peak in window 30: 1.210 Hz, prominence 38.3, quality 0.949, peak-to-peak velocity 25.1 mm/s
  consensus: 1.210 Hz, agreed by 11 of 28 voting windows (39%), 10 distinct answers, largest contiguous block 9
  cull: 1.210 Hz from 9 of 28 windows surviving (SNR 0, sigma 0, neighbours 14 removed), surface gates N/A
```

**Read the two accounting lines before the answer.** `deposited into 8000 of
8000 pulses` is the check that the injection happened at all — a target whose
range falls outside the loaded window deposits nothing, and a null with no
injection in it looks exactly like a null the chain could not see. And
`sub-aperture response 0.944` is the check that this operating point can see the
frequency you chose: below about 0.5 the sub-look averages the motion away and
the tool says so. **The `f_max` line is the Nyquist limit, not that ceiling** —
they differ by `0.829/(1-overlap)`, a factor of 8.3 at 90% overlap, which is how
a 1.0 Hz injection was once made into a scene whose ceiling was 0.25 Hz.

All four selection policies return 1.210 Hz against a 1.2 Hz injection, inside
the 0.0252 Hz half-bin — and the scene's own 0.5 Hz is nowhere in the output,
because a scatterer at 20x the median swamps it. That is the control working, and
it is also why the run tells you nothing about the scene.

**0b. The zero-amplitude twin, and `--probe-hz` to compare against it.** The two
runs differ in nothing but amplitude, so anything they share is the injection
machinery rather than the motion.

```sh
./build/micromotion mmotion --cphd scene.cphd ... --probe-hz 0.163 \
    --inject-vib 0.163,0.5,20 --out real
./build/micromotion mmotion --cphd scene.cphd ... --probe-hz 0.163 \
    --inject-vib 0.163,0.0,20 --out zero      # the twin: same code path, no motion
```

`--probe-hz` adds `probe_psd` and `probe_prominence` to every row of
`PREFIX_windows.csv`, measured at that one frequency for every window. **The
dominant-peak columns cannot be differenced between runs** whose strongest peaks
sit at different frequencies, which is exactly how the motionless target won
above. Subtract the twin's `probe_prominence` from the real run's, window by
window. On the Giza collect at 0.5 mm:

```
  win  8   +5.15     <- the injected window
  win 18   +5.06     }
  win 17   +5.06     }  its aliasing ghosts, from a 1.0 m cell against a
  win 13   +5.06     }  0.051 m azimuth resolution -- mmotion warns about this
  MEDIAN over 25 windows:  +0.00
```

The median window gains **exactly nothing**, which is the point: the injection
machinery contributes zero at the probed frequency, so what is left is the
motion.

**The pairing must differ in nothing but amplitude.** Difference the *uninjected*
run against the zero-amplitude twin instead and you get +7.71 at the injected
window and +18.35 elsewhere — larger than a real 0.5 mm signal — because those
two runs differ by the scatterer's presence, not its motion. Paired wrongly the
statistic is worse than useless, since it looks like a stronger result.

This validates injection experiments and **transfers nothing to a real target**:
a collect with a suspected vibrating object has no zero-amplitude twin to
subtract.


**1. A null control.** `--null-static N` runs the identical processing over a
scene known to be motionless. This is the only check that catches common-mode
artefacts, and it is what `README.md` names as the credibility check that
matters.

```sh
./build/micromotion mmotion --cphd scene.cphd ... --null-static 5
```

**Budget for it.** Each trial re-simulates and refocuses the whole stack, so the
control costs far more than the measurement: on the synthetic fixture at 128
looks and `--upsample 200`, the run itself takes about 9 s and `--null-static 5`
takes **23 minutes**. Measured, not estimated — see
`runs/synthetic/2026-08-01-e2e-check/`. Plan the trial count accordingly rather
than discovering this mid-run, and do not let the cost become a reason to skip
the check; it is the only thing that catches a common-mode artefact.

**`--null-static` numbers produced before 2026-08-02 should be recomputed.**
`rs_simulate_static_like()` wrote each scatterer's phase against one reference
while telling the focuser to undo another, so the simulated scene never focused —
peak-to-mean 3.6 where the same scatterers reach 93.7. The null was a field of
noise rather than a motionless scene. Fixed and now tested; the direction of the
old error is not assumed, and `FOLLOW-UPS.md` item 27 has the detail.

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

**Free, and worth reading first: the `cull` line.** It costs nothing — the
statistics come from correlations the tracker already performed — and it fails
early and loudly when the correlation surfaces are noise, which is the case where
items 1 to 3 are about to be spent on nothing. It is *not* a substitute for the
null control: it is blind to the same common-mode artefacts the consensus is
blind to, for the same reason.

---

## 8. What gets written

With `--out PREFIX`:

| file | contents |
|---|---|
| `PREFIX_freq.png` | dominant frequency per window, with a colour bar in Hz |
| `PREFIX_quality.png` | tracking quality per window, colour bar 0–1 |
| `PREFIX_scene.png` | **the scene the measurement was taken from**, with the tracking grid on it and the selected window boxed |
| `PREFIX_spectrum.png` | **the spectrum the reported frequency was read from**, with a marker at the selected bin |
| `PREFIX_spectrum_mm.png` and `PREFIX_spectrum_mms.png` | the same spectrum as a **velocity in mm/s**, always, whatever the estimator measured — the phase route's observable is displacement, so it is converted with `v = 2*pi*f*A` rather than relabelled. For placing a result against the literature's envelope. Calibrated: reads 0.90–0.94× a known synthetic injection and scales linearly, the shortfall being Hann scalloping. Still labelled qualitative — the independent assessment of this method reports 40–76% time-domain error while getting every dominant frequency right |
| `PREFIX_windows.csv` | **per-window evidence behind the selection** — every window's frequency, prominence, quality, excursion, correlation SNR and offset uncertainty, whether it passed the gates, whether it agrees with the consensus, and whether it survived the cull |

The two maps are figures, not raw rasters: the window grid is enlarged by an
integer factor with nearest-neighbour sampling and carries a labelled colour bar,
so a colour can be converted to a number without opening the CSV. Nearest
neighbour is deliberate — windows overlap already, and a smoothed image would
imply a spatial resolution the data does not have. Every solid block is exactly
one window, and the caption underneath says how many windows and how many
screen pixels each was drawn as.

**`PREFIX_spectrum.png` is the one to look at first, `PREFIX_scene.png`
second.** The whole selection
argument is about which bin won and by how much over its neighbours, and that is
invisible in a reported number and in a per-window map alike. The red dashed
marker is where the peak-picker landed, so a disagreement between the marker and
the visible peak is legible at a glance. A peak that barely clears its
neighbours, or a marker sitting beside a taller bin, tells you more than the
frequency does.

### Reading the axes and colour bars

Each figure labels its own units — `HZ`, `0-1`, and `POWER, (M/S)^2/HZ` or
`POWER, M^2/HZ` depending on the estimator. What each axis means, and what it
does not say:

**`PREFIX_spectrum.png`.** One window's spectrum, the one named in the title.

- *x, `FREQUENCY, HZ`* — spans 0 to the Nyquist of the **sub-look rate**, not the
  PRF: `fs = 1/dt` with `dt` the time between sub-apertures, so the axis ends at
  `fs/2` and the bin spacing is `df = fs/n_looks`, printed in the header of
  `PREFIX_windows.csv` as `df_hz`. Nothing about the collect's pulse rate
  appears here. **The first three bins are always excluded from peak-picking**,
  as is anything below `--fmin`, but all of them are still drawn — so a tall
  spike at the far left is normal and is one the picker was told to ignore. It is
  usually a residual trend, and it is often the tallest thing in the plot.
- *y, `POWER, (M/S)^2/HZ`* — the one-sided **power spectral density** of that
  window's tracked series: `|X(f)|²` normalised by the Hann window's power and by
  `fs`, with interior bins doubled for the one-sided fold. Summing the curve
  times `df` returns the variance of the detrended series, which fixes the units.
  Which series it is depends on the estimator, and so does the label:

  | estimator | series transformed | y axis label |
  |---|---|---|
  | `correlation` (default), `splitband` | line-of-sight **velocity**, `v_r = dx·V/R` | `POWER, (M/S)^2/HZ` |
  | `phase` | line-of-sight **displacement**, `d = -ψλ/4π` | `POWER, M^2/HZ` |

  So the figure states which observable produced it, and `mmotion` also prints a
  line saying so when the estimator is `phase`. The split is deliberate: `phase` measures
  displacement directly, and taking its spectrum through velocity would
  differentiate the series first, multiplying every component by its own
  frequency and tilting a flat noise floor blue. Measured on the Giza control,
  that put the median dominant frequency at 20.7 Hz with 47 of 49 windows above
  12 Hz, against 0.53 Hz and none above 12 Hz through displacement. The 20.7 Hz
  was an artefact of differencing.
- *The absolute height is not calibrated.* Read ratios inside one plot, never the
  number itself and never a height against another run's. Patch size biases the
  tracked magnitude systematically, and `--reference adjacent`/`lag` are first
  differences whose `|2·sin(π·f·lag·dt)|` response rescales the axis as a
  function of frequency. This is the same limitation that makes the `amplitude`
  field qualitative — see `microm.h`. What the axis is *for* is prominence: peak
  over the surrounding floor, which is the statistic `rs_spectrum_best_window()`
  selects on. That is why y starts at zero rather than at min(y); a cropped
  baseline would manufacture prominence the data does not contain.
- *The red dashed line* is the picked bin, not the truth.

**`PREFIX_freq.png`.** Dominant frequency per window, colour bar in Hz.

- Rows run along **azimuth**, columns along **range**; window `w` in the CSV and
  in the spectrum title is at row `w / n_win_rg`, column `w % n_win_rg`, matching
  the `iaz`,`irg` columns of `PREFIX_windows.csv`.
- The bar is **autoscaled to this map's own min and max**, so an identical colour
  in two runs means nothing — always read the numbers on the bar. Viridis runs
  dark purple at the low end to yellow at the high end.
- **Every window is coloured, including the ones the gates and the consensus
  rejected.** The map is not a filtered result and it is not the answer; a
  patchwork of unrelated colours across the grid is what a failed measurement
  normally looks like. `passed_gates` and `agrees_with_consensus` in the CSV are
  what separates them.
- A non-finite value clamps to the bottom of the ramp, so the darkest block can
  be a genuine minimum or a window with no answer at all. The CSV disambiguates.
- **A window masked by `--coherence` has its series zeroed, not marked absent.**
  Its spectrum is then flat zero and the peak-picker returns the lowest
  admissible bin, so masked windows all report exactly `3 * df_hz` (or the first
  bin above `--fmin`, whichever is higher). A patch of identical low-frequency cells at that one value is
  a mask, not a measurement — check `quality` and `passed_gates` in the CSV. The
  same thing happens to the whole map if the collect carried no usable geometry:
  the azimuth-shift-to-velocity conversion is skipped rather than guessed when
  `az_spacing`, platform speed or slant range is missing, which leaves every
  velocity series zero.

**`PREFIX_quality.png`.** Mean correlation-peak value per window, in [0,1]. This
bar is **fixed at 0–1**, not autoscaled, so unlike the frequency map it is
comparable across runs. It is the quantity the `--coherence` gate thresholds.

**`PREFIX_scene.png`.** The image everything else is an abstraction of. Without
it a window index is unanchored: nothing says whether the window carrying the
reported peak sat on a target, on clutter, or on the edge of the patch.

- It is the **reference sub-look** — look 0, the one the tracker correlated
  every other look against — not a full-aperture focus. The window grid is
  defined on this image's dimensions, so the overlay lands where the windows
  actually were. In the `pulse` route the stack is not a decomposition of a
  full-aperture image at all, so there is no other image the grid would fit.
- *The blue lattice* marks where windows **start**, so its spacing is the
  stride, which is half the patch by default. *The red box* is the selected
  window's **full patch**, so it is twice the lattice cell and overlaps its
  neighbours — that is not a drawing error, it is what the tracker did. The
  caption gives both numbers.
- *The axes are metres from the grid centre*, azimuth across and range down, so
  zero is the point `--at` or `--offset` selected rather than a corner of the
  array. They appear only when the caller passes the cell size; a scene whose
  spacing is unknown gets no metre scale, because a guessed one reads as
  authoritative.
- *Greyscale is amplitude* on a 40 dB log stretch clipped at the 99th
  percentile, the same stretch `focus` writes, and the bar on the right keys it.
  **That bar is relative**: 0 dB is this image's own brightest cell, not a
  calibrated sigma-nought, so it compares cells within one figure and says
  nothing across two. It is for looking, not measuring — bright scatterers
  saturate, so a point-spread function measured off it comes out broader than it
  is. Use `focus --raw` for that.
- **It shows the patch against the sub-look's resolution, which is not the
  resolution the aliasing warning is about.** That warning compares `--cell` to
  the *full-aperture* resolution. A sub-look is coarser than the full aperture
  by roughly the number of looks, so a cell that undersamples the full aperture
  can still oversample every image the tracker sees — the synthetic run in
  section 3 is warned about a 0.5 m cell against 0.064 m full-aperture
  resolution while its sub-looks resolve 8.26 m, sixteen cells. What you see
  there is the sub-look point-spread function, not point targets, and a 32 px
  patch spans about two resolution cells. Neither fact is visible in any
  statistic computed downstream.

**The caption under both maps** — `7 X 7 WINDOWS, 60 PX EACH` — gives the grid
size and how many *screen* pixels one window block was drawn as. The 60 is a zoom
factor chosen to make the figure legible; it is not the correlation patch size
and not a resolution. The patch size is `--win`.

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

Use `tools/new-run.sh <scene> <suffix> "<question>"` to seed a run directory under
`runs/<scene>/<date>-<suffix>/` with a `RUN.md` recording the commit, host,
commands and result. A null result stays there rather than being deleted — it is
the more common outcome here and the more informative one.

Figures, CSV evidence and logs under `runs/` are committed: `.gitignore` blocks
`*.png` and `*.csv` everywhere else, and opts them back in there specifically,
because a run's output is the record of what happened. Raw `.f32` cubes and
collects stay out. See [`runs/README.md`](../runs/README.md).

---

## 9. Choosing estimator and reference

Two orthogonal choices. The honest summary is below, and it changed with items
30–43: **injected frequencies now recover on real data**, which the earlier
version of this section denied.

Keep two things apart when reading it.

- **Recoverability** — a frequency put into the data comes back out. Established
  on the real Giza collect, down to 0.0625 mm of injected motion (items 30, 37).
- **Detectability** — deciding something moved without already knowing the
  answer. **Not established, by anything here.** A bright scatterer that does not
  move outscores every real injection on prominence, on `--null-static` and on
  the scene-derived null; only the reported frequency separates them, and on a
  real target you do not have the frequency in advance (item 38).

Three estimators pass the project's slope-and-rms bar on *some* fixture. None of
that is evidence about detection.

`--estimator correlation|phase|splitband|argmax`

- `phase` — phase of a single dominant pixel, with the geometric carrier removed.
  **The one setting that has passed the bar** (`FOLLOW-UPS.md` item 14): slope
  1.008, rms 0.0070 Hz against a 0.0252 bound, pooled over three clutter seeds,
  static controls outside the swept band. Closest to the published work that
  reports success. Three things to know before using it: the observable **wraps
  beyond about λ/4 of line-of-sight motion** (~8 mm at X band), so it needs a far
  smaller motion than `correlation` and fails completely above that; it needs
  **one dominant scatterer per sub-look resolution cell**, which is a real
  constraint on the scene and not a formality (`FOLLOW-UPS.md` item 15); and it
  is unvalidated on real data, where the sub-look decorrelation the simulator
  cannot produce is the obvious threat.

  **On real data it recovers injected frequencies**, down to 0.0625 mm, provided
  the first three spectral bins are excluded — which they now always are. Below
  2 mm without that exclusion it reported bin 1 instead, with *rising* confidence
  as the target weakened (items 30, 37).

  **That threat has now been measured, and the recovery does not survive it**
  (`FOLLOW-UPS.md` items 24-25). Given scatterers bright over only part of the
  aperture — the mechanism the simulator lacked — item 14's own sweep loses the
  relationship: slope goes negative at three of four settings, rms rises to
  0.59-1.78 Hz against the 0.0252 bound, and two of twelve *motionless* controls
  return a confident in-band frequency. Selecting on amplitude dispersion is the
  only policy that survives, refusing where it cannot tell and returning item
  14's figures where it answers. So `phase` does what it claims on the scene it
  was measured on, and expecting that to transfer to a real collect has no
  measured basis.

  **Do NOT use it with high `--overlap`.** This section used to say the opposite,
  on the strength of item 14's synthetic sweep. Measured on the real Giza collect
  (item 48), the separation between an injection and its control collapses by four
  orders of magnitude between 0.75 and 0.90, and at 0.95 neither selection policy
  finds a 2 mm injection at all:

  ```
   overlap   injected local peak   control   separation
      0.00     816,997 (correct)      22.9      35,676x
      0.50   1,207,566 (correct)      12.6      95,838x
      0.75   1,856,891 (wrong bin)    12.9     143,945x
      0.90         431.8 (correct)    39.2          11x
      0.95          46.7 (WRONG)      26.0         1.8x
  ```

  Overlap does buy sub-look coherence, which was item 14's premise. It also
  correlates the noise between adjacent looks, and correlated noise is red, and a
  red floor is where a spurious peak lives (item 47) — so what overlap buys in
  coherence it spends on manufacturing the thing the coherence was for. Where the
  optimum sits is **not** established; what is established is that 0.90–0.95 is far
  worse than anything below it.
- `correlation` (default) — cross-correlation peak, sub-pixel refined. Has not
  been shown to recover a frequency on a distributed-clutter fixture; it does
  recover on a single-dominant-target fixture (slope 1.006, rms 0.0035), which is
  what `FOLLOW-UPS.md` item 26 uses as its control. Against aspect dependence it
  degrades differently from `phase`: it keeps a slope near 1 and misses on
  scatter, rms 0.13-0.25 Hz, where `phase` loses the relationship outright. Still the right choice for
  motion too large for phase, since it has no ambiguity at all.
- `argmax` — the azimuth position of the window's **brightest pixel**, tracked
  across sub-looks. No correlation, no phase, no reference look. This is the
  published method — Suppi et al. (IWSHM 2025) track "the azimuthal displacement
  of the brightest pixel in each sub-aperture" against a shaker-driven corner
  reflector with an LVDT, reaching Pearson 0.98 — and it is what `FOLLOW-UPS.md`
  item 6 measured carrying 93% of its variance at the injected frequency where
  the correlator on the *same stack* carried 4.1%. Passes the bar on an isolated
  dominant target: slope 1.008, rms 0.0061 Hz against a 0.0252 bound over five
  frequencies. **Quantised at one cell** by construction, so `quant_px` is 1.0
  and an excursion under 2.449 cells is refused as rounding. Untested against
  clutter, aspect dependence, or real data.
- `splitband` — split-band phase linking over all N² interferograms. Returned one
  fixed frequency at every configuration swept.

**The default is still `correlation`.** The phase route's real-data recoveries
are all of *injected* signals whose frequency was known in advance, its aspect
dependence failure (items 24–25) is unaddressed, and the default should not
quietly become the path whose preconditions are hardest to check.

Choose `phase` deliberately, for small motion. Run `--null-static` beside it —
and **also run the zero-amplitude twin**, `--inject-vib F,0.0,REL`, because
`--null-static` alone passes a target that never moves (items 38–39, and section
7 step 0b for how to compare the two).

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

**No test exercises this gate**, and the default sits above what the fixtures can
reach, so every real-data run recorded here passes `--coherence 0`. Why it cannot
be tested is `FOLLOW-UPS.md` item 12f.

**It masks a different quantity under `--estimator phase`**, which reads one
pixel's phase and forms no correlation surface at all. There `quality` is the
**spatial dominance** of the window's brightest pixel over its own window,
`1 − mean/peak` — the same measure `argmax` uses, and the thing item 15's
precondition actually names.

It used to be amplitude *stability*, which made `--coherence F` exactly the
criterion `D_A <= 1 − F` and made a `quality` map and a `d_a` map one measurement
shown twice. **That was a defect**, not a curiosity: a vibrating scatterer is not
amplitude-stable, so the gate rejected moving targets (`FOLLOW-UPS.md` items 45–46).
The two are now independent — `quality` asks whether there is a dominant
scatterer, `d_a` whether it is a persistent one.

**The shared `quality >= 0.5 × max` gate is inert on real scenes** as a result:
speckle alone scores about 0.67 on a 1024-pixel window and real imagery 0.81–0.94,
so a threshold near 0.50 removes nothing. An inert gate is better than one that
removed the signal, but do not read a pass as evidence. See `rs_microm_t.quality`.

`--no-optimize` is an audit baseline, not a better measurement: it searches the
whole upsampled correlation surface rather than the neighbourhood of the integer
peak, and runs serially. Backprojection is bitwise identical either way.

### Flags this guide does not otherwise cover

Real options, documented at length in each subcommand's own `--help` and listed
here so the guide does not imply the set above is complete:

| flag | command | what it does |
|---|---|---|
| `--inject-wave FILE[,RATE_HZ[,AMP_MM[,REL]]]` | `mmotion` | drives the injected target from a measured displacement record instead of a tone; mutually exclusive with `--inject-vib` |
| `--wave FILE[,RATE_HZ]` | `sim_cphd` | drives every vibrating target, including a `--clutter-vib` patch, from a measured record |
| `--inject-at DX,DY` | `mmotion` | offsets the injected scatterer in metres from the grid origin, which is otherwise where it always lands |
| `--fmin HZ` | `mmotion` | raises the peak-picking floor above the always-excluded first three bins; it cannot lower it |
| `--probe-hz HZ` | `mmotion` | adds `probe_psd` and `probe_prominence` at one nominated frequency, so two runs can be differenced there |
| `--no-detrend` | `mmotion` | skip the least-squares line removal before the periodogram |
| `--b-shift HZ` | `mmotion` | master/slave band separation; only `--reference pair` uses it |
| `--null-trials N` | `mmotion` | trials for the shuffled-look floor, beside `--null-static` |
| `--range-taps N` | `focus`, `mmotion` | range-interpolator width in the backprojection kernel |
| `--pulse-start N` | `focus`, `validate` | first pulse to read, with `--max-pulses` making the read a window |
| `--max-pulses N` | `validate` | shortens the dwell the checks are answered for — the remedy when a collect's sub-apertures are too long for the target frequency |
| `--dyn-range DB` | `focus` | quicklook display range below the 99th percentile |
| `--ccd-win N`, `--ccd-loading F` | `mmotion` | window and noise floor for the `--ccd-out` change-detection locator |
| `--amplitude MM`, `--alpha F` | `validate` | target amplitude and aperture fraction to answer the checks for |

---

## 10. Gotchas

- **A null on real data means nothing without `--inject-vib`.** Every real
  collect this project has processed returns a null, and a null looks identical
  whether the scene is still or the chain cannot see motion in that data. Section
  7 item 0. This is the single most common way to over-read an output here.
- **`--null-static` figures from before 2026-08-02 are against a defocused
  null.** The simulated motionless scene never focused — peak-to-mean 3.6 against
  93.7 — so the control was a field of noise rather than a scene. Fixed and
  tested; recompute anything quoted from before then. `FOLLOW-UPS.md` item 27.
- **Truncating the dwell raises the observable band and costs resolution in the
  same proportion.** `f_max` scales as `1/T_dwell` and so do the frequency
  resolution and the azimuth resolution. On ICEYE's Houston collect, 15.3 s → 6.1 s
  takes the band from 1.509 Hz to 3.771 Hz and the frequency resolution from
  0.065 Hz to 0.163 Hz. Quote which one you bought. `FOLLOW-UPS.md` item 44.
- **`validate` cannot read `sim_cphd` output.** `info`, `focus` and `mmotion`
  sniff the file magic and accept both real CPHD and the simulator's internal
  format; `validate` accepts real CPHD only and fails with *"does not begin with
  a CPHD version line"*. Known gap, not a decision.
- **`rs_slc_t.r0` is now `rs_slc_t.r_scene_m`**, the slant range to the scene
  reference point at mid-dwell, and every reader writes that one meaning. It
  used to be documented as the first range sample's range, which no reader ever
  wrote. If you have code holding an `img->r0`, it will fail to compile — that
  is the point of the rename. Anything indexing range bins against it must
  offset from the CENTRE bin. See `docs/FOLLOW-UPS.md` item 5.
- **Read the `located at` line, not the window index, for position.** The window
  named by `strongest peak in window N` is wrong by exactly one window every time
  at 50% overlap — the target sits in four windows and the one it is centred in
  scores lowest. The `located at` centroid beside it is accurate to 0.13 m on a
  2 mm target, finer than the grid cell — but that scales with signal strength,
  reaching 2.24 m at 0.125 mm, so quote it with the amplitude. If it says `CLIPPED at the grid edge`, the
  target is at the boundary with half its footprint off the grid: move the grid.
  `FOLLOW-UPS.md` items 41–42.
- **The velocity beside a reported frequency is not a calibrated amplitude.** It
  is the raw peak-to-peak of the tracked series before detrending, so it carries
  the trend and the noise: a motionless Giza scene prints 124.8 mm/s through that
  same line, and across an amplitude sweep it flattened at 0.8–1.1 mm/s while the
  true amplitude fell sixteenfold. `mmotion` now labels it `UNCALIBRATED` and adds
  `AT THE TRACKING FLOOR` when the excursion is within twice the quantisation.
  Read the frequency; do not quote the velocity as an amplitude.
- **`--probe-hz F` is the only honest way to compare two runs.** It adds
  `probe_psd` and `probe_prominence` at one nominated frequency to every row of
  `PREFIX_windows.csv`. The dominant-peak columns cannot be differenced between
  runs whose strongest peaks sit at different frequencies. Pair a real injection
  with its zero-amplitude twin and subtract: the median window gains +0.00 and the
  injected window gains. The two runs must differ in **nothing but amplitude** —
  paired against an uninjected run instead, the increment is larger than a real
  signal, because it is measuring the scatterer's presence. `FOLLOW-UPS.md` item 39.
- **`--inject-vib` at ZERO amplitude produces a bigger "detection" than any real
  one.** A bright scatterer that does not move reaches prominence 56.3 on the
  Giza collect, against 38-47 for real injections, and it clears the
  `--null-static` controls. Prominence, the null and the scene-derived null all
  rank it first; only the reported FREQUENCY separates it (zero amplitude answers
  at the band floor, a real injection at the injected value). **A positive control
  is not evidence unless its zero-amplitude twin has been run.** `FOLLOW-UPS.md`
  item 38.
- **The first three frequency bins are never reported, and this is not
  configurable.** A Hann window's main lobe is ±2 bins, so bins 1 and 2 hold the
  skirt of any residual trend and cannot be separated from it at any SNR;
  physically they are one and two cycles across the whole dwell, which is not a
  periodicity. `--fmin` can raise the floor and cannot lower it. Before this,
  a 0.163 Hz injection into the real Giza collect was reported as 0.033 Hz — bin
  1 — at every amplitude below 2 mm, with prominence *rising* from 32.0 to 56.0
  as the target got weaker, and prominence, quality, `D_A` and the `--null-static`
  control all endorsed it. `FOLLOW-UPS.md` item 37.
- **Excluding those bins relocates a trend, it does not remove one.** A window
  containing only a trend still has to answer somewhere, and it answers at the
  first admissible bin — measured at prominence 28.09 against 19.67 for a window
  holding a real tone under the same trend. A cluster of windows all reporting
  the floor frequency is a trend field, not a detection.
- **The amplitude field is qualitative.** Frequencies recover where relative
  amplitudes do not. Label it as such wherever it is presented.
- **The frequency map's colour bar is autoscaled to that map**, so the same
  colour in two runs is not the same frequency, and a window masked by
  `--coherence` reports the lowest bin rather than being marked absent. The
  spectrum's y axis carries its units, but its absolute height is uncalibrated —
  read prominence, not amplitude. Section 8 has the details.
- **`--json` is `info --cphd` only.** Other sources refuse it rather than
  falling back to the human report, which would hand a parser something it would
  either fail on or silently misread.
- **The cull's `sigma_px` column is not an error bar.** It ranks windows against
  each other within one run; the constant relating it to a true standard
  deviation is omitted because it is identical for every window. Do not quote it
  as a precision.
- **`-ffast-math` must stay out of the build.** It permits reassociation and
  flushes denormals, which perturbs exactly the sub-pixel correlation peaks and
  interferometric phase this project measures.
- **Every function needs a preceding block comment** stating units and
  conventions. `scripts/check_docs.py` runs as the `docs_coverage` ctest and
  fails the suite otherwise.

---

## 11. Where to read next

- `README.md` — what the project is and what the bar for a result is.
- **`docs/GLOSSARY.md`** — short definitions of the radar and measurement terms
  used here, including terms that are easy to confuse such as transmit PRF and
  vibration sampling rate.
- **`docs/FOLLOW-UPS.md`** — the map of dead ends, with measurements. It records
  that sub-look images can be correct while the tracker fails to read them; that
  the reference scheme looks like the binding constraint and is not; and that a
  gate derived from window geometry refused correct measurements. It includes
  entries that withdraw earlier entries. Each cost hours to establish and none is
  recoverable from the code.
- **`docs/CODE-REVIEW.md`** — defects found by reading the code against its own
  documentation rather than by measuring it: what is dead, what is described
  inaccurately, and which run artefacts do not say what they appear to. Read it
  before trusting a column in `PREFIX_windows.csv` or a field in `rs_slc_t`.
- The headers. `include/resonarsat/microm.h` is 1343 lines for 17
  declarations, and carries the derivations, measured tables and reasoning behind
  every constant. `subaperture.h` and `validate.h` are the same. When a
  measurement settles something in this project it is retired *into* a header,
  beside the code it constrains.
