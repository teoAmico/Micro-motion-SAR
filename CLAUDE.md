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
an index of all 116 items and their status.

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

**A NETWORK QUERY THAT FAILS LOOKS EXACTLY LIKE ONE THAT FOUND NOTHING, AND
THIS HAS NOW COST FIVE WRONG ANSWERS.** Every remote helper here returns a falsy
value on error, so a dead endpoint, a rejected argument, a misrouted archive and
a genuinely silent sensor are one symptom. Each of these was believed until a
control contradicted it:

- `service.iris.edu/fdsnws/availability` answers **HTML**, and
  `service.earthscope.org/.../availability` answers **410**. That produced the
  withdrawn "0 of 553 with data", and the retired endpoint was reached for a
  SECOND time a year later. Use `dataselect` and measure bytes: it costs the
  samples and cannot lie about whether they exist.
- The FDSN **federator refuses a purely spatial query** — it needs `net` or
  `sta` — and the 400 read as "no stations in this footprint".
- A hand-written **datacentre-name to client map** silently fell back to IRIS for
  every name outside it, so 59 of 76 stations were never measured and the audit
  funnel looked clean. Build clients from the **service URLs the federator
  already returns**; the map is redundant as well as wrong.
- Hardcoding **ODC** for network `ES` returned 204 for every request and read as
  "no neighbours exist" — the same bug, one hour after fixing it.
- A bare `except: pass` around a fetch made all of the above invisible.

The rule: **before believing a negative from a remote service, run the identical
query against something whose answer you already know.** `NN.UNVG` returning
48640 bytes is what proved the routing worked; `NP.2030` returning zero was only
meaningful beside it. And **count failures by reason** — an audit that cannot say
what it did not measure is not an audit. This is the same lesson as the zsh entry
below, which is why both are here: a negative result from an unverified harness
is not a negative result.

**The shell here is zsh, which does NOT word-split unquoted parameters.** A loop
of the form `for a in "" "--max-pulses 60000"; do prog $a; done` passes the whole
string as ONE argument in zsh where bash would split it into two, so the flag is
silently not recognised and the run looks exactly like a flag that does not work.
That cost a wrong diagnosis here: `--max-pulses` was reported as unimplemented in
`mmotion`, and a reader defect was hunted for, when the option worked and the
test harness did not. Write the arguments out in full, or use `${=a}` to force
splitting. The general form of the lesson is the one this codebase repeats: a
negative result from an unverified harness is not a negative result.

**`--rbins` MEANS TWO DIFFERENT THINGS AND THE DOMAIN DECIDES WHICH.** On a
TIME-domain product the samples are delay bins, so it selects a near-range window
and costs scene extent. On an **FX-domain** product — which is what **Capella and
Umbra both ship** — the samples are TRANSMITTED FREQUENCIES and the cap is
applied BEFORE the inverse transform (`cphd.c:704` against the transform at 885),
so it narrows the BANDWIDTH and coarsens range resolution in proportion. Keeping
4000 of 21343 FX samples is a **5.3x loss of range resolution, not a smaller
scene**, and nothing warns. It is not a memory remedy: expressing "a range window
at full bandwidth" on an FX product needs the window applied AFTER the transform,
or a streaming read. Related: **`info` loads the whole signal array**, so it
cannot describe a product larger than RAM — 282,972 x 27,650 is 62.6 GB, and the
Kilauea collects fail it on a 24 GB machine.

**THERE IS NO p-VALUE FOR A STABILIZATION DIAGRAM, AND THE FIELD DOES NOT
PRETEND OTHERWISE** (item 116, implemented; **fixes item 115's defect**). A
stabilization diagram is an **ACCEPTANCE CRITERION, not a significance test**:
poles are marked stable when frequency, damping and mode shape persist across
consecutive model orders under tolerances, the result is refined by CLUSTERING,
and **no probability is attached anywhere**. The number of orders required is
explicitly **application-dependent and chosen by the user**; the usual five is a
convention. So `rs_stable_p()` is REMOVED rather than repaired.

**Two nulls were considered and BOTH FAIL**, which is why the number is measured:
calibrating on **per-window chains** fails because a scene-wide artefact and a
scene-wide injection are structurally identical in window statistics (**item
11**); and **re-dividing the dwell** fails because re-dividing returns the same
answers, so no randomisation destroys a real mode while preserving a scene-pinned
artefact (**item 114's wall on a second axis**).

`RS_STABLE_MIN_CHAIN = 5` therefore comes from the null's OWN measured
distribution (item 80's rule). **The threshold was fitted to twelve motionless
scenes, so re-running those tests NOTHING** -- the run was designed with a
consistency arm labelled arithmetic and a real test on **twelve UNSEEN seeds**.
**The independent arm reproduced the fitted arm exactly**: eleven chains of 0 and
one of 4, both times. **Over all 24 motionless scenes 22 reached chain 0, 2
reached 4, NONE reached 5; over all 12 injected, every one reached 6. The gap at
the threshold is EMPTY.** Item 107's surviving false positive and item 115's are
the same scene class and both are now rejected. `--stable` also refuses a verdict
outright when the ladder is shorter than the criterion. **Bounds: it is an
operating characteristic of ONE fixture, NOT a probability, and must be
re-measured before being quoted elsewhere; two scenes in 24 reached chain 4, one
rung below the criterion.**

**THE STABILIZATION TEST IS A LADDER, NOT A PAIR** (item 115, implemented).
Item 107 built the two-point special case; **the field sweeps model order over a
RANGE and marks a pole stable only when it persists across SEVERAL CONSECUTIVE
orders** -- five is the usual figure, with automated methods clustering poles
across the whole diagram (Reynders et al. 2012). Eighth time a search found the
field ahead. `--stable` now takes a comma-separated list; the statistic is the
**longest chain of consecutive agreeing rungs**, and a refusing rung BREAKS a
chain instead of ending the test.

**Measured on 96/128/160/192/224/256 looks: the ladder gives a DEFINITE verdict
on 12 of 12 motionless scenes where the pair managed 1**, keeps **6 of 6**
injected (every one a full 6-rung chain), and reports 1 of 12 motionless. That
last row is what item 114 cost and this repays: the pair could not decide,
because its partners were refusing.

**ITS CHANCE MODEL WAS WRONG, FOR THE THIRD TIME AND ON A THIRD AXIS** (fixed in
item 116). Seed 31 -- a MOTIONLESS scene -- holds **0.954 / 0.958 / 0.950 /
0.965 Hz across four consecutive look counts**, a chain of 4 that `rs_stable_p()`
priced at 1.3e-5 and which happened on 1 of 12 scenes: **wrong by four orders of
magnitude**. Rungs are NOT independent -- every rung re-divides the SAME pulses
over the SAME dwell.

**THE RULE THIS MAKES EXPLICIT: in this chain anything built by RE-DIVIDING ONE
DWELL is correlated with everything else built the same way, and no chance model
may assume otherwise.** Windows overlap in pixels (item 113), rungs overlap in
pulses (item 115). Three chance models have now assumed independence and been
wrong.

**ITEM 108 IS REFUSED, AND NO PERMUTATION NULL ON THE NOMINATIONS CAN BE EXACT**
(item 114, implemented). Item 113's residual guess -- correlation reaching beyond
the 2x2 tile -- was **WRONG and measured wrong before anything was built on it**:
shared nominations run 6.00 / **2.16** / 0.59 at separations 0 / 1 / 2 against a
random-pair baseline of 0.63, so **correlation is confined to ADJACENT windows**,
exactly the pixel-sharing range of a 32 px window at stride 16. Fourth wrong
explanation in this arc, first caught in advance.

**The real residual is that ANY FIXED PARTITION destroys correlation across its
own boundaries** -- half of all adjacent pairs straddle one, so the tile-shift
null reproduces only **66%** of observed adjacent sharing. Jittering the origin
gives 63%; a boundary-free field reproduces it only as it becomes GLOBALLY
CONSTANT (95% at copy probability 0.99), which preserves the structure the null
exists to destroy. **At short range the mechanical correlation and the structure
under test are the same thing.**

**So bracket it.** The SHIFT draw under-correlates (66%) and gives an optimistic
p; the DILATE draw makes a 2x2 tile share one member's nominations, each window
keeping its OWN typical ratio, over-correlates (152%) and gives a conservative
one. Both reported; **admission gates on the conservative end**, so the
family-wise rate is controlled at or below nominal by construction.

**MEASURED: the collect that has led with 0.997 Hz against a sought 1.00 Hz since
item 108 now returns "nothing recurs".** That artefact's p across four items:
**0.001 -> 0.010 -> 0.342**. Both real motionless controls refused BY THE CHAIN,
not by `--stable`. **H1 5 of 6, H3b 0 of 12 with recall 6 of 6, and 9 of 12
motionless synthetic scenes silent** against item 96's 12-of-12 answer rate.
**Every pre-registered prediction was correct**, including which recall point
would be lost.

**Two costs. C10 at 0.13 mm is refused** (p 0.942; item 113 had it at exactly
0.050 and called it the threshold, not a recovery). **And NOT PREDICTED:
`--stable` reportable falls 5 of 6 to 3 of 6** -- it rejects nothing, but the
256-look runs now REFUSE, so there is no partner to compare against. A more
specific chain gives the stabilization test less to work with.

**THE CHANCE MODEL ASSUMED WINDOWS NOMINATE INDEPENDENTLY, AND 50% OVERLAP MAKES
NEIGHBOURS CORRELATED** (item 113, implemented; **this is item 108's cause**).
Windows are laid at half their width, so neighbours share half their pixels and
track the same dominant scatterer -- item 41 measured them returning
BIT-IDENTICAL series. Measured on the real motionless collect, an adjacent pair
shares **2.37 of 6** nominations against **0.71** for a random pair, and the
independent null's own 300-trial maximum block is **9** where the scene shows
**17**. That was quoted at **p = 0.001** against an honest **0.013**, and one
motionless scene had **EIGHT bins clearing p <= 0.05** where the family-wise
design intends 0.05 in total.

**SEVENTH TIME A SEARCH FOUND THE FIELD ALREADY THERE.** Eklund, Nichols &
Knutsson (**PNAS 2016**), *Cluster failure: why fMRI inferences for spatial
extent have inflated false-positive rates* -- parametric cluster-extent inference
is invalid because the assumed spatial autocorrelation is wrong; a nonparametric
PERMUTATION gives nominal rates. Window overlap is their smoothing,
`n_contiguous` is their cluster extent. **Their remedy for the power cost is also
theirs**: cluster **MASS** beats extent and is specifically better for SMALL
INTENSE clusters -- exactly the injected target an extent gate refuses. Item
108's false positive is the **largest block in its scene and nearly the weakest
mass**, so extent cannot separate it and mass can.

`rs_modal_null()` now shifts **2x2 TILES** independently, preserving correlation
within a tile and destroying it across -- the 2x2 tile being the same half-width
stride the block floor and `freq_se`'s `n_eff = n/4` come from -- and admission
tests `evidence` rather than block size.

**The reports went quiet.** H1 **6 of 6**, H2 passes, H3 2 of 2, H3b 0 of 12 with
recall 6 of 6. **Every injected run now admits EXACTLY ONE mode and it is the
injected frequency**, where item 112 admitted 8-10; C10's motionless collect
**refuses outright**, the first real motionless collect to do so; and **item 96
is broken for the first time** -- 4 of its 12 motionless scenes now return NO
modal answer at all, against the 100% answer rate it measured.

**ITEM 108 IS IMPROVED BY AN ORDER OF MAGNITUDE AND NOT SOLVED.** C14's
motionless control still leads with 0.997 Hz against a sought 1.00 Hz, now at
**p 0.010** rather than 0.001 and with 2 admitted modes rather than 8. The
residual is most likely correlation extending **beyond the 2x2 tile**; a fully
dilated null puts the same block at p ~ 0.5, bracketing the truth. `--stable` is
still what rejects it. **Bound: C10 at 0.13 mm sits exactly on p = 0.050** --
the threshold, not a recovery.

**THE RANKING'S STRENGTH TERM WAS MEASURING THE BACKGROUND** (item 112,
implemented). `median_ratio` was the median over EVERY window that nominated a
bin, and most nominate by chance: each window picks `RS_MODAL_PER_WINDOW` bins
wherever they fall, so every bin of a K-bin band collects about `n_win * M / K`
from noise alone -- **22 of 225 at the 65-bin operating point every figure here
is quoted at**, against reported supports of 28-46. Measured, **a factor of five
in signal moved it by seven percent** (5.97 to 6.39). Taken over the windows of
the mode's own BLOCK -- its measured footprint -- the same experiment moves it
**14.48 to 67.74**. **H1 reaches 6 of 6**, the first time, against 5 of 6 in
items 110 and 111; **H3 2 of 2**; and **H3b falls to 0 of 12** motionless
synthetic scenes against a bar of 1, with injected recall held at 6 of 6. Seed
31, which survived items 107, 110 AND 111, is finally rejected.

**H1 AND `--stable` NOW DIFFER AND MUST BE QUOTED SEPARATELY.** H1 is the
128-look modal answer; `--stable` then adjudicates it and ABSTAINS on C10 at
0.13 mm, whose 256-look answer is above the 128-look Nyquist. **The selection
recovers 6 of 6 and the stabilization test reports 5 of 6.** Every earlier item
could quote one number because they agreed.

**The pre-registered risk was the right one and did not happen**: removing the
dilution raises EVERY candidate, so item 108's C14 control was named in advance
as what might gain most and fail H3. It gained **+0.4** where the injected runs
gained **+7.6 to +14.9** -- because that control's block is NOT internally clean
(ratio 5.3), so it gains nothing from a statistic that rewards a clean block.

**THIS REVISES ITEM 103's COMPETITION FLOOR.** C10 now recovers at **0.13 mm**,
below the 0.13-0.26 mm item 103 measured, because **the competition floor is a
property of the scene AND THE SELECTION together** -- item 103 measured it with
the selection of the time and three items of work moved it. Target and clutter
floors remain scene properties; **the competition floor is not one.** Bounds:
C10 at 0.13 mm is recovered by the modal set and NOT adjudicated; 0 of 12 is a
rate on twelve realisations, not a zero. **Item 108 is untouched by four items
of work.**

**THE BAND-EDGE BIAS IS REAL, AND THE FIX IS TO NARROW THE NEIGHBOURHOOD**
(item 111, implemented). Measured on **200 realisations of noise containing
NOTHING**, recording which bin won `rs_local_ratio()`'s maximum: the 39% of the
band whose neighbourhood was clipped took **72% of the maxima**, 2.98% per bin
against 0.75% -- **4.0x**. It is a VARIANCE effect, not a level one: the
background is a median, the median of 10 draws is twice as variable as the median
of 20, and the statistic is then MAXIMISED over the band. Item 110 tried
WIDENING every neighbourhood to mid-band's 20 and `test_tracking`'s red-floor
case killed it. **Levelling the count DOWN works** -- every bin takes the 10
NEAREST references outside the guard, what the band floor itself can supply, so
the span never exceeds what it was anywhere. **4.0x becomes 0.89x.** Nothing was
lost: **H1 5 of 6, H3 2 of 2, H3b 1 of 12 with recall 6 of 6**, item 110's
numbers exactly, and all three pre-registered predictions were correct for the
first time here. **What changed is an artefact vanishing**: the 10.148 Hz answer
that led C10's motionless control was **bin 61, inside the starved zone**, and
the control now leads with a mid-band 5.823 Hz. **The practical gain is in the
ABSTENTIONS** -- 5 of 12 to 3 of 12 synthetic, 2 of 8 to 1 of 8 real -- because
fewer answers land on edge bins whose 256-look partner falls above the 128-look
Nyquist. A discriminator that abstains less at the same false-positive rate is
strictly better.

**`tests/test_modalset.c` IS THE FIRST TEST EVER TO TOUCH
`rs_spectrum_modal_set()`** (item 111) -- not one test contained the word "modal"
before it, and `test_modalfit.c` is a different estimator despite the name. It
reproduces item 109's geometry and item 107's, so reverting to either ranking key
alone is caught in BOTH directions, and its calibration case admits 1 of 20
motionless scenes against a nominal 5%. **Writing it is what exposed item 112's
defect**, which is the argument for it: the function had been the subject of
seven items with no test at all.

**A LOCALISED TARGET IS REFUSED BY THE SUPPORT GATE AND THEN OUT-RANKED ON
EXTENT** (item 110, implemented; **corrects item 109**). Pre-registered at
`fce5869` with the kill criterion written first. **Item 109's guard-band
explanation is wrong — the third wrong explanation of the same failure**, after
the window boundary (item 108) and local clutter (item 109). Measured by
replicating the whole nomination offline from a `--shifts` dump: the replica
reproduces the binary exactly, and sweeping `RS_LOCAL_GUARD_BINS` **2 to 8 never
recovers the line**. The Hann-skirt argument does not even apply here — at
`--overlap 0` the floor is flat, 1.3x across the band.

**The target was lost TWICE.** (1) `support_min` was **34 of 225** and the
injected bin's support is **28**, so it was refused before block, ratio or
ranking were consulted. That threshold is a correct family-wise budget and a
*fraction of the whole window grid*, so a mode on a handful of windows cannot
reach it however strong — **CLAUDE.md's own localised-target rule, in the one
gate it had never been checked against.** (2) Block-first ranking then put it
fourth, behind three artefacts beating it by **one window**, while it led every
rival on strength: median local ratio **8.98 against 4.5-5.5**, its own window
nominating it at **41.5** on a psd peak **107x** that window's median.

**Fixed both ways.** Admission is `RS_MODAL_BLOCK_MIN` on support — the 2x2 block
floor restated, refusing only what the block gate refuses anyway — with
`rs_modal_null()` drawn under the SAME rule so the chance block rose 7 to 9 and
the family-wise correction is not lost with the threshold. Ranking is by
`evidence = n_contiguous * log(median_ratio)`, the exponential periodogram model
`rs_twin_llr()` already states, summed over carrying windows. **Each key alone
fails on the other's case and both were measured**: block alone loses the
localised target; **ratio alone is worse**, handing item 107's injected fixture
to seed 7's **1.512 Hz artefact (ratio 38.8, block 11)** over the true **0.504
(ratio 25.0, block 30)**.

**H1 5 of 6** against item 109's 3, **H3 2 of 2**, kill criterion **H3b 1 of 12 —
item 107's rate exactly — with injected recall held at 6 of 6.** And the
recovery is now a **threshold you can read off the report**: the injected line's
`ev` against its scene's competition runs 16.6 / 24.0 / 28.5 on C10 against a
competition of 23.8, and 40.5 / 51.6 / 55.1 on C14 against 15.5, so **C10
crosses between 0.13 and 0.26 mm — item 103's competition floor, reached
independently through a different statistic.** **NOT fixed: C14's motionless
control still leads with 0.997 Hz at `ev` 28.3** against an injected 1.00 Hz, so
item 108 stands and `--stable` is still the only thing rejecting it. A defect
found and left for item 111, which fixed it: `rs_local_ratio()` scored band-edge
bins against **10 reference bins against mid-band's 20**. Read item 111's figures
in preference to this item's `ev` values, which were measured under that bias.

**THE STABILIZATION TEST CAUGHT A FALSE POSITIVE SITTING ON THE SOUGHT
FREQUENCY** (item 108). Pre-registered at `25f7351`; `--stable` put on a
LOCALISED point target 20 m off the origin at 0.13-0.53 mm on REAL Kilauea
clutter. **C14's MOTIONLESS control reports 0.997 Hz at 128 looks against an
injected 1.00 Hz** — on a real collect, at the look count used throughout this
project, a scene with nothing in it gave an answer 0.003 Hz from the signal being
sought. **`--stable` rejected it**, because the same scene says 7.828 Hz at 256
looks. **Every statistic in items 38-99 would have endorsed it.** H3 passes: both
motionless controls refused. **But recall is another matter and item 107's 6-of-6
does NOT transfer**: only 2 of 6 injected runs report, because **C10 recovers at
256 and not at 128** (0.998 against 0.499) at amplitudes item 103 recovered on
the SAME collect with the target at the GRID ORIGIN — the only change is
`--inject-at 20,20`, which is items 40-41's split across four overlapping
windows, and it costs the signal before `--stable` is consulted. **Three of eight
comparisons ABSTAINED** ("not comparable") because the 256-look answer landed
above the 128-look Nyquist; closer look counts would share more band, untested.
**Honest summary: `--stable` is a SPECIFICITY instrument.** Nothing measured says
it preserves recall on a weak localised target, and item 108's own design
prevented that question from being answered.

**`--stable WINDOWS_CSV` IMPLEMENTS THE STABILIZATION TEST** (item 107,
implemented). It compares this run against another run of **the same scene at a
different look count** and reports what survives; `stable_hz` and `stable` join
the per-window CSV. **The verdict is on the MODAL SET's leading frequency, NOT on
any window's `dominant_hz`** — verified, an injected scene whose modal answer is
0.504 Hz at both look counts can have its strongest window report 0.504 at one
and 2.571 at the other, so comparing windows rejects true recoveries; the first
implementation did exactly that and was wrong. Reproduces item 107's measured
values on seed 7: motionless **1.512 -> 0.806 MOVED**, injected **0.504 -> 0.504
STABLE**. **Refuses what it cannot test**: equal look counts (vacuous, with the
reason), a different window grid, a missing or column-less file; warns when `df`
differs because that is a comparison across DWELLS, untested. Frequencies above
the lower run's Nyquist are marked NOT COMPARABLE rather than unstable.

**A BLIND DISCRIMINATOR THAT WORKS: REQUIRE THE FREQUENCY TO SURVIVE A CHANGE OF
LOOK COUNT** (item 107). Pre-registered at `f6ad9f4` with the prediction recorded
first; both hypotheses pass. **False positives fall from item 96's 12 of 12 to
1 of 12, and all 6 injected scenes survive.** Twelve motionless scenes and six
injected, each processed at **128 and 256 looks** with nothing else changed; a
scene reports only if the two answers agree within half a bin. Seven statics
moved within the band (0.15-2.07 Hz) and four moved out of it; every injected
scene gave **0.504 Hz at both counts, to three decimals**. **This is the OMA
STABILIZATION DIAGRAM — "spurious modes vary, physical modes stay constant across
model orders" — with the LOOK COUNT as model order. Item 70 identified the
principle and substituted the SPATIAL WINDOW, which is the wrong axis**; the
analogue of model order is a processing parameter that changes what the estimator
fits. **Why it matters more than anything in items 96-106: it needs NO TWIN, NO
NULL CONTROL and NO GROUND TRUTH** — only the collect, processed twice — where
`--null-static`, item 97's `--twin` and item 38's zero-amplitude control each
need something a single real collect cannot supply. **Bounds**: the injected
fixture is whole-scene `--clutter-vib` at 2 mm, the easy case, so 6 of 6 is not a
claim about real structures; one static survived, so the rate is 1 in 12 and not
zero; `df` was identical at both counts because the dwell was fixed, so a
comparison across different DWELLS is untested; and the rule includes restricting
to the common band, since a frequency above the lower count's Nyquist has nothing
to be stable against.

**THE TRANSITION IS LOOK-COUNT INDEPENDENT, AND A REAL BRIDGE HAS BEEN MEASURED
WITHOUT CORNER REFLECTORS** (item 106). Pre-registered at `833c160`: the
brightness transition at **256 looks is identical to 128**, collect for collect
(C10 recovers only at REL 20, C14 at 20 and 10). **The sub-look SCR penalty is
dead in every regime tested** and the 11-17 dB gap against PS-InSAR is not about
sub-look resolution. Kept detail: the uninjected artefact MOVES with look count
and differently per collect — **C10 goes 0.665 to 1.331 Hz, exactly 2x and the
same fraction of Nyquist, so it is periodic in LOOK INDEX (16 looks per cycle);
C14 goes 0.499 to 20.486 Hz**, band floor to near Nyquist. Two motionless scenes,
two mechanisms. **CORRECTION TO ITEM 104**: Lotti et al., *Monitoring Bridge
Vibrations via Spaceborne SAR Micro-Doppler* (Struct. Control Health Monit.,
publ. 13 Jan 2026, open access) measures the **South Portland Street Suspension
Bridge** from two Umbra-04 spotlights against **synchronous** ground truth, with
**NO corner reflectors** — because *"the bridge is highly reflective... due to its
steel structure and the presence of sharp features and corners"*. Peak LOS
velocities **0.5-2 mm/s**, frequencies **1.5-2 Hz**, velocity error **~1 mm/s**,
**spectral correlation up to 0.88**, `df` **0.06 Hz from 16 s**. **So a real
structure CAN supply the 20-26 dB itself if it is steel with sharp corners** —
item 104's requirement stands, its pessimism does not. **This also updates item
70**, which quotes the EVACES 2025 conference version at `df` 0.138 Hz and
Pearson 0.33-0.47; the journal figures are **0.06 Hz and up to 0.88**, and those
are the ones to quote.

**THE PREDICTED SUB-LOOK SCR PENALTY IS NOT THERE** (item 105). I reasoned from
the literature that N sub-looks each carry `B/N` of the bandwidth, so each cell
holds N times more clutter and costs ~21 dB at 128 looks — which would have
explained item 104's 11-17 dB gap against PS-InSAR's ~9 dB (`D_A <= 0.25`).
**Measured, it is absent**: with a fixed target the phase sd is **FLAT at
0.006-0.008 rad from 32 to 256 looks**, where a real penalty would move it by
9 dB. The floor instead FALLS as **N^-0.36** against N^-0.50 for pure averaging,
so more looks is simply better: **0.0036 mm at 32 looks, 0.0017 mm at 256**, with
the scene's clutter floor falling 1.01 to 0.40 mm alongside. **N = 16 fails for
an unrelated reason** — 9 bins of which 3 are leakage, so prominence is bounded
near 6 and the statistic is starved; its 1.353 rad sd is the SCENE's, because the
target was never selected. **The 11-17 dB gap is therefore UNEXPLAINED**, and two
untested possibilities remain: the penalty may bite only at low REL where the
target stops dominating its enlarged cell (one sweep away), or the comparison
with PS-InSAR may be meaningless because it estimates slow deformation over many
passes while this estimates a spectrum inside one dwell. **Practical finding: the
look count has been fixed at 128 throughout this file for reasons never measured,
and 256 gives a 23% lower floor** at a sub-aperture response of 0.999.

**RECOVERY NEEDS 20-26 dB OF SIGNAL-TO-CLUTTER, AND THE FIELD PUTS A CORNER
REFLECTOR ON THE STRUCTURE** (item 104). Pre-registered at `c0de7f5`; only the
target's brightness varies, amplitude fixed at 0.26 mm. **The transition is
between 14 and 26 dB and it is SHARP**: at REL 20 both collects recover with a
target-window floor of 0.0022 mm; at REL 5 and below neither does and the floor
jumps to the clutter's ~0.49 mm. **The floor is BIMODAL** — the target either
dominates its pixel or it does not exist as a scatterer, nothing between — and
below the transition the reported frequency is exactly the uninjected scene's own
artefact. **My prediction of a REL 2-5 transition was WRONG**; it needs far more
brightness. **This does NOT contradict item 51**, which swept REL against the
target's OWN quadratic residual — that scales as REL^2 like the signal, so it is
brightness-neutral; here the competition is the SCENE's clutter artefact, which
does not scale with the target. **Quote each with its competition named.**
**THE LITERATURE AGREES AND HAS ALREADY ACTED ON IT**: Vattulainen et al. (IEEE
2026, Strathclyde/Trento/Houston/DLR), assessing this exact technique against
synchronous accelerometer ground truth on Umbra and ICEYE, mount **CORNER
REFLECTORS ON LINEAR ACTUATORS** — buying the 20-26 dB by hardware. Their
demonstrated floor is **0.10 mm RMS radial displacement**, against item 103's
independently-measured 0.13-0.26 mm competition floor. **They have not measured a
structure**: "these results pave the way for future measurements of
infrastructure", and theirs are the first synchronous SAR-and-accelerometer
vibration measurements at all. **So items 83-89's failure to find a
structure-under-aperture pairing is not a shortfall against the field — the field
has not done it either.**

**THE PREDICTED FLOOR IS NOW PER WINDOW** (item 103, implemented).
`rs_microm_floor()` takes a window's own phase series, summarises it CIRCULARLY
(`sd = sqrt(-2 ln R)`, item 101's correction) and returns
`sd * lambda/(4*pi) * sqrt(2/N)`. Every phase-route run prints the floor at the
strongest-prominence window beside the scene median and the quietest window, and
`floor_mm` joins the per-window CSV. Verified against item 103's hand
computation: **0.0157 mm and phase sd 0.049 rad at the injected target against a
hand-computed 0.0154 and 0.050, ratio 34.4x against 34.0x.** **A NEW MEASUREMENT
FELL OUT: on the UNINJECTED Kilauea collect no window is quiet — floors run
0.4237 to 0.6491 mm with a ratio of only 1.3x**, so that scene contains no
naturally bright scatterer at all and item 102's conclusion is stronger than it
was stated: it is not that the median is bad, it is that there is no good window
anywhere in the scene. **Valid as a floor only where nothing is injected** — a
moving window's circular sd includes the motion — and it is the TARGET floor, not
the COMPETITION floor that actually governs recovery.

**THE FLOOR IS PER-TARGET: THREE FLOORS, 34x APART** (item 103). The injection
framework run on real Kilauea clutter, pre-registered at `201d897`. **H1 FAILED
as stated**: recovery occurs BELOW item 102's predicted floor — at 0.5x on one
collect and 0.25x on the other — so the prediction is not the boundary. **Cause:
item 102 predicted from the scene's CLUTTER and `--inject-wave` defaults to
`rel = 20`, a BRIGHT POINT TARGET.** Measured with `--shifts` on the injected
run, **the target's window is 34x quieter than the scene median — circular sd
0.050 rad against 1.693 — so its floor is 0.0154 mm, not 0.522 mm.** That is
item 101's own conflation recurring in my prediction one item later. **THREE
FLOORS**: **target** ~0.015 mm (item 53's 0.0055 mm is this quantity),
**clutter** 0.52 mm (item 102), and **COMPETITION 0.13-0.26 mm — what an
injection must reach to beat the scene's own artefact, which is the OPERATIVE
one** and is neither of the others. **Item 102's Kilauea conclusion still
stands**, because a seismometer measures the ground, which is clutter, at
0.137-1.728 um against a 0.52 mm clutter floor. **Item 102's protocol was right
in form and wrong in the scatterer: predict the floor AT THE WINDOW THE TARGET
OCCUPIES, and always say which of the three floors a number is.** H3 passed
(neither control reported the injected frequency); H4 was weak — the twin LLR
rose but plateaued and **never reached p < 0.05**, exactly item 98's ceiling that
a single-look pair needs a power ratio above 19.

**THE FLOOR IS PREDICTABLE FROM AN UNINJECTED RUN, AND IT KILLS THE KILAUEA TEST**
(item 102). Item 101's arithmetic — circular phase sd, times `lambda/(4*pi)`,
times `sqrt(2/N)` — is a PREDICTION obtainable before any injection, which this
project had never made. On three complete real Kilauea collects: **circular sd
1.686-1.716 rad, coherence 0.381-0.387, 4.2 mm per look, predicted floor
0.520-0.529 mm** — strikingly consistent, a property of the scene type.
**REAL DATA IS 1.8x WORSE THAN THE SYNTHETIC FIXTURE, not better.** I expected
the reverse from item 12f's "a real collect's 0.85" — **that 0.85 is the
CORRELATION PEAK, not the sub-look PHASE coherence, and they are different
quantities.** The cause is in the same run: **amplitude dispersion median 0.567,
0 of 225 windows meeting D_A <= 0.25** — Kilauea lava has no persistent
scatterers, so item 15's precondition is unmet scene-wide, which is item 19's
Giza finding on a second real scene. **CONSEQUENCE: the Kilauea truth is
0.137-1.728 um against a 529 um floor — the BEST scene is 306x below it**, so
item 67's correlation test cannot work: below threshold the estimator decouples
from truth (item 82) and the correlation is zero by construction. Item 67 scoped
it against the 0.0055 mm figure, which is **96x too optimistic for distributed
lava** because it was measured on a bright coherent point target. **The
injection framework needs no new code** — `--inject-wave`, `--inject-at`,
`--shifts`, `--probe-hz`, `--twin` all exist. What was missing is the protocol:
**predict the floor from an uninjected run, inject at amplitudes BRACKETING the
prediction, and report recovery against it.** Every measurement in FOLLOW-UPS so
far chose an amplitude and found out afterwards whether it was above the noise.

**THE ARTEFACT IS JUST PHASE NOISE, AND THE QUOTED FLOOR DOES NOT APPLY TO THE
FIXTURES** (item 101). `--shifts` dumps the tracked series and nobody had looked
at it. On a MOTIONLESS scene, `--estimator phase` at 128 looks gives **circular
sd 0.940 rad (R = 0.643) = 2.32 mm rms of apparent displacement — 421x item 53's
0.0055 mm floor**, implying a sub-look coherence of ~0.70. The periodogram of a
128-sample noise series HAS a peak by construction, at a different frequency per
window (**0.302-3.024 Hz, median 1.663, no preferred value**), holding **14.8% of
band power against white noise's 4.8%** — which IS the prominence 12-37 of items
96 and 99. **No selection statistic can fix a 421x noise-to-floor ratio**, so
items 91-100 were arguing about how to choose among peaks in 2.3 mm of noise.
**THIS FIXTURE'S REAL FLOOR IS 0.29 mm** — 2.32 mm times sqrt(2/128) — **53x the
0.0055 mm this project quotes**, because that figure was measured on an injected
BRIGHT COHERENT POINT TARGET and not on clutter; averaging 128 looks buys 11.3x,
not the 422x the two differ by. That makes a THIRD floor beyond item 66's two:
per-look CRLB, end-to-end, and now **per-FIXTURE**. **Quote a floor with the
scatterer it was measured on attached.** It also explains the recall in items 91
and 95 with no appeal to policy: a 2 mm injection is 6.9x this floor, 0.5 mm is
1.7x, 0.3 mm is at it. **The lever is SUB-LOOK COHERENCE, not statistics** — and
item 12f showed this fixture family cannot exceed 0.323 by construction, so
**items 96 and 99's false-positive rates are bounded to this fixture and are not
properties of the method.** A correction recorded there: a 14.27 mm peak-to-peak
against a 7.75 mm ambiguity looked like saturation and is not — a wrapped
quantity must be summarised CIRCULARLY.

**THE MOTIONLESS-SCENE ARTEFACT IS NOT THE OFFSET-DRIVEN CARRIER** (item 100).
Item 99 pointed the work at the carrier removal; that pointer is WRONG. If the
artefact were item 63's `(4*pi/lambda)*dX*dx/R`, shrinking the analysis cell must
move it, since `dx` is bounded by half a cell — and item 63 measured exactly that
on real Giza data, **70.7 / 36.3 / 159.0 at 1.0 / 0.25 / 0.125 m**, a 4.4x
non-monotone swing. With the SCENE HELD FIXED and only the grid varied, item 99's
metric gives **25.6% / 18.0% / 19.9% / 19.2%** at 1.0 / 0.5 / 0.25 / 0.125 m —
**flat**, moving under 10% from 0.5 m down while the window count changes 20x.
So the artefact is **independent of the imaging grid**, which puts it UPSTREAM OF
THE WINDOWING — in the sub-aperture formation or the tracked series itself — and
**refining cells or the polynomial fit will not remove it**. All three candidate
explanations for item 96's 100% false-positive rate are now eliminated: the
frequency search (~20% SNR, item 98), multiplicity (item 99), and the residual
carrier (this item). **Bound**: item 63 measured real Giza with an injected
target and this is synthetic motionless clutter, so item 63's mechanism is not
disproven where item 63 measured it. **The untouched next thing is the TRACKED
SERIES ITSELF** — what a motionless scene's per-window phase series looks like
before any spectrum is taken, which has never been plotted here.

**ITEM 1 IS ANSWERED AND THE ANSWER IS NO** (item 99). Multiplicity does not
explain item 96's 100% false-positive rate. The modelled family-wise threshold
over 49 windows x 62 bins is prominence **11.01**; the twelve motionless scenes
report **20.1-36.9**, all clearing it, with implied family-wise p from 5.7e-6 to
2.9e-13. The **empirical null** (588 per-window prominences over 12 motionless
scenes, `runs/synthetic/2026-08-06-null-distribution/`) shows why the model is
useless: **mean 14.21 against Exp(1)'s 1.00, p95 27.06 against 3.00 — fourteen
times too large.** Since prominence is the peak over the mean of 62 bins, **a
prominence of 14 means one bin holds 23% of the band's power on a scene where
NOTHING MOVES**, and the worst holds 60%. That is a COHERENT LINE, not a chance
maximum, and item 63's per-realisation residual carrier is the mechanism.
**Both candidate explanations are now eliminated** — the frequency search costs
only ~20% in SNR (item 98), and multiplicity cannot bridge 11 to 37. **What
remains is that the artefact is REAL SIGNAL in the tracked series, so the
CARRIER REMOVAL (items 51-53, 63) is where to attack it** — a different half of
the codebase from everything items 91-98 touched. A self-calibrated threshold
above all twelve statics must exceed 36.9, which admits item 38's real
injections at 38-47 **and its motionless bright scatterer at 56.3**: it separates
motionless CLUTTER from an injection and still cannot separate a motionless
SCATTERER from a moving one.

**THE TWIN LIKELIHOOD RATIO IS IMPLEMENTED, AND ITS CEILING IS THE FINDING.**
`rs_twin_llr()` is the two-sample GLRT for exponentially-distributed periodogram
bins: **`LLR = 2 log((1+r)/2) - log r`**, one-sided, **scale-free** (only the
ratio matters, where a difference is part level and part change), with the
p-value **`1/(1+r)`** from the F(2,2) tail — no simulation. **THAT p-VALUE
ASSUMES THE TWIN IS AN INDEPENDENT OBSERVATION, WHICH A SIMULATOR TWIN IS NOT**:
measured, `sim_cphd` at one seed produces a BIT-IDENTICAL file, so under the null
the ratio is exactly 1 on every window rather than F(2,2)-distributed, and the p
is then grossly conservative — there is no noise floor and any ratio above 1 is
the motion. `mmotion` detects the case (ratio 1 to within the evidence file's
`%.12g` round-trip) and warns. The p is right for two SEPARATE acquisitions of
one scene, which is where CCD uses it. `--twin`
reports it; the CSV carries `twin_llr` and `twin_p`. **Do NOT use a chi-squared
asymptotic**: Wilks is asymptotic in sample size and there is ONE periodogram
sample per mean — measured, `2*LLR` reaches 4.67 at p95 against the half-mass
chi-squared's 2.71. **Two degrees of freedom means the power RATIO must exceed 19
before p < 0.05**, so a single-look pair cannot call a modest excess however
large the raw difference looks; multilooking is the remedy and this chain has no
independent looks to give it. The report also states how many windows clear 0.05
against how many chance expects, because **the per-window p is UNCORRECTED for
testing 49 of them — item 1, open since the beginning**.

**`--twin` IS COHERENT CHANGE DETECTION AND THE PEAK SEARCH IS THE LOOK-ELSEWHERE
EFFECT** (item 98). **Sixth time the field already had this.** (1) CCD is a mature
SAR discipline doing exactly this paired same-scene difference, with the same
stated geometry precondition, and it has already ranked the statistics: **the LOG
LIKELIHOOD RATIO change statistic beats the power ratio and the sample coherence**
— `--twin`'s raw prominence difference is the crude member of that family, and
CCD's **double change map** (two twins, not one) is a known false-alarm reduction
never tried here. (2) The GLRT over unknown frequency costs about **20% more SNR
than a known-frequency detector at 95% detection** — real but MODEST, so it
**does not explain item 96's 100% false-positive rate**; that bounds the
diagnosis rather than confirming it. (3) **Item 1 — this project's OLDEST open
entry, "no multiplicity correction" — is the LOOK-ELSEWHERE EFFECT**, standard in
cosmology and particle physics, normally corrected by a simulated trials factor,
which is what item 80's `p_chance` reinvented; **Bayer & Seljak (MNRAS 508, 1346,
2021) self-calibrate it from the observed peak heights with no simulation**, a
direct replacement that works PER WINDOW where `p_chance` works on the block.
(4) The twin's failure mode is SHM's best-documented one — baseline subtraction,
where "operational and environmental variations masquerade as damage"; their
remedies are **Optimal Baseline Subtraction** (a library of baselines in small
temperature steps) and **Optimal Signal Stretch**, and the absence of any such
library on a real collect is a sharper statement of item 97's "there is no twin
to pair against".

**`--twin WINDOWS_CSV` REPORTS THE TWIN-DIFFERENCED STATISTIC** (item 97,
implemented). It differences this run against a previous run's evidence file at
the frequency `--probe-hz` names, and **requires `--probe-hz`** — differencing
dominant-peak columns when two runs peak at different frequencies is how a
motionless control outscored a real injection (item 38). It refuses a twin
probed at a different frequency, over a different grid, without probe columns, or
that cannot be read. **Read the EXCESS of the best window over the median, not
the total**: the median is what the whole scene did, and on a `--clutter-vib`
fixture a large median is expected. It **gates nothing**, and on a real collect
there is no twin to pair against — which is why `--null-static` exists. What it
CANNOT check is whether the twin is the right kind of control: paired against an
UNINJECTED run rather than a ZERO-AMPLITUDE one it measures the scatterer's
presence and exceeds a real signal (item 39).

**THE PAIRED SAME-SCENE TWIN FINDS THE SIGNAL THAT THE REPORT LOSES** (item 97).
Pre-registered at `8543974`. 24 injected points each paired with a twin on the
SAME seed and clutter differing only in whether the target moves;
`D = probe_prominence(injected) - probe_prominence(twin)` at the injected
frequency. **H1 passes at exactly the threshold, `D > 0` in 20 of 24.** The
finding: **of the 17 points whose REPORTED frequency was wrong, 13 (76%) have a
POSITIVE D at the frequency injected** — including three seed-7 runs that reported
the 1.512 Hz artefact while carrying D of **+7.2, +16.4 and +19.5**. **The chain
measures the injected frequency and reports the artefact instead**, so item 96's
100% false-positive rate is a property of the SELECTION and not of the
measurement — items 7-9 and 30 restated with a paired statistic behind them. D
tracks source modal prominence (seg 02 +3.2..+28.3, seg 13 +0.08..+3.9) and rises
almost monotonically with target frequency (item 47's red floor). **H2 was
MIS-SPECIFIED by me**: it required the median-window difference to be under a
tenth of the centre's, but this fixture uses `--clutter-vib` which moves the WHOLE
patch, so a scene-wide increment is correct and H2's form belonged to item 39's
single-scatterer `--inject-vib` setup. **H2 is inapplicable, not failed** —
recorded rather than quietly dropped. **The open work**: nothing in `mmotion`
reports a twin-differenced statistic; every policy in `spectrum.c` selects a peak
from ONE scene's spectrum, and D needs two runs. Bounds: `D > 0` is measured with
the truth known and a real collect has no twin to pair against.

**IT IS NOT SEED 7: TWELVE OF TWELVE MOTIONLESS SCENES REPORT A CONFIDENT
FREQUENCY** (item 96). Item 95 said the 1.512 Hz artefact was seed-bound, which
implied seed 11 was clean. **It is not** — seed 11 answers 1.210 Hz. Twelve
static scenes, identical processing: **12 answered, 0 refused, 9 DISTINCT
frequencies** spread over 0.151-2.369 Hz, **every one clearing the chance model**
at p <= 0.012 and blocks 7-16 against the 6 chance reaches (item 80). Nothing
moved in any of them. **The false-positive rate of the reported modal set on
motionless clutter is 100% at this operating point**, and the modal set and
`rs_spectrum_best_window()` **agree on 0 of 12 scenes**. So the fixture cannot be
"fixed": each realisation has its own dominant-scatterer geometry and so its own
residual carrier (item 63), and **a different seed renames the artefact rather
than removing it**. This bounds items 91 and 95 hard — their scores are an
injected line winning against a scene-specific artefact of comparable strength,
not a quiet chain given a signal — and it is **item 11 with a number on it: a
per-scene null control is the only thing between this chain and a 100%
false-positive rate.** Bounds: one fixture family, one estimator, one operating
point; real collects are not covered, and item 17's Giza run did return a null.

**ITEM 91 REPLICATES, AND THE 1.512 Hz ARTEFACT IS SEED-BOUND** (item 95). The
first run here with hypotheses **pre-registered before it ran** (commit
`cb47b06`); the recorded prediction that H1 would fail was correct. Naples
injected through settings identical to items 74/77/80/81/91: **7 of 24 correct,
slope +0.779, rms 0.7295** against Granada's **1 of 24, slope -0.948**. H1 and H3
fail on both, so the kill criterion applies and *this chain does not recover
ambient structural motion* stands at the bar. **But: ALL SEVEN 1.512 Hz answers
are SEED 7, and seed 11 returns it NOT ONCE** — seed 7's static control reports
1.512, seed 11's reports 1.210. **Item 76 called 1.512 Hz common-mode "in all
three scenes"; it is common to one CLUTTER REALISATION, not to the processing**,
and since every sweep in items 80-95 used seeds 7 and 11, the 1.512 Hz recurring
throughout `FOLLOW-UPS.md` is seed 7's signature — quote it as such. Two further
structures in the failure: **source modal prominence predicts recovery** (segment
02 scores 5 of 12, segment 13 scores 2 of 12, both unit-normalised so it is
spectral SHAPE not amplitude), and **recovery rises with target frequency**
(1,0,1,1,1,**3** of 4 from 0.30 to 0.90 Hz — item 47's red floor). The full
ordering is now **sine 6/6, burst 3/12, ambient-with-sharp-mode 7/24,
ambient-diffuse 1/24**: the chain degrades smoothly with how concentrated the
injected motion is, which is what a prominence-based selection must do.

**A SECOND BUILDING RECORD, AND ITS MODE IS INSIDE THE BAND** (item 94).
`zenodo.org/records/20667124` — former military hospital, Naples: a multi-setup
AVT, 4 tri-axial + 2 bi-axial accelerometers, 4 setups x 20 min at 200 Hz,
**CC-BY-NC** (Granada is CC-BY), with **sensor x,y,z coordinates so MODE SHAPES
are reconstructable** — what items 70-71's spatial test has never had from a real
structure. **No pairing** (0 collects over Naples centre; nearest Umbra centre
14.1 km). **Its loudest ACCELERATION peak is setup-dependent — 14.4, 12.9, 11.8,
19.5 Hz across the four setups — so it is mounting resonance, not a mode**; a
building's modes do not move 60% between setups, and only the multi-setup design
exposes it. Same lesson as Granada's 0.22 Hz microseism on 5 of 6 sensors: **the
tallest line in an SHM record is routinely not the structure.** In DISPLACEMENT
it is **2.64 Hz, stable** against corners of 0.8/1.0/1.2 Hz. **That is INSIDE the
band at a 20 s dwell and 128 looks (0.150-3.20 Hz)** where Granada's 3.78 Hz was
not — so **Naples is the better injection source**, needing no change to the
operating point everything here is quoted at. Real peak displacement 0.80-1.23 um,
below the floor like every instrument in items 83-91.

**PRE-REGISTRATION IS NOW SEEDED WITH EVERY RUN** (item 92, adopted).
`docs/PREREGISTRATION.md` holds the form and the reasoning; `tools/new-run.sh`
writes a filled-in `PREREG.md` beside `RUN.md` and prints **"commit it BEFORE
running anything"**, so the git history shows the hypotheses predate the data. It
encodes this project's own failures as checks: H1 needs a SWEEP not a point
(item 2), H2 is scored on the INJECTION GEOMETRY (item 40), H4 requires the
ZERO-AMPLITUDE TWIN (item 38), the pipeline freeze demands `sigma_px` be checked
against the injected amplitude before anything is built on a tracked series
(item 76), the null section records that a SHUFFLE NULL IS INVALID for
`--estimator phase`, and the reporting rules bind every figure to its
configuration (items 77, 84, 86). Runs may be marked **exploratory** — that is
legitimate; exploration reported as confirmation is not.

**FOUR THINGS TRANSFER FROM THAT REPOSITORY'S VALIDATION PRACTICE** (item 92).
**(1)** Their `ERRATUM_KOMATI.md` is this project's recurring rule from outside: a
table row computed at `n_sub = 128` where every other used 11, UNDISCLOSED, put a
result "precisely on the > 5x decision rule"; re-run at the standard setting it
is 2.8x/1.3x. This project has written the same rule three times — item 77 (look
count), item 84 (sample size), item 86 (time filter). **Four instances, two
codebases, one failure: a number carries its configuration or it means nothing.**
**(2)** Sweeping `n_sub` 11→128 gave a **17.8x** contrast spread on the native
axis, collapsing to **1.9x** on a FIXED window — the axis changing extent, not
the signal. That is item 47's "prominence is not comparable across `--fmin`" in
another domain; score on a fixed support. **(3)** Their 5.0x threshold is
calibrated on **400 synthetic null runs** (p95 4.35, p99 5.03, ~2% FPR) — derived
from the null's own distribution, exactly item 80's `p_chance` reasoning, reached
independently. **(4)** A sensitivity axis never swept here and now
deprioritisable: **96 configurations x 200 runs** over window (Blackman/Hann/
Hamming/rectangular) x precision (float32/64) x coregistrator moved **no
verdict**, float32 and float64 agreeing to three significant figures. That does
NOT license `-ffast-math` (reassociation and denormals, not word size) but it
says the window and the coregistrator are not where answers live. **Worth
adopting: `VALIDATION_PROTOCOL.md`, a pre-registration template** — falsifiable
H1-H4, frozen pipeline and null model, BLINDING, written kill criteria, "report
all slices including misses". This project pre-registers exactly one thing,
`runs/kilauea/.../analyse.py`.

**A REAL BUILDING'S AMBIENT MOTION GOES IN AND THE ANSWER IS THE STATIC SCENE'S**
(item 91). Item 89's Granada record injected and swept at settings identical to
items 74/77/80/81: **1 of 24 correct, which is chance**; the most common answer
is **1.512 Hz seven times** — item 76's common-mode artefact — and **9 of 21
answers are a value a STATIC control also returns**. Ordered against the earlier
injections: **sine 6/6, earthquake burst 3/12, ambient 1/24**. That ordering is
the finding: a sine is one line holding all the energy, a burst has a dominant
arrival, and **AMBIENT response is stationary, broadband and multi-modal so no
line is ever prominent** — the selection falls through to the processing's own
artefact. Every synthetic recovery here used the easiest possible motion and this
measures how much that mattered. Waveform prep matters and is recorded: the
2.5 Hz high-pass was chosen by measurement, because below 2 Hz the dominant
DISPLACEMENT frequency tracks the filter corner rather than the building.

**AN INDEPENDENT REFUTATION OF THE DEEP GIZA CLAIM ENDORSES THE FRONT END**
(item 90). Foreman (June 2026 preprint) reproduces Biondi & Malanga from the
patent and refutes the DEPTH inversion while stating that the surface-vibration
front end — what this project builds — "is legitimate and well-precedented" and
"that work stands". Its added controls are the ones arrived at here separately:
a **look-order-shuffle null** (`rs_shuffle_looks`, `--null-static`) and an
**in-data positive control** (`--inject-vib`; item 38's zero-amplitude twin goes
further). Three of its results confirm this project's from outside: **stacking
five passes over a known-empty pit reinforces a surface-pinned artefact at 96.7x
the null** (item 11, and items 64-65 from the other side); **a contrast-vs-null
ratio of 1720x can be an artefact** (item 38's prominence 56.3 on a motionless
target, item 49's desert at 34.4) — **a large ratio against a null is not a
detection**; and **free archives are in the wrong MODE**, spotlight tasking being
required (item 58). Its mechanism for a false positive — "a DFT returns a
structured, peaked spectrum from any input" — is, with the depth axis stripped, a
statement about `rs_spectrum_compute_opts()`.

**A CONTINUOUS 6-SENSOR BUILDING RECORD EXISTS, AND ITS MODES ARE ABOVE THE BAND
AT 128 LOOKS** (item 89). `zenodo.org/records/17358241` — Hospital Real, Granada,
six accelerometers at 200 Hz, **CONTINUOUS for 7 days**, CC BY 4.0. **No collect
over it in any archive** (nearest centre 91 km), so it is not a pairing — but it
is the best INJECTION source here: continuous where item 88's structural
instruments are event-triggered, AMBIENT where item 69's Oroville was a
transient, six sensors so a mode SHAPE exists for items 70-71's spatial test, and
7 days at 200 Hz so a sweep can pool over thousands of INDEPENDENT real-structure
realisations, which `rs_track_fit` demands and item 69's single record could not
give. **Its modes were measured without downloading 8 GB** — Zenodo serves range
requests, and a 40 MB slice inflated through BOTH layers (zip deflate, then gzip)
gives 47 minutes at 200 Hz. **Dominant mode 3.78 Hz on 4 of 6 sensors, family
3.27-4.32 Hz**; the 0.22 Hz peak on 5 of 6 is the secondary ocean MICROSEISM, not
a mode. **3.78 Hz is OUTSIDE the observable band at 128 looks** (0.100-2.13 Hz at
30 s) — it needs **256 looks**, changing an operating point items 13, 76 and 77
are all quoted at. **A stiff masonry building is a poor match for a long dwell**:
long dwell buys resolution and spends Nyquist.

**ESM WITHDRAWS "NO DAM HAS EVER BEEN IN A FOOTPRINT", AND THE WALL MOVES TO THE
INSTRUMENT** (item 88). Item 87 recorded ESM as refused with 413; retried it is
the richest source in the search. Query it with `network=` (`net=` is AMBIGUOUS
on that server) and tile geographically, SUBDIVIDING RECURSIVELY ON 413 — 3890
stations, 69 empty tiles counted. **Items 83, 86 and 87 each concluded no
instrumented dam had ever been inside a footprint. That is WRONG.** `CH.DIX` and
`CH.SDIF` sit at **GRANDE DIXENCE, the tallest gravity dam in the world, inside
a 34.7 s Capella footprint** — missed because every one of those screens found
structures by SITE NAME and these contain no "diga"/"dam"/"barrage". **A name
heuristic finds the word, not the structure, in both directions**: it invented
bridges from communes containing "Pont" and lost the largest dam in Europe, so
every structural count in items 83-87 is a lower bound on coverage and an upper
bound on precision. ESM also yields far more pairings — Capella **24**
dwell-adequate against RESIF's 1 and INGV's 5, including **`A.FAT` Fatih-Tomb
Istanbul at 60.0 s**, the longest anywhere, and Roma Palazzo Valentini/Spada at
33.2 s. **But the structural stations are EVENT-TRIGGERED**: `CH.SDIF`,
`IT.RMPV`, `IT.RMPS`, `IT.RMUL` all return NO DATA, holding a record only when
something shook them. So item 83's duty-cycle wall reappears from the INSTRUMENT
side. The one continuous instrument there, a bedrock vault station, reads
**0.113 um, 48.7x below the floor**. Every continuously-recording instrument on
four networks reads 7-50x below: Oroville 0.5-0.8, FR.CURIE 0.510, Etna
0.589-0.803, Grande Dixence 0.113 um.

**INGV JOINED: A THIRD NETWORK, THE SAME FLOOR** (item 87). 1719 stations / 66
networks. Italy instruments its structures and INGV publishes them — `IV.ME05`
Diga Del Menta, **`7C.SPAOL` Catania Viadotto S. Paolo (ANAS)**, the `XO`
Amandola array (Torre del Podestà, Ospedale, **and its BASE station**) — and
**not one is inside any footprint of any archive**. Funnel space/operating/dwell:
Capella 15/6/**5**, Umbra 33/17/**1**, ICEYE 0/0/0. The five are four **Etna
summit** stations sharing ONE 29.1 s collect (2024-07-09, inside the Voragine
paroxysm sequence) plus Venezia Lido. **Measured: 0.589-0.803 um RMS, 6.9-9.3x
BELOW the floor** — about 2x each station's own 6-hours-earlier control, so the
tremor is REAL and still an order of magnitude too small. **Items 83-87 together:
motion above the floor exists (43% of CESMD structural records, up to 10.9 cm),
is never inside an aperture (0 coincidences over 13,735 collects), and everything
that IS inside one reads 7-20x below — Oroville 0.5-0.8, FR.CURIE 0.510, Etna
0.589-0.803 um. The consistency is the finding.** NOT queried: **ESM
(`esm-db.eu`) refuses every request with HTTP 413**, including `level=network`,
and it is the European strong-motion archive where structural arrays with
event-triggered records would live. Unqueried source, not a negative result.

**RESIF ADDS ONE EUROPEAN PAIRING AND IT IS PROVEN-STATIC** (item 86). The
French FDSN node, **5932 stations / 118 networks**, closes item 83's US-only
limit. It HAS the structures CESMD-style typing would want — four dam stations
including **Barrage de la Manzo at BASE and TOP**, and `RA.PYTO` "Tour Ophite
Terrasse" — but **no `sttype` field**, so they are found by site name, and
**every bridge match is a false positive** because French communes contain
"Pont". **The operating-period filter is where this turns**: space-only counts
are 1 / 45 / **292** for Capella / Umbra / ICEYE, and after filtering to stations
actually RUNNING at collect time they are 1 / 5 / 48, then after dwell >= 15 s
they are **1 / 0 / 0**. ICEYE's 292 is real Piton de la Fournaise coverage that
dies at 10.0 s dwell; Umbra's 243-collect Yasur coverage does not survive the
time filter at all. **Quote a footprint-join count with its time filter attached
or not at all.** The one pairing — `FR.CURIE`, Paris, 26.6 s Capella, 40,960
bytes during the aperture against a 430,080-byte control — measures **0.5101 um
RMS in 0.3-3 Hz, 10.8x BELOW the floor**, with a midday traffic control at
0.4132 um. **A second Oroville**: interpretable null, not a positive control.
**And no dam again** — two networks, two continents, three archives, and no
instrumented dam has ever been imaged by a long-dwell collect.

**ALL THREE PUBLIC LONG-DWELL ARCHIVES ARE NOW JOINED, AND THE SEARCH IS
FINISHED** (item 85). Capella 940 collects / 12 structures / **0 bridges, 0
dams**; Umbra 12,404 / 81 / **4 bridges**, 0 dams; ICEYE 391 / **0 structures**
(control 391 of 391, so it is a measurement) — its open programme points at
global CATASTROPHES, not instrumented civil structures, and its worth here stays
the six `dwell-precise`/`dwell-fine` CPHD in `runs/screens/iceye/RUN.md`.
**Earthquake coincidences across all three: 0**, so item 83's 0.0040 expectation
now stands over 13,735 collects. **Exactly one pairing exists in public data**:
Umbra over the San Francisco Bay Bridge/West (24.8 s) and Fremont Channel B
Bridge (27.4 s), **SICD only**. **No dam, anywhere, ever.** The archive search is
over; what remains is whether a bridge under traffic clears the 5.5 um floor,
which is a measurement on identified data rather than another search.

**ITEM 63'S UMBRA DWELL VERDICT DOES NOT SURVIVE THE FULL ARCHIVE** (item 84).
Item 63 sampled **250** products to measure a **sub-2% tail** — it had no power
to see what it was quoted for. Over **12,404** collects harvested from Umbra's
STAC sidecars: median **5.80 s** (not 3.50), max **496 s** (not 23.5), >= 15 s
**8.6% / 1071** (not 0.4%), and **>= 30 s: 228 collects where item 63 said
NONE** — **more than Capella's 156**. "Disqualified on dwell" is **withdrawn as
a statement about the archive**; what stands is the MEDIAN, 5.80 s giving
`df` 0.17 Hz, so 98% of Umbra is still unusable for this band. **Umbra covers
instrumented BRIDGES and Capella never has**: 81 structures inside footprints
(77 buildings, 4 bridges), 19 inside a dwell-adequate one including the **San
Francisco Bay Bridge/West at 24.8 s** and **Fremont Channel B Bridge at 27.4 s**.
**The catch: none of the 19 carries a CPHD** — the bridge products are SICD only,
so they force the `--subap paper`/`uniform` image-domain route with item 13's
response ceiling, not the `pulse` route every real-data recovery here was
measured on. Earthquake coincidences remain **0 on both archives**. What opens is
the AMBIENT route: the Bay Bridge is under continuous traffic load and needs no
earthquake, and whether that clears the 5.5 um floor is the next measurement.
**Quote a tail figure with the sample size that measured it, or not at all.**

**INSTRUMENTED STRUCTURES ARE ABOVE THE FLOOR AND ARE NEVER OBSERVED MOVING**
(item 83). CESMD filtered to `sttype` B/Br/D gives **288 structures — 239
buildings, 35 bridges, 14 dams — and 1063 records**, each carrying `pgd`.
**437 records (43%) are above item 53's 5.5 um floor, the largest at 10.9 cm =
19,800x it.** So the AMPLITUDE problem is solved by targeting structures instead
of ground — item 62's "another factor of 4" was a statement about quiet ground
and for structures the factor needed is under one. **The TIME problem then kills
it: 12 structures have ever been inside a Capella footprint, all BUILDINGS in
the LA basin, NO BRIDGE AND NO DAM EVER; 0 records had shaking overlapping an
aperture; nearest miss 122 DAYS; expected coincidences 0.0040, needing 252x this
archive.** That splits a failure this project treated as one — item 62 is
amplitude, item 83 is time — and it is the third confirmation that the data
problem is not a search problem (62, 75, 83). What is left is a TASKED collect,
not a query. **The first run of this join reported 0 spatial hits and that was a
BUG**: the harvested ring holds `[lat, lon]` and the test read `[lon, lat]`,
caught only because a scene's own centre failed to fall inside its own ring
(0 of 940, then 940 of 940). `join.py` runs that control first and refuses to
print counts without it.

**THE THRESHOLD EFFECT PREDICTS ITEM 81, AND NAMES TWO THINGS IT IS NOT**
(item 82). Frequency estimation has a documented **threshold SNR**: below it the
variance departs sharply from the Cramer-Rao bound and **no estimator attains
the bound**, so above threshold estimators separate by efficiency and below it
they fail together. That is item 81 stated in advance — periodogram, max-hold
STFT and a fitted transient model agree here because the tracked series is BELOW
THRESHOLD, not because they are equally good. **Quote item 81 in that form**,
and note where it points: SNR on the tracked series (items 51-53, 64-65), not
the spectrum stage. Two named, unbuilt alternatives — **matrix pencil / ERA**,
which the comparative literature reports as lower-variance, more noise-robust and
cheaper than the Prony family `rs_transient_fit()` belongs to; and **LSCF with
initial-and-final-condition transient terms** (Cauberghe & Guillaume), which is
what item 79 was actually describing — a FREQUENCY-DOMAIN leakage fix that
improves damping estimates in particular, not the time-domain envelope fit item
81 built. Neither is predicted to help below threshold. **Fifth time a search
found the field already had what was being built here.**

**JOINT TRANSIENT-AND-MODE ESTIMATION IS BUILT AND CHANGES NOTHING** (item 81).
`rs_transient_fit()` / `mmotion --tfit N` fits damped sinusoids WITH ONSETS to
the unwindowed series by variable-projection least squares — the free-decay
model of the OMA literature, and unlike a periodogram it reports DAMPING. As an
estimator it works: frequency sweep **slope 0.9929, rms 0.0072 Hz** against a
0.0250 bound, damping sweep **slope 1.1016**, a sustained tone returning zeta ~ 0.
**The damping ceiling is `RS_TFIT_DECAY_MAX/(2*pi*f*T)`** — 0.080 at 0.8 Hz over
20 s, 0.021 at 3 Hz — and past it the fit SATURATES, so a zeta on the ceiling
means AT LEAST. At the chain level it is **worse: 2 of 12 against the
periodogram's 3 of 12**, and both statics still answer. **Two hypotheses formed
on its output and both died to their own controls** — "fitted damping separates
driven from static" looked clean on n=2 statics (0.0047-0.0079 against
0.0026-0.0028) and dies at n=10 (statics reach 0.0044, bursts fall to 0.0023);
"better on sustained tones" dies to the paired periodogram run, which gives the
SAME blocks on the same six sine seeds. **The limiting factor is not the
spectral estimator** — windowed periodogram, max-hold STFT and a fitted modal
model all agree, because the mode is not reliably in the tracked series to be
estimated. Item 79's two directions are now both closed and neither was where
the problem is.

**THE CHANCE MODEL IS BUILT, PRICES THE BLOCK CORRECTLY, AND DETECTS NOTHING**
(item 80). `rs_spectrum_modal_set()` now reports a per-mode `p_chance` from a
Monte Carlo over its own null — nominations reshuffled across the band 1000
times, the statistic being the LARGEST block anywhere, so look-elsewhere is
inside it — plus a per-mode frequency spread from the sub-bin estimate across
nominating windows. **The threshold it derives swings by a factor of three with
the look count: chance reaches block 6 at 128 looks and block 20 at 48**, so
item 77's "quote the block with its look count attached" is now enforced by the
code. It changes almost nothing: at 128 looks no verdict moves, at 48 it refuses
two answers and **both were already wrong** (2-of-12 correct becomes 2-of-10).
**Both static controls still pass at both look counts** — because they are not
agreeing by chance, they are agreeing because 1.512 Hz is the common-mode
artefact and 0.151 Hz is the trend field. **A chance model over nominations
cannot substitute for a null over scenes: item 11, third demonstration, on the
statistic built to fix it.** One diagnostic fell out unplanned — `freq_sd`
EXACTLY 0.000 is the band-floor pile-up's signature, and the largest block in
either sweep, **39 of 49, is one of those artefacts**. Block size is not
evidence. Report the BIN CENTRE as the leading figure; the sub-bin ± is a
SPREAD, not a posterior.

**THE FIELD DOES NOT THRESHOLD A SPECTRAL STATISTIC — IT REPORTS A POSTERIOR**
(item 79). Bayesian OMA fits a modal model and returns a per-mode posterior with
identification uncertainty; other lines estimate confidence bounds alongside the
parameters. **That dissolves item 78's search for a configuration-free block
constant rather than solving it** — the successor to `rs_spectrum_modal_set()` is
per-mode uncertainty, not a better block null. Item 80 built both and confirms
it: the block null works and buys nothing. Also a standing critique of code
here: for SHORT records the field estimates the TRANSIENT JOINTLY with the modal
parameters *instead of* Hann-windowing, where `rs_spectrum_compute_opts()`
windows and items 51-53 fit carriers in a separate stage. And item 77's recall
limit is a KNOWN property — "dominant modes are reliably estimated with minimal
decay data, challenging modes need multiple free decays" — whose remedy is more
records, i.e. more collects over one structure. **Fourth time a search found the
field already had what was being invented here** (after `RS_MICROM_EST_ARGMAX`,
item 13's overlap figure, and `phaselink.c` in items 64-65). Search first.

**THE BLOCK NEEDS A CHANCE MODEL, NOT A NULL RUN** (item 78). Calibrating the
block against a matched static run — this project's own doctrine — was tested on
both sweeps' existing static runs BEFORE being implemented, and fails: at 128
looks 3 correct / 1 false positive, at 48 looks **1 correct / 6 false positives**.
Because 48 looks gives **25 spectral bins against 65**, chance agreement inflates
every block, so signal and static are drawn from the same widened distribution
and no threshold separates them. The gap this names is exact:
`rs_spectrum_modal_set()` derives `support_min` from a binomial null over
`n_bin`, but gates the BLOCK on a fixed floor of 4 — **support is normalised for
chance, the block is not**. The fix is a Monte Carlo over the same null (shuffle
nominations across bins, take the largest 4-connected block) so the threshold is
configuration-free. Not implemented.

**THE BLOCK THRESHOLD IS CONTINGENT ON THE LOOK COUNT** (item 77). Item 76's
clean separation — true modes at block 30-31, static at most 12 — survives a
12-point sweep at 128 looks (**3 of 12 correct**, all at 0.300-0.400 Hz, static
at 7-12) and **DIES at 48 looks**: both static controls return confident modes at
**block 21-23**, one of them at **0.301 Hz**, the same frequency that reads as
recovery of a true 0.300. A wrong answer reaches block 31. Fewer looks DOES raise
the sub-aperture response (0.7586 to 0.9654), so item 13's ceiling is real and
movable — but it bought more confident answers and fewer correct ones, so
whatever limits recall above 0.450 Hz is not only the response. **Quote item 76's
block with its look count attached or not at all**; the geometric 2x2 bound is a
floor, never a separator.

**ITEMS 69-74 MEASURED A TRACKER THAT COULD NOT SEE** (item 76). Every run in
that arc used the DEFAULT `correlation` estimator, whose precision scales with
the sub-look RESOLUTION CELL — 8.26 m at 128 looks. Measured `sigma_px` was
**46.7 m per look and the excursion 787x the 20 mm injected motion**: the tracked
series was noise, so item 74's 1-correct-in-6 is chance, as predicted. On
`--estimator phase` at 2 mm the same fixtures give **0.504 Hz for the sine and
0.302 Hz for the BURST against a true 0.300** — a real accelerometer record
recovered — at **block 30-31 against the static control's best of 12, no
overlap**. Item 71's shape test was starved of evidence, not wrong. `1.512 Hz`
appears in all three scenes including static: common-mode, item 11, catchable
only because the control ran. **Item 71 is withdrawn as a statement about the
method**; items 72-74 stand only as measurements of the correlation route. The
phase result needs item 74's sweep before it is a recovery — check `sigma_px`
against the injected amplitude BEFORE building anything on a tracked series.

**FEDERATING THE SCREEN MULTIPLIES PAIRINGS, NOT AMPLITUDE** (item 75). Item
59's screen queried `service.iris.edu` alone — 289 of its 315 hits were IRISDMC —
so `footprint_sensor_join.py` now uses the FDSN federator: **410 hits over 648
dwell-adequate footprints, 76 stations against 27**, with `CE ES CH BW HL CA CI`
newly reachable. **Read the shape, not the total: 324 of 410 are Kilauea and 1 of
410 has a structural site name.** The audit ran 115 raw exceedances to **zero
credible** — the La Palma headline, `ES.CJED` at 55.62 um, reads **131x its
neighbours on the same island at the same instant**, which is `HV.UWB`'s
signature. What came closest to real is `ES.CENR` during the September 2021
Cumbre Vieja eruption, decaying 5.87→5.14→2.01 um in step with the tremor — but
that is 1.07x the floor and it is GROUND, not a structure. **Second independent
confirmation of item 62**: the data problem is not a search problem.

**THE SWEEP REFUTES ITEM 73** (item 74). Twelve points -- six burst frequencies
from 0.300 to 0.850 Hz, two clutter seeds -- give the modal set six answers, of
which **one** is near the truth; the other five are wrong by 0.7-1.7 Hz and all
sit ABOVE the injection, the signature of an artefact being selected. Slope is
nowhere near 1. **Item 73's 0.353 Hz was the one point that worked**, and nothing
distinguished it from the wrong five at the time. What survives: **both static
controls refuse**, where prominence reports a confident frequency at 8.6 on the
same motionless scenes. That is not precision-versus-recall though -- the policy
answers WRONGLY on five of six MOVING scenes, so it is a statement about what
motion does to the noise, not about recovery. **Items 69-74 are one arc and it
ends here**: a real structure's motion goes in, no policy reports it correctly
across a sweep, and the only measurement worth keeping is item 71's -- the true
mode is not in the per-window spectra to be selected.

**ON A REAL BURST THE SHAPE-RANKED MODAL SET ANSWERS AND THE CONTROL REFUSES**
(item 73). A burst was cut from the same Oroville record -- the earthquake's
ARRIVAL, 76% of its energy in a quarter of the dwell. The modal set returns
**0.353 Hz at block 6**, the record's second mode to three decimals and 1.05 bins
from its dominant, while the **static control refuses** through identical
processing. `best_window` is wrong on the burst (2.571) and confidently wrong on
the static scene at a HIGHER prominence, 8.6 against 8.5 -- item 38 unchanged.
**This is the first policy here to return a true frequency from a real
structural waveform and refuse on a motionless control.** It also kills `--stft`:
on the very case it was written for, the short-time run reports 2.520 and the
modal set refuses. **NOT a recovery yet** -- one burst, one seed, no sweep, and
`rs_track_fit()`'s slope-and-rms bar is unrun. Item 72's `--stft` result looked
this good and was a coincidence; the difference here is the control, not the
sweep.

**THE SHORT-TIME ESTIMATOR IS BUILT AND FAILS ITS CONTROLS** (item 72).
`rs_spectrum_maxhold()` / `mmotion --stft L` segments the tracked series and
takes the per-bin MAXIMUM — the standard analyser mode for transients, since
averaging suppresses the intermittent events being chased. At `--stft 64` the
record reports 0.504 Hz against a true 0.550 and it is a COINCIDENCE: the same
setting reports **3.175 Hz for the sine that whole-dwell gets right**, and
**2.974 Hz at prominence 4.9 on a motionless scene**. At `L = 32` the modal set
confidently returns three wrong modes. Segmenting a 128-look series costs more
than non-stationarity does at this duty cycle — the injected record is 20 s of
continuous shaking, not a burst, so the estimator is built for a case the fixture
does not contain. **Peak-hold is not a linear operation**: valid for auto-power
spectra, invalid for FRF or coherence, so read nothing from `--coherence` on a
max-hold run. Kept because it is correct, opt-in and prints its own resolution
cost; it is not a result.

**THE REPORTING STAGE IS NOT WHAT LOSES A REAL STRUCTURE'S MOTION — THE
PER-WINDOW SPECTRUM IS** (item 71). Ranking the modal set by SPATIAL CONTIGUITY —
`n_contiguous`, the largest 4-connected block of nominating windows, with the 2x2
geometric floor ENFORCED because this function selects — turns item 70's
confident wrong answer into a REFUSAL while keeping the sine correct (0.504,
block 4). That is the first policy here to refuse the multi-modal case rather
than answer it wrongly. But `--probe-hz` at each scene's own true dominant shows
**neither is a patch**: the record reads median 0.84 / max 6.0 across the window
grid, the sine 1.44 / 9.5, both scattered. A whole-scene `--clutter-vib` fixture
does not produce contiguous windows carrying the driving frequency even for a
SINE — the sine's answer is right because a handful of windows win, not because
the scene agrees. **`RS_MODAL_PER_WINDOW` 6 is measured, not chosen**: at 12 a
scattered artefact accumulates into a block and the record gets a confident wrong
answer at block 14, so raising it DOES loosen what is believed. The blocking
problem is that a periodogram over the whole dwell is the wrong estimator for a
non-stationary record — item 56 from the other direction, signal rather than
scatterer. **Single-periodogram micro-motion has a stationarity precondition this
project never stated, every synthetic recovery satisfied it by construction, and
a structure under transient excitation does not.**

**A MODAL SET IS THE RIGHT SHAPE AND DOES NOT FIX IT** (item 70).
`rs_spectrum_modal_set()` has each window nominate its strongest peaks against
their own local background and reports the bins whose cross-window SUPPORT clears
a threshold derived from a binomial null at a family-wise budget of half a bin —
nothing tuned. On the sine it finds exactly one mode and is right; on item 69's
record it reported 2.671 Hz, because **the true bin and a noise bin both reached
support 12 of 49** — fixed by item 71's shape test. Item 11 predicted this: agreement is blind to whatever the
processing puts in every window, and a set inherits that whole. The unused
discriminator is the SPATIAL one — a real mode's amplitude across the window grid
is a mode shape, contiguous and smooth, which is `rs_spectrum_centroid()`'s
clustering applied per candidate mode. **The SAR field reports two peaks by
height with no acceptance criterion at all** (Lotti et al., EVACES 2025, South
Portland Street Bridge, Umbra-04: 9.252 s, `df` 0.138 Hz, **overlap 0.17-0.20**,
Pearson 0.33-0.47 against accelerometers, second mode undetectable on 2 of 4
pixels) — so that overlap figure is a second source against item 13's "~99%", and
0.47 is what "validated" means there. The OMA field settled this with the
STABILIZATION DIAGRAM: physical modes repeat across model orders, spurious ones
scatter. Item 70 is that principle with the spatial window substituted for model
order; it is not new.

**A REAL STRUCTURE'S MOTION NOW GOES IN, AND THE REPORTED ANSWER GOES WRONG**
(item 69). `sim_cphd --wave FILE[,RATE_HZ]` and `mmotion --inject-wave` drive the
target from a measured displacement record instead of a tone; both normalise to
unit peak so `AMP_MM` means the same thing either way, and
`rs_simulate_inject_vibrator()` is behaviourally unchanged. On the Oroville Dam
accelerometer's record of the M5.5 Lake Almanor earthquake — at the DWELL's own
resolution 0.550 Hz at 1.00, 0.600 at 0.43, 0.300 at 0.28, so score against the
record processed to the SAME resolution, never the instrument's — the same
scene, seed, amplitude and processing
that report a sine correctly at 0.504 Hz against 0.500 report **1.966 Hz**, with
no mode within 1.3 Hz; window-level hit rate halves, 20% to 10%. The tracker is
not blind to it (consensus 0.605 Hz, the second feature), so this is
items 7-9 again and worse: the energy is SPLIT across modes, none is ever
prominent, and `rs_spectrum_best_window()` goes to the tallest noise line. **A
statistic that reports one frequency is the wrong shape of answer for a
structure.** Every earlier synthetic recovery here was measured on the easiest
possible motion.

**EVERY INJECTION BEFORE ITEM 69 WAS A PURE SINUSOID** (item 68), and real structures
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

## Start here

**`docs/HANDOFF.md` is the current state of play** — what the last session
established, the one named defect to work on next, and the assets on disk. Read
it before `FOLLOW-UPS.md`, which is 116 items and is the record rather than the
plan.

The short version: the tracker recovers signals the SELECTION discards, and item
110 found where — a localised target is refused by the modal set's binomial
SUPPORT gate (28 of a required 34) and, had it been admitted, out-ranked on
EXTENT by artefacts covering one more window. Item 111 removed the band-edge
bias manufacturing the artefacts it lost to; item 112 stopped the strength term
measuring the background; items 113-114 replaced a null that assumed independent
windows where overlap makes them correlated, and bracketed what no permutation
null can compute exactly. **Item 108's false positive is now REFUSED by the
chain at p 0.342**, both real motionless controls are silent, 8 of 12 motionless
synthetic scenes return nothing, and recall is 5 of 6 on real data and 6 of 6 on
synthetic.

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
