# Micromotion

<p align="center">
  <img src="assets/logo.png" alt="Micro-Motion SAR logo" width="420">
</p>

**Measuring vibration of the ground and of structures from a single satellite pass.**

One radar pass is cut into sub-apertures — snapshots of the same ground at different
instants — and what moves between them is tracked and turned into a per-window vibration
spectrum. No second pass, no interferometric stack, no repeat cycle.

## AI full disclosure

This software is developed with strong assistance from Claude Opus 5, GPT-5.5 and GPT-5.6,
with humans leading the ideas, the testing and the debugging. We say this openly because it
shaped how the project was built. If you are not happy with AI-developed code, this software
is not for you.

## Why single-pass

Repeat-pass interferometry measures millimetres per year over days to years. This measures
millimetres per second within a single dwell, in the band where structures actually resonate.
They are different instruments for different questions, and almost no open software does the
second one.

The targets are stable man-made structures — bridges, dams, towers — under long-dwell
spotlight coverage, whose dominant modes project usefully onto the satellite line of sight.
That last condition is a real constraint and no software can check it for you.

## Status

Nothing here has detected real motion. Everything below is measured, and the
numbers behind each claim are in [`docs/FOLLOW-UPS.md`](docs/FOLLOW-UPS.md).

**What exists.** Readers for CPHD, SICD and UAVSAR; image formation; the
sub-aperture and tracking chain; `validate`, which checks whether a collect can
support the measurement you want before any of it is processed; and the controls
that make a result worth reporting — a motionless-scene null, a per-window
evidence file beside every run, and `--inject-vib`, which adds a vibration of
known frequency to real data so that a blank result can be told apart from a
blind pipeline. 21 tests pass.

**On simulated data it recovers vibration — until the simulation gets harder.**
The phase estimator reads injected frequencies back almost exactly on a simple
scene. Give the simulated scatterers a realistic property they had been missing —
brightness that changes with viewing angle, as a building face or a bridge
girder's does — and the recovery collapses, and motionless scenes start returning
confident answers. Items 14, 24 and 25.

**On real data it finds an injected vibration exactly, and then refuses to report
it.** Five vibrations were added to a real Giza collect, one run each, plus one
run with nothing added. The patch of ground holding the added vibration returned
the right frequency all five times, with an error fifty times smaller than the
test requires, and the untouched run returned something outside the tested range.
The tool printed *no frequency* every time — because it decides by asking whether
many patches agree, and 135 patches of empty desert outvote the one patch that
was right. So the measurement works and the thing that reports it does not. That
is the open problem. Item 30.

**Why that is not a detection.** The right patch was known in advance, because
that is where the vibration was put. Finding an unknown target means choosing the
right patch out of hundreds, which is exactly the step that failed. The added
vibration is also an idealisation: a real scatterer's brightness changes with
angle and this one's does not, which is the property that broke the simulated
case above.

**No collect available to this project contains motion known to be there**, so no
output should be read as sensitivity to anything real.

## The bar

A result counts when it recovers an injected frequency with **slope near 1 and rms under
half a bin**, pooled over independent clutter realisations, on real clutter, with a static
control through identical processing.

Not a single frequency matched once. A chain that emits one fixed spurious frequency passes
a per-point test wherever that value happens to fall near the injection, which is why the
criterion is a fit across a sweep rather than a comparison at a point.

## A wrong setting does not fail loudly

This is the difficulty that shapes everything here. Ask for a measurement a collect cannot
support and you do not get an error — you get a complete, well-formed spectrum with a
confident peak in it, produced by the processing rather than by the ground. A motionless
scene run through the same settings produces one too, sometimes a stronger one.

Hence `validate` before processing, a null control beside every result, and a consensus
statistic that can report disagreement instead of a number.

## Read this before starting

[`docs/FOLLOW-UPS.md`](docs/FOLLOW-UPS.md) is the map of dead ends. It records, with
measurements, that sub-look images can be correct while the tracker fails to read them; that
the reference scheme looks like the binding constraint and is not; that cross-window
agreement is blind to artefacts produced by the processing rather than the scene; that a
gate derived from window geometry refused correct measurements; and that `validate`, the
command whose job is to warn before an expensive run, answered every question for the
correlation estimator and so refused a phase configuration that was correct. Each cost hours
to establish and none is recoverable from the code.

[`docs/DATASETS.md`](docs/DATASETS.md) lists open phase-preserving SAR datasets suitable for
experiments, with direct catalogue links and notes on their limitations.

## Build

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

No external dependencies. C11 and CMake; the FFT is vendored. `ctest` passes on a fresh
clone with no data — every test builds its own fixture.

Two build facts that bite: OpenMP is optional and silently absent on macOS unless Homebrew's
keg-only `libomp` is found, so read the configure line rather than assuming; and
`-ffast-math` is deliberately absent, because it permits reassociation and flushes denormals,
which perturbs the sub-pixel correlation peaks and interferometric phase this project
measures.

## Running it

[`docs/USER_GUIDE.md`](docs/USER_GUIDE.md) covers the four subcommands, the flags that
decide whether a large collect fits in memory, and how to read what `mmotion` prints —
including its two refusal paths, which are the output you should expect most often.
