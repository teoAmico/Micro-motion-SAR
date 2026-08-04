# Micromotion

<p align="center">
  <img src="assets/logo-1.png" alt="Micro-Motion SAR logo" width="420">
</p>

**An experimental toolkit for measuring vibration from a single satellite SAR pass.**

Micromotion divides one long-dwell radar acquisition into a sequence of
sub-aperture images, tracks changes between them, and calculates a vibration
spectrum for each spatial window. It is designed for structures such as bridges,
dams and towers whose motion projects onto the radar line of sight.

> [!IMPORTANT]
> This is research software, not a validated vibration instrument. It can recover
> known vibrations injected into synthetic and real radar data, but it has not yet
> detected independently confirmed natural motion in a real collect.

## What it does

```text
phase-preserving SAR data
        ↓
inspect and validate the acquisition
        ↓
form an image and a sequence of sub-aperture images
        ↓
track displacement or phase in overlapping spatial windows
        ↓
calculate vibration spectra and evaluate them against null controls
```

The command-line program has four main commands:

| Command | Purpose |
|---|---|
| `info` | Show product geometry, timing and radar parameters |
| `validate` | Check whether a collect can support the requested measurement |
| `focus` | Form a SAR image from phase-history data |
| `mmotion` | Build sub-apertures, track motion and calculate vibration spectra |

It includes readers for CPHD, SICD and UAVSAR products, backprojection image
formation, several motion estimators, synthetic fixtures, static-scene controls
and per-window diagnostic output.

## What it does not do

- It cannot recover vibration from an ordinary PNG, JPEG or single photograph.
  Vibration requires phase-preserving radar samples acquired over time.
- It does not independently measure every display pixel. Most estimators use a
  spatial patch and assign the result to that window.
- A spectral peak is not automatically evidence of motion. Processing artefacts
  and motionless scenes can produce strong, plausible peaks.
- It has not validated the later subsurface interpretations proposed by Doppler
  tomography papers. The vibration measurement must stand on its own first.

## Inputs and outputs

Supported inputs are phase-preserving SAR products:

- CPHD phase history
- SICD complex imagery
- UAVSAR SLC with its annotation file

A typical `mmotion` run writes:

| Output | Meaning |
|---|---|
| `PREFIX_scene.png` | SAR scene, tracking grid and selected window |
| `PREFIX_freq.png` | Dominant frequency in each spatial window |
| `PREFIX_quality.png` | Tracking quality in each window |
| `PREFIX_spectrum.png` | Spectrum used for the reported candidate |
| `PREFIX_spectrum_mm.png` | Displacement interpretation of that spectrum |
| `PREFIX_spectrum_mms.png` | Velocity interpretation of that spectrum |
| `PREFIX_windows.csv` | Evidence and selection state for every window |

Use `--shifts FILE.csv` to save the raw tracked displacement series before
detrending and spectral analysis. This is often the most useful output when
investigating a suspicious result.

## Build and test

The project is written in C11, uses CMake and has no required external runtime
dependencies. Its FFT implementation is vendored.

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

The tests create their own fixtures, so no external dataset is required.

OpenMP is optional. On macOS, CMake looks for Homebrew's keg-only `libomp`. The
program still works without it, but backprojection runs on one CPU core and is
considerably slower. Check the CMake configure output for either `OpenMP enabled`
or the warning that it was not found.

`-ffast-math` is intentionally disabled because reassociation and denormal
handling can perturb the sub-pixel correlation peaks and interferometric phase
being measured.

## First run: synthetic data

The safest introduction is a simulated scene whose vibration is known in
advance. Build the simulator, generate a 0.5 Hz scene, and process it:

```sh
./build/sim_cphd scene.cphd 0.5 0.02 --clutter 400 --clutter-vib --seed 7

./build/micromotion mmotion --cphd scene.cphd \
  --n 128 --overlap 0 --size 128 --cell 0.5 \
  --win 32 --upsample 200 --coherence 0 --out run1
```

This exercise demonstrates the complete pipeline, but a matching frequency in
one run is not validation. Change the injected frequency, repeat across clutter
seeds, and compare against zero-motion controls.

See the [user guide](docs/USER_GUIDE.md) for real-product examples, memory
controls, estimator selection and a detailed explanation of every output.

## Recommended workflow on real data

Use the commands in this order:

```text
info → validate → focus → mmotion → inspect evidence and null control
```

1. Run `info` to verify the product geometry and timing.
2. Cross-check important metadata with an independent reader when possible.
3. Run `validate` for the intended frequency band and estimator.
4. Form a small image around a known target before processing a large area.
5. Run `mmotion` and save raw shifts with `--shifts`.
6. Inspect the scene, raw time series and selected spectrum together.
7. Process a motionless or simulated null through the identical configuration.

A wrong configuration often does not fail loudly. It can produce a complete,
well-formed spectrum with a confident peak caused by the processing rather than
the scene. Refusing to report a frequency is therefore a valid and expected
outcome.

## Motion estimators

| Estimator | Observable | Best suited to | Main limitation |
|---|---|---|---|
| `correlation` | Sub-pixel translation of an image patch | Structured or distributed targets and larger motion | Correlation bias and sub-aperture averaging |
| `phase` | Phase of a dominant coherent scatterer | Small line-of-sight motion | Phase wrapping, decorrelation and aspect-dependent scattering |
| `argmax` | Position of the brightest pixel | Isolated bright targets | One-cell quantisation and target switching |
| `splitband` | Phase linking across the look stack | Experimental coherent scenes | Has not recovered reliably in current tests |

`correlation` is the default. The best choice depends on target structure,
motion amplitude, wavelength, dwell time and sub-aperture coherence. Running two
independent estimators is useful corroboration, but agreement does not replace a
null control.

## Current research status

The detailed measurements and failed hypotheses are recorded in
[`docs/FOLLOW-UPS.md`](docs/FOLLOW-UPS.md). The short version is:

- The complete reading, focusing, sub-aperture, tracking and spectrum pipeline
  exists, together with 21 automated tests.
- The phase estimator accurately recovers swept injected frequencies on simple
  synthetic scenes.
- That recovery collapses when scatterer brightness changes with viewing angle,
  a property expected in real structures.
- A known 0.163 Hz vibration injected into real Giza clutter was recovered
  correctly at every amplitude from 2 mm down to 0.0625 mm.
- **That recovery is of the frequency only.** Running the same injection at ZERO
  amplitude — a bright scatterer added to the phase history that does not move —
  produces a LARGER peak than any real injection, and it clears the same 19
  motionless controls. The `p = 0.05` this project previously reported therefore
  measured that a scatterer had been added, not that it vibrated. Only the
  reported frequency separates the two: zero amplitude answers at the band
  floor, a real injection answers at the injected value (item 38).
- Earlier injected signals were measured in the correct patch but rejected by
  scene-wide selection rules; the current verdict instead uses the null control.
- The injected patch and frequency were known in advance. Automatically locating
  an unknown vibrating target among hundreds of windows remains untested.
- No available collect contains independently confirmed natural vibration, so
  the current results do not establish real-world sensitivity.
- The distinction that matters is between **recoverability** — a frequency put in
  comes back out, which is established — and **detectability** — deciding
  something moved without already knowing the answer, which is not.

The project considers a frequency-recovery experiment successful only when a
sweep produces a fitted slope near 1 with RMS error below half a spectral bin,
pooled over independent clutter realizations and accompanied by static controls.
One matching frequency is not enough.

## Documentation

- [User guide](docs/USER_GUIDE.md): commands, examples, memory settings and output interpretation
- [Glossary](docs/GLOSSARY.md): definitions of radar and measurement terminology
- [Experimental follow-ups](docs/FOLLOW-UPS.md): measured successes, failures and open questions
- [Dataset guide](docs/DATASETS.md): phase-preserving SAR datasets and their limitations
- [Run archive](runs/README.md): conventions for recorded experiments

Read the user guide before processing a large real collect. Spotlight phase
histories can be tens of gigabytes, and parameters that reduce memory can also
change resolution or the observable vibration band.

## AI disclosure

This software is developed with strong assistance from Claude Opus 5, GPT-5.5
and GPT-5.6, with humans leading the ideas, testing and debugging. This is stated
explicitly because it shaped how the project was built and reviewed.

## License

See [LICENSE](LICENSE) and [THIRD-PARTY-NOTICES.md](THIRD-PARTY-NOTICES.md).
