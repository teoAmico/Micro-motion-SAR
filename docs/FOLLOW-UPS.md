# Open follow-ups

Work identified and deliberately not folded into the change that found it.
Each entry says what the defect is, what evidence exists, and what would have
to be true for the fix to be correct.

Tracked, unlike most of `docs/`. The allow-list exists to keep development notes
and plans local, and this began as one. It is not one now: it is the record of
what has been tried and disproven, including entries that withdraw earlier
entries, and losing it means the next person repeats the work rather than reading
it.

Resolved entries are not kept here **when nothing else depends on the reasoning**:
once a finding is settled it belongs next to the code it constrains, and the
reasoning that produced it is in the commit that made the change. Four were
retired that way -- the correlation-transcendental identity, the quantisation
floor's derivation, the distributed-texture fixture questions, and the sub-look
coherence measurement behind the phase estimator's non-accumulating design. They
now live in `rs_microm_estimator_t`, `rs_spectrum_best_window()` and
`rs_microm_track()`.

Several entries below are nonetheless marked resolved and kept -- items 6, 7 and
16. They are kept because later entries argue against them by number, and an
argument that withdraws item 6 is unreadable once item 6 is gone. Where a
resolved entry's *tables* have been retired into a header they are not
duplicated here; the entry says where they went.

Some entries cite development notes that were kept local rather than tracked:
`IMPLEMENTATION-VERIFICATION.md`, `POSITIVE-CONTROL.md` and
`MODIFIED-BACKPROJECTION.md`. Those citations are marked **[local note, not in
the repository]** where they appear, so a reader outside this working copy knows
the reference cannot be followed rather than assuming they missed a file.

Defects found by reading the code rather than by measuring it are in
`docs/CODE-REVIEW.md`, which also records corrections made to this file. The
2026-08-02 review corrected item 5 (the UAVSAR reader was worse than this file
said, not better) and item 7's line numbers.

---

## Index

| # | status | what it is about |
|---|---|---|
| 1 | open, partly implemented | the quantisation floor has no multiplicity correction |
| 2 | answered, negative | no distributed-texture fixture on which the chain works |
| 3 | open, premise unestablished | the Capella SGN override is keyed on a vendor string |
| 4 | open, untried | long dwells may need deliberate truncation |
| 5 | resolved | `rs_slc_t.r0` meant three things; renamed to `r_scene_m` |
| 6 | resolved | sub-look images are correct; the tracker does not read them |
| 7 | resolved | `REF_LAG`, its frequency floor, and the selection policy discarding recoveries |
| 8 | **withdrawn** | "the defect is the reference scheme" |
| 9 | partly implemented | consensus and contiguity statistics |
| 10 | done | re-scoring documented results through the consensus gate |
| 11 | structural limit | the consensus gate is blind to common-mode artefacts |
| 12 | instruments, not results | ampcor-style cull (a-e), SARPy cross-check (b), second fixture family (f) |
| 13 | closed | overlap buys nothing for correlation; zero is the balanced setting |
| 14 | recovery, bounded by 25 | the phase estimator was wrapping a ramp it should have removed |
| 15 | closed | item 14's anomalies were one fixture mistake; the real-data configuration exists |
| 16 | resolved | `validate` was estimator-blind; it now takes an estimator |
| 17 | null, meaningful | Giza at 90% overlap: the sawtooth artefact is gone on real data |
| 18 | screening | `validate --xml`, and the Istanbul candidate |
| 19 | bounded by 30 | amplitude dispersion, and item 17's null as a precondition failure |
| 20 | done | `D_A` becomes a selector; its threshold measured; `AmpSF` found unapplied |
| 21 | **retraction** | `AmpSF` applied, and it does NOT explain the `D_A` gap |
| 22 | done | notes from NGA's six-library; streaming stages 1 and 2 |
| 23 | negative | neither density, selection bias nor the antenna taper explains the `D_A` gap |
| 24 | mechanism found | aspect-dependent scattering, and the first fixture that refuses |
| 25 | bounds item 14 | item 14's recovery does not survive aspect dependence; the PS selector does |
| 26 | open | the correlation route against aspect dependence |
| 27 | fixed | `--null-static` was comparing against a defocused scene |
| 28 | fixed | the Giza positive control was invalid: the injected target did not focus |
| 29 | superseded by 30 | the first positive control that asked the question |
| 30 | bounded by 31 | the bar met on real data by the tracker, not by anything that reports |
| 31 | bounded by 32 | the reported policy had the right frequency; the scene-wide GATE discarded it |
| 32 | closed | the published estimator is brightest-pixel argmax; item 6 measured it and it was never built |
| 33 | fixed by 34 | the static null is not comparable to Giza; item 31's gate cannot adjudicate it |
| 34 | done | null density derived from geometry; it now separates signal from nothing on real data |
| 35 | **the result** | ADJUDICATED at p = 0.05: a real-data measurement a proper null could not reproduce |
| 36 | done | Umbra and ICEYE read correctly; the per-vendor SGN override is confirmed right |
| 37 | done | bin 1 outscored the truth below 2 mm and every gate endorsed it; the first three bins are now unreportable, which relocates a trend rather than removing it |
| 38 | done | a ZERO-amplitude injection outscores every real one, so the p-value measured the scatterer being added, not it moving |
| 39 | done | the paired increment at a nominated frequency answers item 38: the scene gains exactly zero, the target gains |
| 40 | superseded by 41 | the target can be moved off the grid origin at last, and the reported window follows it |
| 41 | done | the 16 m offset is not geometry: overlapping windows track one scatterer and the centred one scores lowest |
| 42 | done | the centroid over the agreeing cluster turns item 41's 16 m localisation bound into 0.1 m |
| 43 | done | that 0.1 m is amplitude-dependent: 0.13 m at 2 mm, 2.24 m at 0.125 mm, linear in 1/amplitude |
| 44 | done | dwell truncation works and `validate` silently ignored the flag that does it |
| 45 | done | first ICEYE measurement: urban misses the PS precondition too, and the quality gate discards the true positive |
| 46 | **the current state** | the phase route's quality is now spatial dominance, which fixes item 45 and leaves the gate inert |

---

## 1. The quantisation floor is a per-window test with no multiplicity correction

`rs_spectrum_best_window()` applies the 2.449-step floor to each window
independently and returns the most prominent survivor. Nothing accounts for how
many windows were tested, so the chance that *some* window crosses the floor on
quantisation noise alone grows with the grid.

**Measured on the real Giza collect**, same scene and same chain at two grid
sizes:

| | 225 windows | 961 windows |
|---|---|---|
| excursion exactly zero | 225 (100%) | 958 (99.7%) |
| cleared the floor | 0 | 2 |
| reported | `RS_ERR_RANGE` | 0.183 Hz, prominence 29.9 |

The observable is the same in both. Only the number of opportunities changed,
and that flipped an honest refusal into a confident-looking frequency. The
winner sits at an observation ratio of exactly 3.00 -- one of the frequencies
where this observable's response is exactly zero -- and its series is a
five-level staircase: an artefact by every available test.

**Implemented so far:** the count is reported. `rs_spectrum_best_window()` takes
`out_n_candidates` and `mmotion` prints `N of M windows were eligible` beside
every selection, warning below four on a bound taken from the window geometry --
windows overlap at a stride of half their width, so a resolvable target occupies
a 2x2 block at minimum.

**Not implemented, and what a fix has to do.** Not merely raise the threshold;
that trades one arbitrary constant for another. The floor is three sigma for
*one* window. Testing `n` needs either a correction over the EFFECTIVE number of
independent windows -- fewer than `n`, because overlapping windows are
correlated -- or a different statistic: the *fraction* of windows clearing the
floor and its spatial contiguity is far more informative than the best single
one, since a real localised mode should occupy a patch rather than two scattered
windows.

---

## 2. No distributed-texture fixture exists on which the chain demonstrably works

The current one cannot work, for reasons now understood and recorded in
`rs_microm_estimator_t`: a lone vibrating point inside static clutter is diluted
roughly 9:1 in its own correlation window, and the paper front end's sweep puts
every resolvable bin at an integer observation ratio.

`sim_cphd --clutter-vib` was added so the whole patch can vibrate coherently,
which is what a structure's surface does. A configuration that actually recovers
an injected frequency on distributed texture -- coherent motion, a window
spanning several sub-look resolution cells, sub-looks that share most of their
spectrum, or the phase estimator -- has **not** been demonstrated.

**ANSWERED 2026-07-31, and the answer is negative for both estimators.**

Swept with `sim_cphd --clutter-vib` -- 400 coherently vibrating Rayleigh
scatterers, so the patch moves as a whole -- at the one operating point ever
demonstrated to track anything: pulse route, 128 looks, zero overlap, 0.5 m
cell, 32 px window, upsample 40. Displacement held constant at 10 px
peak-to-peak by scaling amplitude as 1/f, so amplitude is not confounded with
frequency. Five frequencies from 0.3 to 1.1 Hz, three clutter seeds, each with a
static control on the identical clutter.

```
                 0.3    0.5    0.7    0.9    1.1   static   slope    rms
correlation s7  2.671  2.117  0.353  0.806  1.109  2.974   -2.217  1.2935
correlation s11 1.159  0.504  1.109  2.772  2.369  0.554   +2.344  1.0973
correlation s23 0.806  1.462  0.302  1.411  2.319  0.806   +1.487  0.7857
phase       s7  2.369  1.008  1.058  2.319  1.462  1.008   -0.252  1.1672
phase       s11 1.764  1.764  1.764  1.764  1.764  1.663   +0.000  1.1010
phase       s23 0.857  2.974  1.411  1.815  0.302  1.109   -1.135  1.2970
```

Neither tracks. Correlation's slope swings from -2.2 to +2.3 across seeds, which
is noise rather than a trend; phase on seed 11 returns a fixed 1.764 Hz for every
injection with its static control at 1.663. Every static control lands inside the
spread of the moving reports. Doubling the displacement to 20 px changes nothing
-- phase then returns 1.109 Hz for all five injections AND for the static
control.

**The operating point was admissible, which is what makes this decisive.** The
tool reported sub-look resolution 8.26 m and an observable band to 3.23 Hz, so
the ambiguity ceiling is 24.8 px against a 4 px textured floor: the injected
10 px sits mid-window, and 20 px is still inside it. Every injection is well
inside the band, and the observation ratio at 0.5 Hz is 0.33. This is not a
misconfigured run.

**Note the near-misses.** Correlation seed 11 reports 0.504 against an injected
0.5, and seed 23 reports 0.302 against 0.3. Read one at a time those are
recoveries; read across the sweep they are the coincidences that make a
per-point criterion useless, and they are why `rs_track_fit()` exists.

**What this closes.** The synthetic evidence for the tracker now rests on
exactly one configuration -- correlation, isolated point target, empty scene,
128 looks, zero overlap -- which is the easiest case that exists and the
furthest from a real structure. Distributed texture was the last standing
explanation for the negatives that was not "the chain does not work". It no
longer stands.

**Not written as a regression test, deliberately.** The correlation sweep costs
2 minutes 48 seconds for three seeds, which would quadruple `ctest`. The result
is recorded here and in `IMPLEMENTATION-VERIFICATION.md` **[local note, not in
the repository]** instead. If it is ever
worth locking in, one seed at three frequencies is about a minute and would
catch a change of sign.

---

## 3. The Capella SGN override is keyed on a vendor string

`rs_read_cphd()` inverts the FX-to-delay transform for Capella products because
their signal does not follow their declared `Global/SGN`; read to the standard,
their imagery comes back mirrored in range. The trigger is a substring match on
`CollectorName`.

That is blunt in both directions. If Capella corrects the metadata the override
keeps firing and silently starts mirroring; if another vendor ships the same
defect it is not caught.

A robust check would test the data rather than the label -- compressing a pulse
both ways and choosing the direction whose energy lands inside the declared
`TOA1`/`TOA2` support would decide it per product, without a name. Not
implemented.

### Item 3, measured: THE PROPOSED TEST PICKS THE WRONG DIRECTION

Measured before implementing, on the Giza collect. Sixty-four consecutive pulses
were compressed both ways, the power falling inside that pulse's `[TOA1, TOA2]`
integrated, and the larger fraction taken as the winner. Capella needs the
INVERTED transform, so "inverse" is the correct answer at every row:

```
        support (bins)   asymmetry   forward   inverse   picks
start   [-6945, +5031]        1914   54.794%   38.768%   FORWARD, wrong by 16.0 pp
mid     [-5950, +6027]          77   53.897%   54.282%   inverse, right by  0.4 pp
end     [-6343, +5634]         710   48.016%   44.756%   FORWARD, wrong by  3.3 pp
```

**It is wrong where it is confident.** The test has discriminating power only in
proportion to the support's asymmetry -- a symmetric window cannot distinguish a
profile from its mirror -- and the asymmetry is largest at the dwell ends. Those
are exactly the two rows it gets wrong, by 16 and 3.3 percentage points. The one
row it gets right is mid-dwell, where the support is almost symmetric (77 bins
out of ~12000) and the 0.4-point margin is noise. Implemented as written, this
would silently mirror Capella imagery, which is the failure the vendor override
exists to prevent.

**The premise is what fails, not the arithmetic.** Between 46 and 55 percent of
the energy lands OUTSIDE the declared support in EITHER direction: 90% of the
power spans about +/-12100 bins while the support is only ~12000 bins wide.
`TOA1`/`TOA2` do not bound where this product's compressed energy sits, so
"the correct direction is the one that lands inside" has nothing to key on.

**CAVEAT, AND IT IS NOT A SMALL ONE.** The compression above is a plain FFT of
the raw FX samples with `TOA` mapped straight onto signed FFT bins. If
`TOA1`/`TOA2` are referenced to something other than that delay axis -- a deramp
reference, or a convention tied to `SC0` -- then the mapping is wrong and these
numbers say nothing about the idea, only about this reading of the fields. Half
the energy sitting outside the declared support is itself evidence that
something in the convention is not understood, and is the thing to explain
before any detector is written.

**So the open item is no longer "implement the detector".** It is: establish what
`TOA1`/`TOA2` actually bound for this product. Until that is answered, a
data-driven SGN test has no established statistic to use.

**And when one exists, it should not REPLACE the vendor override.** Run both and
warn on disagreement, the way the cull is reported beside the consensus. The
override is known correct for the one product family that exhibits the defect;
swapping it for an automatic test whose failure mode is a silent mirror is the
wrong trade. There is also no conformant CPHD in `sar-data` to exercise the
other branch against, so a detector written today could only ever be tested on
the case it must invert.

---

## 4. Long dwells may need to be deliberately truncated, and never have been

Every run this project has made feeds the whole collect in: 32.869 s at Giza,
and the Istanbul candidate would be 25 or 60 s. The published work appears not
to do that.

**The direct evidence.** `MODIFIED-BACKPROJECTION.md` **[local note, not in the
repository]** records that the Trento
corner-reflector validation reconstructs displacement from **the first three
seconds of a roughly 20 s dwell**, and that the authors note the spectrogram's
clarity degrades after about 11 s. It is already flagged there as "a design
choice this project has not tried."

**The arithmetic that independently implies the same number.** The tracker's
amplitude window reaches its 4.4x margin -- the only ratio ever demonstrated to
work -- at `t_sap` of roughly 0.16-0.21 s across every X-band sensor surveyed in
`DATASETS.md`, because the window depends on `t_sap` in seconds and not on the
aperture fraction. The published campaigns state `alpha` of 4.5-7.6%. Since
`alpha = t_sap / T`, those two together give

```
T = t_sap / alpha = 0.16/0.076 .. 0.21/0.045  =  2.1 .. 4.7 s
```

So the literature's *effective* dwell is a few seconds, arrived at from its
stated aperture fraction without reference to Trento, and Trento's three seconds
sits inside that range.

**Why this is not merely a curiosity.** If the usable dwell is ~3 s, then a
32.9 s collect is not a better version of a 3 s one -- it is eleven windows, and
processing it as a single span buys frequency resolution while risking whatever
degrades after 11 s. Candidates for what that is: orbital curvature the
backprojection grid does not model over the full arc, atmospheric phase drift,
target decorrelation, or the phase-unwrap accumulation `microm.h` already
predicts and which run A exhibited.

**What would settle it.** Cheap, and no new machinery: cut a collect into
consecutive ~3 s spans, run the identical configuration on each, and compare.
Three outcomes distinguish themselves:

- the reported frequency is stable across spans -- the long dwell was fine, and
  the truncation buys nothing but costs `df`
- it is stable early and wanders late -- something accumulates, and the point at
  which it starts is measurable
- it is unstable everywhere -- the configuration was never measuring anything,
  which is a different finding and worth having

The third is the one the null tests already suggest, and this is a cheaper way
to reach it than another full-dwell run. Scoring must use `rs_track_fit()` over
an injected sweep, not a single reported value, and must be pooled over seeds.

**AND IT CANNOT CURRENTLY BE DONE FROM THE CLI.** `mmotion` hard-codes
`max_pulses = 0` and `pulse_first = 0` in its read options; `--max-pulses` and
`--pulse-start` are parsed by `focus` only. `rs_cphd_read_opts_t` already carries
both fields, so this is an exposure gap rather than a missing capability -- but
until it is closed, "never have been" understates it: the experiment this item
asks for is not reachable. Found while screening a Los Angeles collect against
the published operating point (`runs/screens/`), where the literature's 3.6
percent aperture fraction on a 25.4 s dwell gives a 0.91 s sub-look and an
averaging ceiling of 0.547 Hz -- a FAIL that is an artefact of the untruncated
dwell rather than of the collect.

**Where it bites first.** The Istanbul 25 s collect is eight such spans. Deciding
before the run whether to process it whole or in spans is a configuration choice
that belongs in the `RUN.md` question, not a post-hoc reinterpretation.

---

## 5. `rs_slc_t.r0` means different things in different readers

`slc.h:65` documents `r0` as "slant range of first range sample, m". **No reader
fills it that way**, and there are three conventions in play rather than two.

`readers/sicd.c:450` sets it from the product's `SCPCOA/SlantRange`, which is the
range to the scene centre point. On a wide swath that differs from the first
sample by half a swath -- tens of metres on a spotlight product, kilometres on a
stripmap one.

**CORRECTED 2026-08-02: the UAVSAR reader is worse, and this entry said it was
right.** `readers/uavsar.c:199` reads the annotation field `Average Altitude` and
`:213` assigns it to `img->r0`. An altitude is not a slant range at all: at
UAVSAR's ~12.5 km flight altitude and look angles from about 20 to 65 degrees the
near-range slant distance is 13 to 30 km, so the field is low by a factor of one
to two and the error grows across the swath. The same four lines read
`Global Average Terrain Height` into a local named `inc` and never use it, so
`img->incidence` stays zero for every UAVSAR product. See `docs/CODE-REVIEW.md`
findings 3 and 4.

That changes what a fix must preserve. The two options below were written on the
belief that UAVSAR held the documented meaning and only SICD deviated; neither is
a simple re-pointing now, because there is no reader whose convention is the one
to keep.

**Found by** writing `validate --sicd`. The first version added
`0.5 * n_rg * rg_spacing_m` to reach the scene centre, which is right for the
documented meaning and double-counts for what the SICD reader actually stores.

**Why it mattered beyond that one call site.** Every consumer of an `rs_slc_t`
had to guess which convention it was holding, and the guess is invisible when
wrong: a slant range off by half a swath still produces a complete image and a
complete spectrum. `rs_geo_slant_to_ground()` and the sub-look resolution both
take it, so the error propagates into geolocation and into the ambiguity ceiling.

### Item 5, RESOLVED 2026-08-02: the field is renamed and means one thing

Of the two options this entry proposed -- derive a first-sample range in every
reader, or redefine the field as the scene-centre range and rename it -- the
second is done, and the measurement that decided it is that **every producer was
already writing the scene-centre range.** Only the comment said otherwise:

```
focus.c        r_ref[p_mid]              range to the SRP at mid-dwell
subaperture.c  the same, per sub-look
sicd.c         SCPCOA/SlantRange         range to the scene centre point
uavsar.c       (alt - terrain)/cos(look) at the scene average look angle
```

So there was no first-sample convention to preserve, and deriving one would have
meant four call sites adding half a swath back to reach what they wanted.
`rs_azimuth_resolution()`, the shift-to-velocity conversion in
`rs_microm_track()` and the incidence derivation `acos(|pz|/R)` all want the
range to the patch being measured.

**`r0` is now `r_scene_m`**, "slant range to the scene reference point at
mid-dwell". The rename is the part that makes it safe, exactly as this entry
argued: a stale `img->r0` no longer reads a quantity that moved underneath it,
it fails to compile. It did -- `tests/test_readers.c` was the caller it caught.

**ONE CALLER HELD THE OLD ASSUMPTION AND WAS WRONG.** `rs_crop()` in
`tools/crop_slc.c` advanced the range by `rg0 * rg_spacing_m`, correct for a
first-sample range and wrong for a scene-centre one -- on a stripmap swath, wrong
by kilometres. The correct update is the change in CENTRE bin,
`(rg0 + n_rg/2) - (n_rg_src/2)`, which differs from the old formula everywhere
the crop is not centred and moves the range BACKWARDS for a crop at bin zero
where the old formula moved it not at all.

That defect survived because a static helper in a tool is unreachable from the
suite. The function is now `rs_slc_crop()` in the library and
`tests/test_readers.c` pins three cases chosen so the two formulas cannot agree
on all of them, plus that `t0` still offsets from the first LINE and that an
unset range stays unset rather than acquiring an offset.

**AND A SECOND FIELD MOVES THE OTHER WAY, which the first version of this fix got
wrong.** `rs_slc_crop()` carried `plane` across untouched, under a comment
claiming the image plane is referenced to the scene reference point so cropping
does not move it. It is not. `rs_geo_plane_point()` computes
`origin + x*u_x + y*u_y` from image coordinate (0,0), and `info` calls it with
(0,0) and labels the result *"first az, first rg"* -- so `plane.origin` is the
ECF position of the FIRST SAMPLE and must advance by the crop offset:

```
origin += az0 * az_spacing_m * u_x  +  rg0 * rg_spacing_m * u_y
```

Left unmoved, a cropped product geolocates every sample to where the UNCROPPED
scene had it. Measured on the fixture: 43.4 m of error at every probe, on a crop
cut at (9, 17). `info` would print the original scene's corners and `--at` would
resolve to the wrong ground position, both with complete and plausible output.

So one struct carries two reference points and the crop has to know which is
which -- `r_scene_m` from the scene centre, `plane.origin` from the first sample,
moving by different amounts in different directions. The test asserts the
property rather than the arithmetic: a surviving sample must reach the same ECF
point through the cropped plane as through the source plane, checked through
`rs_geo_plane_point()` so it cannot restate the implementation. Verified to fail
against the old behaviour before being relied on.

The reasoning is retired into `rs_slc_t.r_scene_m`'s contract in `slc.h`, which
is where it constrains anything.

---

## 6. The sub-look images are correct; the tracker does not read them

`POSITIVE-CONTROL.md` **[local note, not in the repository]** localised the
failure to "what reaches the correlator --
patch extraction, the reference look, or the sub-look images themselves" and
said "that is the next thing to bisect and it is not done here." It is now done
one level further, and the sub-look images are eliminated.

**The images carry the motion.** Taking the plain argmax of `|look[i]|` over the
whole image, for an isolated target injected at 0.5 Hz and 20 mm (predicted
azimuth excursion +/- 8.4 cells at a 0.5 m cell):

| configuration | fitted amplitude | predicted | variance at f | phase |
|---|---|---|---|---|
| 128 looks, overlap 0.00 | 8.24 cells | 8.4 | **0.933** | 0.01 rad |
| 159 looks, overlap 0.88 | 10.65 cells | 10.5 | **0.832** | 0.01 rad |
| 64 looks, overlap 0.50 | 7.58 cells | 8.4 | **0.979** | 0.01 rad |

Right amplitude, right phase, most of the variance at the injected frequency --
and the failing configurations are as good as the working one. The peak
magnitude varies by only 1.16x across the series, so the target is not fading.
An integer argmax with no sub-pixel refinement at all recovers the motion.

**The tracker does not.** Handed the same stack, for the window containing the
target:

```
tracker shift series, first 10:  +0.0 -12.0 -15.2 +14.6 +10.3 +8.3 +7.0 +7.9 +8.7 +14.2
argmax trajectory,   first 10:   +7.0  +7.0  +2.0  -3.0  -3.0 -8.0 -8.0 -8.0 -3.0 -3.0
```

Correlation between them **-0.190**, with **4.1%** of the tracker's variance at
the injected frequency against 93% for the argmax. The tracker's excursions
reach +/-15 px where the target moves +/-8.4, which is the saturating-argmax
signature already recorded.

**Some window does track.** Scanning all windows, the best reaches correlation
**+0.832** and 74% of variance at f -- but it is not the window containing the
target, and which window wins moves with the target's position. So the
information survives into the tracking stage and is then attributed to the wrong
place.

**What this settles.** The fault is downstream of the sub-look images, in the
tracker's consumption of them. It is not missing physics, not a defect in
focusing or sub-aperture formation, and not a sensitivity limit -- the signal is
demonstrably present and recoverable by the crudest possible estimator. That
eliminates two of the three explanations `README.md` leaves open.

**What is NOT established.** Why. Two hypotheses were tried and neither holds:

- *Window-edge crossing.* The idea that the tracker reads the target entering and
  leaving the analysis window rather than its displacement. **Refuted:** with a
  64 px window and 32 px stride the target sits 24 px clear of both edges and a
  window still tracks at +0.861.
- *Excursion as a fraction of window size.* Suggested by the containing window
  failing at 32 px and a window tracking at 64 px. **Not measured** -- the sweep
  written for it mis-identified the containing window and returned nothing
  usable. It remains the most promising next cut.

The next step is to compare, for a single window and a single look pair, the
correlation surface the tracker computes against the shift the argmax implies.
That is one window and two images, and it is where the +0.832 and the -0.190
have to diverge.

### Item 6, resolved: the sub-looks are too decorrelated to correlate

Done as described above -- one window, the same patches `rs_microm_track()`
extracts, handed to the same primitive it calls.

**The primitive is sound.** Look 0 against itself returns shift +0.00 at peak
1.000. Where the correlation peak is high the shift is right: look 16 peaks at
0.571 and returns -10.25 px against an argmax truth of -10.0. Where the peak
collapses to 0.02-0.2, the returned shift is unrelated to the truth and reaches
the search extent.

**The peak collapses because consecutive sub-looks share no pulses.** Measured
mean correlation peak against look 0, over the window containing the target:

| overlap | mean peak | rms(coreg - truth) | looks agreeing within 2 px |
|---|---|---|---|
| 0.00 | 0.090 | 17.76 px | 10 of 128 |
| 0.50 | 0.107 | 16.07 px | 19 of 128 |
| 0.75 | 0.177 | 13.49 px | 28 of 128 |
| 0.90 | 0.310 | 10.29 px | 40 of 128 |
| 0.95 | 0.340 | 8.30 px | 27 of 128 |

`rs_microm_estimator_t` already carries the table this reproduces -- coherence is
very nearly the fraction of pulses two sub-looks have in common, reaching 0.07 at
zero sharing. The measured 0.090 at zero overlap is that number. **The
configuration this project calls its working operating point -- 128 looks, zero
overlap -- correlates sub-looks that share no pulses at all.** Its own header
predicts a coherence of 0.07 for that, and the tracker is being asked to find a
displacement between independent speckle realisations.

**And raising overlap does not rescue it.** At 0.95 the peak only reaches 0.340
and 27 of 128 looks agree. Meanwhile high overlap sharpens the sub-look and
collapses the wrap ceiling -- 0.88 overlap gives a 4.8 px ceiling against a 7 px
floor, measured, so the window closes.

**The two constraints oppose each other on the same knob, and that is new.**
Coherence needs overlap; the ambiguity ceiling forbids it. Both are documented
in this codebase, separately, and neither is stated as the other's opposite.
Together they explain why no configuration has worked: there may be no overlap
that satisfies both, and if so the correlation estimator cannot work on this
geometry at any setting rather than merely failing at the ones tried.

**What would settle that.** Compute both curves over overlap on the same
collect -- coherence from pulse sharing, ceiling from sub-look resolution -- and
find whether they cross. That is arithmetic on quantities `validate` already
derives, needs no processing run, and either produces an admissible overlap band
or proves there is none. It is the single most valuable thing left.

### The curves cross, and the binding constraint is the reference look

Computed on the Giza geometry as `validate` derives it -- T 32.869 s, lambda
0.0322 m, R 762.8 km, V 7263 m/s, 0.4 m cell, 7 px floor. Ceiling from sub-look
resolution, coherence from pulse sharing via the table in
`rs_microm_estimator_t`.

**There is an admissible band.** Seventeen (N, overlap) combinations clear both
constraints -- ambiguity ceiling above the floor AND adjacent-look coherence
above 0.6:

| N | overlap | t_sap | ceiling | gamma | band |
|---|---|---|---|---|---|
| 256 | 0.75 | 0.508 s | 12.5 px | 0.61 | 0.98 Hz |
| 512 | 0.90 | 0.631 s | 10.1 px | 0.78 | 0.79 Hz |
| 1024 | 0.90 | 0.318 s | 19.9 px | 0.78 | 1.57 Hz |
| 2048 | 0.75 | 0.064 s | 98.9 px | 0.61 | 7.80 Hz |

So the two constraints do NOT exclude each other, and the earlier suspicion that
they might is withdrawn. High look counts at moderate-to-high overlap satisfy
both, with margins up to 14x the floor and bands out to 7.8 Hz.

**Every one of them is unreachable with a fixed reference.** The last column of
the full sweep is the fraction of the series that shares any pulses at all with
look 0, and across all seventeen it runs **0% to 3%**. Looks i and j overlap only
while `|i-j| < 1/(1-overlap)`, so with a fixed reference the coherent span is a
handful of looks out of hundreds and the rest is independent speckle. That is
what the measured peak of 0.090 at zero overlap was showing, and raising overlap
does not fix it because the span grows as `1/(1-overlap)` while the series grows
as N.

**So the defect is the reference scheme, not the geometry.** `--reference first`
is the default and it discards the coherence the admissible band exists to
provide.

**And the estimator built for exactly this has never been tested.**
`rs_microm_estimator_t` describes `RS_MICROM_EST_SPLITBAND` as using "all N^2
interferograms rather than the N-1 formed against one reference", coming "within
0.5 dB of the Cramer-Rao bound", and requiring "interferometric coherence between
looks; it has nothing to track if that is absent". That requirement is precisely
what the admissible band supplies (gamma 0.61-0.85) and what zero overlap denies
(gamma 0.07). Correlation and phase have now both been swept and both fail.
Split-band phase linking is the untested third, and it is the one whose stated
design matches the constraint that turns out to bind.

**Next.** Sweep `RS_MICROM_EST_SPLITBAND` with `rs_track_fit()` at N=512,
overlap 0.90 -- ceiling 10.1 px, gamma 0.78, band 0.79 Hz, all comfortably
admissible -- and again at N=1024, overlap 0.90 for a 1.57 Hz band. Static
control on the same clutter, pooled over seeds. If it tracks, the negatives so
far are a reference-scheme defect and the method is recoverable. If it does not,
all three estimators have failed at operating points this project's own
arithmetic calls admissible, and that is the end of the line.

---

## 7. RS_MICROM_REF_LAG implemented; it fixes coherence and exposes a frequency floor

A fourth reference mode: each look against the one `ref_lag` places before it,
with no accumulation. `--reference lag --lag N`. Twenty-odd lines beside the
other three, plus `ref_lag` in `rs_microm_params_t`.

**It does what it was designed to do.** On the distributed fixture at 512 looks
and 0.75 overlap, tracking quality goes from **0.049 to 0.806**. The coherence
the fixed reference was throwing away is recovered exactly as predicted.

**And it is not sufficient, for a reason that is now arithmetic.** Three
constraints have to hold at once:

```
(1) ceiling      1.5 * res_sap / cell  >  2A        excursion fits, no wrap
(2) coherence    gamma(1 - L(1-overlap)) > 0.6      patches correlate at the lag
(3) differential 2A * sin(pi f L dt)   >  7 px      the difference clears the floor
```

(3) is the new one and it is what a differencing observable costs. At lag 1 on
the fixture tested, the differential is `2 x 8.4 x sin(pi x 0.5 x 0.0388)` =
**1.0 px** against a 7 px floor -- the signal is real, coherent and far too small
to measure. Raising the lag raises the differential and destroys (2).

**Swept over N, overlap, lag and frequency on the Giza geometry, 15 combinations
satisfy all three -- and every one of them starts above 1.5 Hz:**

| f_min | f_max | N | overlap | lag | gamma | ceiling |
|---|---|---|---|---|---|---|
| 1.56 | 15.59 Hz | 4096 | 0.75 | 1 | 0.61 | 197.7 px |
| 1.56 | 7.80 Hz | 2048 | 0.75 | 1 | 0.61 | 98.9 px |
| 1.96 | 6.24 Hz | 2048 | 0.80 | 1 | 0.67 | 79.2 px |
| 2.60 | 9.36 Hz | 4096 | 0.85 | 1 | 0.72 | 118.7 px |

**So the differencing observable has a hard low-frequency floor near 1.5 Hz on
this collect**, because the differential goes as `f` at low `f` while the lag is
capped by coherence. Every measurement this project has attempted has been at
0.05-1 Hz, which is exactly where this observable has no sensitivity, and the
absolute reference that does have low-frequency sensitivity is the one that
decorrelates. That is the trade, stated as an inequality rather than a suspicion.

**The constructive part.** 1.56-15.6 Hz is not a useless band -- it contains the
1-3 Hz where medium-span bridge decks live, which is the target class the archive
survey in `DATASETS.md` settled on. The Istanbul and Bratislava candidates were
chosen for structures whose modes sit inside the one band this observable can
reach.

**Untested, and this is the next step.** The table above is arithmetic on
constraints, not a measurement. A confirming sweep needs N=2048 at 0.75 overlap,
lag 1, injected 2-4 Hz at roughly 15 mm, scored with `rs_track_fit()` and pooled
over seeds. At about four minutes a run it is roughly twenty minutes for five
frequencies on one seed. Nothing below 1.5 Hz should be expected to work, and a
sweep that includes sub-Hz points will show the floor rather than a failure.

### Split-band, for completeness

`RS_MICROM_EST_SPLITBAND` was swept at N=512/0.90, N=1024/0.90 and N=2048/0.90.
All three return one fixed frequency for every injection and for the static
control: 0.417 Hz, 0.391 Hz and 0.391 Hz respectively, slope 0.000, identical
across three clutter seeds. So all three estimators fail with the FIRST
reference, which is consistent with the reference being the defect rather than
the estimator -- and is why the lag mode was written.

### Item 7, first measurement: the lag mode tracks above its predicted floor

N=2048, overlap 0.75, lag 1, excursion held at 44 cells, one clutter seed.

```
f=2.5 Hz  differential  6.7 px (BELOW the 7 px floor)  ->  0.391 Hz
f=3.0 Hz  differential  8.1 px                         ->  2.604 Hz
f=3.5 Hz  differential  9.4 px                         ->  3.125 Hz
f=4.0 Hz  differential 10.8 px                         ->  3.581 Hz
f=4.5 Hz  differential 12.1 px                         ->  4.492 Hz
STATIC    no motion                                    ->  0.391 Hz
```

Over the four points above the floor: **slope +1.224, rms 0.3439 Hz**.

**This is the first configuration in the project whose output follows its
input.** Every other estimator and operating point tested -- correlation, phase,
split-band, at every setting -- returned slope 0.000 and a single fixed value.

Three things make it more than a fluke:

- **The static control returns 0.391 Hz**, well clear of every supra-floor
  reading. The rising sequence is not something the processing produces
  regardless of the scene.
- **The one sub-floor point returns that same 0.391 Hz.** The only injection
  whose differential falls below the tracking floor is the only one that reports
  the artefact, and it reports exactly the artefact the static scene does. The
  split lands where the arithmetic put it.
- 0.391 Hz is also what split-band returned at N=512, 1024 and 2048, so it is a
  property of the chain rather than of this mode.

**It does not pass `rs_track_fit()`.** The rms is fourteen times the half-bin
bound, there is a systematic 0.4 Hz low bias on three of four points, and the
slope is 1.224 rather than 1.0. **And it is one seed**, which by this project's
own standard settles nothing about a configuration.

So: tracks, imprecisely, above the frequency floor its own arithmetic predicts,
on one realisation. Much more than "fails", much less than "works".

**Next, in order.** Repeat over seeds 11 and 23 and pool. Then chase the low
bias -- a 0.4 Hz offset that vanishes at 4.5 Hz is not obviously a scale error,
and the differencing response |2 sin(pi f L dt)| is not flat across the sweep, so
compensating for it before peak-picking is the first thing to try.

### Item 7, pooled over three seeds: the trend survives, the precision does not

Same configuration, seeds 7, 11 and 23.

| f | seed 7 | seed 11 | seed 23 | spread |
|---|---|---|---|---|
| 2.5 Hz (sub-floor) | 0.391 | 2.083 | 2.083 | 1.692 |
| 3.0 Hz | 2.604 | 2.604 | 2.604 | **0.000** |
| 3.5 Hz | 3.125 | 3.516 | 3.125 | 0.391 |
| 4.0 Hz | 3.581 | 3.581 | 3.581 | **0.000** |
| 4.5 Hz | 4.492 | 4.492 | 4.102 | 0.390 |
| static | 0.391 | 0.391 | **12.174** | -- |

**Pooled over the supra-floor points: slope +1.120, rms 0.3461 Hz, n = 12.**
Per seed the slopes are +1.836, +1.159 and +1.003 -- all positive, all near 1,
none collapsing or changing sign. No other estimator or operating point in this
project has produced a non-zero slope at all, so the trend is real and
reproducible.

**Four things qualify it, and the third is the one that matters.**

*The rms is 14x the half-bin bound.* It does not pass `rs_track_fit()`. What has
been demonstrated is a response to the injected frequency, not a measurement of
it.

*The static control is not stable.* 0.391, 0.391, 12.174 Hz. The last is near
this configuration's 12.8 Hz Nyquist, which is what a high-pass observable does
with noise. It never lands inside the 2-4.5 Hz band the injections occupy, so it
still separates -- but "the static control returns 0.391" was a seed-7 fact, not
a property.

*There is a systematic offset of about 0.39 Hz, and it is the chain's own
artefact frequency.* The spectrum bins here are 0.0651 Hz, and every reported
value is an exact bin: 32, 40, 48, 54, 55, 63, 69. The injections fall at bins
38.4, 46.1, 53.8, 61.4, 69.1. The reported bin is consistently about six below
the injected one, and 6 x 0.0651 = 0.391 Hz -- the same 0.391 Hz that the static
scenes, the sub-floor injection and every split-band run all return. That is
unlikely to be coincidence and it is a concrete lead.

**A bias survives redundancy**, so this must be understood before a network is
built on top of it. If the offset is the artefact mixing into the peak
selection, a bigger estimator inherits it intact.

*The pooling is weaker than it looks.* `--clutter-vib` moves every scatterer
coherently with the target, so changing the seed changes the speckle but not the
motion. Three seeds sharing an identical signal is a weaker control than three
independent realisations of a lone mover would be, and the exact 0.000 spreads at
3.0 and 4.0 Hz are the visible sign of it. Worth repeating against
`--clutter` without `--clutter-vib` before the result is leaned on.

### Item 7, corrected: the floor claim was computed on the wrong dt

Dumping the shift series with `--shifts` shows the tool's own layout, and it is
not what the arithmetic above assumed:

```
sub-apertures: 2048 looks, dt 0.0075 s
spectra: 1025 bins, 0.0651 Hz resolution
```

`dt` is 0.0075 s, not the 0.00975 s that `t_sap*(1-overlap)` with
`t_sap = T/denom` predicts. The series therefore spans **15.36 s of a 20 s
dwell** -- `1/0.0651` -- leaving 23% of the collect unused, and `df` is 0.0651 Hz
rather than 0.05. Whether the layout is meant to leave that tail is a separate
question and is not answered here.

Recomputed on the tool's dt, the differentials are:

| f | differential | vs 7 px floor |
|---|---|---|
| 2.5 Hz | 5.2 px | below |
| 3.0 Hz | 6.2 px | **below** |
| 3.5 Hz | 7.2 px | above |
| 4.0 Hz | 8.3 px | above |
| 4.5 Hz | 9.3 px | above |

So **two** injections were below the floor, not one -- and the 3.0 Hz point
reported 2.604 Hz on all three seeds, which is a tracking-like response rather
than the artefact.

**The claim that "the split lands where the arithmetic put it" is withdrawn.**
It rested on a dt this code does not use. The pooled slope of +1.120 stands, and
so does the fact that no other configuration produces a non-zero slope; the
supporting story about the floor does not.

### A provenance bug, introduced with the lag mode and now fixed

`--shifts` wrote `reference=first` for a run made with `--reference lag`: the
label site carried a three-way conditional that fell through to "first". Every
lag run before the fix is misrecorded in its own metadata. The live site is
`main.c:1653-1655` and now handles all four modes.

*(Line numbers corrected 2026-08-02. This entry recorded the fix as landing at
`main.c:1469` and `main.c:2205`, calling the second "the `.meta` sidecar". There
is no `.meta` sidecar in the tree and there is exactly one reference-label site
today. Either the sidecar was removed later or the second site was the `--shifts`
header itself.)*

Worth noting how it was caught -- not by a test, but by reading a dump while
chasing something else. The same three-way fallthrough pattern would silently
mislabel any future mode, and nothing in the suite checks that a run's recorded
provenance matches what produced it.

**That is still true and it now costs more.** `PREFIX_windows.csv` -- the
per-window evidence file item 30 turns on having read -- records no
configuration at all: not `--estimator`, not `--overlap`, not `--n`, not
`--reference`, not `--inject-vib`. Item 30's sweep left six such files whose only
distinguishing mark is the filename the operator chose. See
`docs/CODE-REVIEW.md` finding 7.

### Item 7, the bias explained: a 0.391 Hz multiplicative modulation

Dumped the shift series with `--shifts` and took its spectrum independently in
Python. **The tool's peak-picking is correct**: my spectrum of window 42 peaks at
2.6042 Hz, exactly what `mmotion` reported. The spectrum stage is faithfully
reporting the strongest bin of the series it is given, so nothing downstream of
the tracker is at fault.

The series is what carries the defect. Its strongest six bins, for a 3.000 Hz
injection:

```
2.604   0.391   2.995   5.599   3.385   6.250
```

- **2.995 Hz is the injected carrier, and it IS there** -- third strongest.
- **0.391 Hz is the artefact**, second strongest.
- **2.604 = 2.995 - 0.391**, the lower sideband, and it is the strongest bin.
- **3.385 = 2.995 + 0.391**, the upper sideband, fifth strongest.

Both sidebands present, symmetric about the carrier, spaced by the artefact
frequency. That is multiplicative modulation: the tracked series is being
multiplied by something oscillating at 0.391 Hz, and peak selection then picks a
sideband instead of the carrier.

**This explains the bias completely**, including why it is inconsistent. The
reported value is whichever of carrier and lower sideband happens to be stronger,
so most injections come back about 6 bins low and the 4.5 Hz case -- where the
carrier won -- came back exact.

**And it is good news for the method.** The injected frequency is present in the
tracked series. The tracker is recovering the motion; a modulation is stealing
the peak. That is a different and far more tractable problem than "the chain does
not measure anything", which is where this investigation started.

**What 0.391 Hz is remains unidentified.** It is exactly bin 6 of the 0.0651 Hz
axis, a period of 2.558 s fitting 6.0 times into the 15.36 s record, and it is
the same value the static scenes, the sub-floor injections and every split-band
run return. A modulation whose period divides the record length exactly is more
suggestive of the sub-aperture layout than of the scene. The layout here is 12
pulses per look stepping 3 pulses, which is exact and leaves 23% of the dwell
unused.

**Next:** demodulate, or find the source. If the modulation can be identified and
removed, the carrier is already there to be read -- and that is worth more than
the network, because it fixes the answer rather than averaging it.

### Item 7: the multiplicative-modulation explanation is WITHDRAWN

The static scene was dumped and the 0.391 Hz component measured in every window.
If it were a modulation imposed by the processing it would have the same phase
everywhere. It does not:

```
0.391 Hz component across 49 windows, static scene
  amplitude   min 0.0009   median 0.0046   max 0.0141 px
  phase       spread 5.09 rad over a possible 6.28
  phase concentration |R| = 0.174        (1.0 would be identical everywhere)
```

Random phase, and an amplitude of a few thousandths of a pixel. Folding a
window's series at the 341-look period shows no coherent waveform. **There is no
0.391 Hz modulation.** It is where the noise happens to peak in whichever window
wins the prominence contest.

The sideband reading was over-read from a top-six list. Its own evidence was
already against it: multiplicative modulation produces symmetric sidebands, and
the observed pair was not symmetric -- 2.604 Hz was the strongest bin in the
spectrum while 3.385 Hz was only fifth. Picking six bins out of a thousand and
finding two near `f +/- f0` is not the coincidence it looked like.

**What survives, and it is the important part:**

- The spectrum stage and peak-picking are correct. An independent spectrum of
  the dumped series matched the tool's reported 2.6042 Hz exactly.
- **The injected carrier is present in the tracked series** -- 2.995 Hz for a
  3.000 Hz injection, third strongest bin. The tracker recovers the motion.
- The reported peak is a different bin, consistently about 6 bins low across
  three injections and exact on the fourth. That the offsets cluster near 6 is
  still unexplained; it is not a modulation, and it is not spectral leakage or
  the differencing response, both of which bias the other way.

So the problem is **selection, not recovery**: the right frequency is in the
series and is not the largest thing in it. That is a much narrower problem than
where this investigation started, and it points at the peak-picking policy rather
than at the tracker.

**A concrete thing to try**, in preference to another hypothesis: report the
strongest few bins rather than one, and see whether the carrier is reliably among
them across a sweep. If it is, the recovery is real and the failure is entirely
in `rs_spectrum_best_window()` choosing between candidates it has no basis to
rank. That is measurable with the series already dumped, at no processing cost.

### Item 7 resolved: the recovery is real; the SELECTION POLICY discards it

For a 3.000 Hz injection, with the lag reference, the carrier is in the **top ten
bins of all 49 windows** -- rank 1 in 23 of them, median rank 2, worst rank 8.
The tracker recovers the injected frequency essentially everywhere.

The rank-1 frequency of each window, tallied:

```
  2.995 Hz   23 windows      <- the injection
  2.604 Hz   16 windows
  0.391 Hz    8 windows
  6.380 Hz    1 window
 15.169 Hz    1 window
```

**A plurality vote returns 2.995 Hz, within half a bin of the truth.** The tool
returned 2.604 Hz, because `rs_spectrum_best_window()` reports the rank-1 bin of
the single most prominent window and that window was one of the sixteen.

So the ~6-bin bias, the rms of 0.35 Hz and the "does not pass `rs_track_fit()`"
verdict are all artefacts of the selection policy, not of the measurement. The
measurement is right and is being thrown away at the last step.

**This is item 1 of this file, arriving from the other direction.** That item
already argued the case on multiplicity grounds -- "the *fraction* of windows
clearing the floor and its spatial contiguity is far more informative than the
best single one, since a real localised mode should occupy a patch rather than
two scattered windows" -- and proposed exactly this replacement. It was never
implemented. This is the measurement that shows what it costs: the difference
between reporting the injected frequency and reporting a neighbouring bin.

**What to implement.** A consensus statistic beside
`rs_spectrum_best_window()`: for each candidate bin, the number of windows
ranking it first (or within the top few), reported with that count so a caller
can see whether a peak is a consensus or a single window's opinion. Spatial
contiguity of the voting windows is the natural refinement and is what item 1
asks for.

**Before implementing, verify it generalises.** This is one frequency on one
seed. The dumped series for 3.5, 4.0 and 4.5 Hz would settle whether the
plurality lands on the carrier every time or whether 3.0 Hz was lucky -- and the
static scene must be checked too, since a consensus that also produces a
confident plurality on a motionless scene would be worse than what it replaces.

### The consensus statistic has a null behaviour, and it is a good one

The static control was the test that mattered, because a consensus that also
agrees on a motionless scene would be worse than the policy it replaces. It does
not agree:

| | distinct rank-1 winners | plurality | share | runner-up |
|---|---|---|---|---|
| **static, no motion** | **19** of 49 windows | 0.391 Hz | 29% | 12.174 Hz at 27% |
| **injected 3.000 Hz** | **5** | 2.995 Hz | 47% | 2.604 Hz at 33% |

Two things separate them, and the first is the sharper:

- **The number of distinct winners: 19 against 5.** With no signal the windows
  disagree about what the peak is; with signal they concentrate.
- **The plurality's margin.** The static scene's leader beats its runner-up by
  14 to 13 -- a tie, which is what noise looks like. The moving scene's leads
  23 to 16.

So this is not merely a better point estimate. It is a detection statistic with a
measurable null: agreement when there is signal, fragmentation when there is not.
That is the property `rs_spectrum_best_window()` has never had -- a single
window's argmax is equally confident either way, which is why prominence turned
out to be anti-correlated with correctness.

**Report the winner count alongside the frequency.** Nineteen distinct winners
over 49 windows should read as "no consensus" however prominent the leader is.

---

## 8. WITHDRAWN: "the defect is the reference scheme"

Same scene, same configuration, 3.000 Hz injected, the two references compared
by cross-window vote:

| reference | plurality | share | windows voting for the carrier | distinct winners |
|---|---|---|---|---|
| `lag` | 2.995 Hz | 47% | 23 of 49 | 5 |
| **`first`** (default) | 2.995 Hz | **61%** | **30 of 49** | 11 |

**`FIRST` recovers the injected frequency in MORE windows than `LAG` does.** The
reference this file spent item 6 and item 7 indicting is not the binding
constraint, and the mode built to replace it is worse on the measure that
matters.

**What was right.** The decorrelation is real and measured: look 0 against the
rest averages a correlation peak of 0.090 at zero overlap and 0.310 at 0.90,
while adjacent pairs reach 0.913. The sub-look images are correct. The tracker's
series in any ONE window correlates poorly with the truth.

**What was wrong.** Inferring from those that the reference was what stopped the
chain measuring. It does not: with the default reference the carrier is the
top bin in 30 of 49 windows. The chain was recovering the frequency the whole
time, in most of the scene, and `rs_spectrum_best_window()` was reporting one
window's argmax.

**This is item 1 of this file, and it has now been the answer twice.** Item 1
argued on multiplicity grounds that a single window's peak cannot be a
measurement and proposed a consensus statistic. Item 7 arrived at the same place
from the lag experiment. Neither was implemented, and every subsequent
conclusion in items 6 through 7 -- including a new reference mode, now committed
-- was built on top of the unfixed defect.

**The lesson worth keeping.** Three separate estimator "failures" (correlation,
phase, split-band), a reference diagnosis, a coherence analysis, a frequency
floor derivation and a new code path all followed from measurements taken
through a selection policy that cannot express disagreement. The policy should
have been fixed first. It is the cheapest thing in this file and it has been open
the longest.

**`LAG` should not be removed** -- it is documented and harmless, and the
coherence reasoning behind it stands on its own -- but it should not be presented
as a fix for anything until it beats `FIRST` on a consensus measure, which it
currently does not.

*(CORRECTED 2026-08-02: this said "documented, tested and harmless". It is not
tested. `RS_MICROM_REF_LAG` appears in no test in the suite, so the lag branch of
`rs_microm_track()` -- the lag clamp, the `k < lag` skip and the moving reference
extraction -- has never been executed by `ctest`. The measurements in this item
were made through the CLI. `RS_MICROM_EST_SPLITBAND` is in the same position: the
primitive `rs_splitband_shift()` is covered by `test_phaselink.c`, but the
estimator branch that calls it is not. See `docs/CODE-REVIEW.md`.*
*Both branches were given tests immediately afterwards, which found two further
defects: the lag mode reported its phase against look 0 rather than against look
k-lag, so its `--shifts` dump differenced over two different intervals in two
columns, and `rs_splitband_shift()` returned a coherence above 1. Both fixed.)*

---

## 9. Item 1 partly implemented: consensus and contiguity

`rs_spectrum_consensus()` now sits beside `rs_spectrum_best_window()`, returning
the frequency the most windows agree on together with how many agree, how many
distinct answers there are, and **the largest 4-connected block of agreeing
windows**. `mmotion` prints all four and warns on two conditions.

Measured on the same scene with and without motion, 3.000 Hz injected:

| | best_window says | agreement | distinct | largest block |
|---|---|---|---|---|
| moving | 2.995 Hz, prominence 45.5 | 23/49 (47%) | 18 | **15** |
| **static** | **12.565 Hz, prominence 23.1** | 8/49 (16%) | 33 | **3** |

On the motionless scene `rs_spectrum_best_window()` reports 12.565 Hz at
prominence 23.1 with nothing to mark it as noise. The consensus line reports 16%
agreement over 33 distinct answers in a largest block of 3, and both warnings
fire.

**Contiguity is the sharper of the two statistics.** Agreement separates the
cases 47% to 16%; the largest block separates them 15 to 3, and the static case
falls below a bound that comes from the window geometry rather than from tuning
-- overlapping windows put a resolvable target in a 2x2 block at minimum, so a
largest block under four cannot be a spatially resolved mode. That is the bound
`rs_spectrum_best_window()`'s own header already derives for its candidate
count; it applies unchanged here and is not a constant anyone chose.

**What is implemented and what is not.** Item 1 asked for either a multiplicity
correction over the effective number of independent windows, or ranking on the
qualifying fraction and its spatial contiguity. The second is now available as a
statistic a caller can read. Nothing RANKS on it yet: `rs_spectrum_best_window()`
still selects by prominence and is still what the tool reports as its answer, with
the consensus printed beside it. Replacing the selection rather than annotating
it is the remaining half, and it should wait for more evidence than three
detections on one seed.

**The threshold evidence remains thin** and is stated where the thresholds live.
`rs_spectrum_consensus()` applies none. `mmotion` warns below one third
agreement, from measurements putting correct recoveries at 47-61% and everything
wrong at 14-29%, and below a contiguous block of four, which is geometric.

23/23 pass in Release and under ASAN.

---

## 10. Re-scoring the documented results through the consensus gate

Step 3 of the plan in item 9. Every case below is one
`IMPLEMENTATION-VERIFICATION.md` **[local note, not in the repository]**
already records, re-run through the agreement gate.

**The documented false positives mostly become declines:**

| recorded | re-scored | agreement |
|---|---|---|
| 0.90 Hz -> 1.811 Hz, "false second harmonic" | **NO FREQUENCY** | 22% |
| 1.10 Hz -> 2.213 Hz, "false second harmonic" | reports 0.102 Hz | **33%** |
| pair, 0.5 Hz -> 0.100 Hz | **NO FREQUENCY** | 30% |
| pair, 1.0 Hz -> 3.300 Hz | **NO FREQUENCY** | 32% |

**And the documented recoveries survive:** 0.3, 0.5, 0.7 and 0.9 Hz at the
working operating point all report, at 75%, 80%, 67% and 57% agreement, with the
consensus frequency correct in every case.

So the gate keeps what worked and refuses three of four things that did not.

**Two honest limits.**

*The 1.1 Hz case sits exactly on the threshold* -- 3 of 9 windows is one third,
the gate does not fire, and it reports 0.102 Hz against a 1.1 Hz injection. A
case landing precisely on a tuned constant is where that constant is guaranteed
to be arbitrary.

*The margin is thinner than item 9 claimed.* Correct cases run 57-80% and false
positives 11-33%: a gap, but the pair cases clear the threshold by two points
rather than comfortably. A threshold of 0.25 would pass both.

**What this means for the verification document.** Several entries reading "the
chain reports a wrong frequency" are more accurately "the chain reports a
frequency that a consensus read refuses". That is a materially different claim
and the document should distinguish them. It does NOT mean the chain now works:
the frequencies are still not recovered, only the false confidence is removed.

**Not yet re-scored:** the phase estimator sweep, the split-band sweeps, and the
distributed-texture results of item 2. Those were all run before the consensus
existed and all reported single-window answers.

---

## 11. The consensus gate is BLIND to common-mode artefacts

Re-scoring the phase estimator found the case that bounds everything in items 9
and 10. At `test_tracking`'s operating point, isolated point target:

```
inj 0.2 Hz -> 0.407 Hz   9 of 9 windows agree (100%)
inj 0.3 Hz -> 0.407 Hz   100%
inj 0.4 Hz -> 0.407 Hz   100%
inj 0.5 Hz -> 0.407 Hz   100%
inj 0.6 Hz -> 0.407 Hz   100%
inj 0.7 Hz -> 0.407 Hz   100%
STATIC     -> 0.407 Hz   100%
```

**Unanimous agreement on a pure artefact, including on a scene with no motion in
it.** The gate passes every one of these, at the highest confidence the statistic
can express.

**This is structural, not a threshold problem.** Agreement detects noise that is
INDEPENDENT across windows -- correlation's scattered peaks, speckle-driven
static answers, everything item 10 successfully refused. An artefact produced by
the PROCESSING rather than by the scene appears identically in every window, so
the windows agree about it unanimously. No value of the constant helps: 100% is
the ceiling.

**So the gate catches one class of false positive and passes another.** Item 9's
framing -- "a fragmented vote and a motionless scene look alike" -- is true only
for scene-driven noise. For common-mode noise a motionless scene looks like a
perfect detection.

**The only thing that catches this is a null control**, because a common-mode
artefact is by definition identical whether or not the scene moves, so the sole
way to see it is to run the identical processing over a scene known to be
motionless and compare. That is `--null-static`, and it is what `README.md`
already names as the credibility check that matters. Today's work does not
replace it and must not be described as though it does.

**A run needs both.** Agreement for scattered noise, a null control for coherent
noise. Neither substitutes for the other, and the phase estimator is the case
that proves it: item 10's gate refused three of four correlation false positives
and would pass every phase-estimator result ever produced.

**Where this leaves item 9's claim.** The consensus statistic is worth having and
its null behaviour is real for the failure mode it addresses. But it is a partial
check, and the sentence in `rs_spectrum_consensus()`'s header about a fragmented
vote and a motionless scene looking alike needs the qualification that it holds
for scene-driven noise only.

---

## 12. Two things borrowed from existing tools: an ampcor-style cull, and a reader cross-check

Both came from reading how the established offset-tracking implementations do
this -- ISCE's `ampcor`, GMTSAR's `xcorr`, and SARPy for the parse -- rather than
from a new measurement. Neither is a result. They are instruments, and this entry
records what they are and what they are not, so that the next person does not
mistake either for evidence.

### 12a. The selection policy now has a third option, reading the correlator

Items 7-9 leave one open finding: the tracker recovers the injected carrier in
most windows and `rs_spectrum_best_window()` discards it. Both existing policies
-- prominence and consensus -- read only the spectrum. The ampcor family has
always asked a prior question, which this pipeline was computing the answer to
and throwing away: **was this offset determined well enough to be worth
transforming?**

`rs_coreg_shift_q()` now reports two quantities beside the peak value, and
`rs_spectrum_ampcor_window()` culls on them plus a neighbourhood test. Derivations
are in `coreg.h` and `microm.h`; the thresholds are derived rather than tuned
except for one factor of two, which is reported in the output so no result
depends on knowing it.

**What the two new quantities add over `quality`, which is the whole question.**
The peak value says how alike two sub-looks are. It cannot say whether the
surface had one distinguished maximum or a field of comparable ones, and it
cannot say whether that maximum was sharp. `tests/test_coreg.c` pins both
separations against controls where the peak value gives the same answer:

```
one lobe:  peak 1.0000, snr 301.7        <- identical peak value
two lobes: peak 1.0000, snr 103.9        <- the SNR sees the rival lobe
sharp:     peak 1.0000, sigma_az 0.0007 px
broad:     peak 1.0000, sigma_az 0.0029 px
```

**The first measurement, on the documented synthetic operating point.** At the
configuration `USER_GUIDE.md` section 3 uses -- 128 looks, 32-pixel windows,
`--upsample 200`, the clutter-vib fixture at 0.5 Hz -- all 49 windows fail the
SNR gate. Measured SNRs run **5.9 to 8.3 against a noise-alone value of 7.5**.

That is the interesting part. The surfaces are not marginal, they are AT the
value a surface with no signal in it produces. The consensus already refuses this
configuration, at 8% agreement, so the two policies agree on the verdict --
but they reach it from independent evidence, and the cull's version is the
stronger statement: not "the windows disagree" but "there was nothing in the
correlation surfaces to agree about". Whether the same holds at operating points
where the consensus PASSES is unmeasured and is the obvious next question.

**IT IS NOT A NULL CONTROL AND ITEM 11 APPLIES TO IT UNCHANGED.** A common-mode
artefact produced by the processing has a genuine, sharp, well-determined
correlation peak behind it in every window. It passes all three gates. The cull
narrows which windows are believed; it does not decide whether the ground moved,
and describing it as a credibility check would repeat exactly the error item 11
records against the consensus.

**What it does not yet do:** it does not rank; it culls and then takes the mode
of the survivors. Whether ranking the survivors by SNR beats taking their mode is
untested. And no sweep has been run through it -- `rs_track_fit()` over injected
frequencies, pooled over seeds, is the bar and this has not been put to it. The
numbers above are one configuration and one seed. **Nothing here has been shown
to recover a frequency.**

### 12b. The CPHD reader is now checked against SARPy, and agrees

`tools/sarpy_crosscheck.py`, against `info --cphd --json`.

**The hole this fills.** Every test in the suite builds its own fixture, and
`sim_cphd` writes the project's private "RSCH" container rather than a conformant
CPHD -- SARPy refuses to open its output, correctly. So **no test in this
repository has ever exercised the CPHD parse**. A reader that misparsed a real
product consistently would produce no failure anywhere: the pipeline would
measure a different collect from the one on disk and report a confident spectrum
for it. Item 3's Capella SGN override is direct evidence that this is not
hypothetical, and it was found by comparison with another reader -- nothing here
would have found it, and nothing here would notice a second one.

**Result, on the whole Giza collect** -- all 335,141 vectors surviving the
validity screening, which the script replicates rather than trusts:

```
n_pulse 335141, n_rbin 512, fc 9.3 GHz, lambda 0.032235748172 m,
prf 10196.3524146 Hz, dwell 32.8686167733 s, dr 0.249827048333 m,
r_near 762749.526717 m, r_ref_first 762813.482441 m, r_ref_last 762812.628754 m
```

Every field agrees to floating-point round-trip. That covers the PVP field
offsets, the byte order, the SIGNAL-flag and finite-geometry screening, the
bistatic reference-range convention, and the SCSS-to-bin-spacing arithmetic.

**One defect found, in this project, by running it.** `--json` initially printed
`%.12g`, and `lambda_m` and `dr_m` disagreed at 1.3e-12 relative -- the print
format, not the parse. A machine interface has to round-trip, so it is `%.17g`.
Worth recording because the failure looked exactly like a reader disagreement.

**WHAT IT CANNOT SEE: the signal samples.** It compares the geometry and timing
the pipeline reads, not the phase history it reads them for. The SGN convention
affects only sample values, so **the one vendor defect this project has actually
hit is outside what this checks**. Two readers agreeing here does not mean the
image is right. Item 3's suggestion -- compress a pulse both ways and choose the
direction whose energy lands inside the declared `TOA1`/`TOA2` support -- remains
the thing that would test that, and remains unimplemented.

### 12c. The cull swept: two formulation errors found, and a high-precision, low-recall policy left

Item 12a landed `rs_spectrum_ampcor_window()` and said plainly it had not been
put to a sweep. `tests/test_cullsweep.c` is that sweep: six injected frequencies
across three independent clutter realisations, an isolated-point condition, and a
static control, with all three selection policies read off the SAME spectra so
that any difference is attributable to the selection and nothing else.

**It found two errors in the gates as shipped. Both were formulation errors, not
mis-tuning, and neither was visible at the single operating point item 12a
measured.**

**ERROR ONE: gate 2 used an uncalibrated quantity in a calibrated comparison.**
It read `excursion_px >= 3 * sigma_px`. `rs_coreg_quality_t` states in terms that
could not be plainer that sigma is not calibrated in an absolute sense -- the
estimator omits the patch's independent-sample count -- and then the gate
compared it against a real pixel excursion. Measured, on an isolated point target
whose surfaces scored an SNR near 80, ten times the noise-alone value, so gate 1
culled nothing at all:

```
inj   in/snr/sigma/nbr/surv    snr med   sigma med    excursion med
0.3    7/ 0/ 7/ 0/ 0             78.9    154.71 px      10.00 px
0.5    9/ 0/ 9/ 0/ 0             83.2    200.89 px      10.70 px
1.3    7/ 0/ 7/ 0/ 0             70.4    156.53 px      18.30 px
```

A 155-pixel offset uncertainty on a 32-pixel patch is not a number about the
world. The gate removed 100% of windows at every frequency of every run. Replaced
with the relative form the quantity supports and the sources actually use:
sigma above twice the MEDIAN sigma of the entrants, which is ampcor's
median-based rejection applied to the covariance. The excursion is not left
unguarded -- the quantisation floor still tests it, and that test IS calibrated.

**ERROR TWO: gate 3's derivation was applied to a population it was not about.**
The neighbourhood threshold is geometric -- each cell of a 2x2 block has exactly
two in-block 4-neighbours -- but only windows surviving gates 1 and 2 were
allowed to vote. That derivation describes a target's FOOTPRINT, i.e. the whole
block. After gate 1 removes a third to two thirds of the population, the
survivors are too sparse to form blocks, and gate 3 then removed everything gate
2 had not: `20/11/1/8/0`, `19/10/0/9/0`, and so on for every row. Neighbours now
vote if they entered the cull at all, which also makes the vote consistent with
`rs_spectrum_consensus()`, where every gated window is an equal voter.

**THE RESULT AFTER BOTH FIXES.** Pooled over 6 frequencies x 3 seeds on
coherently vibrating clutter, `df` 0.0504 Hz, half a bin 0.0252 Hz:

```
policy      answers  distinct      slope    rms Hz
best             18         6      0.811    0.2360
consensus        18         6      1.080    0.3667
cull              5         2      1.008    0.0035
```

and the static control, three seeds, nothing moving:

```
best        3.024 Hz    1.462 Hz    -- consensus
0.403 Hz    0.353 Hz
1.613 Hz    0.403 Hz     cull: refused at all three seeds
```

**The cull's profile is the opposite of the other two policies'.** Over both
fixtures it answered 7 times from 24 offered and every one of the 7 was within
half a bin of the injection. It refused all three static scenes, where `best` and
`consensus` each emitted a confident frequency. `best` and `consensus` answer
everywhere and miss by ten times the bar.

**THIS IS NOT A RECOVERY, AND THE REASON IS THE `distinct` COLUMN.** The five
clutter answers sit at two injected frequencies, both at the bottom of the band.
`rs_track_fit()` exists to separate a chain that follows the injection from one
emitting a fixed value, and it does that by requiring the reported frequency to
track across a SWEPT range; a slope through two abscissae fits perfectly whatever
produced it. So the numbers in that row -- slope 1.008, rms 0.0035 Hz, nominally
inside the bar -- do not mean what a row of the same shape would mean at six
distinct injections. A policy that answers only where the answer is easy scores
well and has demonstrated nothing. `test_cullsweep.c` ASSERTS the limitation
(`RS_CHECK(dk < n_freq)`) so that the day recall improves the test fails and
forces this paragraph to be rewritten rather than quietly outgrown.

**What it does assert, and these are real:** every answer the cull gave was
correct, and it refused every static control. One wrong answer fails the suite.

**The open question is now recall, not correctness.** Gate 1 is doing most of the
removing on clutter -- median SNRs of 8 to 15 against a gate of 15, so the
population is halved before the neighbourhood test sees it -- and whether that
gate's factor of two is right at these coherences is unmeasured. That is the next
thing to sweep, and it is a different experiment from this one.

**Item 11 is untouched by all of this.** The static control here is scene-driven.
A common-mode artefact still passes every gate the cull applies, and only
`--null-static` catches it.

### 12d. The SNR gate factor swept: it is load-bearing, and 2.0 sits on a plateau

Item 12c closed with recall as the open question and named gate 1 as the
suspect: it removes a third to two thirds of the population on clutter, and its
factor of two had never been measured. Swept in `tests/test_cullsweep.c`, over
the SAME spectra at every factor so that the threshold is the only thing varying
-- re-running the chain per factor would confound the threshold with the
realisation and cost eight times as much for a worse answer.

`rs_spectrum_ampcor_window_opts()` was added to make this possible at all. A
tuned constant compiled into a selection policy is a claim nobody can check
without editing the source, which is most of why this one went unexamined
through two commits.

```
factor  gate    clutter (18 pts)          isolated (6)     static (3)
        x null  ans corr dist    rms      ans corr dist    answered
 0.00    0.0     8    7    4   0.7338      2    2    2      1  DISQUALIFIED
 1.00    7.5     7    6    4   0.7844      2    2    2      1  DISQUALIFIED
 1.25    9.4     6    5    3   0.8473      2    2    2      0
 1.50   11.3     6    5    3   0.8473      2    2    2      0
 1.75   13.1     5    5    2   0.0035      2    2    2      0
 2.00   15.0     5    5    2   0.0035      2    2    2      0
 2.50   18.8     5    5    2   0.0035      2    2    2      0
 3.00   22.5     5    5    2   0.0035      2    2    2      0
```

**THE GATE IS LOAD-BEARING, WHICH WAS NOT KNOWN.** Disabled, and set exactly AT
the noise-alone value, the cull answers on a scene where nothing moves. That is
the false positive the entire policy exists to avoid, and it is the first direct
evidence that gate 1 does anything a null control would care about. It appears
the moment the factor drops to the null and not before, so the boundary is
measured and it falls where the derivation put it. `test_cullsweep.c` asserts
both halves: every factor above the null refuses all three static scenes, and at
least one factor at or below it does not.

**AND 2.0 IS ON A PLATEAU, NOT AN EDGE.** Every factor from 1.75 to 3.0 gives
identical counts on both fixtures. So the incumbent costs nothing and buys
nothing against its neighbours above, which is the only honest reason to leave a
tuned constant alone. A constant that had to sit exactly where it sits would be
fitted to this fixture; this one does not. Asserted, so that a future change
which moves the plateau fails rather than passing quietly.

**The trade below 1.75 is real and is not taken.** Factors of 1.25 and 1.5 buy
one more answer and one more distinct injection -- three rather than two, which
matters, since two abscissae cannot support a slope -- at the price of one WRONG
answer, taking the rms from 0.0035 Hz to 0.85. Precision is the only thing this
policy currently has that the other two do not, and trading it for a third point
of coverage would leave it with neither. Anyone who wants the trade can have it
through `rs_spectrum_ampcor_window_opts()`.

**WHAT THIS DOES NOT SETTLE, and it is the same thing item 12c left open.**
Recall is still 5 of 18 on clutter over two distinct injections, and the sweep
shows gate 1 is not what is holding it there: from 1.75 upwards the factor makes
no difference at all, so the windows being lost are being lost elsewhere.
Attention should move to gate 3 and to the shared coherence gate, not to this
constant. Note also that the isolated-point column is 2/2/2 at EVERY factor
including zero -- surfaces there score an SNR near 80, so gate 1 is irrelevant on
that fixture and its recall of 2 of 6 is entirely someone else's doing.

**One measurement to be careful with.** Three seeds and one fixture family. The
static false positive at factors <= 1.0 is a single occurrence out of three
seeds, so its existence is established and its rate is not.

### 12e. Gate 3 swept: the derived value is exactly the boundary, and it costs the recall

Item 12d showed gate 1 is not what holds the cull at 5 answers of 18 -- above a
factor of 1.75 it changes nothing -- and pointed at gate 3 and the coherence gate
instead. Both are answered here, and they need different answers.

**GATE 3, SWEPT OVER THE SAME SPECTRA.** Its threshold is derived rather than
tuned: two is the in-block 4-neighbour count of a 2x2 block, the smallest
footprint a resolvable target can occupy given that windows overlap at the
tracking stride.

```
min nbrs   clutter (18 pts)        isolated (6)   static (3)
           ans  corr  distinct     ans  corr      answered
    0       18   12     6            6    3         3   DISQUALIFIED
    1       15   11     6            4    2         1   DISQUALIFIED
    2        5    5     2            2    2         0
    3        0    0     0            0    0         0
    4        0    0     0            0    0         0
```

**The derivation predicted the boundary and the measurement lands on it.** Below
two, a scene with nothing moving gets an answer -- at zero it gets one at every
seed. Above two, nothing gets an answer at all. Two is the only value that both
refuses every static control and answers anything. That is a stronger result than
gate 1's, where 2.0 merely sits on a plateau: here the constant is pinned from
both sides, and by an argument made before the data.

**AND IT IS WHAT HOLDS THE RECALL DOWN.** Disabled, the cull answers all 18
clutter points, 12 of them correct, across all six distinct injections. So the
coverage a meaningful fit needs is PRESENT IN THE TRACKING and is being discarded
by the selection -- which is item 7-9's finding again, one level up: the cull was
built to fix a selection policy discarding real recoveries, and its own strictest
gate now discards them too.

**But the coverage exists only together with false positives.** Every setting
that reaches six distinct injections also answers on a motionless scene. There is
no threshold that gives both. That is a statement about the operating point, not
about this constant: at these coherences the chain cannot simultaneously answer
across the band and refuse a null, and no value of any of the three gates changes
that. Item 12c's "recall is the open question" is therefore answered and the
answer is that recall is not separately obtainable here.

**THE COHERENCE GATE CANNOT BE SWEPT ON THIS FIXTURE FAMILY, MEASURED.** The
default is 0.4 and the published campaigns work near 0.85, measured between
95-percent-overlapped looks on the Giza collect. What this fixture reaches:

```
overlap   coherence min / median / max
  0.00      0.059 / 0.075 / 0.135
  0.50      0.063 / 0.100 / 0.186
  0.90      0.143 / 0.194 / 0.265
  0.95      0.196 / 0.244 / 0.323
```

Coherence rises with overlap exactly as it should -- the mechanism is right, and
zero overlap decorrelating totally is correct behaviour rather than a fixture
defect, since look 0 and look 127 then share no pulses and a `first` reference
compares images a full aperture apart. But the ceiling is 0.323 even at 95
percent overlap, below the gate's own default and far below the regime the gate
exists to discriminate within. **Sweeping the coherence gate here would measure
the fixture's ceiling and report it as a property of the gate**, which is item
12c's error in a different costume. Asserted as a limitation in
`test_cullsweep.c` so that a fixture which can reach 0.4 makes the test fail.

**WHAT A SECOND FIXTURE FAMILY HAS TO DO, now specified rather than guessed.** It
must reach coherence of order 0.85 at high overlap. Overlap alone does not get
there, so it is a scene-content change and not a parameter: 96 ideal point
scatterers over 24 m at 8 m sub-look resolution give roughly ten per cell, whose
relative phases decorrelate with aspect far faster than a real cell's effective
thousands. Candidates, in increasing cost: many more scatterers per resolution
cell; a persistent dominant scatterer per cell, which is what a built structure
actually presents; or a physical sub-resolution model. Until one exists, every
number in items 12a to 12e rests on one fixture family and the coherence gate's
default has never been tested by anything.

### 12f. The second fixture family is built, and it DISPROVES item 12e's specification

Item 12e specified what a second fixture family had to do -- reach coherence of
order 0.85 at high overlap -- and said it was "a scene-content change, not a
parameter", naming a persistent dominant scatterer per cell as the candidate.
`rs_sim_dominant_patch()` implements exactly that: an 8x8 lattice of dominant
scatterers, jittered within their cells, over 256 diffuse Rayleigh scatterers
whose per-cell power is 1/`dominance` of a dominant's.

**The specification was wrong, and the fixture is what proves it.** Coherence
does not move with dominance at any overlap:

```
overlap   dominance    predicted    measured min / med / max
  0.00        0          0.000       0.054 / 0.075 / 0.113
  0.00        1          0.500       0.054 / 0.074 / 0.121
  0.00        6          0.857       0.055 / 0.073 / 0.113
  0.00       30          0.968       0.056 / 0.072 / 0.108
  0.90        0..30      0.0..0.97   0.145 / 0.19  / 0.31
  0.98        0..30      0.0..0.97   0.267 / 0.33  / 0.49
```

Dominance moves the median coherence by **0.022** across a range of 0 to 30.
Overlap moves it by **0.267** over the same runs -- twelve times more. The
prediction is wrong by up to a factor of thirteen.

**WHY, AND IT IS THE USEFUL PART.** The model `gamma ~ A^2/(A^2 + S^2)` describes
decorrelation caused by SPECKLE: the changing interference of comparable
scatterers within one resolution cell as aspect sweeps. `rs_sim_scene()` has no
speckle to suppress. Every scatterer is an ideal point carrying analytically
exact propagation phase, so a scene of 320 of them is exactly as DETERMINISTIC as
a scene of one, and two sub-looks of it differ only in which aspects were used to
form them. A dominant scatterer suppresses a random component that was never
there.

So the coherence this simulator reports is a property of the SUB-LOOK SEPARATION
and of nothing else a fixture can vary. That is sharper than
`test_tracking.c`'s standing note that the generator has no sub-resolution
scatterer model: it means **no fixture built on `rs_sim_scene()` can exercise the
coherence gate at all**, because the gate's input is invariant to everything such
a fixture can change. Item 12e's remaining candidates -- more scatterers per
cell, a physical sub-resolution model -- are not equivalent: the first is
disproven here along with dominance, and only the second could work, because only
it introduces the random aspect-dependent component the whole mechanism needs.

**What a third attempt would have to change.** `rs_sim_scene()` itself, not the
target list. It needs a per-scatterer phase that varies randomly with aspect over
a finite correlation angle -- which is what a real resolution cell's unresolved
sub-structure produces -- so that coherence falls off with aspect separation for
a physical reason rather than a processing one. That is a change to the
propagation model and it will invalidate the coherence figures of every existing
test, which is the honest cost of having one.

**THE FIXTURE IS KEPT ANYWAY, on different grounds than it was built for.** Run
through the same sweep as the clutter family, 6 frequencies x 3 seeds:

```
                clutter fixture              dominant fixture
policy      ans  dist  slope    rms      ans  dist  slope    rms
best         18    6   0.811   0.2360     18    6   0.802   0.8248
consensus    18    6   1.080   0.3667     18    6   1.596   0.6077
cull          5    2   1.008   0.0035      3    2   1.008   0.0052
static answers by the cull: 0 of 3          0 of 3
```

The structured scene is HARDER for the two spectrum-only policies -- `best`'s rms
triples -- and the cull's profile is unchanged: it answers less, every answer
stays inside half a bin, and it refuses every static control. That is the first
evidence that the cull's precision and null behaviour are not artefacts of the
fixture they were measured on, which is what items 12d and 12e both flagged as
the standing weakness. It is now two fixture families, and still one scene
generator.

**Cost.** `test_cullsweep` runs about two minutes, most of `ctest`'s wall time.
Deliberate: the alternative is one operating point on one seed, which is what
items 12a and 12b did and which missed two formulation errors this file found in
a single run.

---

## 13. Overlap buys nothing for the correlation estimator, and zero is optimal for a reason

The published campaigns form sub-apertures at roughly 99 percent overlap with
thousands of looks; `rs_microm_estimator_t` records that, and records that this
project has only ever run tens to 128 looks at zero overlap. That gap was the
obvious next thing to close. It is now measured, and it should not be closed --
not with this observable.

**Method.** Two exploratory probes, run outside `ctest` because backprojection
work scales as `n_looks * t_sap * prf * n_cells` and at 98 percent overlap that
is some 40 times the incumbent cost -- 163 s for a single 1024-look run against
5 s for the incumbent. Stage one measured geometry, coherence and one injected
frequency across seven `(n_looks, overlap)` settings; stage two swept six
frequencies at three of them. Vibrating-clutter fixture, seed 7 throughout.

**STAGE ONE, at 0.5 Hz.** `resp` is `rs_spectrum_subaperture_response()`:

```
looks  overlap   t_sap    f_max   resp   coherence med   reported
  128    0.00   0.1550     3.23  0.990       0.075       0.504  ok
  256    0.90   0.7525     6.67  0.782       0.152       0.521  ok
  256    0.98   3.2775     7.69  0.176       0.264       0.120  WRONG
  512    0.95   0.7525    13.33  0.782       0.153       0.521  ok
  512    0.98   1.7825    14.29  0.120       0.214       0.112  WRONG
 1024    0.98   0.9300    28.57  0.679       0.161       0.502  ok
 1024    0.99   1.7800    28.57  0.121       0.215       0.112  WRONG
```

Seven for seven on a prediction made from the response alone before the runs.
Note also that coherence rises monotonically with `t_sap` and with nothing else,
which is the same finding as item 12f from the other direction.

**STAGE TWO, six frequencies at three settings.** Pooling every point of both
stages, 25 in all: **every point whose sub-aperture response was 0.608 or better
recovered the injection, and every point at 0.481 or worse failed.** No
exceptions, and a clean gap between 0.48 and 0.61.

```
                  0.3    0.5    0.7    0.9    1.1    1.3   slope    rms
 128/0.00  resp  0.996  0.990  0.981  0.968  0.953  0.935
           best  0.302  0.504  0.706  0.907  1.058  1.260  0.950  0.0239
1024/0.98  resp  0.877  0.680  0.435  0.186  0.022  0.161
           best  0.279  0.502  0.391  0.391  0.391  0.223 -0.088  0.5800
2048/0.98  resp  0.967  0.910  0.828  0.725  0.608  0.481
           best  0.326  0.521  0.716  0.911  1.107  0.391  0.326  0.3716
```

**THE ARITHMETIC THAT GENERALISES IT.** Two ceilings bound the vibration
frequency a stack can carry, and they move differently. Nyquist on the sub-look
series is `f_N = 1/(2*t_sap*(1-overlap))`. The response reaches one half at an
observation ratio of 0.6034, so the response ceiling is `f_R = 0.6034/t_sap`.
Their ratio depends on neither the look count nor the dwell:

```
f_N / f_R  =  0.829 / (1 - overlap)
```

At zero overlap that is **0.83** -- the two ceilings coincide to within twenty
percent, with sampling marginally the tighter. Every overlap above zero raises
`f_N` by `1/(1-overlap)` and leaves `f_R` exactly where it was. At 0.98 the
sampling headroom is 41 times the usable band: the extra rate is spent entirely
on frequencies the sub-aperture has already averaged away.

So **zero overlap is not a legacy setting, it is the balanced one**, and the
2048-look run makes the point concretely: 52 Hz of Nyquist band, of which 1.27 Hz
is usable, against the incumbent's 3.20 Hz of band with 3.86 Hz usable. Forty
times the compute for a narrower measurement. The relation is asserted in
`test_cullsweep.c`, where it costs nothing to check.

**THIS DOES NOT CONTRADICT THE PUBLISHED WORK, AND THE RECONCILIATION IS THE
USEFUL PART.** `rs_spectrum_observation_ratio()` already records two
accelerometer-confirmed recoveries at observation ratios of 18.0 and 36.3 --
seventy times what the sinc picture would permit -- and notes that 18.0 is an
exact integer, which is where the sinc nulls. Those campaigns read the PHASE of
each pixel in each sub-aperture. This one defaults to correlation-based offset
tracking. The sinc attenuates the tracked DISPLACEMENT, which is exactly and only
what a correlator measures; a phase or micro-Doppler observable reads the
sidebands the averaging moves energy into rather than the averaged position. The
ceiling derived here is therefore **the correlation estimator's, not the
method's**.

**WHAT THAT IMPLIES FOR THE NEXT STEP, and it is a redirection.** High overlap is
the regime the validated literature works in, and this project cannot enter it
through the correlation estimator -- not because the configuration is untried but
because the observable forbids it. Entering it means making the PHASE estimator
work, which `test_tracking.c` records as returning a fixed 0.407 Hz for every
injection. That is the door to the literature's operating point, and it is shut
for a reason that is now understood rather than merely observed.

**Caveats, and they matter.** One seed for the whole of this, on one fixture
family; item 12f's warning applies unchanged. The 0.5 response threshold is where
the measured gap falls, not a derived constant -- the derivation gives the shape
of the ceiling, not its height. And the incumbent's own sweep at seed 7 scores
slope 0.950 and rms 0.0239, inside the bar, where the same sweep pooled over
three seeds in `test_cullsweep.c` gives 0.811 and 0.2360: single-seed numbers
overstate, including the ones in this entry.

---

## 14. The phase estimator was wrapping a ramp it should have removed. It now recovers.

Item 13 closed by naming this the door to the literature's operating point and
noting it was shut: the phase estimator returned a fixed 0.407 Hz for every
injection from 0.2 to 0.7 Hz, and the same 0.407 Hz at higher prominence for a
scene with no motion in it. Twelve operating points had been scanned. It was the
project's most-investigated negative.

**THE DEFECT.** A scatterer sitting anywhere but exactly at its pixel's centre
has a range to the platform that changes linearly as the aperture sweeps, so its
phase in sub-look k is linear in k, at a rate `(4*pi/lambda) * dX * dx / R` --
platform travel per look, times the offset from the pixel centre, over the slant
range. Measured on the isolated-point fixture: **1.1 to 1.9 radians per look**,
which is 23 to 39 full cycles across a 128-look stack, on a scene where nothing
moves.

Wrapping that ramp into (-pi, pi] makes a **sawtooth**, and a sawtooth has a
strong line at its own repetition rate. That rate is set by the target's
sub-pixel offset and by the geometry -- so it does not move when the scene does,
which is exactly the behaviour recorded: fixed across injections, present on
static scenes, and "the artefact's value moves with the configuration; it does
not move with the scene". This is also the common-mode artefact item 11 uses to
show that the consensus gate is structurally blind. Item 11's conclusion is
unaffected; its example now has a cause.

Detrending the displacement series cannot undo it. By then the wrap has happened
and a sawtooth is not a trend. The removal has to happen on the phasors, before
any angle is taken.

**THE FIX, AND THE ESTIMATOR THAT ALMOST WORKED.** The carrier is removed by
de-ramping each pixel's phasor series at the frequency that maximises
`|sum_k z[k] * exp(-i*nu*k)|` -- the maximum-likelihood frequency of a phasor in
noise, computed with no unwrapping anywhere. For a phase modulated by a zero-mean
vibration this returns the carrier and leaves the modulation, which is the split
wanted: the carrier is geometry, the modulation is the target.

The obvious one-line estimator is not good enough, and the margin is instructive.
The mean lag-one product `arg(sum_k z[k+1]*conj(z[k]))` carries a bias of order
beta^2 from the modulation itself: it returned -0.694 rad/look against a true
-0.760 at 64 looks, and -1.879 against -1.909 at 128. Those look like small
errors. They are not, because the residual ramp is the error times the LOOK
COUNT -- 4.2 radians over 64 looks, which wraps, which puts the sawtooth straight
back. With lag-one the chain recovered 0.2, 0.3 and 0.4 Hz and failed at 0.5, 0.6
and 0.7. The requirement is error << pi/N, and only a proper peak search meets
it.

**THE RESULT.** Isolated point target, 64 looks at 0.5 overlap, six injections
plus a static control:

```
injected  0.20   0.30   0.40   0.50   0.60   0.70   STATIC
reported  0.203  0.305  0.407  0.508  0.610  0.711  0.051
slope +1.016, rms 0.0078 Hz against a half-bin bound of 0.0254 Hz
```

Coherently vibrating clutter, 128 looks at zero overlap, six injections at each
of three seeds:

```
seed    7   slope 1.008   rms 0.0070 Hz     static control 3.024 Hz
seed   23   slope 1.008   rms 0.0070 Hz     static control 2.823 Hz
seed  101   slope 1.008   rms 0.0070 Hz     static control 1.462 Hz
per-frequency: 0.302 0.504 0.706 0.907 1.109 1.310 against 0.3 0.5 0.7 0.9 1.1 1.3
```

Every static control lands outside the swept band, and at a different frequency
per seed. **This meets the bar in README.md** -- slope near one, rms under half a
bin, pooled over independent clutter realisations, with a static control through
identical processing -- and it is the first thing in this project that has.

**WHAT IT IS NOT, AND THESE ARE NOT SMALL.**

*It is synthetic.* `rs_sim_scene()` gives every scatterer analytically exact
phase. Item 12f established that coherence in this simulator is a property of
sub-look separation alone and invariant to scene content, which means the
sub-look decorrelation a real collect imposes -- the very thing that destroys a
phase series, and the reason the temporal unwrap was removed -- is absent by
construction. A phase estimator is exactly the observable most exposed to that
gap. **No real collect has been run through this.**

*It is amplitude-bounded.* The sweeps inject 2.442 mm, a 0.81 radian swing. At
the 20 mm the correlation fixtures use the swing is 6.6 radians and the estimator
fails completely -- correctly, since phase wraps beyond lambda/4 of
line-of-sight motion. Any comparison that puts this estimator on a correlation
fixture measures the wrap.

*It fails on the dominant-scatterer fixture.* The same sweep on
`rs_sim_dominant_patch()` at 320 scatterers gives slopes of -0.324, -1.375 and
-2.477. That fixture was built in item 12f to supply persistent bright
scatterers, which is the precondition this estimator's own header names, so the
failure is the opposite of what was expected and is unexplained. Scatterer
density is the obvious suspect -- 320 against the clutter fixture's 96 -- and it
is untested.

*Only one selection policy recovers.* `best` (prominence) tracks; the consensus
and the cull do not, returning slopes of -0.338 on the same spectra. After three
commits arguing prominence is the weakest policy, it is the one that works here.
Unexplained, and worth a measurement of its own.

*One configuration.* 128 looks at zero overlap, and 64 at 0.5 for the isolated
point. No sweep over look count or overlap.

**WHERE THIS LEAVES THE PROJECT.** The sentence "nothing here has been shown to
recover a vibration frequency it was not told" is no longer true of the phase
estimator on synthetic data. It remains true of the correlation estimator, and it
remains true of everything on real data. The next thing is a real collect, and
the caveats above say what to expect and what to check first.

---

## 15. Both of item 14's anomalies were one mistake, and the real-data configuration exists

Item 14 recovered a frequency for the first time and left four caveats. Two were
open questions rather than limitations: the estimator failed on the
dominant-scatterer fixture built expressly to satisfy its stated precondition,
and only the prominence policy recovered where consensus and the cull did not.
They are the same mistake, and it was mine, in the fixture rather than the
estimator.

**SEPARATING DENSITY FROM DOMINANCE.** Uniform Rayleigh clutter, zero overlap,
seed 7, phase estimator, `best` policy:

```
scatterers   per resolution cell   slope    rms Hz
     64             0.92          -2.016   1.5162   no
     96             1.38          +1.008   0.0070   YES
    128             1.84          +1.008   0.0070   YES
    200             2.87          -1.959   1.4871   no
    320             4.59          -1.887   1.4351   no
    640             9.18          -1.346   1.1614   no
```

and with EQUAL amplitudes instead of Rayleigh, at 64, 96 and 128, it never
recovers. So the amplitude tail matters: with Rayleigh draws some cells get a
scatterer clearly brighter than its neighbours, and that is what the estimator
needs. Dominance as `rs_sim_dominant_patch()` parameterises it does not rescue a
dense scene -- raising it from 6 to 100 to 1000 changes nothing.

**THE MISTAKE.** `rs_sim_dominant_patch()`'s `dominance` controls
dominant-against-DIFFUSE. It says nothing about dominant-against-DOMINANT, and
only the lattice spacing does. Item 12f used 8x8 over 24 m, a 3 m spacing against
an 8.26 m sub-look azimuth cell -- **2.75 equal dominants in every resolution
cell**. Three equal scatterers in one cell is not a dominant scatterer. The
fixture built to supply the precondition violated it.

Spacing it from the resolution instead, with 128 diffuse behind it:

```
n_side  spacing  dominants/cell   best            consensus       cull
   2     12.0 m       0.69       +1.008/0.0070   +1.008/0.0070   +1.008/0.0070
   3      8.0 m       1.03       +1.008/0.0070   +1.008/0.0070   +1.008/0.0070
   4      6.0 m       1.38       +1.008/0.0070   +1.008/0.0070   +1.008/0.0070
   8      3.0 m       2.75        FAIL            FAIL            FAIL
```

**Both anomalies dissolve together.** On a fixture that actually has one dominant
per cell, all three selection policies recover -- so item 14's "only prominence
works" was a symptom of the marginal 96-scatterer fixture, not a property of the
policies. Across three seeds at 3x3, consensus and cull recover at 3 of 3 and
prominence at 2 of 3, which reverses the ranking again and vindicates the cull
work of items 12a-12e. Static controls land at 2.0-3.2 Hz throughout, outside the
0.3-1.3 Hz swept band.

**AND THE REAL-DATA CONFIGURATION EXISTS.** This was the reason for doing the
sweep. `rs_microm_estimator_t` records sub-look coherence on a real X-band
collect as very nearly the fraction of pulses two looks share -- 0.85 at 95
percent overlap, 0.07 at zero. Item 14's recovery ran at ZERO overlap, the worst
possible real setting, so it said nothing about whether a real run could be
configured at all. On the corrected fixture:

```
overlap  t_sap    quality   best            consensus       cull
  0.00   0.155     0.748   +1.008/0.0070   +1.008/0.0070   +1.008/0.0070
  0.50   0.310     0.887   +1.008/0.0070   +1.008/0.0070   +1.008/0.0070
  0.90   1.458     0.902   +0.978/0.0162   +0.978/0.0162   +0.978/0.0162
  0.95   2.720     0.949   +0.984/0.0181   +1.455/0.4491   +1.381/0.3783
```

Recovery holds to 95 percent overlap on the prominence policy, at rms well inside
the half-bin bound throughout, while the quality metric climbs to 0.949. **The
regime a real collect needs for coherence is a regime in which this estimator
still works.**

**IT ALSO CONFIRMS ITEM 13'S RECONCILIATION BY MEASUREMENT RATHER THAN ARGUMENT.**
Item 13 established that overlap buys nothing for the CORRELATION estimator,
because the sub-aperture response ceiling binds first and recovery there requires
a response above about 0.5; it argued the ceiling was the correlator's rather
than the method's, on the grounds that phase reads sidebands where correlation
reads an averaged position. At 0.90 overlap here `t_sap` is 1.458 s, so a 1.3 Hz
injection sits at a sub-aperture response of **0.055** -- a tenth of what the
correlator needs -- and phase recovers it at rms 0.0164 Hz. The argument was
right, and it is now a measurement.

**WHAT IS STILL OPEN.**

*Still synthetic.* Nothing here changes item 12f: `rs_sim_scene()` gives every
scatterer analytically exact phase, so real sub-look decorrelation is absent and
a phase observable is what it would hurt most. The quality figures above are
amplitude stability, not interferometric coherence, and must not be read as the
0.85 the real collect measures.

*Grid-fragile at zero overlap.* The 3x3 result holds on a 96-cell grid and does
NOT survive cropping to 64 cells, which leaves 9 windows instead of 25. The
high-overlap case is not fragile that way. Unexplained, and asserted at the grid
it was measured on so that the fragility cannot be forgotten.

*The working density band is narrow* -- 96 to 128 uniform scatterers, or a
lattice at one dominant per cell. Real clutter will not be tuned to it, which is
precisely what the real collect will test.

The next thing is the Giza collect, at high overlap, with `--estimator phase`,
`--null-static` beside it, and this table as the read-out checklist.

---

## 16. `validate` is estimator-blind, and says FAIL for configurations the phase estimator handles

Running the Giza collect through `validate --frequency 1.0 --overlap 0.90
--amplitude 2.0` returns **VERDICT: FAIL** on four checks. Three of them do not
apply to `--estimator phase`, and the one that does apply passes.

```
FAIL  observable band   ... the band reaches 0.304 Hz; the 0.1643 s step would
                        suggest 3.042 Hz, which overlap does not buy. The target
                        is ABOVE the band: past the sub-aperture's own averaging
                        response, where a reported peak cannot be signal.
FAIL  sensitivity       the floor measured at 0.4 m cells is 7.0 px p2p ...
FAIL  ambiguity         ... a wrap ceiling of 1.5 px p2p against a 7.0 px
                        artefact floor.
PASS  phase floor       at the 0.40 mask over 994 independent samples the CRLB is
                        0.051 rad, a line-of-sight noise of 0.1318 mm per look.
```

**The three failures are the correlation tracker's limits, in its units.**
`sensitivity` and `ambiguity` are stated in TRACKING PIXELS -- a 7.0 px artefact
floor, a 1.5 px wrap ceiling -- which are properties of a correlation surface and
have no meaning for an estimator that reads pixel phase. `observable band` is the
sub-aperture response ceiling, and item 13 established that ceiling is the
correlator's: item 15 then measured phase recovering an injection at a response
of **0.055**, a tenth of what correlation needs, so "past the averaging response,
where a reported peak cannot be signal" is false for this estimator.

Meanwhile `phase floor` -- the one check written for this observable -- passes
with a line-of-sight noise of 0.13 mm per look against the 2 mm asked for.

**This inverts the command's whole purpose.** `validate` exists because a wrong
setting does not fail loudly, so it warns before the expensive processing. Here it
does the opposite: it produces a confident refusal of a configuration that the
measurements say is the right one, and a user following the documented order
(`info` -> `validate` -> `focus` -> `mmotion`) would stop. A false alarm in a tool
whose value is its authority is worse than a missing check.

**The fix is not to relax the thresholds.** `rs_validate_req_t` carries no
estimator field, so the checks cannot know which observable they are judging. It
needs one, and then:

- `RS_VALIDATE_SENSITIVITY` and `RS_VALIDATE_AMBIGUITY` should report
  RS_V_UNKNOWN, not FAIL, for the phase estimator -- they measure a quantity that
  estimator does not produce, and `RS_V_UNKNOWN` exists in the enum for exactly
  this distinction.
- `RS_VALIDATE_BAND` should use the sampling ceiling for phase and the response
  ceiling for correlation, per item 13's arithmetic, rather than one rule for
  both.
- `RS_VALIDATE_PHASE_FLOOR` should be the sensitivity check for phase, and is
  already computed.

### Item 16, RESOLVED: `validate` takes an estimator

`rs_validate_req_t` now carries one, `validate --estimator correlation|phase|
splitband` sets it, and it defaults to correlation so that every result recorded
before this change is reproduced exactly. On the Giza configuration of item 17:

```
                correlation                       phase
observable band FAIL, averaging ceiling 0.304 Hz  PASS, sampling ceiling 3.042 Hz
sensitivity     FAIL, 6.874 mm at a 7 px floor    UNKNOWN, defers to phase floor
ambiguity       FAIL, ceiling below the floor     PASS, 2.00 mm is 0.19x the fold
phase floor     PASS, "bounds only its phase refinement"
                                                  PASS, "THE sensitivity bound"
VERDICT         FAIL                              WARN
```

The remaining WARN is the observation ratio, which is a genuine warning at eta
1.64 and is estimator-independent.

**The ambiguity check reports rather than declines, and that was the interesting
decision.** Returning UNKNOWN for the phase route would have been the easy fix and
the wrong one: that route HAS an ambiguity, the lambda/4 line-of-sight fold, and
it is far tighter than the pixel wrap. At Giza's 39.5 degree incidence it caps a
vertical amplitude at 10.4 mm; the 20 mm the correlation fixtures inject is 1.9x
that and now FAILS with the reason, which is exactly the mistake item 14's caveats
warn about and could not previously be caught in advance.

**Sensitivity returns UNKNOWN, not PASS.** The enum has three outcomes for a
reason -- `validate.h` says UNKNOWN "is a different thing from a pass and is
reported separately so it cannot be mistaken for one" -- and a check that cannot
answer must not look like one that answered favourably. `WORST()` promotes only
on FAIL and WARN, so an UNKNOWN never flatters a verdict.

Three cases in `test_validate.c` pin it: the two estimators judged differently on
one configuration with the correlation verdict asserted unchanged, the lambda/4
fold passing at 2 mm and failing at 20, and the default staying correlation.

---

## 17. Giza with the repaired phase estimator at 90% overlap: a null, and the artefact is gone

The first real-data run of the estimator repaired in items 14-15, at the high
overlap item 15 showed it tolerates and real sub-look coherence requires.
`runs/giza/2026-08-01-phase-highoverlap/RUN.md` has the full provenance.

```
./build/micromotion mmotion --cphd "$C" --at 29.979175,31.134186 \
    --estimator phase --n 128 --overlap 0.90 \
    --size 256 --cell 1.0 --win 32 --rbins 4096 --coherence 0
```

44 min 55 s, 442% mean CPU, 225 windows over a 256 m patch centred on Khufu.
128 looks at 0.90 overlap give t_sap 2.40 s, dt 0.2399 s, a 2.08 Hz band and
0.70 m sub-look resolution.

```
NO FREQUENCY REPORTED: only 27 of 170 windows agree (16%), which is what a
  MOTIONLESS scene produces.
  strongest window 55: 0.033 Hz, prominence 24.3, quality 0.483
  consensus 0.195 Hz, 12 distinct answers, largest contiguous block 9
  cull     0.065 Hz from 18 of 170 surviving, 105 removed on neighbours
  the two disagree, and the tool says so
```

**THE ARTEFACT IS GONE, AND THIS IS THE RESULT WORTH HAVING.** Item 11 recorded
the old phase estimator returning ONE fixed frequency at **100 percent** window
agreement -- on moving scenes and on motionless ones alike, at a prominence
higher than any real case. That was the sawtooth of item 14's diagnosis. Here the
225 windows spread across 12 distinct bins and the modal answer takes 16 percent:

```
0.163 Hz  35 windows      0.065 Hz  28
0.195 Hz  33              0.130 Hz  26
0.098 Hz  30              0.228 Hz  25
```

A flat histogram over the low bins is noise. A line at 100 percent agreement is
an artefact. The carrier removal was built and validated in simulation; this is
the first evidence it holds on a real collect, where the sub-pixel offsets,
geometry and decorrelation are all things the simulator does not produce.

**And 16 percent is the project's own motionless signature.** Item 9 calibrated
correct recoveries at 47-80 percent agreement and motionless scenes at 11-16.
This lands at the top of the motionless band, and the tool refused, which is the
behaviour every gate in the selection stage exists to produce.

**WHAT THIS DOES NOT SHOW.** Nothing in this scene is known to move.
`RS_VALIDATE_GROUND_TRUTH` reports unknown and always will; there is no
accelerometer at Giza and `DATASETS.md` records that no collect with synchronous
ground truth is in any open archive. **A null over a pyramid is the expected
outcome and says nothing about sensitivity.** It is not a negative result about
the method, and it must not be quoted as one.

So the standing summary is unchanged in substance and sharper in detail: the
phase estimator recovers injected frequencies in simulation, produces no artefact
on real data, and **has still never been shown to detect real motion, because no
collect available to this project contains motion known to be there.**

**Two things this run makes concrete for the next one.**

*Cost.* Work scales as `n_pulse * n_cells / (1 - overlap)`. At 0.90 overlap over
65536 cells that is 45 minutes; 0.95 would be 90, and a 512-cell grid four times
that again. Budget before configuring, and note `--null-static` multiplies it by
the trial count -- which is why it was not run here, there being no positive to
adjudicate.

*The quality metric is weak on real data.* Amplitude stability ran 0.000 to 0.619
with a median of 0.417, against 0.75-0.95 on the synthetic fixtures, and its map
shows no structure following the pyramid's edges. The coherence gate admitted 170
of 225 windows on a relative threshold of 0.31. Whether amplitude stability is
the right precondition proxy on real clutter is now an open question that the
synthetic work could not have raised.

---

## 18. Screening a collect from its metadata block, and the Istanbul candidate

`validate --xml FILE` runs the full arithmetic on a CPHD's metadata block alone.
`rs_read_cphd_meta()` takes either a whole collect -- parsing the ASCII header for
`XML_BLOCK_BYTE_OFFSET` and seeking -- or the extracted block, told apart by the
`CPHD/` magic and never by the extension. **Screening the 36 GB Giza file takes
0.3 seconds**, and over HTTPS it is two range requests totalling about 12 KB
against a 17-61 GB download.

**What it agrees with the full reader about, and what it does not.** Measured on
Giza, and asserted in `test_cphd.c` against the synthetic fixture:

```
              screen        full read
dwell         32.869 s      32.869 s     agree
platform      7264 m/s      7263 m/s     agree
carrier       9.3000 GHz    9.3000 GHz   agree -- both the band START
pulses        335149        335141       DECLARED, not validity-screened
slant range   754.2 km      762.8 km     ReferenceGeometry vs FIRST pulse
incidence     38.58 deg     39.50 deg    likewise
```

The carrier agreement is the one that had to be got right and nearly was not.
`rs_read_cphd()` takes `fc` from the first vector's `SC0` because that is where
transforming the FX samples leaves the residual phase; on this product `SC0` is
FxMin at 9.30 GHz where the band CENTRE is 9.60. Taking the centre -- the obvious
reading of "carrier" -- would have shifted lambda by 3.2 percent and scaled every
phase-derived displacement by the same amount, in a tool whose whole purpose is
to predict what the run will do. The screen therefore reads FxMin, and
`test_cphd.c` asserts the two match to a hertz.

The other two differences are definitional and harmless for a download decision:
the pulse count is `NumVectors` as declared where the full reader drops the eight
vectors this file flags invalid, and range and incidence are stated at the
reference time where the full reader measures them at the first pulse.

`RS_VALIDATE_PRF_STABILITY` now reports **UNKNOWN** when no per-pulse times are
supplied, rather than vanishing from the list. The instantaneous spread and the
largest dropped-vector gap live in the PVP block, which a screen does not read; a
check that disappears reads as a check that passed.

### The Istanbul 25 s collect screens clean

`CAPELLA_C09_SP_CPHD_HH_20230321101754_20230321101819`, 17.3 GB, 249424 vectors,
dwell 24.999 s, incidence **19.53 deg**, 13:18 local. Screened for a 2 Hz target
of 3 mm on the phase route at alpha 0.67 percent:

```
PASS     observable band   sampling ceiling 2.985 Hz; 2 Hz is inside
PASS     ambiguity         lambda/4 fold caps 8.241 mm; 3 mm is 0.36x
PASS     phase floor       0.507 mm per look, against a 3 mm target
WARN     observation ratio eta 0.335
WARN     aperture fraction 0.67% against a published 4.5-7.6%
UNKNOWN  PRF stability     needs the PVP block
UNKNOWN  sensitivity       not the phase route's question
UNKNOWN  ground truth      always
VERDICT: WARN
```

**No FAILs.** The two warnings are real and known: the aperture fraction is an
order of magnitude below the published range, which is the regime item 4 and
`validate` both flag as untested rather than impossible. Against Giza's
three-orders-of-magnitude gap between plausible target amplitude and instrument
window, this is the first collect where the two ranges touch.

The incidence is why: 19.53 degrees gives cos 0.943 against Giza's 0.782, so
vertical modes project 20 percent better onto the line of sight, and the lambda/4
fold caps a *vertical* amplitude at 8.24 mm rather than 10.44.

**What is in the footprint**, verified against OpenStreetMap rather than inferred
from the scene centre: the M2 cable-stayed metro bridge over the Golden Horn,
Galata Koprusu, Ataturk Koprusu, Marmaray rail segments -- and the Valens
Aqueduct, a Roman masonry arch at the same range through the same processing.
That aqueduct is an **in-scene static reference**, which no simulated null can
be. 13:18 local matters as much: traffic is the principal excitation for these
decks, and the 60 s Istanbul collect is 01:39 local with the metro not running.

**Still no ground truth.** No accelerometer, no reflector. Published modal
frequencies for these bridges would be weak external truth -- more than Khufu
offers and less than validation. What changes is that a target class whose
plausible amplitudes overlap the instrument's window is in front of this software
for the first time.

**Before downloading, the open question from item 17 applies.** At Giza no window
met the phase estimator's precondition: amplitude dispersion `D_A` ran 0.381 at
best against the persistent-scatterer criterion of 0.25, so 0 of 225 windows
qualified and the run could not have succeeded whatever the pyramid was doing.
A bridge pylon or a masonry arch should score far lower, but nothing in the tool
reports `D_A` against that criterion yet, so a null over Istanbul would be as
uninterpretable as the null over Giza. That check is the thing to land next.

---

## 19. Amplitude dispersion, and item 17's null is a precondition failure

Item 17 left the Giza null ambiguous: "nothing moved" and "the estimator was
never applicable here" produce the same output, and nothing in the tool told them
apart. Item 15 had already established the phase route's precondition -- one
dominant scatterer per sub-look resolution cell -- but there was no way to check
it against a real scene.

`rs_microm_t.d_a` is that check. It is the **amplitude dispersion** of each
window's brightest pixel across the sub-look stack, `sigma_A / mu_A`, which is
Ferretti et al.'s (2001) persistent-scatterer statistic with its criterion of
`D_A <= 0.25`. `mmotion` now reports the best, the median and the count meeting
the criterion, and warns when none do. It is computed for **every** estimator,
because the most useful thing it can say is "switch estimator", and a statistic
produced only by the route it recommends cannot say that.

**IT SEPARATES THE FIXTURES, WHICH IS THE ONLY THING THAT MAKES IT WORTH
REPORTING.** Against cases whose recovery or failure item 15 already
established, seed 7, zero overlap:

```
fixture                  phase recovers?   D_A best / median   windows <= 0.25
96 uniform clutter       YES  1.008/0.0070    0.079 / 0.314           9
3x3 dominant lattice     YES  1.008/0.0070    0.084 / 0.112          20
8x8 dense lattice        no  -0.324/2.0519    0.381 / 0.483           0
640 uniform clutter      no  -1.346/1.1614    0.397 / 0.477           0
```

No overlap, and the criterion falls in the gap: the recovering fixtures reach
0.079 and 0.084, the failing ones bottom out at 0.381 and 0.397. `test_tracking.c`
asserts the separation on the 3x3 and 8x8 pair, including that the failing
fixture's BEST window is well clear of the criterion rather than marginally over
it -- if that stops holding, the statistic has stopped predicting what it is
reported for and the warning becomes misleading rather than merely unhelpful.

**AND IT REINTERPRETS ITEM 17.** The Giza run's best window was **0.381** and its
median 0.583, so **0 of 225 windows met the criterion**. Giza sits with the
failing fixtures, not near the boundary. The null there was not evidence about
the pyramid: it is what the precondition being unmet across the entire scene
guarantees, and the run could not have succeeded whatever the ground was doing.
Item 17's headline finding -- that the sawtooth artefact is gone on real data --
is unaffected, because that rests on the flat frequency histogram rather than on
the null being meaningful.

**A caution recorded when it nearly caused an error.** The first check of this
was run against a `sim_cphd` scene carrying 400 clutter scatterers, which
returned a best `D_A` of 0.407 and looked like a counter-example to the whole
idea -- a fixture "where phase recovers" scoring with the failures. It was not a
counter-example: 400 scatterers is in the failing regime by item 15's own density
table, and the recovering fixture has 96. The statistic was right and the fixture
was misidentified. Anyone re-checking this should confirm which scene they are
looking at before concluding the statistic does not work.

**The calibration caveat stands.** 0.25 was established over independent passes,
where each acquisition is a fresh realisation; sub-looks of one aperture share
their scatterers and are not independent, so the null distribution differs. The
measured gap here is 0.084 against 0.381, wide enough that the question does not
change any of the readings above -- but the number should be read as a scale
rather than a bright line, and the separating threshold for sub-looks has not
been measured.

---

## 20. Amplitude dispersion becomes a selector, its threshold is measured, and a reader bug turns up under it

Item 19 added `D_A` as a scene-wide warning. That is the wrong shape, and the
source literature says so: Biondi et al. (Mosul Dam, arXiv 2007.05326), the
origin of the sub-aperture tracking this project implements, do not gate on
amplitude dispersion -- they **select** on it. "Measurement points are chosen
based on the amplitude dispersion index... the displacements are estimated on
these points." On a bridge over water the distinction is the whole result: two
hundred windows of water drown any scene-wide summary while the handful on the
deck are the measurement.

That paper also says the threshold is "experimentally found", which settles item
19's open caveat about whether Ferretti's multi-pass 0.25 transfers to sub-looks
of one aperture. It had to be measured.

**MEASURED.** 1800 windows -- four fixtures spanning the recovery boundary
established in item 15, six injected frequencies, three seeds, phase estimator --
scored by whether each window's OWN dominant bin matched its injection:

```
D_A <= t    windows   correct    rate
   0.20         234       230   98.3%
   0.25         318       302   95.0%
   0.30         387       354   91.5%
   0.40         627       478   76.2%
   0.50        1374       560   40.8%
   all         1800       600   33.3%
```

The knee is at 0.25 and **the borrowed constant survives contact with sub-looks**:
selecting on it lifts a 33 percent window-level hit rate to 95. Item 19's caveat
is retired -- not by argument but because the number was checked.

`rs_spectrum_ps_window()` is the selector, reported beside prominence, consensus
and the cull, gating nothing. `test_tracking.c` pins the case that motivates it:
a scene where four low-dispersion windows carry the injection and twenty-one
high-dispersion ones carry something else. Prominence and consensus follow the
21-window majority to the wrong answer; the selector does not. With the criterion
disabled it falls back to the crowd, which is what shows the selection rather
than the spectrum did the work.

### The bridge, correctly placed, still has no persistent scatterers

Item 18's Istanbul runs measured water: the coordinate lookup put the grid 183 m
off the M2 deck, and a 256 m grid has no margin for that. Re-placed on the deck,
verified by focusing candidates until the bright double line of a bridge over
water sat centred:

```
                          D_A best / median   windows <= 0.25   agreement
Giza (desert)                0.381 / 0.583           0            16%
Istanbul, water, ov 0.90     0.402 / 0.601           0            18%
Istanbul, water, ov 0.40     0.472 / 0.621           0             6%
Istanbul, BRIDGE, ov 0.90    0.391 / 0.893           0            20%
Istanbul, BRIDGE, ov 0.40    0.516 / 0.847           0            10%
```

**Four real scenes, best-window D_A between 0.38 and 0.52, never below.** The
synthetic fixtures reach 0.079. That gap is now the central open question: it is
too consistent across a desert, a waterway and a bridge deck to be scene content.

### A reader bug found while chasing it, which explains part of the gap

Both Capella products declare `AmpSF` in their PVP -- the per-vector amplitude
scale factor the CPHD standard requires be applied to the signal samples --
and `rs_read_cphd()` never reads it. Measured on the Istanbul collect:

```
AmpSF over 249424 vectors: min 0.00266  max 0.1329  mean 0.00442
  dispersion sigma/mu = 0.7108        max/min = 50x
  mean per sub-look (128 non-overlapping): sigma/mu = 0.0864
```

So every sub-look carries about 8.6 percent of unapplied gain variation, plus
rare 30x spikes. **It cannot affect phase** -- `AmpSF` is real and positive -- so
no reported frequency changes. It perturbs every amplitude-derived statistic,
which is now what the selection depends on.

**It is not the whole explanation.** Combining in quadrature, an 8.6 percent gain
dispersion takes a true 0.37 to 0.38 -- it accounts for the last hundredth of the
observed floor and not the gap from 0.079 to 0.38. Applying it is correct and
required by the standard; expecting it to unlock the real-data measurement would
be wishful.

**What remains to explain the gap.** Candidates, none measured: an aperture
amplitude taper the synthetic scenes do not have; the 16-bit integer sample
quantisation of the Capella products against the simulator's floats; genuine
scene content, if no real resolution cell in any of these scenes is dominated the
way an isolated simulated scatterer is. The third would be the substantive answer
and the first two must be excluded before it can be claimed.

### 20a. The spectrum is written twice, and the millimetre axis is calibrated

`PREFIX_spectrum.png` stays as a power density, because prominence is a ratio of
powers and that figure shows what the selection actually compared. Beside it,
`PREFIX_spectrum_mm.png` carries the same window as an amplitude in millimetres.

The reason is legibility against the literature rather than taste. Nobody reads
`(m/s)^2/Hz`, and every number this project is measured against is in millimetres:
Vattulainen et al. report displacements from 10.43 down to 0.10 mm and velocity
errors of order 1 mm/s. A squared density cannot be placed against that.

For a bin-centred tone in a Hann-windowed one-sided periodogram,
`PSD_peak = A^2/(2*ENBW)` with `ENBW = 1.5*df`, so `A = sqrt(3*PSD*df)`. The Hann
ENBW was checked numerically at n = 128 and came to 1.5118, converging to the
textbook 1.5.

**Validated against ground truth rather than against the textbook:**

```
injected 2.442 mm vertical -> 2.002 mm line-of-sight   axis reads 1.880 mm  0.94x
injected 1.221 mm vertical -> 1.001 mm line-of-sight   axis reads 0.898 mm  0.90x
halving the injection scales the reading by 0.478
```

Linearity is the assertion a constant scaling error cannot satisfy, and it holds.
The 6 to 10 percent shortfall is Hann scalloping: the injection is at 0.500 Hz and
the bin at 0.5040, and scalloping alone costs up to 15 percent for a tone between
bins. `test_tracking.c` asserts both the ratio and the linearity.

**The axis is still labelled QUALITATIVE, and that is not hedging.** The
arithmetic is right to within scalloping on synthetic data; the caveat comes from
the independent assessment, which reports time-domain RMSE of 40 to 76 percent of
peak velocity on real collects while getting every dominant frequency correct.
Frequencies survive real data and amplitudes do not, so the number is for placing
a result in an envelope rather than for quoting.

**Read at the right window.** The figure plots the most PROMINENT window, and on
the fixture used above that is a noise window at the lowest bin while the
dispersion selector names the one holding the injection. The calibration test
therefore reads the selector's window. Which window the figure should plot, now
that a better selector exists, is an open question this did not settle.

---

## 21. AmpSF was unapplied and is now applied. It does NOT explain the D_A gap.

Item 20 recorded that `rs_read_cphd()` never read `AmpSF`, the per-vector
amplitude scale factor the CPHD standard requires be applied to the signal
samples, and that both Capella products declare it. It is applied now, with a
fixture test that writes a scale rising with pulse index so a reader ignoring it
shows a flat gain where a correct one shows a ramp.

**IT CANNOT MOVE A FREQUENCY**, being real and positive, so nothing about phase
or any reported frequency changes. What it moves is amplitude, and amplitude is
now a selection criterion.

### The claim that it explained the real-vs-synthetic gap was WRONG, and this is the retraction

Four real scenes sit at a best-window `D_A` of 0.38 to 0.52 where the synthetic
fixtures reach 0.079. Injecting a per-pulse gain into the synthetic fixture
appeared to reproduce that exactly:

```
float, calibrated (baseline)     D_A best 0.083   median 0.314
+ 16-bit quantisation                     0.083          0.314
+ unapplied per-vector gain               0.337          0.553
```

which sat squarely in the real range and looked conclusive. **Applying the real
`AmpSF` to the real collect changed nothing:**

```
Istanbul M2 bridge   before   D_A best 0.391   median 0.893   0 of 225
                      after            0.409          0.846   0 of 225
```

**The synthetic injection was not a faithful model.** It drew the gain as
`exp(0.65*(-ln u - 1))`, which is unbounded and produces occasional enormous
outlier pulses; the real `AmpSF` spans a bounded 50x. The outliers did the damage,
not the dispersion, and the 0.083 to 0.337 movement was a property of the
injection rather than evidence about the collect.

The general lesson is the one this file exists for: a synthetic experiment that
reproduces a real number is not thereby an explanation of it. The check that
settled it was applying the real correction to the real data, which took twenty
minutes and should have come first.

### Ruled out, and what is left

**16-bit sample quantisation is excluded.** Capella ships `RE16I_IM16I` where the
simulator uses floats; quantising the synthetic scene to 16 bits moves `D_A` by
nothing at all, best and median alike.

**`AmpSF` is excluded**, by the measurement above.

**Scatterer density is the remaining candidate and is probably the whole answer.**
*(SUPERSEDED BY ITEM 23 -- swept on real data, density does not explain it. The
synthetic table below stands; what does not stand is the inference from it.)*
Item 15 measured `D_A` rising with scatterers per sub-look resolution cell on the
synthetic fixtures:

```
scatterers   per cell   D_A best
     96        1.38      0.079
    320        4.59      0.381
    640        9.18      0.397
```

Real terrain has hundreds to thousands per cell, not one. Fully developed speckle
has a dispersion of `sqrt(4/pi - 1) = 0.523` by construction, and the real MEDIANS
measured here are 0.583 to 0.893 -- at or above it. So the real scenes are behaving
exactly as speckle should, and the synthetic fixtures are sparse in a way no real
scene is.

**If that is right, the "gap" is not a defect at all** -- it is the difference
between a fixture with 1.4 scatterers per cell and a world with thousands, and the
synthetic `D_A` values were never comparable to real ones.

**Which puts item 20's threshold in question.** `D_A <= 0.25` was measured on
sparse synthetic fixtures and lifts a 33 percent window-level hit rate to 95 there.
Whether 0.25 is the right operating point on a dense real scene is untested, and
cannot be tested without a real collect containing motion known to be there.

**The one test that would separate density from everything else** is sub-look
resolution: finer resolution puts fewer scatterers in a cell. Two data points
already point that way -- at the same bridge position `D_A` was 0.516 at 4.41 m
sub-look resolution and 0.391 at 0.78 m -- but the controlled version, holding
grid, cell and window fixed while varying only `t_sap`, was started and abandoned
when `AmpSF` looked like the answer. It is the obvious next measurement.

---

## 22. Notes from NGA's six-library, and what is worth taking

`ngageoint/six-library` is NGA's reference C++ implementation of SICD, SIDD and
CPHD. Read for ideas rather than adoption -- it is a large C++ dependency and
this project builds with none -- and three things came out of it.

**It validates the optional-field shape of the AmpSF fix.** `cphd::PVPBlock`
exposes `hasAmpSF()` and `getAmpSF(channel, set)`, with `mAmpSFEnabled` keyed on
whether the field's PVP offset is defined. That is the same detection this
reader now uses, arrived at independently from the standard.

**It shows why the omission survived so long.**
`cphd::Wideband::read(channel, firstVector, lastVector, firstSample, lastSample,
vectorScaleFactors, ...)` takes the per-vector scale factors as a parameter the
CALLER supplies. The reference does not apply AmpSF either -- it hands the
machinery to the application. Nothing in the format or in the reference forces
the correction, so a reader that never does it looks fine until something
compares amplitudes between sub-looks.

**It settles the standing of the SGN override.** `cphd::Global` stores and prints
`SGN` and the reading path never acts on it. The reference implementation
declines to choose a transform direction. So item 3's Capella override is not a
deviation from a reference that chose otherwise -- it is making a choice the
reference leaves open, and making it per-vendor is a judgement about data rather
than about the format. That is better ground than the note previously stood on,
and both headers now say so.

**One optimisation, now taken.** `Wideband::allOnes()` skips the scaling pass
entirely when every scale factor is 1.0. `rs_read_cphd()` now branches the unity
case out of its copy loop.

The saving is SMALLER HERE than in the reference, and the difference matters more
than the change does. six-library scales in a separate pass over the array, which
`allOnes()` elides completely; this multiply is fused into a copy that has to
happen regardless, so what is elided is one complex-by-real multiply per sample
in a loop that also does a modulo -- and the modulo is plausibly the larger cost
of the two. The saving is UNMEASURED: benchmarking the CPHD read path needs a
real vendor product, since `sim_cphd` writes this project's own container.

The case it pays for is a product carrying no AmpSF at all, which is the case the
field being optional exists to allow. A Capella product has a per-vector factor
with a dispersion of 0.711 and takes the scaling path on nearly every vector, so
this does nothing for the collects this project actually processes. Branched per
vector rather than once per array, which catches the reference's all-unity case
and also a product that is unity on some vectors and not others.

**And a structural point it shares with GDAL.** The reference's fundamental read
is WINDOWED -- a range of vectors and a range of samples -- and GDAL's raster
model is block-based with a cache. Both take streaming as the primitive. This
reader loaded `n_pulse * n_rbin` complex floats up front, which is 12.3 GB for the
Istanbul collect at `--rbins 6144` and 39.7 GB at full range extent, and that
ceiling shaped several decisions in this project: reconnaissance focusing had to
use `--max-pulses`, and the M2 bridge sat near the edge of the affordable range
window.

**STAGE 1 IS DONE (2026-08-02): `focus --stream N`.** `rs_cphd_read_opts_t` gained
`pulse_first`, which with `max_pulses` makes the read windowed -- the same
primitive `Wideband::read()` exposes -- and `rs_focus_opts_t` gained `accumulate`.
`focus` now walks the collect in blocks of N pulses and sums them.

Backprojection is a sum over pulses, so blocking is EXACT rather than an
approximation, and that is the test: the streamed image is BIT-IDENTICAL to the
monolithic one, same SHA-256, at block sizes 8192, 16384 and 65536.

```
  block size   blocks   resident        image
  monolithic        1   2.56 GB         (reference)
       65536        6   0.50 GB         bit-identical
       16384       21   0.12 GB         bit-identical
        8192       41   0.06 GB         bit-identical
```

Measured at `--rbins 1024`; the saving scales with `--rbins`, so at the 4096 used
for the Giza runs it is 11 GB against 0.25 GB. Runtime cost is about 6% at 16384
(79.4 s against 75.0 s), from re-parsing the header and PVP block per block.

One defect found and fixed while doing it: the block count derived from
`NumVectors`, which counts the 8 vectors of 335149 that the validity screen then
rejects, so the loop could ask the reader for pulses past the end. It now ends on
the first short block instead.

**STAGE 2 IS DONE (2026-08-02): `mmotion --stream N`.**
`rs_subaperture_from_cphd_stream()` walks the collect ONCE in pulse blocks and
accumulates each block into every sub-look whose pulse window intersects it. The
obvious alternative -- read each sub-look's range, focus it, free it -- re-reads
the file about ten times at 0.90 overlap, which over USB costs more than the
memory it saves.

Measured on the Giza collect, `--rbins 4096`, 16 looks over a 64 m grid:

```
  resident   6.07 GB   strongest 0.063 Hz prom 3.4, consensus 0.063 Hz 23/49
  streamed   0.92 GB   strongest 0.063 Hz prom 3.4, consensus 0.063 Hz 23/49
```

Identical output, 6.6x less memory. Two things were needed to make the resident
container that small: the sub-aperture stage streams the signal, and `mmotion`
now loads the geometry at TWO range bins rather than the full window, since
`--at`, the sampling warning and `--null-static` read the pulse track and not the
samples.

**A CORRECTION TO STAGE 1'S VERIFICATION.** Stage 1 claimed the streamed image was
bit-identical because the PNGs matched. That proved agreement to 8-BIT DISPLAY
PRECISION and nothing more. Comparing the actual samples showed the streamed
sub-look stack differed from the resident one in 46747 of 65536 samples, worst
4.3e-05 -- float rounding, because summing blocks into the float image
reassociates the sum.

Small, and still the wrong trade here: `-ffast-math` is banned in this project
precisely because reassociation perturbs the sub-pixel correlation peaks and
interferometric phase the measurement reads, and a streaming path that
reassociates is that hazard arriving by another door. So `rs_focus_opts_t` gained
`accum`, a double buffer the kernel sums into and the caller rounds to float once
after the last block. With it the streamed stack is bit-identical to the resident
one -- 0 of 65536 samples differ, at block sizes 8192 and 16384 -- and so is
`az_resolution`, which required computing each look's geometry from its whole
pulse window rather than from whichever block happened to contain its centre.

`--stream` is refused with `--inject-vib`, because the injection is written into
the resident phase history and a streamed read would never see it: a positive
control silently missing its injection is exactly what item 28 cost half a day to
find. It is also refused for the spectral routes, which need a full-aperture image
first -- the thing that does not fit.

### Item 22, verified at the operating point that matters -- and it is I/O bound

Stage 2's check was 16 looks over a 64 m grid. Re-run at the configuration the
Giza runs actually use -- 128 looks, 0.90 overlap, 256 m grid, `--rbins 4096` --
against the committed resident control of item 30
(`runs/giza/2026-08-03-stream-control/`):

```
                    wall        user     mean CPU   peak RSS
  resident      44 min 55 s        --        442%     ~11 GB of phase history
  streamed       4 h 00 min    9626 s         67%       1.05 GB
```

**Exact.** All twelve substantive per-window columns identical digit for digit,
and the cull's header line byte-identical. So the streaming path reproduces the
resident one at ten times the look count and sixteen times the grid area of the
original check.

**But the wall-clock cost is 5.3x, not the 6% stage 1 measured for `focus`, and
the CPU figure says it is not extra work.** Total user time went DOWN -- 9626 s
against the resident run's implied ~11900 s -- while wall time went up 5.3x, so
the process was waiting rather than computing. Mean CPU fell from 442% to 67% on
eight cores. The collect is on external USB and `--rbins 4096` of 29160 samples
makes every pulse a strided read; the streamed path adds a header and PVP re-parse
per block on top.

Not isolated further: separating drive bandwidth from seek cost from per-block
re-parsing needs the same run against local storage. What is established is that
`--stream` at this operating point is an I/O-bound trade on this hardware, and a
budget should assume hours rather than the minutes stage 1 implies.

**Nothing worth taking on safety.** GDAL's NITF truncation guard fires only above
a million blocks and is self-described as "really a very safe bound"; this
reader checks every declared block lies wholly inside the file, in subtraction
form so a hostile offset cannot wrap an addition. GDAL's CPHD driver is a
RAWDataset container reader with no CPHD semantics at all -- no range
compression, no PVP geometry, no AmpSF, no SGN -- so there is nothing in it to
learn from for this format.

---

## 23. Neither scatterer density nor the brightest-pixel selection explains the D_A gap

Item 21 left scatterer density as the last standing candidate for why real
collects reach an amplitude dispersion of 0.38-0.52 at best where the synthetic
fixtures reach 0.079. Two experiments here, and both come back negative. The gap
survives.

### 23a. Resolution: the density axis, swept on real data

A first attempt was confounded -- it changed cell size and window size along with
resolution, so a median D_A above 1.0 could have been oversampling rather than
scene content. Redone with the physical window FIXED at 32 m of ground and the
cell TRACKING sub-look resolution, so every pixel is one independent resolution
cell and the window always searches the same patch. Istanbul M2 bridge,
`--offset -488,-1864 --estimator phase --rbins 6144 --coherence 0`:

```
    n      ov   t_sap   res_m   cell   win   grid   DA_best    DA_med
  128    0.40   0.324    4.41    4.0     8     32     0.539     1.425
   64    0.75   1.492    0.96    1.0    32    128     0.481     1.865
   32   0.898   6.006    0.24   0.25   128    256     1.394     1.636
```

Cell area falls from 19.5 m^2 to 0.06 m^2 -- roughly 340x fewer scatterers
competing inside one resolution cell -- and D_A does not fall with it. Best goes
0.539, 0.481, 1.394; no median comes near the 0.523 of fully developed speckle.
If density were what separates real scenes from the fixtures, this is the axis
that would have shown it.

TWO CAVEATS, ONE OF THEM INHERENT. At fixed dwell, sub-look resolution cannot be
varied without varying look count and overlap, and both move D_A on their own:
resolution, `n` and overlap are one knob here, not three. And only AZIMUTH
resolution changes -- range resolution is set by bandwidth -- so the finest point
oversamples in range. This sweep bounds the density effect rather than isolating
it.

### 23b. The brightest-pixel selection bias: real, and far too small

`rs_microm_track()` computes D_A at the ARGMAX OF THE REFERENCE LOOK and then
measures that pixel across all looks (`src/core/microm.c`). That is a selection on
one realisation followed by a measurement over all of them, so it is biased
upward, and the bias must grow with the number of candidate pixels in the window.
23a ran windows of 64, 1024 and 16384 pixels, and the synthetic D_A work used
32x32 windows where the real runs used the default 64x64 -- so the bias was a live
explanation for part of the gap, in the direction that flatters the fixtures.

Measured on synthetic, tracking ONE shared sub-aperture stack at both window
sizes so nothing but candidate count differs, over the same 3 seeds x 6
frequencies x 4 fixtures:

```
  win     n   DA_med   DA_min   D_A<=0.25   hit rate   ungated
   32  1800    0.452    0.073   318 win        95.0%     33.3%
   64   288    0.477    0.093    44 win       100.0%     27.1%
```

A 4x increase in candidates moves the median by 0.025 and the minimum by 0.020.
The bias is real and it is negligible: it cannot account for 0.079 against 0.38.
**The gap is not an artefact of how D_A is measured.**

Two things worth keeping. The `D_A <= 0.25` threshold of item 20 SURVIVES the
window change -- 95.0% at win 32, 100.0% at win 64, against ungated 33.3% and
27.1% -- so that threshold is not an artefact of the window size it was measured
at. And the effect is not uniform across fixtures: `lattice3` degrades (median
0.264 to 0.430, hit 66.0% to 38.9%) while `clutter96` improves (0.372 to 0.351,
47.8% to 63.9%), which is what a window growing past one lattice dominant should
do.

### 23c. Real data responds to window size, and the fixtures do not

The same control on the real collect, everything fixed except `--win` (n 64,
overlap 0.75, cell 1.0, grid 128 -- the middle row of 23a):

```
  win   px_in_win   n_windows   DA_best   DA_med
   16         256         225     0.422    0.915
   32        1024          49     0.481    1.865
   64        4096           9     2.021    2.590
```

The win-32 point reproduces 23a's middle row exactly, so that row is confirmed
rather than a single-run fluke.

READ THE MEDIAN, NOT THE BEST. `DA_best` is a MINIMUM over the windows in the
run, and a fixed grid gives 225, 49 and 9 windows as the window grows -- so the
best degrades partly because it is drawn from a smaller sample. That caveat
applies to the synthetic table above too: min 0.073 over 1800 draws against 0.093
over 288 is about what pure sampling would give, which makes the selection bias
smaller still. The median is the statistic to compare, and it rises 0.915, 1.865,
2.590 (the last from only 9 windows, so it is noisy).

**Real data responds strongly to window size where the fixtures barely respond at
all**: a 4x candidate increase moves the real median by 0.95 and the synthetic
median by 0.025, a factor of nearly 40. Since the two runs execute the same code
over the same window geometry, the difference is in the SCENE, not the estimator.

### 23d. What this leaves: aspect-dependent scattering, not density

A bigger window finds a brighter argmax. On the fixtures that pixel is MORE
stable across sub-looks; on the bridge it is LESS. The natural reading is that
the strongest returns in a real scene are the most ANGLE-SELECTIVE ones --
specular faces, dihedrals and multipath off bridge structure, bright over part of
the aperture and gone over the rest -- so a window that reaches further finds a
scatterer that is stronger in the reference look and less persistent across looks.
The fixtures cannot reproduce that: item 12f already established that
`rs_sim_scene()` gives every scatterer analytically exact phase and isotropic
response, so it has NO SUB-LOOK DECORRELATION to speak of. A simulated dominant
is dominant in every look by construction.

That makes aspect-dependent scattering the leading explanation of the D_A gap,
and it is the same modelling deficiency item 12f identified from the coherence
gate. Two independent measurements now point at the propagation model rather than
at the target list. It is a hypothesis, not a result: the test is to give
`rs_sim_scene()` an aspect-dependent response and see whether synthetic D_A moves
into the 0.4-0.9 band real scenes occupy, and whether the window-size response
appears with it.

### 23e. The antenna gain taper is excluded, by four orders of magnitude

Item 21's candidate list opened with "an aperture amplitude taper the synthetic
scenes do not have". It is a good candidate on its face: a gain that varies
across the dwell gives each sub-look a different amplitude, which is exactly
what raises `sigma_A/mu_A`, and `rs_sim_scene()` has no such taper by
construction.

**The data to test it has been in the file all along and nothing reads it.**
Capella's own notebook (`CPHD_by_Example.ipynb`) spends four cells on
`Antenna/AntPattern`, and the Giza XML carries it populated: `GainZero` 49.58
dBi, a 2x2 `Array/GainPoly` in direction cosines, and an `AntCoordFrame` giving
the antenna axes as 5th-order polynomials in time. The polynomial implies a 3 dB
beamwidth of **0.676 deg** in azimuth and 0.680 in elevation, which matches the
notebook's stated "nominal beamwidth is 0.7 deg" -- the check that says the
coefficients are being read correctly.

**MEASURED, on the Giza collect.** The line of sight from the antenna phase
centre to each target was put into the antenna frame per pulse and the gain
polynomial evaluated there. 335141 of 335149 vectors pass the validity screen,
which is the same count `rs_read_cphd()` reports, so the PVP parse agrees with
the project's own.

```
  off-boresight at the SRP   |dcx| max 0.007 deg, |dcy| max 0.002 deg
  one-way gain across the whole 32.87 s dwell        span 0.001 dB

  offset from SRP    gain span across dwell    D_A from the taper alone
        0 m               0.001068 dB               0.000029
      128 m               0.002190 dB               0.000049
      500 m               0.008743 dB               0.000064
     2500 m               0.160751 dB               0.001024
```

Against an observed real-scene floor of **0.38**. At the edge of the 256 m
patches the Giza runs actually used the taper contributes 0.00005, and even
2500 m off the reference point -- ten times any grid this project has placed --
it reaches 0.001, still 380 times too small.

**The reason is that the spacecraft tracks the target almost exactly.** The
antenna frame rotates 18.04 deg across the dwell at 0.549 deg/s while the target
stays within 0.007 deg of boresight, so a 0.676 deg beam never presents the
scene with anything but its flat top. The taper is real, it is simply four
orders of magnitude below what would matter.

*What this does not cover:* the ELEVATION pattern across the swath is a static
per-pixel gain, not a per-look one, so it cannot move `D_A` whatever its size --
`D_A` is a variation across sub-looks at a fixed pixel. And this is one collect;
a squinted or poorly-tracked acquisition would not behave this way, which is an
argument for reading the pattern rather than assuming it is always negligible.

RULED OUT SO FAR, for the record: 16-bit sample quantisation (item 21, no
effect), per-vector AmpSF (item 21, measured, no effect), scatterer density
(23a), the brightest-pixel selection bias (23b), and the aperture amplitude
taper (23e). **Aspect-dependent scattering (23d) is what remains**, and it is now
the leading explanation by elimination as well as by the window-size evidence.


---

## 24. Aspect-dependent scattering: the mechanism the fixtures lacked

Item 23d predicted that what separates real collects from these fixtures is that
`rs_sim_scene()` makes every scatterer isotropic, so a simulated dominant is
dominant in every sub-look by construction. `rs_sim_scene_aspect()` adds the
missing mechanism -- a flat facet with a sinc lobe in the along-track direction
cosine, bright over only part of the aperture. See `rs_sim_aspect_t` for the
derivation; parameterising the lobe by its width as a fraction of the aperture
makes the facet length drop out, leaving `lobe_frac` as directly the quantity
that matters: what fraction of the sub-looks a scatterer is bright over.

**It is opt-in and `rs_sim_scene()` is unchanged.** Every result in this file was
measured on the isotropic scene and stays reproducible by the call it was made
with. Aspect is deliberately not a field on `rs_sim_tgt_t`: callers build target
lists in uninitialised stack arrays and assign field by field, so a new member
would be read as garbage by every existing fixture.

### 24a. A facet must be BRIGHTER than clutter, not merely modulated

The first version gave facets the same peak amplitude as diffuse scatterers and
it moved D_A the WRONG WAY -- median 0.36 isotropic against 0.29 at the narrowest
lobe. The reason is that a unity-gain lobe only ever makes a target DIMMER, so
the brightest-pixel statistic preferentially avoided exactly the targets being
modelled, and what was left was a cleaner scene: the hit rate went UP, 52% to
80%. That is physically wrong -- a flat face or dihedral returns far more at its
specular angle than clutter does at any angle -- and `peak_gain` fixes it. It is
bounded and explicit, unlike the unbounded gain item 21 had to retract a
conclusion over.

Median D_A at win 32 over three seeds, uniform clutter, sweeping both:

```
lobe_frac    gain 1              gain 4              gain 16
  1.00       0.285 0.454 0.386   0.679 0.633 0.666   0.684 0.632 0.677
  0.50       0.248 0.392 0.366   0.528 0.666 0.615   0.793 0.793 0.871
  0.25       0.207 0.401 0.340   0.485 0.528 0.698   0.917 1.058 1.050
  0.12       0.207 0.334 0.339   0.445 0.493 0.619   1.132 1.150 1.184
isotropic control                                    0.314 0.405 0.372
```

**The level real collects occupy is reproduced.** Real medians are 0.58-0.89;
`lobe_frac` 0.50 at gain 16 gives 0.79-0.87.

### 24b. The window-size response appears, at about a third of the real slope

Item 23c's signature -- D_A rising with window size on real data and flat on the
fixtures -- run again with aspect on. Filled grid at held-constant scatterer
density, giving 225/49/9 windows at win 16/32/64, the same counts as the real
run so the medians are comparable draw for draw:

```
   lobe  seed |  med16   med32   med64 | d(16->64) | hit16
  -1.00     7 |  0.399   0.352   0.349 |    -0.050 |  45.3%    isotropic
  -1.00    23 |  0.421   0.402   0.308 |    -0.113 |  45.3%    control
  -1.00   101 |  0.401   0.317   0.424 |    +0.023 |  48.9%
   0.50     7 |  0.831   0.967   1.042 |    +0.211 |   4.0%
   0.50    23 |  0.778   0.925   0.968 |    +0.189 |   7.1%
   0.50   101 |  0.781   0.867   0.888 |    +0.107 |   8.4%
   0.25     7 |  1.056   1.232   1.232 |    +0.176 |   2.7%
   0.12     7 |  1.156   1.371   1.653 |    +0.497 |   7.6%
   0.12    23 |  1.101   1.170   1.213 |    +0.112 |   8.4%
   0.12   101 |  1.033   1.208   1.538 |    +0.504 |  10.2%
real (Istanbul) 0.915   1.865   2.590 |    +1.675 |
```

The isotropic control is flat or negative in every seed; every aspect-dependent
row rises. **The sign, the monotonicity and the level are reproduced.** The SLOPE
is not: real data rises +1.675 across the same span and the steepest lobe tested
reaches +0.50, about a third. So aspect dependence is a mechanism that produces
the effect, and it is not by itself the whole of what real scenes are doing.

**Not calibrated, and the numbers are not a fit.** `lobe_frac` and `peak_gain`
were chosen to SPAN a range, not fitted to Istanbul, and no collect available to
this project has per-scatterer ground truth to fit against. That the sweep passes
through the real band says the mechanism can produce those numbers, not that real
facets have these parameters.

### 24c. The side effect worth more than the D_A result

The hit rate collapses: 45-49% isotropic against 2.7-10.2% with aspect on, at
every lobe width. **This is the first time a fixture in this project has behaved
like a real collect in REFUSING TO YIELD A FREQUENCY.** Every synthetic recovery
recorded here was measured on scenes where the dominant scatterer is dominant in
every look, and item 15's precondition -- one dominant per sub-look resolution
cell -- is far easier to satisfy when dominance cannot vary with aspect. That
makes aspect dependence a candidate explanation for the standing gap between
"recovers on synthetic" and "returns a null on every real collect", which is a
larger question than D_A was.

Untested, and the obvious next measurement: whether `--estimator phase` still
passes `rs_track_fit()`'s slope-and-rms bar on an aspect-dependent sweep, and if
not, at what lobe width it breaks. That would say how much of item 14's recovery
survives contact with the mechanism, and it is the first fixture this project has
that could take the question seriously.

---

## 25. Item 14's recovery does not survive aspect dependence, and the PS selector does

Item 24c asked whether `--estimator phase` still clears `rs_track_fit()` once
scatterers are aspect-dependent. Item 14's own sweep, unchanged -- six
frequencies over three seeds plus a static control per seed, at item 14's
settings -- with `lobe_frac` the only new variable:

```
   lobe |    best_window        |    PS selector        | cand |   static best     |   static PS
        |   n  slope    rms     |   n  slope    rms     | /scn |                   |
  -1.00 |  18 +1.008 0.0070 PASS|  17 +1.008 0.0068 PASS|  2.9 | 3.02  2.82  1.46  | 1.66  2.62  2.07
   1.00 |  18 -0.662 1.0459 fail|   0  --     --    fail|  0.0 | 3.02  2.57 [1.26] | none  none  none
   0.50 |  18 -0.746 1.7835 fail|   0  --     --    fail|  0.0 | 2.27  2.57  2.52  | none  none  none
   0.25 |  18 -1.442 1.6348 fail|   1  --     --    fail|  0.0 | 3.07  2.22  3.07  | none  none  none
   0.12 |  18 +0.430 0.5910 fail|   6 +1.008 0.0070 PASS|  0.7 |[0.71] 2.02  1.41  | none  2.02  none
```

The bar is slope within 0.15 of 1, rms below half a bin (0.0252 Hz), and each
static control outside the swept band of 0.3-1.3 Hz. Bracketed statics are
INSIDE it. The isotropic control returns 1.008 and 0.0070, item 14's published
figures to four decimals, which is what says this sweep is item 14's sweep.

**The reported policy fails everywhere.** `rs_spectrum_best_window()` -- what
`mmotion` actually prints -- misses at every lobe width, with rms of 0.59 to 1.78
Hz against a 0.0252 Hz bound, 20x to 70x over. Slope goes NEGATIVE at three of
the four. This is not degradation, it is the absence of a relationship.

**Worse, it fails unsafely.** Two of the twelve static controls come back INSIDE
the swept band -- 1.26 Hz at `lobe_frac` 1.00 and 0.71 Hz at 0.12 -- on scenes
where nothing moves at all. Aspect dependence gives amplitude per-look structure
the isotropic model never had, and the prominence policy reads that structure as
a frequency. This is the failure this project's design is organised around
(`--null-static`, the consensus statistic, `warn rather than silently degrade`),
reproduced now in a fixture rather than argued from first principles.

**The PS selector is the only policy that behaves.** `rs_spectrum_ps_window()`
either refuses outright -- no window meets `D_A <= 0.25`, so no answer, at three
of the four lobe widths -- or, at `lobe_frac` 0.12, recovers with slope 1.008 and
rms 0.0070, the isotropic figures to four decimals. It never once returns an
in-band frequency for a static scene. Precision without recall: 6 of 18 points
answered, 100% correct, and the refusals are correct refusals.

**A NON-MONOTONICITY WORTH UNDERSTANDING.** Candidates per scene run 2.9, 0.0,
0.0, 0.0, 0.7 -- the NARROWEST lobe yields persistent scatterers where the
middling ones yield none. The reading is that a facet lit over 12% of the
aperture is effectively absent from the other 88%, leaving cells that a single
isotropic scatterer dominates cleanly, while a facet lit over half the aperture
competes throughout and lets no cell settle. If that is right, the hardest scene
is not the most specular one but the one in between, which is the opposite of
what "more aspect dependence is harder" would predict. Not established: one
sweep, and the candidate counts are small enough that a seed change could move
them.

### What this changes

Item 14's recovery is now bounded: it holds where dominance is guaranteed by
construction and does not survive a mechanism that removes that guarantee. That
does not withdraw item 14 -- the estimator does what it claims on the scene it
was measured on -- but it removes the basis for expecting it to transfer to a
real collect, and it is consistent with every real run so far returning a null.

It also settles the open half of the selection-policy question in
`rs_spectrum_best_window()`'s disfavour on this fixture family. The
`FOLLOW-UPS` record has said since items 7-9 that the tracker recovers the
carrier and the selection policy discards it; here the tracker's information is
enough for the PS selector and prominence not only misses it but manufactures
answers from motionless scenes. That is a stronger argument for changing what
`mmotion` reports than anything measured before it.

NOT DONE, and the next thing: `mmotion` still reports prominence. Changing the
reported answer to the PS selection is a user-visible change to what the tool
claims, and it should not be made off one fixture family -- the correlation route
has never been run against aspect dependence at all, and `rs_spectrum_ps_window()`
answering 6 times in 18 is a recall nobody would accept as a default. The honest
intermediate step is that `mmotion` already prints all four policies side by side,
and this is now recorded beside them.

---

## 26. The correlation route against aspect dependence: degrades, but keeps its slope

Item 25 left the correlation route untested against the mechanism item 24 added.
Run at the correlation route's own operating point -- one dominant vibrating
target against two static ones, 20 mm amplitude, zero overlap, look count from
the ambiguity condition, band 0.3-0.8 Hz where that count serves -- with
`lobe_frac` the only new variable and `frac` 1.0 so the vibrating target is
certainly a facet:

```
   lobe |    best_window        |    PS selector        | cand |  static best
  -1.00 |  18 +1.006 0.0035 PASS|  12 +1.006 0.0028 PASS|  3.6 | 0.40  0.40  0.40
   1.00 |  18 +0.982 0.2496 fail|   0  --     --    fail|  0.0 | 0.40  0.05  0.80
   0.50 |  18 +1.437 0.2203 fail|   0  --     --    fail|  0.0 | 0.40  0.05  0.05
   0.25 |  18 +1.073 0.1329 fail|   0  --     --    fail|  0.0 | 0.40  0.40  0.05
   0.12 |  18 +0.474 0.2510 fail|   0  --     --    fail|  0.0 | 0.86  0.86  0.80
```

Half-bin bound 0.025 Hz. PASS/fail above is slope and rms only -- see the caveats.

**THE FIRST ATTEMPT AT THIS WAS INVALID AND IS WORTH RECORDING.** It reused item
25's 96-scatterer clutter scene so the two routes would be comparable, and its
ISOTROPIC CONTROL FAILED (slope 1.564, rms 0.7675) -- which is exactly what items
7-9 record for this route on distributed clutter: the correlator recovers the
carrier and `rs_spectrum_best_window()` discards it. A fixture that fails before
the manipulation is applied cannot measure the manipulation. The clutter family
cannot control this route; the dominant-target family can.

**The two routes fail differently, and that is the finding.** Against aspect
dependence the phase route lost the relationship entirely -- slope NEGATIVE at
three of four lobe widths, rms 0.59-1.78 Hz (item 25). The correlation route
keeps a slope near 1 at three of four (0.982, 1.437, 1.073) and misses on
SCATTER instead: rms 0.13-0.25 Hz, five to ten times the bound. That is
consistent with what the two estimators read. The correlator tracks a patch's
offset and survives losing any individual dominant; the phase route reads one
scatterer's phase and has nothing left when that scatterer stops being dominant.
Neither clears the bar, but only one still has a proportional response.

**The PS selector never answers on any aspect arm** -- zero windows meet
`D_A <= 0.25` at every lobe width. On this three-target fixture it has no
material to select from once every target is a facet. Item 25's finding, that
selecting on dispersion is what survives aspect dependence, does NOT reproduce
here, and the difference is the fixture: 96 clutter scatterers at `frac` 0.5
leave isotropic scatterers to be found, three targets at `frac` 1.0 leave none.

### Two caveats that limit what the control arm proves

**The isotropic arm is six points, not eighteen.** The only randomness in this
fixture is aspect lobe placement, so on the `lobe -1` row the three seeds are the
SAME SCENE computed three times. Its slope and rms are honest for six distinct
injections; the seed pooling that makes item 14's bar meaningful is absent from
the control specifically.

**A motionless scene returns 0.40 Hz on the isotropic arm**, from both policies,
inside the swept band. The PASS label above tests slope and rms only; on the
static criterion this fixture's own control fails. That is a common-mode artefact
of the fixture at these settings, of the kind item 11 records, and it is present
BEFORE aspect dependence is added. It does not invalidate the slope result -- a
1.006 slope across six injections is not something a fixed artefact produces --
but it means this fixture cannot support a claim about static rejection, and the
aspect rows' statics should not be read as evidence either way.

Neither caveat is a reason to distrust the comparison between rows, which is what
this item is for: every row shares the fixture, so the difference between them is
the manipulation. Both are reasons not to quote the control row on its own.

---

## 27. `--null-static` was comparing against a defocused scene

Found while working out which phase convention an injected positive control must
write. `rs_simulate_static_like()` gave each scatterer the phase `-k*R`, absolute,
copying `rs_sim_scene()` -- and then set `phase_ref_srp = 1`.
`rs_focus_backproject()` reads that flag and undoes `k*(R - r_ref)` when it is
set, `k*R` when it is not. With the two disagreeing, `exp(-i k r_ref[p])` is left
behind: a per-pulse phase of many cycles, common to every pixel, which destroys
the coherent sum for the entire image.

Measured, same scatterers, same geometry, only the convention changed:

```
  phase_ref_srp = 1, absolute phase written : peak/mean   3.6   (noise)
  phase_ref_srp = 1, SRP-referenced phase   : peak/mean  93.7   (focused)
```

**`--null-static` is the only negative control this project has on real data.**
`CLAUDE.md` names it beside every result, item 11 says only a null control
catches common-mode artefacts, and `simulate.h` argues at length that the
shuffled null is the wrong test and this is the right one. All of that was true
of a null built from a defocused image.

FIXED by writing the phase SRP-referenced rather than by clearing the flag. Both
self-consistent choices focus identically, but a real CPHD sets
`phase_ref_srp = 1`, and the value of this control is that it travels the SAME
arithmetic as the data it stands in for -- `focus.c` branches on that flag, so
clearing it would have taken the null down a path real data never takes.

**Why it survived.** Nothing tested it. `rs_simulate_static_like()` is reachable
only through the CLI's `--null-static`, `sim_cphd` writes the project's own
container, and the test suite builds its fixtures with `rs_sim_scene()`, which
leaves the flag at zero and is therefore self-consistent by accident. The
function had a 35-line header comment justifying the design and no test asserting
the output was an image. `tests/test_nullmotion.c` now focuses the null and
requires a peak-to-mean above 20, against 93.7 focused and 3.6 broken.

**WHAT THIS DOES NOT TELL US.** Every `--null-static` figure produced before
2026-08-02 was computed against a defocused null and should be recomputed before
being quoted. The direction of the error is not obvious and is not assumed here:
a defocused null has no bright scatterers, so its tracker sees noise, and whether
that yields a HIGHER or LOWER prominence distribution than a focused static scene
is an empirical question this item does not answer. No result currently in this
file rests on a `--null-static` number -- the real-collect runs (items 17, 19)
returned nulls on window agreement and amplitude dispersion, neither of which
touches this path -- so nothing here is withdrawn. The claim being retired is
that the control was known to work.

---

## 28. The Giza positive control is INVALID: the injected target does not focus off-centre

Item 27's fix made `--null-static` a real control. This is the same lesson
arriving a second time, on the control built to replace it.

`mmotion --inject-vib 1.0,2.0,20` was run on the Giza collect at the settings of
`runs/giza/2026-08-01-phase-highoverlap`, 30 min. The injected 1.0 Hz did not come
back: `D_A` best 0.427 against 0.381 uninjected, 0 of 225 windows meeting the
criterion, no frequency reported. Read naively that says the chain cannot see a
2 mm 1 Hz motion at Giza. **It does not say that, because the injected target
never focused.**

### What was checked, in order

`focus --inject-vib` was added so this costs seconds rather than half an hour,
and the injector was given deposit accounting. Ruled out:

- **Range cropping.** 335141 of 335141 pulses deposited, range bins 3316.6 to
  3507.4 of 4096. Nothing was skipped. The silent-skip failure mode the
  accounting was added to catch is not what happened.
- **Pulse decimation in the check.** The first look used `--max-pulses 16696`;
  re-run at full sampling the smear persists, and the uninjected control at the
  same patch is clean speckle.
- **Conjugate phase.** Flipping the sign to `+k*(R - r_ref)` makes the target
  vanish entirely rather than focus, so the sign in use is the correct one.
- **The `z` convention.** `rs_focus_backproject()` places a cell at
  `origin[2] + height`, which is 0 here, and `rs_resolve_at()` never sets
  `origin[2]`. The injector's implicit z of 0 agrees.
- **Eyeballing.** The injected target is ~80 dB above clutter after integration,
  so its sidelobe skirt saturates an 8-bit autoscaled PNG whether or not the peak
  beneath is clean. Measured instead.

### Where it breaks

```
grid origin        control peak/median   injected peak/median   energy in brightest cell
  (0, 0)                 16.4                   714.3                  22.75%
  (-152, -552)            3.5                     3.5                   0.20%
```

At the scene reference point the injected target focuses to a point holding
nearly a quarter of the patch's energy. At Giza's offset it is indistinguishable
from speckle. The difference between the two is RANGE MIGRATION: at the SRP the
target sits in bin 2048.0 for every pulse, and 552 m away it walks from 3316.6 to
3507.4, 191 bins across the dwell.

Real scatterers at that same offset focus -- the quicklook in
`runs/giza/2026-08-01-phase-highoverlap` shows the pyramid's edges -- so this is a
defect in how `rs_simulate_inject_vibrator()` writes a migrating target, not in
backprojection. NOT YET IDENTIFIED. A residual 1.75 m azimuth offset of the peak
even at the SRP (cell 35 against the expected 31.5) says something small is
already wrong there and grows with offset.

### What this retracts and what it does not

**Retracted: nothing about Giza.** The 30-minute run is void. It is not evidence
that the chain cannot see motion there, and the earlier null (item 17) remains
exactly as untested as item 19 left it.

**Not retracted: the control's design, or its synthetic result.**
`tests/test_nullmotion.c` recovers 0.706 Hz from a 0.7 Hz injection at `D_A`
0.050 with the uninjected arm returning nothing, and that fixture puts the target
at the grid origin of a scene built around it -- zero migration. The synthetic
test could never have caught this, which is the same structural gap item 27
recorded: the fixture satisfies the precondition by construction.

**The general lesson, twice in one day.** Both controls this project relies on
were written, documented at length, and never checked to produce what they
claimed. The check that caught both is the same one: form an image and look at
whether the thing that should be there is there.

### Cause: the deposit envelope's width was in metres and its argument in bins

`src/core/simulate.c` computed `sigma = 2.0 * dr / 2.355` -- METRES -- and then
evaluated `exp(-d*d / 2 sigma^2)` with `d = b - fbin`, which is in BINS.
`tests/rs_sim.h` does the same calculation correctly, converting the offset to
metres with `* dr`, so the two simulators disagreed and only the untested one was
wrong.

At Capella's range sampling that makes the deposited response a SINGLE-BIN SPIKE:

```
   sigma 0.255 (metres, compared against bins)     corrected: sigma 0.849 bins
     d = 0 bins   1.00                               d = 0    1.000
     d = 1        4.5e-04                            d = 1    0.500
     d = 2        4.2e-14                            d = 2    0.062
```

A one-bin spike is not band-limited, and `rs_focus_backproject()` interpolates
between neighbouring bins to read a sub-bin range. A properly sampled response
several bins wide reconstructs accurately; a spike does not, so the amplitude
backprojection recovers swings with the FRACTIONAL part of `fbin`.

That is why the failure looked geometric. A target at the scene reference point
sits at a fixed `fbin` -- 2048.0 every pulse -- so its fraction never changes,
every pulse reads the same point of the envelope, and it focuses perfectly. A
target 552 m off-centre migrates across 191 bins, sweeps the fraction repeatedly,
and the resulting pulse-to-pulse amplitude modulation collapses the coherent sum.

FIXED, in both functions. Measured at the same Giza offset that failed:

```
                    peak/median   energy in brightest cell
  before                  3.5              0.20%
  after                 744.2             13.60%
```

**`rs_simulate_static_like()` carried the same error**, so item 27's fix was
necessary and not sufficient: on a real collect the static null's scatterers are
spread over hundreds of metres, every one of them migrates, and every one was
being deposited as a spike. `--null-static` on real data was wrong twice over,
for two independent reasons, and both are now fixed.

**Why no test caught it.** Every fixture placed its target at the grid origin of
a scene built around it, and `rs_simulate_static_like()` puts its receive window
on `centre` -- so migration was zero by construction no matter what offset was
passed. `tests/test_nullmotion.c` now hosts the case on `rs_sim_scene()`, whose
`r_ref` stays on the scene origin as a real product's stays on the SRP, asserts
the target actually migrates (9.8 bins) before asserting it focuses, and requires
peak-to-mean above 50 with the peak landing where it was injected.

### Still unexplained

The focused peak lands about 3.5 cells -- 1.75 m -- from the injection point in
azimuth, positive at the scene reference point and negative at the Giza offset.
Small, symmetric, and not accounted for. It does not prevent focusing and is not
the defect above, but it is a loose end.

### Next

Re-run the Giza positive control, which is now the first time the question will
actually have been asked.


---

## 29. The first positive control that asked the question, and what it found

Two void attempts preceded this (item 28 for the first; the second injected 1.0
Hz at a sub-aperture response of 0.128, which no operating point can recover and
`mmotion` now warns about). The third injected 0.15 Hz at response 0.800 into the
Giza collect, at the settings of the run that returned item 17's null.

```
  run                          strongest   consensus   cull    agreement
  uninjected (item 17)           0.033       0.195     0.065      16%
  injected 1.0 Hz (blind band)   0.033       0.065     0.261      19%
  injected 0.15 Hz               0.163       0.163     0.163      24%
```

At 0.0326 Hz resolution the injected 0.15 Hz falls in the bin centred on 0.163,
so **all three spectrum policies returned the injection**, having disagreed with
each other in both controls. They read different evidence -- prominence,
cross-window agreement, and the correlation surfaces' own statistics -- so their
convergence on the injected bin, in the one run where there was something to
converge on, is not readily explained as chance.

**The tool reported NO FREQUENCY.** The gate fires on 24% window agreement, and
`rs_spectrum_ps_window()` returned nothing because `D_A` reaches only 0.427
anywhere in the scene. So the tracker extracted the injected motion and the
selection policy discarded it -- items 7-9, reproduced on real data against a
known truth rather than on a fixture, which is the first time that has been
possible here.

**This is one point and not a recovery.** The bar is a slope-and-rms fit across a
sweep pooled over realisations, for the reason item 2 records. A sweep of injected
frequencies on this collect is the measurement that would settle it and is now
possible for the first time.

**Unexplained, and flagged rather than smoothed over.** `D_A` best was 0.136 with
2 qualifying windows when 1.0 Hz was injected and 0.427 with 0 windows at 0.15 Hz
-- same amplitude, same relative brightness, same geometry, only the frequency
differing. Amplitude dispersion should not depend on the injected frequency at
all. Either sub-look amplitude does depend on it in a way nothing here accounts
for, or one of those two numbers is not measuring what it appears to.


---

## 30. The bar, met on real data -- by the tracker, not by anything that reports

Item 29 recovered one injected frequency from the Giza collect and said plainly
that one point is a coincidence by this project's own standard. The sweep it
called for has now run: five injections plus an uninjected control, all six at
the settings of item 17's null, in the only band the sub-aperture response allows
at 0.90 overlap (0.098-0.228 Hz, responses 0.91 down to 0.58).

Window 112 is the centre window of the 15x15 tracking grid -- the one holding the
injected scatterer.

```
 injected  win112 dom    prom     d_a   hit      consensus   cull
  CONTROL    0.065134   13.87   0.542             0.065      0.065
    0.098    0.097701   28.71   0.376   YES       0.065      0.065
    0.130    0.130268   31.25   0.475   YES       0.130      0.065
    0.163    0.162835   31.37   0.556   YES       0.163      0.163
    0.196    0.195402   29.88   0.610   YES       0.065      0.065
    0.228    0.227969   25.60   0.645   YES       0.065      0.065

  window 112 alone : slope +0.9990  rms 0.00033 Hz   PASS (bound 0.0163)
  consensus        : slope -0.202   rms 0.0947  Hz   FAIL
  cull             : slope +0.000   rms 0.0990  Hz   FAIL
```

Five for five, rms fifty times under the bound, control below the swept band.
**This is the first time anything in this project has met the README bar on real
data**, and it is items 7-9 confirmed against a known truth rather than inferred
from a fixture: the tracker recovers the carrier and the SELECTION POLICY
discards it. Every run printed `NO FREQUENCY REPORTED` at 19-24% agreement.

### A retraction of my own reading, mid-sweep

At four points the scene-wide consensus had hit twice and missed twice, and I
called that the shape of coincidence and suggested item 29's single point was one
of the two. That was wrong. The scene-wide statistics were failing because they
pool 136 windows of desert against one window of signal, not because the tracker
missed -- which the per-window evidence file settled in seconds once read. The
lesson is the one this project already writes down: the `*_windows.csv` beside
every run exists so a question about the selection policy can be asked without
reprocessing, and it should be read BEFORE concluding anything from the summary
lines.

### This bounds item 19 and item 20 sharply

`D_A` at the recovering window runs 0.376 to 0.645 and NEVER meets 0.25. So the
persistent-scatterer criterion would have rejected every one of these five
successful recoveries, and `rs_spectrum_ps_window()` reported nothing in all six
runs. **`D_A <= 0.25` is not necessary for phase recovery here.** Item 19's
reading of the Giza null -- that the precondition was unmet across the scene, so
the run could not have succeeded whatever the pyramid was doing -- is too strong
as stated. The precondition it names is not the precondition the estimator
actually has.

### What this is not

**Not a detection method.** The window was known because the target was put
there. A positive control tests whether the chain CAN extract motion, and it can;
finding an unknown target needs a selection policy, and every policy in the tool
failed on this scene. The gap between those two is now measured rather than
argued, and it is the whole of what remains.

**Not a sensitivity bound.** One collect, one operating point, one amplitude --
2 mm at 20x the median non-zero sample magnitude, which is a strong target.

**Not a test of aspect dependence.** The injected scatterer is isotropic. Item 24
established that real dominant scatterers are not, and item 25 that aspect
dependence is exactly what breaks the phase route on fixtures. Injecting an
aspect-dependent scatterer is the obvious next measurement and would be the
harder test by some distance.

**The band is narrow.** 0.098 to 0.228 Hz is 0.13 Hz of span, against 1.0 Hz in
the synthetic sweeps, because the response ceiling allows no more at 0.90
overlap. A slope fitted over that span is a weaker constraint than the number
suggests.

---

## 31. The reported policy had the right frequency all along; the GATE discarded it

Item 30 established that the tracker recovers five injected frequencies from the
Giza collect at slope 0.999 while every run printed `NO FREQUENCY REPORTED`, and
concluded that "the SELECTION POLICY discards it". That is right about the
outcome and wrong about which stage. **Measured offline against the six
`*_windows.csv` files those runs left**, with no reprocessing --
`runs/giza/2026-08-03-policy-offline/`:

```
run       gated  inj bin   best(prom)   consensus   block-rank   block-diff
0.098       123        3   0.098 HIT    0.065  --   0.065  --    0.098 HIT
0.130       136        4   0.130 HIT    0.130 HIT   0.065  --    0.130 HIT
0.163       136        5   0.163 HIT    0.163 HIT   0.065  --    0.163 HIT
0.196       136        6   0.195 HIT    0.065  --   0.065  --    0.195 HIT
0.228       136        7   0.228 HIT    0.065  --   0.065  --    0.228 HIT
CONTROL     171        -   0.130        0.065       0.065        0.033

  best (prominence)          5/5    control answers 0.130 Hz
  consensus (plurality)      2/5    control answers 0.065 Hz
  block-rank                 0/5    control answers 0.065 Hz
  block-rank minus control   5/5    control answers 0.033 Hz
```

**`rs_spectrum_best_window()` -- the policy `mmotion` actually reports -- named
the injected frequency in FIVE OF FIVE runs.** Item 30 did not test this; its
table reports window 112's own dominant bin beside the consensus and cull, and
concluded from those two that selection had failed. The scene-wide agreement
GATE is what suppressed the answer, at 19-24% against a one-third threshold, and
that gate is structurally wrong for this problem: the signal occupies a 9-12
window block and 136 windows of desert dilute it below any fixed fraction.

**Contiguity alone is worse, not better, and this is worth recording because it
was the obvious next idea.** Ranking candidate frequencies by their largest
4-connected block gets 0 of 5: the desert produces a 12-window block at 0.065 Hz
-- the lowest bins -- in every run including the control, and it wins every time.
That is item 11's common-mode artefact defeating a spatial statistic exactly as
item 11 says only a null control can catch.

### What is actually missing is a REFUSAL criterion, and one may exist

The frequency is not the problem. Two independent statistics find it 5/5. What
the tool lacks is something that says *no* on a scene with nothing in it, without
being a scene-wide fraction. The winning window's prominence is a candidate:

```
run        max prom   2nd    ratio   blk@winner   margin vs control
0.098         35.35  35.35   1.000           11                   7
0.130         32.56  32.56   1.000           12                   4
0.163         31.97  31.97   1.000            9                   1
0.196         30.04  30.04   1.000           11                   8
0.228         25.77  25.60   1.007            9                   3
CONTROL       17.48  17.16   1.019            8                   0

injected 25.77-35.35 against a control of 17.48 -- a 1.47x gap, no overlap
```

Note the `ratio` column: max over runner-up is 1.000 in four of five runs,
because the top two windows are NEIGHBOURS OF THE SAME TARGET. Any criterion of
the form "the peak stands clear of its rivals" is therefore useless here -- the
rivals are the signal. It is the ABSOLUTE prominence that separates.

### Four reasons this is a lead and not a result

*Five injected runs and one control.* Any threshold chosen after seeing this
table is fitted to it. Item 12c's warning applies unchanged: a policy that
answers only where the answer is easy scores well and has demonstrated nothing.

*The target is strong.* 2 mm at 20x the median non-zero sample magnitude, which
item 30 already flags as "not a sensitivity bound". A 1.47x prominence gap may be
a direct consequence of that amplitude and shrink to nothing for a weaker one.
Nothing here bounds where it fails.

*The control-differenced block statistic is degenerate on the control*, since
differencing a run against itself gives zero everywhere. Its 5/5 is real; its
"control answers 0.033 Hz" proves nothing about false positives. A deployment
would difference against `--null-static`, which item 27 made a real control, and
that has not been run at these settings.

*Prominence is anti-correlated with correctness everywhere else in this file* --
items 7-9 on synthetic clutter, item 25 where it manufactures in-band answers
from motionless aspect-dependent scenes. This is real data pointing the other
way. Both can hold, since the failure modes differ, but prominence is not
vindicated in general by one collect with a bright injected target in it.

**The concrete next step is therefore not a new statistic.** It is to replace the
scene-wide agreement gate with one that survives a localised target, and to
calibrate its threshold against `--null-static` at these settings rather than
against the five points above.

### Item 31, implemented: the null control decides and the fraction is a diagnostic

`mmotion`'s gate is no longer a scene-wide fraction. `--null-static` now runs
BEFORE the verdict rather than after it -- it had been computing the only
statistic that separates a common-mode artefact from a measurement, item 11's
conclusion, too late to decide anything -- and the verdict is whether any
motionless realisation reached the measurement. No tuned constant, calibrated by
the collect's own geometry, and it scales correctly with how much of the scene is
background because it compares like with like.

Beside the reported frequency the tool now prints how many windows back **that**
frequency and their largest touching block, via `rs_spectrum_block_at()`. The
scene-wide consensus is still printed and no longer gates: on this data it
describes the desert's 0.065 Hz artefact rather than the signal.

**Without a null the tool now REPORTS, marked `NOT ADJUDICATED`.** It answers more
often than before, which is a real cost, taken deliberately: the old refusal was
not a safety property. It rejected all five true positives above while still
passing item 11's artefact at 100 percent agreement, and it justified itself with
a sentence -- "which is what a MOTIONLESS scene produces" -- that item 31 shows
was false on every one of them.

Contiguity is reported and still does not gate, for the reason measured above:
ranking on it gets 0 of 5.

All three paths were exercised before this was committed -- adjudicated pass,
adjudicated refusal (2 of 30 motionless realisations reaching the measurement),
and the unadjudicated report.

---

## 32. The published estimator was the one item 6 measured working, and it was not implemented

Found by searching the literature rather than by a measurement, which is the
point: this file had reasoned for months from a set of papers that had gone
stale, and one search changed three of its beliefs.

**Suppi, Lotti, Vattulainen, Diaz Riofrio, Rollo, Ilioudis, Tonelli, Tubaldi,
Clemente, Zonta and Milillo, "Vibrational Monitoring of Isolated Targets Using
Single-Pass SAR Images", IWSHM 2025.** Umbra X-band spotlight, a corner reflector
on an electromechanical shaker, an LVDT recording true displacement, synchronised
with satellite overpasses, at Trento and Glasgow.

### Their estimator is the brightest pixel, tracked

Their step 5, quoted: *"Pixel tracking: The azimuthal displacement of the
brightest pixel in each sub-aperture is identified and tracked over time."* No
correlation surface, no phase, no reference look.

**Item 6 measured exactly that on this project's own stacks and then it was not
built.** That item recorded a plain integer argmax carrying 93 percent of its
variance at the injected frequency while the correlator, on the identical stack,
carried 4.1 percent -- and the project went on to build correlation, phase and
split-band estimators over the following months. `RS_MICROM_EST_ARGMAX` now
exists. On an isolated dominant target at 128 looks and zero overlap it passes
this project's bar first time:

```
  0.30 Hz -> 0.3024      0.90 Hz -> 0.9073
  0.50 Hz -> 0.5040      1.10 Hz -> 1.1089
  0.70 Hz -> 0.7056
  slope +1.0081, rms 0.0061 Hz against a half-bin bound of 0.0252
```

Comparable to item 14's phase result (slope 1.008, rms 0.0070) on the same
fixture family, with a weaker precondition -- the peak has only to be FINDABLE,
not coherent -- and no lambda/4 wrap. It is quantised at one cell by
construction, so `quant_px` is 1.0 and the quantisation floor refuses an
excursion under 2.449 cells rather than a sub-cell refinement nothing has
evidence for.

**Untested against clutter, aspect dependence, or real data.** Items 12f, 24 and
25 apply unchanged, and item 25 is the specific warning: item 14's recovery on
this same fixture family did not survive aspect dependence.

### Their operating point is nothing like this project's

```
Test  t_obs(s)  AP(%)  OL(%)  NSA   rho   sigma(m/s)  SNR(dB)
1     5.21      1.80    0      54   0.98    0.031     13.80
2     6.04      3.60   30.16   39   0.98    0.015     12.86
3     6.11      3.40   64.21   80   0.95    0.009      9.99
4     5.41      4.90   65.05   52   0.70    0.015      2.95
```

**Item 4 is answered by measurement.** "Long dwells may need to be deliberately
truncated, and never have been" -- their observation time is 5.2 to 6.1 seconds,
not 33. Item 4 derived T = 2.1-4.7 s from the published aperture fractions; this
lands just above that range, from the other direction.

**ITEM 13'S CLAIM ABOUT THE LITERATURE IS TOO BROAD.** It states "The published
campaigns use ~99% overlap because they read pixel phase". This campaign uses 0
to 65 percent and reads pixel POSITION. That does not withdraw item 13's
arithmetic, which is about this project's own correlation estimator and stands;
it withdraws the generalisation about what "the published campaigns" do.

**`validate`'s aperture-fraction warning is mis-set.** It warns below 4.5 percent
on the grounds that "published validation sits at 4.5-7.6%". Their best result,
rho = 0.98, is at **1.8 percent**. Not changed here, because the right lower
bound is a separate question from the fact that the current one is wrong.

### What they do NOT establish, and it is this project's open problem

They never adjudicate. The targets are corner reflectors in *"open and low-clutter
environments... to minimize interference"*, so the position is known and no
detection decision is required; performance is reported as Pearson correlation,
standard deviation of the difference and SNR against the ground sensor.

So the accept/reject rule item 31 is stuck on is not solved in this paper, and a
CFAR formulation of it -- which is the obvious next idea -- is a proposal for this
project rather than established practice in this field.

### And the ground truth this project calls non-existent does exist

`DATASETS.md` states that no collect with synchronous ground truth is in any open
archive, and every null in this file is built on that. This campaign has a corner
reflector on a shaker with an LVDT, on **Umbra** data -- a provider already listed
as open CPHD and SICD. Whether these particular acquisitions are obtainable is
unknown and worth asking; it is a request to make rather than a dead end to
assume.

---

## 33. The static null is not comparable to the scene it stands in for, so item 31's gate cannot adjudicate Giza

Item 31 made `--null-static` the verdict. This is the first time it has been run
on the Giza collect, and it says the instrument is miscalibrated rather than the
threshold. `runs/giza/2026-08-03-null-distribution/`.

Uninjected Giza, 96 m grid, otherwise item 30's settings, with eight simulated
motionless realisations through the identical chain:

```
  static trial 1/8: prominence 31.4      5/8: 36.0
              2/8: 33.5                  6/8: 31.1
              3/8: 38.7                  7/8: 38.8
              4/8: 24.0                  8/8: 25.1
  mean 32.3, sd 5.3, worst 38.8
  detection 16.6 is 0.51x the mean and 0.43x the worst
  8 of 8 reached it -- empirical p = 1.0000
```

**The real scene scores 16.6 and every simulated motionless scene beats it, most
by a factor of two.** The same figure on the 256 m grid is 17.48 (item 31), so
the real value is stable across grid size and it is the null that is displaced.

### It would refuse every known true positive

Item 30's five injected runs measured 25.60 to 31.37. Against this null
distribution:

```
  injected run   nulls reaching it   empirical p
     0.098            6 of 8            0.78
     0.130            5 of 8            0.67
     0.163            5 of 8            0.67
     0.196            6 of 8            0.78
     0.228            6 of 8            0.78
```

Every one refused, at p far from any usable alpha. The injected measurements sit
INSIDE the null's range, while the real motionless scene sits BELOW it. A gate
reading this null does not separate signal from nothing; it separates the
simulator from reality.

### The cause is the one items 21 and 23d already identified

`rs_simulate_static_like()` scatters point targets over the real geometry. Item
21 records that "the real scenes are behaving exactly as speckle should, and the
synthetic fixtures are sparse in a way no real scene is", and item 23d that the
simulator's scatterers are isotropic where real bright returns are
aspect-selective. A sparse scene of isolated bright scatterers gives each window
a sharper spectral peak than distributed desert clutter does, so its prominence
runs high. The null inherits the modelling gap the fixtures have, which is the
same root cause as the `D_A` gap rather than a separate defect.

**Item 27 fixed this function so its output FOCUSES. Nothing checked that its
statistics match a real scene, and they do not.**

### What this changes

*Item 31's implementation is not withdrawn, its instrument is bounded.* Gating on
a null is right -- item 11's argument is untouched, and no scene-wide fraction
would do better. What fails is this particular null on this collect.

*The real uninjected run is a usable null and the simulated one is not, here.*
At 16.6-17.5 against injected runs at 25.60-31.37, the real control separates
every one of the five. Item 30's sweep already produced it. That is n = 1 and
cannot support a false-alarm probability -- a single null's smallest attainable
p-value is 1/2 -- but it is the right n = 1, and repeated uninjected runs over
different patches of the same collect would build a distribution from real
clutter at a cost the simulator was supposed to avoid.

*Do not tune alpha against this.* The obvious next move after item 31 was to gate
on the conformal p-value at an explicit alpha instead of on `nge > 0`. That is
still the right shape, and doing it now would refine a decision rule reading a
biased instrument. Fix the null first, or change which null is used.

### What would settle the mechanism

Compare the two directly rather than inferring: the per-window prominence
DISTRIBUTION of a simulated static scene against that of the real uninjected
collect, at one operating point. If the simulated windows are systematically
sharper, that is the sparse-scatterer explanation confirmed, and
`rs_simulate_static_like()` needs a scatterer density and an aspect response
matched to the scene rather than a plausible-looking one. `rs_sim_scene_aspect()`
already exists for the fixtures (item 24) and has no counterpart in the null
generator.

---

## 34. The null generator matched to real clutter: density derived, and it inverts item 33

Item 33 measured `rs_simulate_static_like()` producing prominences twice the real
Giza desert's, refusing every known true positive. The cause is quantitative and
was hiding in a magic number: **the default was 400 scatterers whatever the scene
covered**, which at the Giza operating point is

```
  scene 9216 m^2, sub-look cell 0.176 m^2 -> 52,700 resolution cells
  400 scatterers  =  0.0076 per cell  -> 99.2 percent of cells EMPTY
```

A field of isolated bright points, not clutter. Each tracking window held a
handful of them, and an isolated point gives a far sharper spectral peak than
distributed desert does.

### Swept on the real collect

Three densities, three trials each, everything else held
(`runs/giza/2026-08-03-null-density/`):

```
  scatterers   per cell   null mean prominence
         400     0.0076          34.6
        4000     0.0764          25.7
       20000     0.3819          13.3        real scene: 16.6
```

Monotone, and it brackets the real scene between the last two. Log-interpolating
puts the crossing at about 13,000 scatterers, **0.25 per resolution cell**, which
is where `RS_NULL_SCATTERERS_PER_CELL` now sits. `mmotion` derives the count from
each collect's own geometry, prints the achieved per-cell density every run, and
warns below half of it -- a check that would have caught item 33 automatically.

**It is not the fully-developed-speckle density.** That needs of order ten per
cell and forty times the cost. What decides prominence is only that no single
scatterer dominates a window, and at 0.25 per cell a 32 m window holds some
fifteen hundred scatterers, which is already enough.

### Verified at the derived default, and it inverts item 33's verdict

Five trials at 13,091 scatterers:

```
  17.1  13.2  23.8  20.7  13.9      mean 17.7, sd 4.0
  real motionless scene 16.6        the null is now 1.07x it, was 1.95x
```

The null sits **on** the scene it stands in for rather than a factor of two above
it. Three of five reach the real motionless scene, p = 0.67, and the tool refuses
-- which is the right answer for a scene with nothing in it.

**And item 30's five known true positives now pass.** Against this null
distribution, none of the five realisations reaches any of them:

```
  injected   before (0.008/cell)        after (0.25/cell)
    28.71    6/8 reach it  p=0.78 REFUSED    0/5  p=0.17  adjudicated
    31.25    5/8           p=0.67 REFUSED    0/5  p=0.17  adjudicated
    31.37    5/8           p=0.67 REFUSED    0/5  p=0.17  adjudicated
    29.88    6/8           p=0.78 REFUSED    0/5  p=0.17  adjudicated
    25.60    6/8           p=0.78 REFUSED    0/5  p=0.17  adjudicated
```

Every true positive refused before, every one adjudicated after, with the
motionless scene still correctly refused. That is the first time this project's
null control has separated signal from nothing on real data.

### Four things this does not establish

*The grids differ.* The null distribution above is measured on a 96 m grid; item
30's injected runs were on a 256 m one. The real scene's own prominence is stable
across the two -- 16.6 and 17.48 -- but the null's grid dependence is untested,
so the comparison in that table is indicative rather than a measurement.

*One scene.* 0.25 per cell is calibrated on Giza desert. Whether it matches a
bridge deck, a waterway or an urban scene is untested, and the quantity to
compare is always the same: the null's prominence distribution against that
collect's own uninjected value.

*Five trials cannot support a false-alarm probability.* p = 0.17 is the smallest
value five nulls can produce.

**DONE, once the instrument was trustworthy.** The gate read `nge > 0` while
printing the conformal p-value beside it and never using it, so its strictness
was an accident of trial count: M = 1, 5 and 30 with no null reaching the
measurement all passed identically, at p of 0.500, 0.167 and 0.032. The first
stamped a verdict on evidence whose p cannot go below one half however clean the
result. It now gates on `p <= --null-alpha`, default 0.05, and REFUSES TO
ADJUDICATE when M cannot reach that -- `p_min = 1/(M+1)`, so 0.05 needs nineteen
trials and 0.01 needs ninety-nine. Four outcomes, all exercised before commit: no
null; a null too small to decide, which names the trial count required; an
adjudicated pass; and an adjudicated refusal.

*The mechanism is inferred, not isolated.* Density is shown to move prominence
onto the real value. That the remaining difference is speckle development,
aspect dependence (item 23d) or something else is not separated here.

### A figure defect found while reading the same runs

`PREFIX_spectrum_mm.png` was always labelled "VELOCITY, MM/S", and on the phase
route reached it by multiplying the DISPLACEMENT spectrum by `2*pi*f`. The
arithmetic is right -- velocity is the derivative, so that is exactly the factor
in amplitude -- but the figure then plotted a TILTED curve while marking the bin
the UNTILTED spectrum selected. They coincide only when a tone dominates; on a
scene without one the tilt is all there is, the curve rises with frequency, and
the marker sits off the visual peak with nothing saying why.

Both quantities are now written, each converted correctly from whichever
observable was measured and each labelled for what it is:
`PREFIX_spectrum_mm.png` in millimetres and `PREFIX_spectrum_mms.png` in
millimetres per second. The observable's own figure carries the marker at its own
peak, because the selection reads that same spectrum; the derived one says in its
title that a tilt can move its maximum.

---

## 35. ADJUDICATED: a real-data measurement a proper null control could not reproduce

Items 30 to 34 are the two halves of one failure, fixed separately. Item 30
extracted five injected frequencies from the Giza collect and could not report
any of them; item 31 found the frequency had been right all along and the
scene-wide gate had discarded it; item 33 found the null control that replaced
that gate was itself unusable; item 34 fixed it. This is the first run with both
halves working. `runs/giza/2026-08-03-alpha-verdict/`.

0.163 Hz injected at 2 mm into the real Giza collect, 19 null trials -- the
minimum for alpha = 0.05, since `p_min = 1/(M+1)`:

```
  null (13091 scatterers, 0.250 per cell)   min 12.0  mean 17.0  sd 3.6  worst 23.8
  measurement                               0.163 Hz, prominence 32.0
                                            backed by 9 windows, largest block 9
                                            1.88x the null mean, 1.35x the worst
  0 of 19 reached it                        empirical p = 0.0500

  ADJUDICATED: p = 0.0500 <= alpha 0.05
```

**The reported frequency is the injected one**, from `rs_spectrum_best_window()`
-- the policy `mmotion` actually prints -- with the verdict coming from the null
rather than from cross-window agreement.

### Four limits, none of them small

*It is a POSITIVE CONTROL, not a detection.* The target was placed, at twenty
times the median non-zero sample magnitude, and its window was known. Item 30's
distinction is untouched: finding an unknown target needs a selection policy over
the scene, and nothing here tests one.

*p = 0.0500 is the weakest possible pass.* Nineteen trials is the minimum for
this alpha, so a clean sweep returns exactly alpha and a single null reaching the
measurement would have given 0.10 and failed. The margin in PROMINENCE is
comfortable -- 32.0 against a worst null of 23.8 -- but the p-value has none, and
a stronger claim needs more trials rather than a better result.

*The null density is calibrated on this scene.* Item 34 set 0.25 scatterers per
cell so the null's prominence matches Giza's uninjected value, so adjudicating a
Giza measurement against it is IN-SAMPLE. Not circular in the fatal sense -- the
calibration target was the motionless scene and the test is of an injected one --
but the constant has not been shown to transfer, and a verdict on another collect
rests on that until it is.

*One frequency, one amplitude, one grid.* Nothing here bounds sensitivity. The
sharpest next measurement is an amplitude sweep downward until adjudication
fails, on this same collect, which needs no new data.

---

## 36. Umbra and ICEYE read correctly, and the SGN override is confirmed right to be per-vendor

`DATASETS.md` recorded both as untried, with a named risk: item 3's SGN override
is keyed on `CollectorName`, so a non-Capella product takes the standard branch,
and if it were mislabelled the same way the image would come back mirrored.
Tested rather than assumed.

### Umbra: reads correctly with no override, verified against the vendor's own image

`2023-09-12-02-36-33_UMBRA-04_CPHD.cphd`, Panama Canal, 0.84 GB. The metadata
answers the question before the imagery does:

```
  CollectorName      Umbra-04
  SGN                -1            <- declared, and honestly
  SignalArrayFormat  CF8           <- float32, where Capella ships CI4
  NumBytesPVP        240           <- 30 words, where Capella uses 33
  AmpSF              ABSENT from the PVP
  9693 vectors x 7218 samples, SPOTLIGHT, dwell 1.991 s
```

**Umbra declares `SGN = -1`, which is what Capella's override produces.** So both
end up taking the inverse FX-to-delay transform, and Umbra gets there by being
labelled correctly rather than by a vendor exception. The reader read it, 9680
usable pulses of 9693, and focused it in 17 s.

**Checked the way item 3 says this must be checked -- against imagery.** The
focused image is the Panama Canal: waterway, lock chambers, basins, sharp
speckle. Compared against Umbra's own GEC for the same acquisition, the
asymmetric features fall on the same side of the canal in both -- the light-toned
basins east of the channel, the built-up strip west of it, the locks in the same
place. **A range mirror would have swapped those.** Not mirrored.

That is the first evidence that keying the override on the collector, rather than
inverting globally, was the right call: item 3 argued a conformant product from
anyone else should still read correctly, and one now does.

*The check is qualitative.* Two images at different scales and projections,
compared by eye on distinctive features. A correlation against a resampled GEC
would be stronger and has not been done.

### ICEYE: six CPHD in the open archive, and they include DWELL modes

Of 374 items in the open catalogue, **six carry a CPHD asset**:

```
  ICEYE-X47  dwell-precise  inc 25.2  Houston      2026-05-06  19.4 GB
  ICEYE-X38  dwell-precise  inc 26.2  Vandenberg   2026-03-14
  ICEYE-X38  dwell-precise  inc 27.6  Vienna       2026-04-02
  ICEYE-X49  dwell-fine     inc 36.9  Mexico City  2026-03-17
  ICEYE-X56  spot-fine      inc 30.3  Paris        2025-10-28
  ICEYE-X50  spot-fine      inc 40.9  Bratislava   2025-10-27  16.5 GB
```

**"dwell-precise" and "dwell-fine" are long-stare modes**, which is the
acquisition this method wants and which no other open provider labels as such.
Bratislava is the site this file already listed without knowing a CPHD existed
for it.

The Houston dwell-precise screens as:

```
  CollectorName ICEYE-X47   SGN -1   CI4   AmpSF PRESENT   ModeType EXPERIMENTAL
  dwell 15.345 s, 100802 pulses x 48000 samples, incidence 25.2 deg, 580.1 km
  phase floor 0.2017 mm per look -- the best of any collect screened here
```

`SGN = -1` again, honestly declared, so the same reasoning applies. `AmpSF` is
present where Umbra omits it, and item 21's handling covers both.

**It FAILS its observable band for a 2 Hz target, and the reason is the dwell.**
At the published aperture fractions the band reaches 0.905 Hz at 3.6 percent and
1.810 Hz at 1.8 percent, both short of 2 Hz -- because 15.3 s makes each sub-look
long. Note that the observation ratio at 1.8 percent is eta 0.552, squarely
inside the published 0.39-0.69. The collect is not the problem; the untruncated
dwell is, which is item 4 again and is now reachable with `--pulse-start` and
`--max-pulses`.

### Neither has been run through the measurement chain

Both read and one focuses. Nothing here has tracked, taken a spectrum, or
adjudicated on either provider, and the PVP layouts differ from Capella's in
ways the reader handles by reading offsets from the XML rather than assuming
them. Treat them as read-verified, not measurement-verified.

## 37. The amplitude sweep found a defect instead of a sensitivity bound

Item 35 adjudicated a 2 mm injection at p = 0.05. The obvious next question is
how far down that goes, so the amplitude was swept with everything else held at
item 35's settings. `runs/giza/2026-08-03-amplitude-sweep/`.

The null trials synthesise their own static scenes and never see the injection,
so item 35's 19-trial distribution applies unchanged to every amplitude. Reusing
it rather than regenerating it made the sweep 5 x 5 minutes instead of 5 x 90.
Its maximum is 23.8, so clearing 23.8 is what p = 0.05 means here.

```
amp mm | injected window 8    | selected      | vs null max 23.8
   2.0 |  0.163 Hz  prom 32.0 | win 8 @ 0.163 | ADJUDICATES   correct
   1.0 |  0.033 Hz  prom 31.4 | win13 @ 0.033 | ADJUDICATES   WRONG ANSWER
   0.5 |  0.033 Hz  prom 47.4 | win13 @ 0.033 | ADJUDICATES   WRONG ANSWER
  0.25 |  0.033 Hz  prom 54.0 | win13 @ 0.033 | ADJUDICATES   WRONG ANSWER
 0.125 |  0.033 Hz  prom 56.0 | win13 @ 0.033 | ADJUDICATES   WRONG ANSWER
```

**Below 2 mm the tool does not stop answering. It answers wrongly, with a
confidence that rises as the injection weakens** -- 32.0 to 56.0, every one of
them clearing the threshold item 35 called an adjudication.

0.033 Hz is bin 1: 128 looks at dt = 0.2399 s is a 30.71 s record, df = 0.0326 Hz.
One cycle across the whole dwell. Windows 8 and 13 agree to within 0.1
prominence at every amplitude, which is one common-mode artefact rather than two
findings.

### Every gate this project has endorses it, and more strongly the wronger it gets

```
amp mm | windows in bin 1 | quality | D_A at window 8
   2.0 |       1/25       |  0.419  |  0.581
   1.0 |      10/25       |  0.675  |  0.325
   0.5 |      10/25       |  0.806  |  0.194   <- passes Ferretti 0.25
  0.25 |      10/25       |  0.863  |  0.137   <- passes
 0.125 |      10/25       |  0.884  |  0.121   <- passes
```

`D_A` falls because a scatterer that barely moves has a stable amplitude, so the
persistent-scatterer criterion of items 19-20 is satisfied **by the target
failing to vibrate**. Quality rises for the same reason. Prominence, quality,
`D_A`, the cull and the null control all agree on the wrong answer.

**The null could not have caught this**, which is the part that generalises. Its
trials are synthetically static, contain no bright dominant scatterer to impose
a common trend, and topped out at 23.8 against an artefact reaching 56. Item 11
said a null control is what catches common-mode artefacts. That is true only of
artefacts the null's own model can produce.

### The code predicted this exactly and stopped one bin short

`src/core/spectrum.c:30` already argued it: removing the mean alone "makes every
window report the same spurious dominant frequency of one bin width, which looks
like a measurement and is not". `rs_spectrum_compute_band()`'s header goes
further -- a curved trend "piles its energy into the first two or three bins" --
and names the band floor as the remedy. The defences shipped were a linear
detrend, which removes a straight line exactly and leaves the curvature, and
exclusion of **bin 0**. `--fmin` existed and defaulted to `0.0`.

### The fix, and what it does not fix

`RS_SPECTRUM_LEAKAGE_BINS` is 3 and is not a tunable. A Hann window's main lobe
is +/-2 bins, so bins 1 and 2 carry the skirt of whatever sits at zero and are
not separable from it at any SNR. `--fmin` can raise the floor, never lower it,
and passing 0 no longer restores the old behaviour.

Re-swept with a floor -- `runs/giza/2026-08-03-amplitude-sweep-fmin/`, at
`--fmin 0.098`, which is `ceil(0.098/0.0326) = 4` bins rather than the default 3:

```
amp mm | 2.0  | 1.0  | 0.5  | 0.25 | 0.125 | 0.0625
freq   | .163 | .163 | .163 | .163 | .163  | .163     <- all correct
prom   | 38.5 | 40.7 | 41.6 | 42.9 | 45.3  | 48.9
```

**Correct at every amplitude down to 0.0625 mm**, a 32-fold improvement on the
1-2 mm the unfloored sweep bottomed out at.

**But prominence still rises as the signal weakens**, which no real detection
does, so this is not yet a sensitivity curve. Two candidate reasons, neither
measured: 2 mm is a 0.78 rad phase swing, near item 14's lambda/4 wrapping
limit, so the strongest injection may be the most degraded; and a weaker target
leaves a quieter spectrum for a fixed peak to stand out against.

**And the trend is relocated, not removed.** The regression test built for this
makes it plain: a trend-only window reports the first admissible bin at
prominence 28.09, against 19.67 for a window holding a genuine tone under the
same trend. Excluding bins moves where a trend is reported, not whether it wins.
A window with nothing in it still has to answer somewhere, and prominence still
prefers it.

### What this costs item 35

**Item 35's 2 mm result stands as measured.** The selected peak was the injected
one, in the injected window, at the injected frequency, and 19 controls failed
to reach it.

**What it loses is generality.** The gate that passed it would have passed a
wrong answer at any smaller amplitude with a higher score. p = 0.05 was a
statement about that operating point, not about the method. Item 35's threshold
also needs re-measuring under the floor, since removing bins moves the null too.

### Recalibrated, and the control that decides it

Removing bins moves the null too, so item 35's 23.8 does not carry over. 19 fresh
trials under the floor: min 11.5, mean 16.3, **worst 22.9** -- so the threshold
barely moved (-0.9) and it is the measurement that did.

```
 amp mm |  prom | x worst null | verdict
    2.0 |  38.5 |        1.68x | p = 0.0500  ADJUDICATED
    1.0 |  40.7 |        1.78x | p = 0.0500  ADJUDICATED
    0.5 |  41.6 |        1.82x | p = 0.0500  ADJUDICATED
   0.25 |  42.9 |        1.87x | p = 0.0500  ADJUDICATED
  0.125 |  45.3 |        1.98x | p = 0.0500  ADJUDICATED
 0.0625 |  48.9 |        2.14x | p = 0.0500  ADJUDICATED
```

**The sweep still never found a floor**, and 0.0500 is the smallest p that 19
trials can express, so all six sit pinned at the bound rather than approaching
it. The sensitivity limit is somewhere below 0.0625 mm and this design cannot
see where.

**THE REAL UNINJECTED SCENE, under the same floor**, is the control that was
missing:

```
strongest peak in window 6: 0.130 Hz, prominence 17.9
```

0.130 Hz is **bin 4 -- the first admissible bin**. The trend did not go away
when its bins were removed; it moved to the edge of the exclusion and reported
from there. A sufficiently strong trend leaks or relocates into the first
permitted bin no matter where the boundary is drawn, because the boundary
removes bins and not energy.

It is refused -- 17.9 against the null's worst of 22.9 -- but **by 1.28x, and by
the null rather than by the floor**. The regression fixture's trend-only window
reaches 28.09, above this null. So the floor is necessary and is not sufficient,
and the thing actually standing between this tool and a false positive on a
motionless desert is a synthetic null with very little margin.

(28.09 and 22.9 come from different scenes. Prominence is dimensionless -- peak
against the mean of the rest -- but it is scene-dependent, so the comparison is
directional, not a measured crossing.)

### What the literature does and does not offer here

Searched rather than assumed, per this project's own rule. The closest published
work is Vattulainen et al., *Assessment of Spaceborne SAR Micro-Motion
Measurement for Vibration-Based SHM*, IEEE Access 14 (2026) 6045-6062 -- the same
SPOT sub-pixel-offset-tracking route this project's correlator implements, on
Capella data, against synchronous accelerometer ground truth.

What it establishes: dominant frequency correctly measured in **every** test,
errors 0.009-0.091 Hz, always below the spectral resolution; 16.01 s acquisition
giving df = 0.064 Hz; **smallest confirmed radial RMS displacement 0.10 mm**
(0.81 mm/s RMS radial velocity).

What it does NOT contain, checked by reading it:

- **No detrending of any kind.** The word does not appear. There is no
  linear-trend removal before the transform.
- **No band floor, no minimum observable frequency, no low-bin exclusion.**
- **No null control, no static scene, no false-alarm rate, no significance
  test.** Every test is a known vibrating target with synchronised ground truth.
  The question "is this peak real" is never asked, because the answer is known.
- **Spurious peaks are reported and not explained**: "A spurious third peak
  appears however at 5 Hz, reaching a magnitude comparable to the lower 2 Hz
  component." It is noted and moved past.

Two consequences for this project. First, item 37's artefact is not a known
failure mode being rediscovered -- a paper with no detrending and no low-bin
exclusion would have been fully exposed to it, and its validation design could
not have detected it. Second, **the 0.0625 mm adjudication above is below the
smallest displacement anyone has confirmed**: 0.0625 mm zero-to-peak is about
0.044 mm RMS against their 0.10 mm. Beating a published floor by 2x with a
synthetic tone injected into real phase history is a reason for suspicion about
the injection, not a sensitivity claim. Nothing here has detected a real
vibrating object.

The CFAR literature is extensive but is about **amplitude** detection of targets
in clutter, not about deciding whether a spectral line in a tracked series is
real. No published false-alarm framework for this measurement was found.

### The open half

A null synthesised from a model calibrates only against noise the model
contains. This artefact was invisible to it because the model has no bright
dominant scatterer; the next common-mode artefact will be invisible for its own
reason. The fix that generalises is a null built from the **real** scene --
permuting or phase-scrambling the actual tracked series so the control inherits
whatever the collect does -- rather than simulated beside it. Not attempted.


## 38. The zero-amplitude control: the adjudication was measuring the wrong thing

Item 37 fixed a defect and left the amplitude sweep adjudicating at every
amplitude down to 0.0625 mm. A critique of that result asked for a control this
project had never run: **the identical injection code path at zero amplitude**.
A bright scatterer written into the phase history that does not move. If the
statistic responds to that, it is responding to the injection rather than to
motion. `runs/giza/2026-08-04-scene-null/`.

**It responds to it, more strongly than to any real injection.**

```
--inject-vib "0.163,0.0,20"            <- ZERO amplitude

strongest peak in window 8: 0.098 Hz, prominence 56.3, quality 0.895
  backed by 9 windows, largest touching block 9
```

| run | reported | prominence | scene-null z |
|---|---|---|---|
| **zero amplitude (negative control)** | **0.098 Hz** | **54.7** | **8.23** |
| uninjected, no injection at all | 0.130 Hz | 17.6 | 1.56 |
| 0.0625 mm | 0.163 Hz | 46.7 | 6.38 |
| 0.125 mm | 0.163 Hz | 44.4 | 5.99 |
| 0.5 mm | 0.163 Hz | 39.0 | 5.34 |
| 2.0 mm | 0.163 Hz | 38.0 | 5.28 |

Against the recalibrated `--null-static` worst of 22.9 the motionless bright
target ADJUDICATES at 2.5x the bar. Prominence ranks it first. The new
scene-derived null ranks it first. **Every amplitude statistic here prefers the
target that does not move.**

**Only the reported FREQUENCY separates them.** Zero amplitude answers 0.098 Hz,
bin 3 -- the band floor's own edge, item 37's trend arriving at the first door
it is allowed through. Every real injection answers 0.163 Hz, the injected
value. Nothing about the peak's SIZE carries the distinction; only its POSITION
does.

### What this costs item 35

Item 35 adjudicated a 2 mm injection at p = 0.05 against 19 synthetic null
trials and is the result this project has been quoting. A zero-amplitude
injection through the same code path would have adjudicated too, and by more.
**That p-value measured "a bright scatterer was added to the phase history", not
"the scatterer vibrated."**

It does not touch the frequency results. `rs_track_fit()`'s slope-and-rms bar
(item 30) scores WHICH frequency comes back, and the zero-amplitude control does
not return 0.163 Hz. What item 35 established is **recoverability**: a frequency
put in comes back out. **Detectability** -- deciding something moved without
already knowing the answer -- was never measured, and the statistic that claimed
to measure it responds to the injection machinery.

That distinction is also what the Vattulainen validation (item 37) lacks, for
the same structural reason: every published test knows where the target is.

### The scene-derived null, built here, and what it does and does not do

`rs_spectrum_scene_null()` scores every window against the rest of its own
scene: matched on `D_A`, with a guard ring of `ceil(win_az/stride_az) - 1` so a
target is not used as its own reference, taking the maximum over windows so the
look-elsewhere cost is inside the statistic.

Four time-domain surrogates were considered and rejected, each circular or
benign, and the reasoning is in the header: random permutation destroys the
trend and recreates the too-benign background that made `--null-static` fail;
phase scrambling preserves the periodogram that prominence is computed from;
a circular shift preserves the spectrum exactly; a residual bootstrap inherits
only what the trend model left, and the trend is the disputed term. The null
must keep the low-frequency nuisance while destroying the localisation, and a
time-domain surrogate of one series cannot separate those. Space can.

It earns its place:

- it refuses the real uninjected scene at z 1.56, against a family-wise maximum
  of 2.53 measured over all 225 windows of a real motionless Giza scene;
- injected targets at 256 m score 5.86 to 9.84 at the injected window;
- in the test fixture it separates a localised target from a common-mode
  artefact, z 38.75 against 2.63 -- the case `--null-static` structurally cannot
  see;
- it costs nothing and carries the collect's own trends by construction.

**It does not fix the negative-injection problem**, because a bright static
scatterer genuinely IS unusual for its scene. A spatial null answers "is this
window unusual here" and correctly answers yes. The question needing an answer
was "did it move".

One cost is recorded in the header rather than hidden: matching gives each
candidate its own reference set, so z is comparable across windows only when
those sets are. Measured while building the test -- with a `D_A` pattern
uncorrelated with position, a window carrying weak spill-over outscored the
target, z 51.9 against 34.6, purely because its matched set had a scale of 0.15.

### What would actually answer it

A paired increment at the nominated frequency,
`P_injected(f0) - P_zero_amplitude(f0)`, asking whether the injection ADDED
recoverable evidence at f0 rather than whether an already-artefact-bearing window
has a large peak. Built and measured in item 39.

**Any positive control here must now ship its zero-amplitude twin.** A positive
control never run at zero amplitude does not establish that the effect came from
the motion. That is a standing requirement, not a note.


## 39. The paired increment: the scene gains exactly zero and the target gains

Item 38 named the statistic that would separate "a scatterer was added" from "it
moved", and could not compute it: comparing two runs at one fixed frequency
needs the PSD there, and `PREFIX_windows.csv` carried only each window's
dominant peak. Two runs whose strongest peaks sit at different frequencies
cannot be differenced row by row, which is precisely how a motionless target
outscored a moving one.

`rs_spectrum_prominence_at()` and `--probe-hz` supply it. The CSV gains
`probe_psd` and `probe_prominence`, measured at one nominated frequency for
every window, using the same prominence definition as the dominant column so the
two are subtractable. Bins inside the Hann skirt are answerable here, unlike in
peak selection -- naming a frequency is not searching for one -- and the run
header flags it when the probe lands there.

Three runs at 96 m, `--probe-hz 0.163`, differing ONLY in injected amplitude,
each differenced against its zero-amplitude twin. `runs/giza/2026-08-04-scene-null/`.

```
T(w) = probe_prominence(w) - probe_prominence_zero_amplitude(w)   at 0.1628 Hz

0.5 mm vs zero              2.0 mm vs zero
  win  8   T =  +5.15         win 13   T = +51.19
  win 18   T =  +5.06         win  8   T = +51.19
  win 17   T =  +5.06         win 18   T = +51.16
  win 13   T =  +5.06         win 17   T = +51.16
  win 16   T =  +4.89         win 16   T = +49.55
  MEDIAN over 25 windows      MEDIAN over 25 windows
           T =  +0.00                  T =  +0.00
```

**The median window gains exactly nothing.** Running the injection machinery --
rewriting the phase history, adding a scatterer at 20x the median sample
magnitude, refocusing -- contributes zero at f0 across the scene. That is the
term item 38 could not subtract, and it subtracts to zero.

**What gains is a contiguous block of five windows, all reporting 0.163 Hz**, at
`iaz` 1-3 and `irg` 1-3: the injected window and its aliasing ghosts. The 1.0 m
grid cell against this collect's 0.051 m azimuth resolution is the documented
cause -- `mmotion` warns about it on every run -- so the ghosts are expected and
are not independent evidence.

The increment scales with amplitude: 4x the amplitude gives 9.9x the increment,
against the 16x a power-law-in-amplitude-squared would predict. Sub-quadratic,
same order, not investigated.

### The pairing has to be exact, and this is not a small caveat

Differencing the **uninjected** run -- no scatterer at all -- against the
zero-amplitude twin gives T(win 8) = +7.71 and a scene maximum of +18.35.
**Larger than the real 0.5 mm signal's +5.15.** Those two runs differ by the
presence of a bright scatterer, not by its motion, so the difference measures
the scatterer.

The increment is only a statement about motion when the two runs differ in
NOTHING BUT amplitude. Paired against the wrong twin it is worse than useless,
because it looks like a stronger result.

### What this does and does not establish

It closes item 38's specific hole: there is now a statistic on which a
zero-amplitude injection scores zero and a real one scores positively, where
every absolute statistic ranked the motionless target first.

It does not make anything detectable. **The increment requires a zero-amplitude
twin of the same scene**, which exists only when the injection is ours. On a
real collect with a suspected vibrating object there is no twin to subtract, so
this validates the INJECTION EXPERIMENTS and transfers nothing to the field
problem. The recoverability/detectability gap of item 38 is unchanged; what has
been fixed is that the injection experiments can now be trusted to be measuring
motion.


## 40. Blind localisation: the reported window follows the target

The README has said for a long time that "automatically locating an unknown
vibrating target among hundreds of windows remains untested". It was untested
because it was untestable: `rs_simulate_inject_vibrator()` has always taken a
`centre`, but `main.c` passed the grid origin and nothing else, so every
injection this project ever ran landed on the same spot -- the exact centre of
the analysis grid. A policy that always looked at the middle would have scored
perfectly without locating anything.

`--inject-at DX,DY` offsets the scatterer in metres along the grid axes. The
library needed no change. `runs/giza/2026-08-04-blind-localisation/`.

Five placements, 2 mm at 0.163 Hz, 96 m grid giving a 5x5 window grid at 16 m
per window. **Ground truth is the injection geometry**: an offset of `(dx, dy)`
moves the target `(dx/16, dy/16)` windows from where a zero-offset injection
lands. Scored on the exact window.

```
 offset m |          expected |      best_window |       scene null
 -32, -32 | (0,1)  CLIPPED    | win  1 (0,1) hit | win  1 (0,1) hit
 -16, +16 | (0,4)             | win  4 (0,4) hit | win  9 (1,4) MISS
  +0, -32 | (1,1)             | win  6 (1,1) hit | win  6 (1,1) hit
 +16, +32 | (2,4)  CLIPPED    | win 24 (4,4) MISS| win 14 (2,4) hit
 +32,  +0 | (3,3)             | win 18 (3,3) hit | win 18 (3,3) hit

EXACT window: best_window 4/5, scene-derived null 4/5
```

Chance of naming one window in 25 is 4 percent. Both are far above it, they fail
on different placements, and **the plain prominence policy does as well as the
scene-derived null built in item 38** -- which is worth saying plainly, because
the null was the more sophisticated instrument and it did not win.

### A scoring mistake worth recording

The first scoring used "the window with the largest probe prominence at
0.163 Hz" as ground truth, and gave best_window 4/5 against the null's 3/5 with
different placements failing. It was wrong. The aliasing ghosts item 39 found --
five contiguous windows all carrying the injected frequency, from a 1.0 m cell
against a 0.051 m azimuth resolution -- mean that the largest probe response
finds a ghost as readily as the target. Ground truth has to come from the
injection geometry, which is independent of the measurement.

### The absolute position is NOT verified -- and the numbers above are WRONG

**Item 41 supersedes this section and the table above it.** The scoring used the
window a zero-offset injection reports as the origin of the mapping. That window
is itself one off, so "4 of 5 exact" is measuring against a displaced reference.

Rescored against the geometry, measured independently by differencing two
focused images: **0 of 5 exact for both policies, 5 of 5 within one window.**
Localisation is good to one window and systematically not better. See item 41
for why, which is neither a height assumption nor a geolocation offset nor an
off-by-one -- the injection lands where it is asked to, sub-pixel.

### And it is still not detection

Five points, one collect, one frequency, and one amplitude at the strong end of
the sweep -- item 37 showed the reported window moving with amplitude even at a
fixed position, so nothing here says this survives at 0.125 mm. Two of the five
placements put the target outside the window grid, which makes them weaker tests
than interior ones; that is a design error in the placements.

Most importantly the policies were searching a scene that certainly contains a
loud injected target. Locating a target known to exist is strictly easier than
deciding whether one does, and the run that matters -- a scene where nothing is
known to move -- still returns a null.


## 41. The 16 m offset chased: the geometry is exact, the selection is not

Item 40 left a systematic one-window offset unexplained and listed three
candidates -- a height assumption, a geolocation offset, an off-by-one in the
window convention. **All three are wrong.**
`runs/giza/2026-08-04-offset-chase/`.

### The target lands exactly where it is asked to

Two `focus` runs over the same collect and grid, identical but for
`--inject-vib`, differenced. Independent of tracking, windows, spectra and every
selection policy:

```
max change 1.584e+04 at azimuth px 47, range px 48
grid origin is at pixel 47.5 in both axes   (focus.c:193, (n-1)/2 with n = 96)
  -> the target is -0.5 m azimuth, +0.5 m range from the origin
```

Sub-pixel exact: the origin lies between pixels 47 and 48 and the energy
straddles them. `rs_grid_t`, `--inject-at`, the backprojector and the window
convention all agree. Window centres are at `15.5 + 16k`, so window (2,2) is
centred on 47.5 and IS the target's window.

### The selection picks a neighbour, and always will

Per-window prominence, zero-offset 2 mm run:

```
      irg=0     1       2       3       4
iaz=1  14.8   38.6    38.6    39.1     5.5
iaz=2  14.8   38.6   [38.6]   39.1    17.6     <- (2,2) is the target's window
iaz=3   8.8   38.5    39.1    39.1    15.5
```

Six windows lie between 38.5 and 39.1 -- **1.5 percent apart** -- and thirteen
pairs are BIT-IDENTICAL across `dominant_hz`, `prominence`, `quality` and
`probe_psd` simultaneously.

That is not overlap similarity, it is the same series. At `win 32` on
`stride 16` the windows overlap by half; they all contain the injected dominant
scatterer; and **the phase estimator tracks one dominant scatterer's phase**,
which is its documented precondition (item 15). Several windows tracking the
same physical scatterer produce the same displacement series, hence the same
spectrum and the same prominence.

**The correctly-centred window scores LOWEST of the cluster**, 38.56 against
39.14. Which neighbour wins turns on 1.5 percent, and item 37 already recorded
that winner moving with injected amplitude.

The image is not the cause: all 96 azimuth rows and all 96 range columns of the
focused scene are distinct, with no periodicity at any shift.

### What localisation is actually worth

```
 offset m | true win |  best_window  |  scene null
 -32, -32 | (0,0)    | (0,1)  d = 1  | (0,1)  d = 1
 -16, +16 | (1,3)    | (0,4)  d = 1  | (1,4)  d = 1
  +0, -32 | (2,0)    | (1,1)  d = 1  | (1,1)  d = 1
 +16, +32 | (3,4)    | (4,4)  d = 1  | (2,4)  d = 1
 +32,  +0 | (4,2)    | (3,3)  d = 1  | (3,3)  d = 1

EXACT 0/5 and 0/5.   WITHIN ONE WINDOW 5/5 and 5/5.
```

Never exact, never worse than one window. **Item 40's "4 of 5 exact" is
withdrawn**; it was scored against a reference that was itself displaced. The
correct claim is that this tool locates a strong injected target to within one
window -- 16 m at these settings -- and systematically no better.

That bound is a property of the window geometry rather than of the estimator: at
50 percent overlap a target at one window's centre also sits inside four
windows, and nothing in the current selection prefers the one it is centred in.
A centroid over the tied cluster, or a preference for the window whose centre is
nearest the energy, would plausibly recover the missing window. Not attempted.

### The harness caught this project again

The first attempt built the command as `EXTRA="--inject-vib ..."` and passed
`$EXTRA` unquoted. **zsh does not word-split unquoted parameters**, so it arrived
as a single argument, the option was never matched, and the differenced images
showed `max |difference| = 0` -- which reads as "the injection never reaches
focus", a dramatic and entirely false finding. `CLAUDE.md` documents this exact
trap because it already cost a wrong diagnosis about `--max-pulses`. Documenting
it was not enough to prevent it.


## 42. The centroid: item 41's 16 m bound becomes 0.1 m

Item 41 measured localisation at exactly one window -- 5 placements of 5, never
better, never worse -- and identified the cause as geometric: at 50% overlap the
target sits in four windows carrying the same evidence, and the one it is
CENTRED in scores lowest. It proposed a centroid over the agreeing cluster and
did not try it. `rs_spectrum_centroid()` tries it.
`runs/giza/2026-08-04-centroid/`.

**No tolerance parameter.** The obvious construction -- "windows within x
percent of the peak" -- needs a threshold nobody can derive. The cluster is
instead the 4-connected block whose dominant frequency matches the seed's to
within half a bin, which is `rs_spectrum_block_at()`'s existing notion of
agreement, so membership comes from the measurement. The weight is prominence
ABOVE THE SCENE MEDIAN, floored at zero: raw prominence lets the background pull
the answer toward the middle of the cluster's bounding box. Measured, raw gives
0.463 windows of error and the excess 0.403; squaring the excess gives 0.405, so
the plain excess is used.

Five INTERIOR placements, truth from the injection geometry verified in item 41
by differencing two focused images:

```
 offset m |        truth |        argmax |           centroid
   +0, +0 | (2.00,2.00)  |  (3,2)  1.00  | (2.00,2.01) 0.01
  -16,-16 | (1.00,1.00)  |  (0,2)  1.00  | (1.00,1.01) 0.01
  -16, +0 | (1.00,2.00)  |  (0,3)  1.00  | (1.00,2.01) 0.01
   +0,+16 | (2.00,3.00)  |  (1,4)  1.00  | (2.00,3.01) 0.01
  +16,-16 | (3.00,1.00)  |  (2,2)  1.00  | (3.00,1.01) 0.01

  argmax   mean 1.000 windows = 16.0 m, worst 1.00
  centroid mean 0.008 windows =  0.1 m, worst 0.01
```

**A factor of 125, and finer than the 1.0 m grid cell.** A centre of mass over
nine windows is no more limited by the window spacing than a star centroid is by
the pixel pitch. Item 41's bound was a property of reporting an integer index,
not of the measurement.

Item 40's placements were all at +-32 m, which puts the target ON the grid
boundary with half its footprint off the grid; that truncation is the whole of
the 0.403 residual those runs showed. The interior placements above have no such
bias.

The systematic +0.01 in range across all five is unexplained. It is a sixth of a
metre and has not been chased.

### The flag had to be redefined, which is itself the finding

`clipped` first meant "the agreeing cluster touches the grid edge". Measured,
that fires on FOUR of these five placements -- every one accurate to 0.01 -- and
would have warned about nothing. Restricting it to the weight-bearing windows
fires on the same four, with 33-55% of the weight at the edge.

Cluster reach is not the thing. What biases a centroid is the TARGET sitting at
the boundary so half its own footprint is missing, so the flag now tests whether
the CENTROID lies within one window of the edge. That separates item 40's +-32 m
placements from these, which is what a warning has to do.

### Still not detection

The cluster is grown from a window the caller has already chosen to believe, and
a scene where nothing is known to move still returns a null. One collect, one
frequency, five placements, all at 2 mm -- the strong end, where item 37 showed
the reported window moving with amplitude. This needs repeating at 0.125 mm
before 0.1 m localisation is claimed generally.


## 43. Item 42's 0.1 m is amplitude-dependent, and the +0.01 bias explained

Item 42 quoted 0.1 m localisation from five placements **all at 2 mm**. Item 37
had already shown the reported window moving with amplitude at a fixed position,
so the claim needed the weak end before it could stand.
`runs/giza/2026-08-04-centroid-lowamp/`.

```
 offset m |  truth | 2 mm centroid       | 0.125 mm centroid | argmax @0.125
   +0, +0 | (2,2)  | (2.002,2.008) 0.008 | (2.04,2.14) 0.140 | win8  (1,3) 1
  -16,-16 | (1,1)  | (1.002,1.009) 0.009 | (1.04,1.16) 0.160 | win2  (0,2) 1
  -16, +0 | (1,2)  | (1.002,2.008) 0.008 | (1.04,2.13) 0.130 | win3  (0,3) 1
   +0,+16 | (2,3)  | (2.002,3.008) 0.008 | (2.04,3.13) 0.130 | win9  (1,4) 1
  +16,-16 | (3,1)  | (3.002,1.009) 0.009 | (3.03,1.14) 0.140 | win12 (2,2) 1

  centroid @ 2.000 mm : 0.0084 windows = 0.13 m
  centroid @ 0.125 mm : 0.1400 windows = 2.24 m
  argmax   @ 0.125 mm : 1.0000 windows = 16.00 m
```

**A 16x weaker signal costs 17x the error** -- linear in 1/amplitude to within
the measurement. The error is background-limited rather than a fixed bias: what
sets it is the target's excess prominence against the background gradient, and
that ratio scales with amplitude. Item 42's figure is not wrong; it was quoted
as though it were a constant.

**argmax stays at exactly 1.000 windows at both amplitudes**, because it is
quantised to an integer index and always lands on a neighbour. The centroid is
still 7.1x better at the weak end.

### The +0.01 bias is a weighting artefact, not an off-by-something

A half-window or half-pixel convention error would give exactly 0.5. This is
0.008, and it traces to the weights: the cluster is a symmetric 3x3 block
centred on the truth whose members agree to about 1.5 percent, with a monotone
gradient toward higher indices.

```
offset 0,0, scene median 14.84
  azimuth marginal weight:  1: 71.72   2: 71.72   3: 72.20
  range   marginal weight:  1: 71.05   2: 71.73   3: 72.85
```

That is item 41's 1.5 percent spread -- windows tracking the same scatterer
scoring almost but not exactly alike -- propagating through the centre of mass.
Range carries the steeper gradient, which is why the bias appears there. At
0.125 mm it grows to +0.13, seventeen-fold, as the background-limited reading
predicts.

### Uniform weighting is exact and not usable

Weighting every cluster member equally is EXACT when the cluster is symmetric
about the target, and it is on four of five:

```
  excess-weighted  0.008  0.009  0.008  0.008  0.009   mean 0.0083 = 0.13 m
  uniform          0.000  0.000  0.200  0.000  0.000   mean 0.0400 = 0.64 m
```

The one failure is the cluster that came out asymmetric at 10 windows, where
uniform weighting is dragged 0.2 windows. Symmetry is not something the method
can assume, and any background window that happened to agree would drag a
uniform centroid while contributing nothing to an excess-weighted one. The
robustness is worth the 0.13 m -- but uniform being exact on symmetric clusters
is the proof that the residual is a weighting artefact and not geometry.


## 44. The ICEYE dwell truncation works, and `validate` could not see it

Item 36 screened ICEYE's Houston `dwell-precise` collect and it FAILED the
observable band for a 2 Hz target -- because of the 15.345 s dwell rather than
the collect, since long sub-apertures average the target away. Item 4's remedy
is `--max-pulses`. `runs/screens/iceye/RUN-truncation.md`.

```
 dwell s   pulses  f_max @3.6%   df = 1/T     az res
   15.35   100802       0.905Hz    0.0652Hz      1.00x   <- fails 2 Hz
    6.14    40320       2.263Hz    0.1629Hz      2.50x   <- passes
```

**It works.** `validate --max-pulses 40320` turns the band from 1.509 Hz to
3.771 Hz and the verdict from FAIL to WARN, with every figure matching the
prediction to the digit.

**Item 4 named the band and not the price.** Frequency resolution IS `1/T_dwell`
and azimuth resolution scales the same way, so reaching 2 Hz costs 2.5x on both.
The band and the resolution trade directly against each other, and any future
truncation has to quote which one it bought.

### The defect

`validate` ACCEPTED `--max-pulses` AND IGNORED IT. Its reader options were
`{ .rbin_window = 8 }`, with no `max_pulses` and no `pulse_first`, so it read the
whole collect and answered about the untruncated dwell.

The consequence is the worst available: truncation is the documented remedy for
this exact failure, `validate` is the command a caller runs to decide whether the
remedy will work, and it reported FAIL for a configuration that passes. A caller
following the documentation would have concluded the collect was unusable.

This is the "warn rather than silently degrade" rule failing in the command that
exists to enforce it. Fixed; both flags are honoured and in the usage line.

**No test covers the fix and none can as things stand.** `validate` reads real
CPHD only and refuses `sim_cphd` output -- a known gap already recorded in the
USER_GUIDE gotchas, and the reason this was never caught. It is verified against
the real product only.

### ICEYE signal read for the first time

Item 36 recorded that no ICEYE signal sample had ever been read. 40320 pulses
backprojected onto 1024x1024 at 2 m in 4 min 26 s give a recognisable Houston:
street grid, building blocks, a circular structure, large parking areas. **The
`CI4` decode works on this vendor.**

Reading the whole product needs 38.7 GB against this machine's 25.8 GB. The
reader refuses with the arithmetic and names `--rbins`, which is how that refusal
should read.


## 45. First measurement run on ICEYE: the quality gate discards the true positive

Item 44 made the Houston collect usable. This is the first measurement run on it:
truncated to 6.138 s, `--estimator phase`, 128 looks at 0.90 overlap, 96 m grid,
with a 1.0 Hz injection at 2 mm and its mandatory zero-amplitude twin.
`runs/iceye/2026-08-04-houston-first/`.

### Urban does not meet the persistent-scatterer precondition either

```
                       best D_A   median   windows meeting D_A <= 0.25
  Giza  (desert, 225)     0.381        —              0 of 225   (item 19)
  Houston (urban,  25)    0.444     0.597              0 of  25
```

**Houston is WORSE than Giza on best `D_A`.** The expectation behind screening a
city was that dense construction would supply the dominant scatterers a desert
could not. It does not, and the reason is printed beside it: the sub-look
resolution here is **2.89 m**, and item 15's precondition is one dominant per
SUB-LOOK RESOLUTION CELL. A 2.89 m cell in a dense city holds many strong
scatterers, not one. Urban density works against the criterion at this cell size.

That closes a hypothesis this project has carried since item 19 -- that the Giza
null was about desert -- without needing a third scene.

### The quality gate discards the true positive

`df` is 0.1629 Hz and six windows report 1.047 Hz, inside half a bin of the
injected 1.000, with the highest prominence in the scene. Every one fails the
gate:

```
 win  (az,rg)   dominant    prom  quality  gates
   0 (0,0)        0.524    25.4   0.5428      1   <- REPORTED
   6 (1,1)        1.047    35.8   0.2061      0
  12 (2,2)        1.047    35.0   0.2406      0   <- the injected window
```

`mmotion` reported 0.524 Hz, the first admissible bin above the band floor --
item 37's trend at the first door it is allowed through.

**This is not a threshold that wants tuning.** On the phase route `quality` IS
amplitude stability, the same quantity as `D_A`. A scatterer vibrating at 2 mm is
not amplitude-stable across sub-looks, because that is what the motion does to
it. **The persistent-scatterer criterion and the signal being measured are in
direct conflict at large amplitude**, and item 37 found the complement: a
barely-moving scatterer has LOW `D_A` and passes. The gate prefers targets that
do not move.

Item 31's failure mode -- the tracker recovers and the policy discards -- with a
different culprit, on a different vendor.

### The scene-derived null and the paired increment both recover it

```
                    plain        injected            zero-amplitude twin
  scene null   0.524 Hz z 3.03   1.047 Hz z 7.13     0.524 Hz z 2.88
               window 1          WINDOW 12           window 12
```

The null names the injected window -- 12 is `(2,2)`, the grid origin -- and the
injected frequency, at more than twice the z of either control. It applies no
quality gate, which is exactly why it survives.

The increment at 1.000 Hz agrees: +86.91 at windows 6, 7 and 11, +80.32 at 12,
**median +0.00** over 25 windows. The machinery contributes nothing, as item 39
measured on Capella, and the centred window again scores slightly lowest of its
neighbours -- item 41 reproduced on another vendor and another scene.

**Item 38 concluded the scene-derived null did not help.** It did not there,
because the confound was a bright static scatterer that genuinely IS unusual for
its scene. Here the confound is a gate, and the null is the only reported
statistic that steps around it. Both readings stand: it is useful against some
confounds and not others, which is the most that can be said for any single
statistic here.

### Fixed: the centroid is now seeded twice and the disagreement is reported

The `located at` centroid was seeded from `best_window` alone, so on the injected
run it reported (0.00, 0.00) -- the artefact's position rather than the target's.
**A centroid is only as good as its seed.**

`mmotion` now computes it from both seeds and prints the second only when they
land more than half a window apart, which is beyond the 0.13 m item 43 measured
and so means a different place rather than the same one twice:

```
  located at window (0.00, 0.00) -- pixel (15.5, 15.5) -- seeded from the reported peak
  DISAGREEMENT: seeded from the scene-derived null instead, the target is at
           window (1.99, 1.99) -- pixel (47.3, 47.3) -- on 1.047 Hz from 8 windows.
```

Truth is the grid origin, window (2.00, 2.00) at pixel 47.5. **The null-seeded
centroid is right to 0.01 windows -- 0.16 m** -- matching item 42's accuracy on
Capella, on a different vendor and a different frequency, while the reported
answer points two windows away.

The two seeds are kept rather than one replacing the other, because they answer
different questions: the reported peak is gated and the null is not, so a
disagreement between them IS the finding -- it says a gate removed what the scene
finds most unusual. Resolving it silently would destroy that.

`test_tracking.c` pins seed-dependence directly: seeded on a background window
the same function returns that window, 2.50 windows from the target.

Nothing in this scene is known to move. The 1.047 Hz is ours.


## 46. The phase route's quality: spatial dominance, and an inert gate

Item 45 measured the defect: on the phase route `quality` was amplitude
stability, `1 - sigma_A/mu_A`, and a scatterer vibrating at 2 mm is not
amplitude-stable because that is what the motion does to it. Six ICEYE windows
carried the injected frequency at the scene's highest prominence and all six
failed the shared gate, so `mmotion` reported a trend artefact instead.

**The fix is to measure the precondition that is actually claimed.** Item 15's
condition is ONE DOMINANT SCATTERER PER SUB-LOOK RESOLUTION CELL -- a statement
about space. A vibrating dominant is still dominant. `quality` on the phase route
is now `1 - mean/peak` of the reference-look patch, the same measure
`RS_MICROM_EST_ARGMAX` already used, taken on the reference look so the estimator
still reads one pixel per look rather than a whole patch.

```
ICEYE Houston, 1.0 Hz injected at 2 mm

  before   strongest peak in window  0: 0.524 Hz, prominence 25.4, quality 0.543
           located at window (0.00, 0.00)          <- a trend artefact
  after    strongest peak in window  6: 1.047 Hz, prominence 35.8, quality 0.994
           located at window (1.99, 1.99)          <- the injected window
```

Truth is the grid origin at window (2.00, 2.00). The reported answer moves from
the wrong frequency in the wrong place to the injected frequency 0.16 m from the
target, and the DISAGREEMENT line of item 45 disappears because both seeds now
agree.

**Discrimination now appears in the reported answer**, where before it existed
only inside the scene-derived null:

```
  injected          1.047 Hz     zero-amplitude twin   0.524 Hz
  no injection      0.524 Hz
```

Both controls report the band-floor trend; only the injection reports the
injected frequency.

Giza is unaffected and slightly better: `--estimator phase` still recovers
0.163 Hz, and the centroid now reads (2.00, 2.01) against a truth of (2.00, 2.00).

### The gate is now inert, which is measured and stated rather than hidden

```
  fully developed speckle, 1024-pixel window, 20000 realisations
      1 - mean/peak    mean 0.673,  5th-95th 0.633-0.718
  real sub-look imagery (ICEYE, 32x32 windows)     0.81 - 0.94
  a dominant injected scatterer                    0.994 - 0.995
```

The shared gate is `quality >= 0.5 * q_max`, which sits near 0.50 when the max is
0.99. **25 of 25 windows pass on every ICEYE run measured.** The discriminating
range is roughly [0.75, 1.0] and the threshold is far below it.

That is a strictly better failure than the one it replaces -- an inert gate
removes nothing, where the old gate removed the signal -- but it says the
RELATIVE form of the threshold is wrong for this quantity, not the quantity. A
floor derived from the speckle expectation above would discriminate. **None is
imposed here**, because fitting a constant to two scenes is what this project
keeps having to undo, and because the verdict is supposed to come from the null
control rather than from a gate.

### What this breaks, deliberately

`quality` and `d_a` were complements to machine precision on this route --
`docs/CODE-REVIEW.md` finding 1, and the reason `--coherence F` was exactly the
criterion `D_A <= 1 - F`. That identity is gone. `quality` now answers "is there
a dominant scatterer here" and `d_a` answers "is it a persistent one", which are
two questions and were being reported as one.

Anything quoting a phase-route `quality` from before this change is quoting a
different quantity. `d_a` is untouched, so items 19, 20, 23 and 45's dispersion
figures all stand.
