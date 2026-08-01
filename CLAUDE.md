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
code, and several conclusions in it reversed after further measurement.

## Build, test, run

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
ctest --test-dir build --output-on-failure          # 19 tests
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

The project was renamed; the internals were not. The binary and CMake project are
`micromotion`, headers live under `include/resonarsat/`, every symbol is `rs_`-prefixed,
and some usage strings still print `resonarsat`. Treat `rs_`/`resonarsat` as the
library's identity and `micromotion` as the tool's.

## Layout and pipeline

`mm_core` is one static library; `src/main.c` (~2000 lines) is the whole CLI, four
subcommands dispatched at the bottom of the file:

- `info` — a product's geometry and timing
- `validate` — thirteen arithmetic checks on whether a collect can support the
  measurement asked for, before any processing (`src/core/validate.c`, `validate.h`)
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
   two selection statistics side by side: `rs_spectrum_best_window()` (prominence, the
   reported answer) and `rs_spectrum_consensus()` (which frequency the most windows
   agree on, how many distinct answers there are, and the largest 4-connected block of
   agreeing windows).

`src/core/ccd.c` is a separate scale-invariant change-detection locator over the same
stack — "where is something moving", not "at what frequency". `src/io/` writes PNG and
raster maps; `src/util/` holds geometry, geocoding, orbit and the thread-local error
buffer.

`tools/sim_cphd.c` generates synthetic phase history and is how nearly every claim in
`FOLLOW-UPS.md` was measured: `--clutter N` for distributed texture, `--clutter-vib` to
make the patch move coherently, `--seed`/`--offset-x` for independent realisations.

## Conventions this codebase actually enforces

**Every function definition needs a preceding block comment.** `scripts/check_docs.py`
runs as the `docs_coverage` ctest and fails the suite otherwise. The rule exists
because this code is full of dimensionally interchangeable quantities that are
physically different — Hz against rad/s, slant against ground range, azimuth line rate
against transmit PRF — and the comment must state units and conventions.

**The headers are the documentation of record.** `microm.h` is 944 lines for ~10
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

Nothing here has yet been shown to recover a vibration frequency it was not told. Every
estimator and reference mode has failed at operating points this project's own
arithmetic calls admissible; the most recent finding (`FOLLOW-UPS.md` items 7-9) is that
the tracker does recover the injected carrier in most windows and the *selection policy*
discards it. `rs_spectrum_best_window()` still selects by prominence and is still what
the tool reports; ranking on consensus and contiguity instead is the open half.

`--reference pair` and `--reference adjacent` do not recover a frequency on the
synthetic fixture and are exposed because the sources describe them, not because they
work. `--reference lag` is documented and harmless but does not beat `first` on a
consensus measure.

`build/`, `data/` and run outputs are gitignored. `tools/new-run.sh <scene> <suffix>
"<question>"` seeds a run directory with a `RUN.md` recording commit, host, commands and
result — a null result stays there rather than being deleted.
