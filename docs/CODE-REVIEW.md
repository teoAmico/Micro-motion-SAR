# Code review

Findings from reading the implementation against its own documentation. Each
entry says what was wrong, how it was established, and what was done.
Unlike `FOLLOW-UPS.md`, which records what has been *measured*, this file records
what has been *read* -- defects visible in the source, in the documentation of
record, or in the artefacts a run leaves behind.

Entries are removed once the fix has been in the tree long enough that nobody
would reintroduce them from memory. The reasoning behind a fix lives in the
commit and beside the code; what is kept here is the *shape* of the mistake,
because several of these are patterns rather than incidents.

## `--rbins` means two different things and documents only one

`src/readers/cphd.c:704` caps `n_rbin` from `n_samp` **before** the FX-to-delay
inverse transform at line 885, so for an **FX-domain** product the flag truncates
the TRANSMITTED FREQUENCY BAND rather than selecting a range window. The usage
text in `src/main.c` said "--rbins reads a window of range bins", which is true
only for a TIME-domain product.

**Capella and Umbra both ship FX-domain CPHD**, so on every real collect this
project uses, the flag is a bandwidth cut. On the Kilauea collect
`CAPELLA_C10_SP_CPHD_HH_20240609091921` -- `NumSamples` 21343, `FxMin` 9.40 GHz,
`FxMax` 9.90 GHz -- `--rbins 4000` keeps 18.7% of a 500 MHz band and coarsens
range resolution about **5.3x**. Nothing warns.

**How it surfaced.** The queued Kilauea job needed a memory remedy: 282,972
pulses x 27,650 bins is 62.6 GB against 24 GB of RAM, and `info` on the finished
download failed outright. `--rbins 4000` made `mmotion` run in 4.96 GB with the
full dwell intact (0.0334 Hz resolution, 29.9 s), which looked like the right
answer and is not one -- the run was at a fifth of the collect's range
resolution.

**Done:** the usage text now states the DomainType dependence and quantifies it.
**Not done:** the flag still cannot express "a range window at full bandwidth" on
an FX product, which is the operation actually wanted. That needs the window
applied AFTER the inverse transform, or a streaming read; both are changes to the
reader's memory model rather than to a flag.

**Second, smaller finding beside it:** `info` loads the whole signal array to
report geometry and timing, so it cannot describe a product larger than RAM --
the one command whose entire job is to tell you what you have.

## The local background is estimated from half as many bins at the band edges

`rs_local_ratio()` (`src/core/spectrum.c:975`) clips its `±RS_LOCAL_HALF_BINS`
neighbourhood to the admissible band, so the first admissible bin is scored
against **10 reference bins where a mid-band bin gets 20**, and the same at
Nyquist. The background is a MEDIAN, so halving the count roughly doubles its
variance; the statistic is then maximised over every bin of every window, and a
noisier denominator wins a maximum more often for no reason in the scene.

**Measured** (item 110, `runs/kilauea/2026-08-07-nomination-fix/`): on the item
109 collect the two frequencies `rs_spectrum_modal_set()` reported — bin 3 and
bin 61 — are both inside the starved zone, and bin 3 held the largest block of
nominating windows at 14 of 225. Replacing the clip with a neighbourhood that
grows outward until every bin has 20 references drops that block to **9**.

**Not fixed, and the obvious fix is the wrong one.** That extension fails
`test_tracking`'s red-floor case (`tests/test_tracking.c:2300`): on a floor
rolling off as sinc² it reaches past the first null into the deep tail, depresses
the median and inflates the low-frequency bins it was meant to demote. The header
for `rs_spectrum_local_window()` already names the remedy this needs and does not
have — **a narrower neighbourhood or a fitted slope** — and reaching further is
the opposite of both.

A fix has to keep the reference count constant *without* enlarging the span the
references are drawn from. The candidates not tried: fitting the background
log-linearly across the neighbourhood and evaluating it at `k` (which
extrapolates rather than extending, and works one-sided by construction), or
normalising the ratio by the count so the maximum is comparable across bins.
Either changes what every policy downstream nominates, so it needs item 110's
two arms re-run.

## Nothing in the test suite exercises `rs_spectrum_modal_set()`

It is the function whose leading frequency `mmotion` reports, whose ordering
`--stable` adjudicates, and the subject of items 70, 71, 77, 78, 80, 109 and 110
— and `grep -rl modal tests/` returns nothing. Every claim about it in
`FOLLOW-UPS.md` rests on run directories rather than on anything `ctest` would
catch. Item 110 changed its sort key and the suite could not have noticed.

What a test needs to pin, at minimum: that the admission gates and the ordering
are separate (a hand-built spectrum with a strong 2x2 block and a weak large one
must report the strong one, and must report NEITHER when both fail `p_chance`),
and that `support_min` still tracks the bin count when the look count changes,
which is item 77's failure.

## Reviews performed

| date | commit | scope | outcome |
|---|---|---|---|
| 2026-08-07 | `fce5869` | Item 109's named defect: `rs_local_ratio()`'s guard band | **The named defect is not the defect** — replicating the whole nomination offline from a `--shifts` dump (the replica reproduces the binary exactly) and sweeping `RS_LOCAL_GUARD_BINS` 2→8 never recovers the injected line, and the Hann-skirt argument does not apply at `--overlap 0` where the floor is flat. The target is lost **twice** instead: refused at the binomial support gate at 28 of a required 34, because that threshold is a fraction of the whole window grid; and then out-ranked on extent by artefacts covering one more window, while leading every rival two-to-one on strength. Both fixed and measured in item 110 — 5 of 6 recover against 3, false positives unchanged at 1 of 12. Two candidate fixes were written, measured and reverted, and both are recorded in the run's `PREREG.md`. Found two further defects left unfixed: the band-edge reference starvation below, and the total absence of any test over `rs_spectrum_modal_set()`. |
| 2026-08-04 | `d1e1e2e` | Measuring the three-sided overlap trade together, and calibrating the local peak on real clutter | **Withdraws item 14's high-overlap advice.** Injection-versus-control separation is 35,676× at overlap 0 and 11× at 0.90, failing entirely at 0.95 — four orders of magnitude, on the setting every phase run here has used and the USER_GUIDE recommended. Also calibrated the local peak on nine disjoint grids of real desert (15.1–34.4, median 20.8) with no simulator, and recorded that nine controls give p_min = 0.10 rather than 0.05. |
| 2026-08-04 | `a183479` | Re-measuring item 37 under the new `quality`, and asking what the "trend" actually is | **Nothing moved**: all seven of item 37's points reproduce exactly, because the shared gate is relative and Giza lacks the stable-bright-window contrast that made ICEYE fail. Found instead that the thing winning on controls is not a trend at all — 83% survives a cubic fit — but a RED noise floor from 90% overlap, 24× the upper band. `rs_spectrum_local_window()` scores against a local background and widens injected-vs-control separation from 3.3× to 37×. Also caught my own bad comparison: prominence is not comparable across `--fmin` settings. |
| 2026-08-04 | `95412fe` | Fixing item 45's quality gate on the phase route | Replaced amplitude stability with spatial dominance, `1 - mean/peak`, the measure item 15's precondition actually names. The ICEYE injection now reports 1.047 Hz at the injected window instead of a 0.524 Hz trend artefact two windows away; Giza unaffected, its centroid now (2.00, 2.01). **Withdraws finding 1**: `quality` and `d_a` are no longer complements on this route. Also measured and stated: the shared `quality >= 0.5*q_max` gate is now inert — speckle alone scores 0.673 on a 1024-pixel window — which is a better failure than removing the signal but leaves the relative threshold form wrong for the quantity. |
| 2026-08-04 | `09278da` | Item 45's open point: the centroid was seeded only from the gated policy | Fixed by seeding it twice and reporting the disagreement rather than resolving it. On the ICEYE injection the reported seed gives `(0.00, 0.00)` and the scene-null seed `(1.99, 1.99)` against a truth of `(2.00, 2.00)` — 0.16 m, matching item 42's accuracy on a different vendor. The two seeds answer different questions (one gated, one not), so a disagreement is itself the finding. Seed-dependence is now a pinned test rather than a caveat. |
| 2026-08-04 | `4a7dcb9` | Item 4's dwell truncation, verified end to end on the real ICEYE product | **The truncation works — and `validate` accepted `--max-pulses` and ignored it.** Its read options were `{ .rbin_window = 8 }` with no `max_pulses`, so it answered about the untruncated dwell and reported FAIL for a configuration that passes. Truncation is the documented remedy for that exact failure and `validate` is the command you run to check it, so a caller following the docs would have concluded the collect was unusable. Fixed. No test can cover it: `validate` reads real CPHD only and refuses `sim_cphd` output, which is why it was never caught. Also closed item 36's last open point by focusing ICEYE signal for the first time. |
| 2026-08-04 | `703858e` | Testing item 42's 0.1 m claim at the weak end, and chasing its `+0.01` residual | **The claim is amplitude-dependent and was quoted as a constant.** 0.13 m at 2 mm becomes 2.24 m at 0.125 mm, linear in 1/amplitude; argmax stays at exactly 16 m at both. The residual is a weighting artefact, not an off-by-something — a convention error would give exactly 0.5. It is item 41's 1.5% inter-window spread propagating through the centre of mass. Uniform weighting is exact on symmetric clusters and was rejected: it fails 0.2 windows on the one asymmetric cluster of five. |
| 2026-08-04 | `80f8818` | Trying item 41's proposed fix: a centroid over the agreeing cluster | **16.0 m becomes 0.1 m**, a factor of 125, on five interior placements against geometry-derived truth. Built with no tolerance parameter — the cluster is `rs_spectrum_block_at()`'s agreeing block, the weight is prominence above the scene median. Two fixture defects found on the way: a "noise" generator linear in the sample index, giving every window the same dominant frequency and a cluster that swallowed the grid; and a `clipped` flag defined as "cluster touches the edge", which fired on four of five accurate placements and warned about nothing. |
| 2026-08-04 | `8b36276` | Chasing item 40's unexplained one-window offset to its cause | **Not geometry.** Differencing two `focus` runs puts the target at azimuth px 47, range px 48 against an origin at 47.5 — sub-pixel exact, so `--inject-at`, the backprojector and the window convention all agree. The offset is the selection: six overlapping windows lie within 1.5% of each other, thirteen pairs are bit-identical across four columns because they track the same dominant scatterer, and the correctly-centred window scores LOWEST. Forced the withdrawal of item 40's "4 of 5 exact", which had been scored against a displaced reference. |
| 2026-08-04 | `cfc6c16` | Whether localisation had ever been tested, and the unqualified velocity readout | **It had not been, and could not have been:** `main.c` passed the grid origin as the injection centre and nothing else, so every experiment put the target at the exact centre of the analysis grid. `--inject-at` fixes that; off-centre the reported window follows the target 4 of 5 against 4% chance. Also found that a zero-offset injection lands one window off the grid's geometric centre in each axis, unexplained. The velocity beside every reported frequency is now labelled `UNCALIBRATED` — a motionless scene printed 124.8 mm/s through it. |
| 2026-08-04 | `d0d22c1` | Building the statistic item 38 said was missing: prominence at a nominated frequency | `rs_spectrum_prominence_at()` and `--probe-hz`. Differencing a run against its zero-amplitude twin gives a median increment of exactly `+0.00` over 25 windows and a positive one at the injected window, so the injection machinery's contribution subtracts out. Also found the failure mode: paired against the *uninjected* run instead of the zero-amplitude one, the increment exceeds a real 0.5 mm signal, because it then measures the scatterer's presence. |
| 2026-08-04 | `47d8970` | The negative control item 35 never had: `--inject-vib` at zero amplitude | **Invalidates the adjudication statistic, not the frequency results.** A motionless bright scatterer scores prominence 56.3 against 38-47 for real injections and clears the same 19 controls, so `p = 0.05` measured the scatterer being added. Built `rs_spectrum_scene_null()` in response; it refuses the real uninjected scene and kills common-mode artefacts, but ranks the zero-amplitude target first as well, because that target genuinely is unusual for its scene. |
| 2026-08-04 | `0b66c27` | Re-execution of item 35's adjudication at other amplitudes | **Found a defect the whole gate stack missed.** Below 2 mm the reported frequency collapses to bin 1 while prominence RISES 32.0 -> 56.0, and prominence, quality, `D_A` and the `--null-static` control all endorse it. Fixed by making the first three bins unreportable (`RS_SPECTRUM_LEAKAGE_BINS`). This is the first review here that re-ran a `FOLLOW-UPS` measurement at a different operating point, which the 2026-08-02 sweep explicitly listed as not covered. |
| 2026-08-03 | `0c717d9` | Capella's `CPHD_by_Example.ipynb` read against this reader | Three conventions confirmed from the primary source (SGN inverse, SRP phase referencing, near-range arithmetic); no gap found. It surfaced the unread `Antenna/AntPattern`, which `FOLLOW-UPS` 23e then excluded as a `D_A` explanation. |
| 2026-08-03 | `55c4aa5` | Full ASAN + UBSAN pass over the whole suite | Clean: 17/17 binaries, **zero** sanitizer diagnostics. Sanitizers verified armed. See below. |
| 2026-08-02 | `cda093f` | Removing the two dead `rs_slc_t` fields | `sizeof(rs_slc_t)` falls 3976 -> 320 bytes, 92 percent of it, saving 7.3 MB on a 2048-look stack. Pipeline output bit-identical. |
| 2026-08-02 | `2e0e1fc` | Closing the check's own open items: tests for the two uncovered branches, and the phase-reference fix they made safe | Both branches now covered. The lag test failed before the fix and passes after; the split-band test found a second defect, an unclamped coherence. Details below. |
| 2026-08-02 | `893d9d2` | Follow-up: the two items the sweep left open, checked | Both were wrong as written. The orbit/Doppler fields are superseded rather than unfinished; two of the four "exposed but non-recovering" modes have no test at all, and `--lag` was unvalidated with platform-dependent undefined behaviour. Details below. |
| 2026-08-02 | `6d42f29` | Full sweep: `docs/FOLLOW-UPS.md` against the source; every `.c` and `.h` under `src/`, `include/`, `tools/`, `tests/`; CLI flags against `USER_GUIDE.md`; committed run artefacts under `runs/` | 21/21 tests pass. Two pre-existing compiler warnings found (see the correction below). Eleven findings, all fixed in the same change; two new test cases pin the substantive one. |

What that review did **not** cover, so nobody reads more into it than was done:
no fuzzing of the readers, no re-execution of any `FOLLOW-UPS` measurement, and
no review of `third_party/pocketfft`. (It also listed "no ASAN or UBSAN run" --
that gap is closed, see the full pass below.)

### A correction to this file's own first draft

The first version of this review said the build was clean under
`-Wall -Wextra -Wshadow -Wconversion`. **It was not.** That claim came from an
incremental build in which the two files carrying warnings had nothing to
recompile, so the warnings were never re-emitted and the grep found nothing. A
clean-tree rebuild showed two:

```
src/main.c:2371       unused function 'rs_parse_offsets'      [-Wunused-function]
tests/test_tracking.c:1801  declaration shadows a local variable  [-Wshadow]
```

Both predate this review, confirmed by rebuilding the stashed tree. Both are now
fixed. The lesson is the one this project already writes down about processing
runs and applies equally to its own tooling: **an incremental result is not a
measurement of the whole.** Verify a build claim against a clean tree, the same
way `--null-static` exists because a result read off one configuration says
nothing about another.

---

## Fixed in this review

### 1. On the phase route, `quality` and `d_a` were the same number, undocumented

`src/core/microm.c` computes amplitude dispersion at the brightest pixel of the
reference-look patch, and then, for the phase estimator, computes
`q = 1 - sigma_A/mu_A` from the same pixel and the same amplitudes. The two are
exact complements, and the identity holds to machine precision on committed
real-data runs:

```
runs/giza/2026-08-02-inject-sweep/sweep_0.163_windows.csv
  quality=0.255033  d_a=0.744967  1-d_a=0.255033   diff  5.6e-17
  quality=0.290199  d_a=0.709801  1-d_a=0.290199   diff  5.6e-17
  quality=0.039791  d_a=0.960209  1-d_a=0.039791   diff  2.0e-13
```

Three consequences, none of them recorded anywhere:

- `--coherence F` on the phase route **is** the criterion `D_A <= 1 - F`. The
  default 0.4 is `D_A <= 0.60` -- a looser form of the test
  `rs_spectrum_ps_window()` applies at 0.25.
- The shared gate every policy applies, `quality >= 0.5 * q_max`, is on that
  route a scene-relative amplitude-dispersion gate.
- `rs_spectrum_ps_window()` is presented as the policy reading evidence the
  others do not. Against correlation it does; against phase it reads the same
  evidence at a tighter threshold.

`microm.h` documented `quality` as "mean correlation peak in `[0,1]`", true only
for correlation and split-band, and the eighty-line block on `d_a` beside it
never mentioned the relationship.

**Fixed** by documenting the identity on both fields in `microm.h`, restating
`--coherence` per estimator in the `mmotion` help, and adding the same to
`USER_GUIDE.md` section 9. The arithmetic is unchanged: amplitude stability is
the right precondition proxy for an observable that reads one scatterer's phase,
and `rs_microm_track()` argues that where it is computed. What was wrong was the
description, and the fact that a reader comparing a `quality` map against a
`d_a` map was looking at one measurement twice.

### 2. `passed_cull` in `PREFIX_windows.csv` disagreed with its own header

The column was recomputed per row from the recorded thresholds, which cannot
express gate 3 -- the neighbourhood test needs to know which windows *entered*
the cull, and a per-row recomputation does not have that. Both committed Giza
runs:

```
run                                   header said          column summed to
2026-08-01-phase-highoverlap   input=170 survivors=65        passed_cull=170
                               neigh_cull=105
2026-08-02-inject-sweep 0.163  input=136 survivors=47        passed_cull=136
                               neigh_cull=89
```

105 and 89 windows marked as surviving a cull that removed them, in the artefact
`CLAUDE.md` calls "the evidence file written beside every run so a later question
about the selection policy can be asked without reprocessing" and that item 30's
retraction turns on having read.

**Fixed** by giving `rs_spectrum_ampcor_window()` and
`rs_spectrum_ampcor_window_opts()` an optional `out_state` buffer carrying each
window's fate (0 did not enter, 1 culled by gates 1-2, 2 culled by gate 3, 3
survived). `mmotion` writes the selector's verdict rather than an approximation,
and records `passed_cull_source` and `expected_sum` in the header so a reader
summing the column knows what it should sum to. Verified on a fresh run with
`neigh_cull=5`: the column sums to 0 against `survivors=0`, where the old code
printed 6.

### 3. The UAVSAR reader filled `r0` from the platform's altitude

`readers/uavsar.c` read the annotation field `Average Altitude` into `img->r0`,
documented in `slc.h` as "slant range of first range sample, m". An altitude is
not a slant range: at UAVSAR's ~12.5 km flight altitude and look angles from
about 20 to 65 degrees the near-range slant distance is 13 to 30 km, so the field
was low by a factor of one to two and the error grew across the swath. It reached
`rs_geo_slant_to_ground()` and the sub-look ambiguity ceiling -- the two consumers
`FOLLOW-UPS.md` item 5 already names as where an `r0` error propagates invisibly.

Item 5 recorded the opposite, stating that UAVSAR held the documented meaning and
only SICD deviated. There were three conventions in play, not two, and item 5's
two proposed fixes both assumed the wrong baseline.

**Fixed** by deriving the slant range from geometry the annotation does support,
`R = (altitude - terrain) / cos(look)`, with both approximations stated in the
code: it is the range at the scene's average look angle rather than to the first
sample, and the look-for-incidence substitution is admissible only because this
reader is airborne. If any field is missing, `r0` stays **zero** rather than
guessed, and `img->source` says so. `tests/test_readers.c` pins the derived value, the incidence, that the answer
exceeds 1.2x the altitude, and that absent geometry leaves the field unset.

**Followed by the rename that closes `FOLLOW-UPS.md` item 5.** With UAVSAR fixed,
all four producers agreed -- and none of them wrote the first-sample range the
field was documented as. `r0` is now `r_scene_m`, "slant range to the scene
reference point at mid-dwell", so the field means one thing and a stale
`img->r0` fails to compile rather than reading a quantity that moved. It caught
one caller immediately: `rs_crop()` in `tools/crop_slc.c` advanced the range by
`rg0 * rg_spacing_m`, right for a first-sample range and wrong by kilometres on
a stripmap swath. That helper was static in a tool and so unreachable from the
suite, which is how it survived; it is now `rs_slc_crop()` in the library with
three cases chosen so the old and new formulas cannot agree on all of them.

**A second field in the same struct moves the opposite way, and the first
version of the crop fix got it wrong.** `plane` was carried across untouched
under a comment asserting the image plane is referenced to the scene reference
point. It is not: `rs_geo_plane_point()` works from image coordinate (0,0), and
`info` calls it with (0,0) under the label "first az, first rg", so
`plane.origin` is the ECF position of the FIRST SAMPLE and must advance by the
crop offset. Unmoved, it put every sample of the fixture crop 43.4 m from where
it belongs -- `info` printing the uncropped scene's corners, `--at` resolving to
the wrong ground position, both with complete output. `r_scene_m` moves by the
change in centre bin and `plane.origin` by the origin offset: one struct, two
reference points, and a crop that treats them alike is wrong about one of them.
Pinned through `rs_geo_plane_point()` so the test states the property -- a
surviving sample keeps its ground position -- rather than restating the
arithmetic, and confirmed to fail against the old behaviour.

### 4. The same reader parsed a terrain height into a variable named `inc`

`readers/uavsar.c` filled a local named `inc` from `Global Average Terrain
Height` and never read it again, so `img->incidence` was zero for every UAVSAR
product and `rs_slc_validate()` permits zero explicitly.

Two defects in four lines. The dead store was harmless; a metre quantity read
into a variable named for a radian one, in the codebase whose comment rule exists
because it is full of dimensionally interchangeable quantities, was not. Had the
assignment been completed, `cos(incidence)` would have been taken of a height in
metres. `-Wall -Wextra` could not see it: the variable is "used" as the address
of an out-parameter.

**Fixed** with finding 3 -- `img->incidence` now carries the look angle in
radians when the annotation provides it, and nothing reads a height into it.

### 5. Seven exported functions were never called

Defined, declared in a public header, referenced from no `.c` file anywhere.
Decided per symbol rather than in bulk:

| symbol | outcome |
|---|---|
| `rs_dopp_poly_eval` | **deleted** |
| `rs_orbit_interp` | **deleted** |
| `rs_microm_max_velocity` | **deleted** -- `validate`'s ambiguity check derives the same quantity, and two derivations of one number is how they diverge |
| `rs_palette_from_name` | **deleted** -- it resolved a `--palette` option that does not exist |
| `rs_geo_project_to_height` | **kept** -- a documented seam paired with the unimplemented range-Doppler locator, whose "what this is not sufficient for" analysis is the valuable part |
| `rs_fft_plan_length`, `rs_slc_row` | **kept** -- one-line accessors that complete an interface and assert nothing |

The first two were the larger finding. `rs_dopp_poly_t` and `rs_orbit_t` are
**never populated by any reader** -- no CPHD, SICD or UAVSAR path writes a
Doppler coefficient or a state vector -- so both structures are zero on every
product this software has read, and `rs_subaperture_split()` propagates zero to
zero. The two functions were the only readers of fields nothing writes, making
the whole chain unreachable and untested. This project has twice found that a
documented, unexercised function does not do what its comment claims
(`FOLLOW-UPS.md` items 27 and 28), which is the argument against keeping such
code on the grounds that it looks correct. The struct fields stay, marked
reserved in `slc.h`, because the parse that would fill them is a real gap rather
than a rejected idea.

Also deleted: `rs_parse_offsets` in `src/main.c`, a leftover of the removed
tomography sweep, together with four orphaned comment blocks documenting
functions that no longer exist.

### 6. Failure returns with no message printed a *stale* one

`CLAUDE.md` states the convention without qualification: every fallible function
calls `rs_set_error()` immediately before returning non-OK. `rs_error_buf` is
never cleared, so a return that skips it does not print "no detail" -- it prints
the detail of an unrelated earlier failure, in parentheses, as though it applied.
`validate --xml` on a truncated block gave `I/O error (<whatever failed last>)`.

104 sites returned non-OK with no message within six lines. 76 were `if (!ptr)`
guards against programmer error and are left alone. The other 28 were reachable
at runtime.

**Fixed both ways, because they are not alternatives.** All 28 now carry a
message naming the sizes or the file involved, except `cphd.c`'s `io:` label,
whose single `goto` sets one at the point of failure and which is now commented
to say a message here would overwrite a better one. And `rs_clear_error()` was
added and is called at CLI entry, so an uncovered site degrades to a bare status
rather than a wrong sentence. Covering the sites is better; the clear is the
floor under the next one somebody forgets.

### 7. `PREFIX_windows.csv` recorded no run provenance

Not the estimator, the overlap, the look count, the reference mode or the
injection. Item 30's sweep left six such files distinguished only by the filename
the operator chose. `FOLLOW-UPS.md` item 7 records the same class of defect found
by hand -- `--shifts` writing `reference=first` for a `--reference lag` run --
and closes with the observation that nothing in the suite checks a run's recorded
provenance against what produced it.

**Fixed:** two header lines now record estimator, reference, looks, overlap,
`dt`, `t_sap`, window and stride, upsample, coherence threshold, and whether
`--inject-vib` was used.

Item 7 also cited the fix as landing at `main.c:1469` and `main.c:2205`, one of
them "the `.meta` sidecar". No such sidecar exists and there was one label site.
Corrected in `FOLLOW-UPS.md`.

### 8. `docs_coverage` skipped `tests/` and vanished silently without Python

The check ran over `src/` and `tools/` only, leaving `tests/` -- 6100 lines
including `rs_sim.h` and `rs_test.h`, the fixture generator and scoring harness
every claim in `FOLLOW-UPS.md` was measured through -- outside the rule. Run
against it, 16 functions lacked the required comment. `include/` passed clean.

Separately, the test was wrapped in `if(Python3_Interpreter_FOUND)` with no
`else`, so without an interpreter the suite quietly became 20 tests while `ctest`
still reported success and both `CLAUDE.md` and the user guide said 21.

**Fixed:** `tests` added to the scanned roots, the 16 functions documented, and a
CMake warning added on the skip path. `check_docs.py` also cited
`IMPLEMENTATION_PLAN.md section 3a` in its docstring and in the message printed
to a developer at the moment the test fails; that file is not in the repository,
and the citation now points at `CLAUDE.md`.

### 9. `FOLLOW-UPS.md` cited three documents not in the repository

`IMPLEMENTATION-VERIFICATION.md` (items 2 and 10), `POSITIVE-CONTROL.md` (item 6)
and `MODIFIED-BACKPROJECTION.md` (item 4) were development notes kept local,
which is what the `docs/` allow-list is for. The cost is that a reader outside
this working copy cannot follow a citation that load-bearing -- item 4's whole
numerical argument rests on a second-hand summary of one of them.

**Fixed** by marking each `[local note, not in the repository]`, so the reference
reads as unavailable rather than as a file the reader failed to find. Whether
those notes should be tracked is a separate decision and is still open.

### 10. Stale counts and claims in the user-facing documentation

| where | said | now |
|---|---|---|
| `CLAUDE.md`, `USER_GUIDE.md` | `microm.h` is 944 lines for ~10 declarations | 1343 lines, 17 declarations |
| `USER_GUIDE.md` opening | "Nothing has passed [the bar] on real data" | states item 30: the *tracker* met it, every policy discarded the result, so nothing that **reports** has |
| `USER_GUIDE.md` section 6 | output-line table | the `persistent scatterers:` row was missing |
| `USER_GUIDE.md` section 9 | `--coherence` masks windows that do not correlate | qualified per estimator, per finding 1 |
| `CLAUDE.md` | "thirteen arithmetic checks" | fourteen checks, thirteen arithmetic, with `RS_VALIDATE_GROUND_TRUTH` named as the one that always reports unknown |

Ten flags were documented in the tool's `--help` and nowhere in `USER_GUIDE.md`.
A table now covers `--no-detrend`, `--b-shift`, `--null-trials`, `--range-taps`,
`--pulse-start`, `--dyn-range`, `--ccd-win`, `--ccd-loading`, `--amplitude` and
`--alpha`.

### 11. Two things in the numerical core

`src/core/coreg.c` computed `*peak = (ref_mag / n) / norm * n` under a comment
saying the direct evaluation had to be scaled against the inverse transform's
`1/n`. The two factors cancel, so the comment described an operation the
expression did not perform -- and the round trip is not a no-op in floating point
unless `n = n_az * n_rg` is a power of two, which it need not be. This project
bans `-ffast-math` because reassociation perturbs exactly this quantity.
**Fixed** to `ref_mag / norm`, with the Parseval argument for why no scaling is
needed written where the wrong claim used to be.

Related, in the same function: `ref_mag` was seeded from `best_mag`, which comes
off the inverse transform at `1/n` of the scale every later comparison uses, so
the seed could never win and was an unreachable fallback. **Fixed** to `-1.0`, so
the first lattice evaluation wins outright -- the lattice contains the coarse
offset, so nothing is lost, and the curvature code below now always has a grid
point for the winner.

`sigma_rg_px` is computed by `rs_coreg_shift_impl()` and read only by
`tests/test_coreg.c`. **Left as is**: `microm.h` states the reason -- "azimuth
alone because azimuth is the observable" -- so this is a documented choice, and
the cost is one curvature evaluation in the innermost loop.

---

## Checked and found clean

Recorded so a later reviewer knows what not to redo.

- **Build.** Clean under `-Wall -Wextra -Wshadow -Wconversion` at `-O2`,
  **verified against a clean tree** rather than an incremental one. No unused
  functions, no unused variables, no shadowing, no flagged conversions.
- **Tests.** 21/21 pass in Release, ~195 s.
- **No unfinished-work markers.** No `TODO`, `FIXME`, `XXX` or `HACK` anywhere.
  The two occurrences of "unimplemented" (`subaperture.c`, `geocode.h`) are both
  deliberate non-implementations with the reasoning recorded and the seam marked.
- **Symbol citations.** Every `rs_*` symbol named in `FOLLOW-UPS.md` exists.
  Every flag named in `USER_GUIDE.md` exists in `src/main.c` or in
  `tools/sarpy_crosscheck.py`.
- **Header coverage.** `check_docs.py` over `include/` reports nothing.
- **Error-handling structure.** Every fallible function returns
  `resonarsat_status_t`; no library code prints; the error buffer is
  `_Thread_local` and safe inside the OpenMP regions in `rs_microm_track()` and
  `rs_focus_backproject_opts()`.
- **Selection-policy gates.** The four duplicated copies of the shared gate are
  equivalent in effect. The duplication is deliberate and argued in `spectrum.c`.
- **Constants against their documentation.** `RS_CULL_SNR_FACTOR`,
  `RS_CULL_SIGMA_FACTOR`, `RS_CULL_MIN_NEIGHBOURS`, `RS_PS_DA_MAX`, `RS_DA_MAX`,
  `RS_COREG_SIGMA_MAX` and `RS_COREG_MAX_SURFACE` all match the values their
  headers and the `FOLLOW-UPS` sweeps quote.

## The open items, checked (2026-08-02)

Both entries this file left open turned out to be wrong as written. Recorded in
full because in both cases the *reason* they were wrong is more useful than the
correction.

### The orbit and Doppler fields are superseded, not unfinished

This file said "the orbit and Doppler parse does not exist -- a gap in the
readers". It is not a gap. Every format either carries something better or
carries nothing to put there:

| | what it has | where it goes today |
|---|---|---|
| CPHD | `rs_cphd_t.pos`, **three doubles per pulse** -- the exact recorded trajectory | read directly by `rs_focus_backproject()` |
| SICD | `ARPPos` / `ARPVel` at aperture centre | **parsed already**, into `plane.sensor` / `plane.sensor_vel`, consumed by `rs_geo_slant_to_ground()` on the live `info` path |
| UAVSAR | no state vectors at all | nothing to parse |

A 64-entry `rs_orbit_t` interpolated with a cubic would be strictly worse than
the per-pulse samples CPHD already provides, and SICD's single state vector is
parsed -- just into a better home. So `rs_slc_t.orbit` is a third place for a
quantity with two, and `rs_slc_t.doppler` likewise: `rs_subaperture_split()`
estimates the centroid from the samples on every call, by design rather than as
a fallback, which is what lets the simulator's annotation-free container run
through identical code.

**"Unfilled field" and "field nothing should fill" look identical from the
field.** That is why this was recorded as a reader gap for two commits. The
distinction is only visible by asking what would fill it and finding the answer
is already stored elsewhere. `slc.h` now says so, and says the fields are
removable rather than reserved.

**A false comment found while checking.** `src/core/subaperture.c` read: *"Doppler
centroid: prefer the annotated polynomial when present, and cross-check it
against the data-driven estimate. A large disagreement is reported rather than
silently averaged."* None of it happens -- `rs_slc_t.doppler` is never read
anywhere, there is no cross-check, and nothing is reported. The comment described
a design that was never built, at the exact place a reader would look to find out
whether an annotation error would be caught. **Fixed**, and it now states what
estimation can and cannot do: it stays correct when an annotation is stale, and
it cannot notice that it has landed on the wrong side of a burst boundary, which
is the one thing an annotated polynomial would be worth having for.

### "Recover nothing" was too strong, and two of the four modes are untested

*Untested, measured rather than assumed:*

| mode | branch in `rs_microm_track()` | covered by |
|---|---|---|
| `RS_MICROM_REF_PAIR` | yes | `test_tracking.c`, `test_subaperture.c` |
| `RS_MICROM_REF_ADJACENT` | yes | `test_tracking.c` |
| `RS_MICROM_REF_LAG` | **no test anywhere** | -- |
| `RS_MICROM_EST_SPLITBAND` | **estimator branch untested** | only the primitive `rs_splitband_shift()`, in `test_phaselink.c` |

`FOLLOW-UPS.md` item 8 argued `LAG` should be kept because it is "documented,
tested and harmless". It is not tested: the lag clamp, the `k < lag` skip and the
moving-reference extraction have never been executed by `ctest`, and every
measurement in item 7 was made through the CLI. Corrected there.

*And "recovers nothing" understates `lag`.* Item 7 measured it at slope +1.120,
rms 0.3461 Hz pooled over three seeds -- failing `rs_track_fit()` by fourteen
times the bound, but the only non-zero slope this project had produced before
item 14. The accurate statement is that none of the four passes the bar, not that
none responds.

**A defect that untested path was hiding: `--lag` was unvalidated.** It went
straight into `(size_t)rs_opt_double(...)`, so:

- `--lag -1` is undefined behaviour and **platform-dependent**. arm64 saturates
  to 0, whereupon `rs_microm_track()`'s `ref_lag > 0` guard silently substitutes
  1 and the run looks normal -- confirmed, it returned bit-identical output to
  `--lag 1`. x86-64's `cvttsd2si` gives `SIZE_MAX`, which skips every look. Same
  source, two behaviours, neither one asked for.
- `--lag 999` on 32 looks skipped every look and produced an all-zero series. The
  chain did refuse -- the quantisation floor caught it, which is the design
  working -- but the message blamed the scene and the upsample factor and never
  mentioned the lag. A misdiagnosis of a configuration error.

**Fixed:** `--lag` is validated as a double before the cast, rejecting
non-positive and non-integral values, and checked against the look count once it
is known. Refused rather than clamped, because a clamp would hide the mistake.

```
--lag 1    -> strongest peak in window 17: 0.359 Hz ...
--lag 999  -> --lag 999 needs at least 1000 sub-looks to leave any sample,
              but this configuration has 32. Lower --lag or raise --n.
--lag -1   -> --lag must be a positive whole number of looks (got -1)
--lag 2.5  -> --lag must be a positive whole number of looks (got 2.5)
```

### Still open after the check — now closed

Written when the check ran, and discharged in the following commit. Kept because
what the tests found is the point.

**`RS_MICROM_REF_LAG` and the `SPLITBAND` branch now have tests**, in
`tests/test_tracking.c`. Both run against a stack whose looks differ only by a
per-look constant phase, which makes the two quantities under test exact rather
than approximate: a constant phase factors out of the cross-spectrum, so every
correlation shift is zero, and the window-averaged interferometric phase between
looks `a` and `b` is exactly `theta[b] - theta[a]`. That is what lets a test
assert **which pair the tracker differenced**, which is the thing the lag branch
had no coverage of.

**The phase-reference defect is fixed, and the test failed before it.** The
refinement block differenced against `look[0]` for every mode but `PAIR`. Against
the fixture, pre-fix:

```
k=3  phase +1.050000   vs lag +0.800000   vs look0 +1.050000   FAIL
k=4  phase +0.800000   vs lag +0.200000   vs look0 +0.800000   FAIL
k=5  phase +0.300000   vs lag -0.750000   vs look0 +0.300000   FAIL
```

Exactly `theta[k] - theta[0]` on every look. After the fix every row matches the
lag difference and differs from the look-0 value. `FIRST` is unaffected and
`ADJACENT` keeps `look[0]` deliberately -- its shift accumulates to an absolute
displacement, so an absolute phase matches it; `LAG` accumulates nothing, which
is the whole reason it exists.

**The split-band test found a second defect.** `rs_splitband_shift()` returned a
coherence of **1.0000000105885025** on a stack that is coherent by construction.
Cauchy-Schwarz bounds it by one; float accumulation does not, and this was the
only one of the three branches that did not clamp -- the correlator clamps its
peak in `coreg.c`, the phase branch clamps its amplitude stability, this did not.
So a perfectly coherent window reported a quality no threshold expressed in
`rs_microm_t.quality`'s documented `[0,1]` could reach. Clamped in
`rs_splitband_shift()`, where the quantity is computed and documented.

**And the first version of that test silently asserted nothing.** It set the gate
to `achieved + 0.5` clamped to 1.0, which against an achieved 1.0 asked for 1.0
and never fired. `coherence_min` is a threshold, not a coherence, and nothing
requires it to be reachable; the case now sets `achieved + 1e-6` unclamped. Both
new cases were checked by disabling the code they cover and confirming they fail.

### The dead fields, removed

`rs_slc_t.doppler` and `.orbit` are gone, along with `rs_dopp_poly_t`,
`rs_state_vector_t`, `rs_orbit_t`, `RS_DOPP_POLY_MAX` and `RS_ORBIT_MAX`. Three
call sites in total, one of them a struct-copy in `rs_subaperture_split()` that
propagated zero to zero.

**They were 92 percent of the struct.** `sizeof(rs_slc_t)` was 3976 bytes, of
which the orbit table was 3592 and the Doppler polynomial 64. It is now 320.
That is not incidental, because `rs_subap_stack_t` holds an ARRAY of these, one
per sub-look:

```
   128 looks:   497.0 KB ->   40.0 KB
   512 looks:  1988.0 KB ->  160.0 KB
  2048 looks:  7952.0 KB ->  640.0 KB
```

`--stream` exists because this pipeline's memory ceiling shaped where a grid
could be placed at all (`FOLLOW-UPS.md` item 22), so 7 MB of never-written
metadata on a large stack was worth removing on its own terms.

**Verified unchanged rather than assumed.** The struct every stage flows through
changed shape, so the check is end-to-end: an `mmotion` run before and after
produces byte-identical `_windows.csv` numbers, and `focus` still runs. 21/21
pass, clean-tree build warning-free.

The reasoning that made the removal safe is retired into `slc.h`'s file comment,
where the next person to consider adding an orbit table will find it: git holds
the interpolator and the evaluator, and a field should come back only WITH a
consumer and a fixture.

### The full ASAN + UBSAN pass

`-fsanitize=address,undefined` over the entire suite, with OpenMP enabled -- so
the parallel regions in `rs_microm_track()` and `rs_focus_backproject_opts()`
were instrumented, which is where a data race or a per-thread buffer overrun
would live. `ASAN_OPTIONS=detect_stack_use_after_return=1:strict_string_checks=1`.

**Clean. 17 of 17 binaries, zero diagnostics, zero failures.** 415 s against 190 s
in Release.

**Checked properly, because `ctest` alone could not have shown this.** With
`UBSAN_OPTIONS=halt_on_error=0`, a UBSAN violation prints to stderr and leaves the
exit code at zero, so a test reports "Passed" with a runtime error in its output --
and `ctest --output-on-failure` shows output only for tests that failed. A green
`ctest` run is therefore not evidence of a clean sanitizer pass. Every binary was
re-run directly with stderr captured and the combined output scanned for
`runtime error`, `AddressSanitizer`, `LeakSanitizer` and `SUMMARY:`.

**And the sanitizers were verified armed rather than assumed.** A canary with a
deliberate heap-buffer-overflow and a signed-shift overflow, built with the same
flags, produced three diagnostics; `nm` shows 25 `__asan` symbols in
`test_tracking`. Zero findings from an uninstrumented build would have looked
identical to zero findings from a clean one -- the same trap as the incremental
build that produced this file's first wrong claim.

**What it does not cover.** No leak detection: LeakSanitizer is unavailable on
arm64 macOS, so allocations never freed are still unchecked by anything. The
suite exercises the readers against a malformed-input corpus but not a fuzzer.
And ASAN sees only paths the tests take -- `--reference pair` and the streaming
CPHD routes get exercised, the CLI's own argument handling does not.

### Still open

- **Neither `lag` nor `splitband` passes `rs_track_fit()`**, and the new tests do
  not claim otherwise -- they cover each branch's contract, not its accuracy.
  What each recovers is `FOLLOW-UPS.md` item 7's business and is unchanged.
