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
an index of all 41 items and their status.

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

Unlike `correlation`, **phase wants high `--overlap`**: recovery holds to 95%, and high
overlap is what buys sub-look coherence on a real collect. Item 13's response ceiling is
the correlator's, now measured — at 90% overlap a 1.3 Hz tone sits at a response of 0.055
and phase still recovers it.

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
