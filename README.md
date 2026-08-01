# micromotion

**Measuring vibration of the ground and of structures from a single satellite pass.**

One radar pass is cut into sub-apertures — snapshots of the same ground at different
instants — and what moves between them is tracked and turned into a per-window vibration
spectrum. No second pass, no interferometric stack, no repeat cycle.

## What this is not

It is not a fork of [ResonarSat](../ResonarSat) in aim. That project exists to replicate a
specific published claim about structures beneath the Great Pyramid, and is bound to
implement the method as the paper and patent describe it — including where the published
geometry defeats itself. This project inherits the working parts of that codebase and none
of the obligation.

Concretely, it is free to choose things ResonarSat cannot:

- a sub-aperture layout that spans the whole dwell, so `df = 1/T` and the frequency bins do
  not land on nulls of the averaging response;
- any reference and any estimator, on merit rather than fidelity;
- the depth stage: dropped entirely. `rs_tomo_*`, the `paper` and `pair` sub-aperture modes,
  `--patent-exact` and the assumed-constant depth axis are all gone.

## Status: baseline only

The chain is being rebuilt a stage at a time, each verified before the next is trusted.
What has been carried over and passes its tests:

- readers for CPHD (32-bit float and 16-bit integer samples), SICD and UAVSAR
- time-domain backprojection
- sub-aperture decomposition, coregistration, phase linking
- `validate` — thirteen arithmetic checks on whether a collect can support a measurement
- the null machinery, `rs_track_fit()`, the consensus statistic and the per-window evidence file

19 tests pass. **Nothing here has yet been shown to recover a vibration frequency it was
not told.** That is the whole of the work ahead.

## The bar

A result counts when it recovers an injected frequency with **slope near 1 and rms under
half a bin**, pooled over independent clutter realisations, on real clutter, with a static
control through identical processing. Not "implements the published method", and not a
single frequency matched once — that criterion produced five withdrawn conclusions in the
project this came from.

## Read this before starting

[`docs/FOLLOW-UPS.md`](docs/FOLLOW-UPS.md) is the map of dead ends, carried over deliberately.
It records, with measurements, that the sub-look images are correct while the tracker does
not read them; that the reference scheme is not the binding constraint though it looks like
it; that cross-window agreement is blind to common-mode artefacts; and that a
geometrically-derived gate refused correct measurements. Each cost hours. None is
recoverable from the code.

## Build

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

No external dependencies. C11 and CMake; the FFT is vendored. OpenMP is optional and
silently absent on macOS unless Homebrew's `libomp` is found — read the configure line.
