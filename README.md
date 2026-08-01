# micro-motion-SAR

**Measuring vibration of the ground and of structures from a single satellite pass.**

One radar pass is cut into sub-apertures — snapshots of the same ground at different
instants — and what moves between them is tracked and turned into a per-window vibration
spectrum. No second pass, no interferometric stack, no repeat cycle.

## Why single-pass

Repeat-pass interferometry measures millimetres per year over days to years. This measures
millimetres per second within a single dwell, in the band where structures actually resonate.
They are different instruments for different questions, and almost no open software does the
second one.

The targets are stable man-made structures — bridges, dams, towers — under long-dwell
spotlight coverage, whose dominant modes project usefully onto the satellite line of sight.
That last condition is a real constraint and no software can check it for you.

## Status: baseline only

The chain is being built a stage at a time, each verified before the next is trusted. What
exists and passes its tests:

- readers for CPHD (32-bit float and 16-bit integer samples), SICD and UAVSAR
- time-domain backprojection
- sub-aperture decomposition, coregistration, phase linking
- `validate` — thirteen arithmetic checks on whether a collect can support the measurement
  you intend, before any of it is processed
- null-test machinery, `rs_track_fit()`, a cross-window consensus statistic, and a
  per-window evidence file written beside every result

19 tests pass. **Nothing here has yet been shown to recover a vibration frequency it was not
told.** That is the whole of the work ahead, and no output should be read as a demonstrated
sensitivity until it is.

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
agreement is blind to artefacts produced by the processing rather than the scene; and that a
gate derived from window geometry refused correct measurements. Each cost hours to establish
and none is recoverable from the code.

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
