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
| 1 | **answered, negative (99)** | the quantisation floor has no multiplicity correction |
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
| 46 | done | the phase route's quality is now spatial dominance, which fixes item 45 and leaves the gate inert |
| 47 | done | the noise floor is RED, so prominence prefers low frequencies; a local background fixes it |
| 48 | **withdraws item 14** | high overlap is WRONG for the phase route on real data: separation collapses 4 orders of magnitude by 0.90 |
| 49 | done | the local peak calibrated on nine disjoint grids of real desert: 15.1-34.4 |
| 50 | done | sensitivity re-measured at the good overlap with a proper twin: floor is 0.0625-0.125 mm |
| 51 | done | the floor is a QUADRATIC phase residual the linear carrier removal leaves; a quadratic fit drops it 2000x |
| 52 | done | quadratic carrier removal implemented: artefact down 38x, floor 0.125 -> 0.0156 mm |
| 53 | done | cubic term added: artefact 171x down in total, floor 0.0039-0.0078 mm, returns now falling fast |
| 54 | done | the carrier fix rescues item 25's recovery, 3 of 4 lobe widths, and NOT its static false positives |
| 55 | done | all three policies on the aspect fixture: none is both safe and useful, and item 47's local peak is a LOSS here |
| 56 | done | the AM/PM discriminator does NOT work: the false positives are not amplitude modulation at the reported frequency |
| 57 | done | KSTL read: ADS-B is a PROXY, no ground aircraft, and 11 seismic stations in the box are all outside the strip |
| 58 | done | STRIPMAP cannot support this measurement at all: per-target observation is 0.71 s, df 1.4 Hz |
| 59 | done | 315 synchronised instrument measurements found in the Capella spotlight archive, including Oroville Dam |
| 60 | done | Oroville Dam moved 0.5-0.8 um, 7-11x BELOW the floor: not a positive control, but the first PROVEN-STATIC scene |
| 61 | done | all 315 screened: NONE has motion above the floor that survives auditing; invert the search to start from earthquakes |
| 62 | done | the earthquake route fails on DUTY CYCLE: expected coincidences 0.14, needs 7x the archive |
| 63 | done | finer cells give 1.4x of the needed 4x and are non-monotone; Umbra is disqualified on dwell despite 4048 products |
| 64 | done | naive multi-pixel combination FAILS -- brightest-K is not statistically homogeneous, and phaselink.c already holds the right estimator |
| 65 | done | SHP + phase linking is WORSE still: homogeneous pixels share the ARTEFACT, so the ML estimator sharpens it |
| 66 | done | GROUND_TRUTH_DATASETS.md corroborates items 61-62 independently; its floor is the PER-LOOK one, 37x pessimistic |
| 67 | done | public-only leaves ONE usable pairing, Kilauea, and the test it supports is CORRELATION over 51 collects, not detection |
| 68 | done | the doc's new SHM datasets have no co-located collect, but they are real WAVEFORMS to inject in place of a sine |
| 69 | done, amended by 70 | a real structure's motion is injected for the first time, and the REPORTED answer goes wrong where a sine is correct |
| 70 | done, extended by 71 | reporting a modal SET is the right shape and does NOT fix it; cross-window support gives a noise bin the same 12/49 as the true mode |
| 71 | done | ranking the set by SPATIAL CONTIGUITY makes the wrong answer a refusal -- and the true mode is not in the tracking to be found, so the limit is upstream of selection |
| 72 | done | the short-time max-hold estimator is built and FAILS its controls: it breaks the sine whole-dwell recovers and answers confidently on a motionless scene |
| 73 | bounded by 74 | on a real BURST the shape-ranked modal set returns a true mode and the static control refuses -- the first policy here to do both -- and `--stft` makes it WORSE |
| 74 | done | THE SWEEP REFUTES IT: 1 of 6 answers correct over 12 points and two seeds, so item 73 was one lucky point. Both static controls still refuse |
| 75 | done | the screen was federated: 410 hits against 315, a continent the first one could not see, and 115 raw exceedances to ZERO credible |
| 76 | **withdraws 71** | items 69-74 measured a tracker that could not see; on `--estimator phase` a real record recovers |
| 77 | bounds 76 | the block threshold is contingent on the LOOK COUNT and dies at 48 looks |
| 78 | answered, negative | calibrating the block against a matched static run fails; it needs a chance model |
| 79 | recorded, not implemented | the field reports a per-mode posterior, not a threshold on a spectral statistic |
| 80 | implemented, does not detect | the chance model is built: it prices the block correctly and refuses nothing that matters |
| 81 | implemented, changes nothing | joint transient-and-mode estimation; the limit is not the spectral estimator |
| 82 | explains 81 | the threshold effect predicts it; matrix pencil and LSCF transient terms are untried |
| 83 | answered, negative | instrumented structures are ABOVE the floor and never observed moving; 0 bridges and 0 dams ever in a footprint |
| 84 | **partly withdraws 63** | Umbra has 228 collects >= 30 s, not none, and covers two instrumented bridges |
| 85 | done | all three public archives joined; ICEYE covers no structure; one bridge pairing exists in total |
| 86 | answered, negative | RESIF joined; one European pairing and it is proven-static; still no dam anywhere |
| 87 | answered, negative | INGV joined; Etna summit during a paroxysm is still 9x below the floor; ESM unqueried |
| 88 | **withdraws part of 83, 86, 87** | ESM queried: Grande Dixence IS in a footprint; the structural instruments are EVENT-TRIGGERED |
| 89 | open, untried | a continuous 6-sensor building record exists; its modes are 3.8 Hz, above the band at 128 looks |
| 90 | external, converges | an independent refutation of the DEEP Giza claim endorses the front end and reproduces items 11, 38, 58 |
| 91 | answered, negative | a building's AMBIENT motion injected: 1 of 24 correct, and 9 of 21 answers are the static scene's |
| 92 | external, adopt | their validation practice: an erratum that is our recurring rule, a derived threshold, and a pre-registration template |
| 93 | answered, negative | the Butte ground-truth chain is geometry only; two "dead" links are 403/429 refusals |
| 94 | open, better source | a second building record at 2.64 Hz, INSIDE the band, with mode-shape geometry; its loudest peak is not a mode |
| 95 | **bounds 76** | item 91 replicates on a second building; the 1.512 Hz artefact is SEED-BOUND, not common-mode |
| 96 | **bounds 95** | 12 of 12 motionless scenes report a confident frequency, 9 distinct; there is no clean seed |
| 97 | **locates the defect** | the paired twin finds the injected frequency in 76% of runs whose report is wrong: the SELECTION loses it |
| 98 | external, actionable | --twin is CCD and the LLR beats it; the peak search is the look-elsewhere effect, which is item 1 |
| 99 | **answers 1** | multiplicity does NOT explain the false positives; the artefact is a coherent line holding 23% of the band |
| 100 | **bounds 99** | the artefact does not scale with the analysis grid, so it is not the offset-driven carrier |
| 101 | **explains 96-100** | the artefact is phase noise at 2.3 mm rms; this fixture's real floor is 0.29 mm, 53x the quoted one |
| 102 | **kills the Kilauea test** | the floor is predictable from an uninjected run; real Kilauea is 0.53 mm, 306x above the best truth |
| 103 | **corrects 102** | the floor is PER-TARGET: three floors 34x apart, and the operative one is competition with the scene's artefact |
| 104 | **answers the reach question** | recovery needs 20-26 dB SCR; the field's own validations put a corner reflector on the structure |
| 105 | **refutes my own prediction** | no sub-look SCR penalty at REL 20; the floor falls as N^-0.36 and 256 looks beats 128 |
| 106 | **corrects 104 and 70** | the transition is look-count independent; a real bridge HAS been measured, without corner reflectors |
| 107 | **first blind discriminator** | require the frequency to survive a change of look count: false positives 12/12 -> 1/12, recall 6/6 |
| 108 | **bounds 107** | it caught a motionless scene answering 0.997 Hz; but recall on a weak OFF-CENTRE target is untested and it abstains 3 of 8 |
| 109 | **names the defect** | the injected line wins on support AND block and is still not reported: the loss is in the local-ratio NOMINATION |
| 110 | **corrects 109 and fixes it** | not the guard band: the localised target is refused by the SUPPORT gate at 28 of 34, then out-ranked on EXTENT. 5 of 6 recover, false positives unchanged at 1/12 |
| 111 | **fixes 110's leftovers** | the band-edge bias is real and measured (72% of maxima on 39% of the band); the fix is to NARROW not widen. Nothing lost, one artefact gone, abstentions 5/12 -> 3/12. Third defect named: median_ratio is a median over CHANCE nominators |
| 112 | **first 6 of 6** | the ranking's strength term was a median over CHANCE nominators; over the mode's own block instead, recall 5/6 -> 6/6 and synthetic false positives 1/12 -> 0/12. Item 103's competition floor is a property of the SELECTION too |
| 113 | **fixes 108's cause** | the null assumed windows nominate independently where 50% overlap makes neighbours correlated -- Eklund/Nichols/Knutsson's published cluster-failure mode. Permutation null on cluster MASS: injected runs admit ONE mode, item 96's 100% answer rate broken, 108's p 0.001 -> 0.010 but not refused |
| 114 | **REFUSES item 108** | correlation is confined to ADJACENT windows, so 113's residual guess was wrong; the real one is that any fixed partition loses correlation at its boundaries and no permutation null can be exact. Bracket p between two nulls, gate on the conservative: item 108 refused at p 0.342, recall 5/6, 8/12 motionless scenes silent |

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


## 47. The noise floor is red, and that is why controls report the band floor

Item 37 called the thing that wins on an uninjected scene a trend, and excluded
the first three bins. Item 45 found it again on ICEYE at the first bin that
remained. Neither asked what it is.

**It is not a trend.** On the real ICEYE series a linear detrend removes 1.4
percent of the spread, a quadratic 9.4 more and a cubic 6.4 more -- 83 percent
survives a cubic fit, so no polynomial describes it and no detrend can reach it.

**The noise is RED.** Mean power spectrum over all 25 windows with nothing
injected:

```
   bin  1  0.174 Hz   15.1x the median bin
   bin  2  0.349 Hz   18.7x
   bin  3  0.523 Hz   15.7x
   bin  9  1.569 Hz    5.1x
   bin 32  5.580 Hz    0.8x

   bins 1-4 carry 24x the power of the band above Nyquist/2
```

The cause is in the processing, not the scene: **at `--overlap 0.90` adjacent
sub-looks share nine tenths of their pulses**, so their phase errors are
correlated and the error series is a moving average rather than white noise.

`prominence` is a bin's power over the mean of every other bin, which is correct
only for a white floor. On a red one every low bin of pure noise scores well,
which is exactly why an uninjected scene reliably reports the lowest admissible
frequency and why excluding bins only moves the answer to the first one left.

**This is a third cost of overlap that items 13 and 14 did not account for.**
Item 14 recommends high overlap for the phase route because overlap buys
sub-look coherence; the same setting manufactures the correlated noise that
beats the signal.

### The fix: score each bin against its own neighbourhood

`rs_spectrum_local_window()` compares a bin's power to the MEDIAN of the bins
around it, excluding two either side so a real peak cannot raise its own
background -- a frequency-domain CFAR. Median rather than mean so a second
genuine tone inside the neighbourhood cannot drag it.

```
                       against the global mean   against the local median
  ICEYE, no injection  26.2 at 0.523 Hz          47.5 at 1.221 Hz
  ICEYE, 1.0 Hz in     87.4 at 1.046 Hz        1757.2 at 1.046 Hz
  Giza,  no injection                            39.2 at 0.163 Hz
  Giza,  2 mm at 0.163                          431.8 at 0.163 Hz
```

The control stops preferring the band floor, and the separation between an
injected run and its control widens from 3.3x to 37x on ICEYE and to 11x on
Giza. Reported beside the other policies; it gates nothing.

**It does not make a control silent.** Pure noise still yields a best local ratio
-- 47.5 on ICEYE, 39.2 on Giza -- because the maximum is taken over every window
and every bin, so the look-elsewhere cost is inside the statistic. The number to
compare against is another scene's maximum, as with the scene-derived null.

### The limit, which the test found rather than the design anticipating

Against a random walk -- power falling as 1/f^2 -- the slope across a 25-bin
neighbourhood is itself steep, so a bin at the bottom of the band still beats its
own background and the bias survives. The first version of the regression test
used exactly that and the method failed it.

This works because the real floor is not that steep. Overlap-induced correlation
is a MOVING AVERAGE over shared pulses, so its power rolls off as sinc^2 and is
locally flat over twenty bins; the fixture now models that. A collect with a
steeper floor would need a narrower neighbourhood or a fitted slope, and neither
is implemented.


## 48. High overlap is wrong for the phase route, and item 14 said otherwise

Item 14 measured that phase recovery "holds to 95% overlap" on synthetic
fixtures and reasoned that high overlap buys sub-look coherence on a real
collect. `docs/USER_GUIDE.md` turned that into advice -- "use it with high
`--overlap`, unlike `correlation`" -- and every phase run in this project has
used 0.90. Item 47 then found that overlap is what makes the noise red.

Nobody had measured the three effects together. `runs/giza/2026-08-04-overlap-sweep/`.

```
 overlap  sub-look res   injected local peak     control   separation
    0.00        6.59 m     816,997  (correct)      22.9      35,676x
    0.50        3.32 m   1,207,566  (correct)      12.6      95,838x
    0.75        1.69 m   1,856,891  (WRONG bin)    12.9     143,945x
    0.90        0.70 m         431.8 (correct)     39.2          11x
    0.95        0.38 m          46.7 (WRONG)       26.0         1.8x
```

**The separation collapses by four orders of magnitude between 0.75 and 0.90,
and at 0.95 neither policy finds the injection at all.** `best_window` reports
0.105 Hz there and the local peak agrees with it; both are wrong.

The ordinary prominence says the same thing more quietly: the control scores
6.6-13.6 at overlap 0-0.75 and 17.9-20.7 at 0.90-0.95. What grows is item 47's
red floor.

### Why the reasoning failed

Item 14's premise was sound and its conclusion does not follow. Overlap does buy
sub-look coherence. It also correlates the noise between adjacent looks, and
correlated noise is red, and a red floor is exactly where a spurious peak lives.
**What overlap buys in coherence it spends on manufacturing the thing the
coherence was for.**

**Sub-look resolution moves the opposite way and does not rescue the high end.**
At overlap 0 each sub-look is 6.59 m against 0.38 m at 0.95, seventeen times
coarser, and item 15's precondition is one dominant scatterer per sub-look
resolution cell -- so the coarse end should be the harder one. It is not. The
noise term dominates the precondition term across this whole range.

### What is and is not established

**Established:** the 0.90-0.95 band this project has used throughout is far
worse than anything below it, on this collect, and item 14's advice as written
should not be followed.

**Not established:** where the optimum is. Five points, one collect, one
frequency, one amplitude. The separation still rises from 0.00 to 0.75 and the
0.75 point names the wrong bin, so "use 0.5" would be fitting a recommendation to
three points -- which is how item 14 got here.

### A caution about the statistic itself

The local peak named the WRONG bin at overlap 0.75 -- 0.187 Hz where bin 5 is
0.156 Hz -- while `best_window` named the right one. It maximises a RATIO, so a
bin with a quieter neighbourhood can beat a bin carrying more signal. That is a
real failure mode and it showed up at the best-separating setting.


## 49. The local peak calibrated on real clutter, with no simulator

Item 47 left the local peak gating nothing because its control maximum was known
from two scenes. `runs/giza/2026-08-04-localpeak-calib/`.

Nine 96 m grids on the same real uninjected Giza collect, a 3x3 lattice at 150 m
spacing so no two share a window, at `--overlap 0.5`:

```
  15.1  16.2  19.2  19.5  20.8  20.9  22.5  25.0  34.4
  min 15.1   median 20.8   MAX 34.4
```

The largest is 2.3x the smallest across nine independent patches of desert. A
2 mm injection at the same settings gives 1,207,566 -- **35,104x the worst
control grid**.

**No simulator anywhere in it.** This is the scene-derived idea of item 38
applied to frequency instead of space: the collect supplies its own controls and
they carry whatever it does, which is the property `--null-static` never had
(items 33, 37).

### What nine controls are worth

**A smallest possible p of 1/10 = 0.10.** A conformal p-value cannot express more
than the control count allows -- the same arithmetic item 35 recorded for
`--null-static`, where nineteen trials were the minimum for 0.05. Nineteen
disjoint grids would reach it and the collect is large enough to supply them;
this run did not.

The p-value is not what is doing the work at 35,104x. The number to carry forward
is the RANGE: **anything under about 35x on this collect at this setting is inside
what empty desert produces.**

It is one collect, one setting, one grid size. The control maximum is a maximum
over every window and every bin, so it grows with the number of windows searched
and does not transfer to a larger grid.


## 50. Sensitivity re-measured where the instrument works, with the right control

Items 37 and 43 measured sensitivity at `--overlap 0.90`, which item 48 has since
shown to be the worst setting tested. Re-swept at 0.5, and extended downward
because the 2 mm point now sits five orders of magnitude above an empty-desert
control. `runs/giza/2026-08-04-amplitude-lowoverlap/`.

```
    amp mm   local peak    freq   vs zero-amp twin  verdict
       2.0    1,207,566   0.153            100.13x  signal
       1.0      630,066   0.153             52.24x  signal
       0.5      372,535   0.153             30.89x  signal
      0.25      111,498   0.153              9.25x  signal
     0.125       35,164   0.153              2.92x  signal
    0.0625       12,147   0.092              1.01x  THE MACHINERY
   0.03125       11,737   0.092              0.97x  THE MACHINERY
  0.015625       10,892   0.092              0.90x  THE MACHINERY
 0.0078125       10,454   0.092              0.87x  THE MACHINERY

  zero-amplitude twin   12,060 at 0.092 Hz
```

`df` is 0.0306, so bin 5 is 0.153 Hz and the injected 0.163 sits within half a
bin; 0.092 Hz is bin 3, the band floor.

**The floor is bracketed between 0.0625 and 0.125 mm.** Below 0.125 the reported
answer stops being the injection and becomes the twin: the ratio plateaus at
10,400-12,100 and stops responding to amplitude across a sixteen-fold change,
which is what an artefact does, and the twin sits at 12,060 in the middle of it.

### The control that matters is not the one item 49 measured

Item 49 calibrated empty desert at 15.1-34.4 over nine disjoint grids, and every
point above -- including the pure-machinery ones -- clears that by two to three
orders of magnitude.

**Empty desert is the right control for an uninjected scene and the wrong one
for an injected experiment.** An injected run contains a scatterer at 20x the
median sample magnitude that empty desert does not, and that scatterer alone
produces 12,060x. The only valid control for an injected run is its own
zero-amplitude twin -- item 38's rule, confirmed here at a different overlap.

Both numbers are real and answer different questions. **Quoting item 49's 34.4
against an injected run would have called 0.0078 mm a detection**, which is the
mistake this arrangement is designed to prevent and which nothing but the twin
catches.

### It agrees with the literature now, where it did not before

Items 37 and 43 reported recovery to 0.0625 mm at overlap 0.90 with no
zero-amplitude twin at that setting. Item 38 had already shown the twin
outscoring real injections there, so the figure was never safe.

0.125 mm zero-to-peak is 0.088 mm RMS against Vattulainen et al.'s smallest
confirmed radial RMS displacement of 0.10 mm. Item 39 flagged the earlier
0.0625 mm as a reason for suspicion precisely because it beat the published floor
by 2x. It no longer does, and the agreement is the first external check any
sensitivity figure here has passed.

### Still recoverability, not detection

The frequency was known in advance, the target was ours, and the scene contains
nothing known to move. What is bounded is recoverability, with a proper control
for the first time.


## 51. What sets the sensitivity floor: a quadratic phase residual

Item 50's floor at 0.125 mm is not noise. It is the zero-amplitude twin: a bright
STATIC scatterer produces 12,060x at the band floor and the real signal loses to
it below 0.125 mm. `runs/giza/2026-08-04-static-artefact/`.

### It scales as brightness squared, and saturates

```
  REL     local peak at the band floor
    1                            37.9     <- empty desert is 15.1-34.4 (item 49)
    5                           963.7
   20                        12,060.1
   50                        38,991.6
  100                        44,697.8
```

Square-law up to 20, saturating between 50 and 100. That is what a fixed FRACTION
of the scatterer's own energy leaking into one bin looks like: the artefact's
power tracks the scatterer's while the clutter around it does not. **At REL = 1 it
is 37.9, barely outside empty desert** -- this is a problem of bright scatterers
specifically, which is also why it will matter on real scenes with towers,
corners and ships in them.

**Signal and artefact both scale as REL^2, so their ratio does not depend on
brightness.** A brighter target does not make smaller motion detectable. Item
50's floor is a real limit on amplitude and cannot be bought down with a brighter
target. What it depends on is the residual phase.

### It is the curvature the carrier removal leaves behind

The estimator removes the geometric carrier by finding the LINEAR ramp maximising
the de-ramped phasor sum. What survives:

```
  window 8, static bright scatterer, zero injected motion
  phase rms 0.4153 rad -- well inside (-pi, pi], so nothing is wrapping

  after fitting and removing:   residual rms      bin-3 local ratio
    linear (what ships)           1.0347 mm             21,601.9
    quadratic                     0.0902 mm                 10.4
    cubic                         0.0054 mm                  2.9

  worst of 25 windows: 21,602 -> 10.4 on removing a quadratic
```

**A quadratic removes it, by a factor of 2,000.** A scatterer's range history is
quadratic in time, so its phase is quadratic, and a de-ramp fitting only
`exp(-i*v*k)` leaves the curvature. This is not item 11's sawtooth -- nothing is
wrapping at 0.42 rad rms -- it is the term below it that item 14's carrier
removal was never designed to reach.

**I read this backwards first**, and wrote that no polynomial removes it, having
looked at residual rms rather than at the bin the artefact occupies. The rms
falls 11x where the artefact falls 2,000x: the artefact is a small part of the
total spread and almost all of the bin-3 power. Recorded because the wrong
reading was the natural one.

### The fix, measured but NOT implemented

**Extend the carrier search to a quadratic term** -- maximise
`|sum_k z[k] exp(-i(v*k + w*k^2))|` over both. That removes GEOMETRY, which is
what the residual is, and leaves the signal alone. It costs a second search
dimension, and the coarse pass is already O(N^2) per window, so it needs a staged
or alternating search rather than a 2-D grid.

The cheaper alternative -- raising the detrend order on the displacement series --
is a much smaller change and `rs_detrend_t` already exists, but `microm.h` argues
against precisely this: under a resonance reading the lowest frequencies are the
DEEPEST structure, so a quadratic detrend attenuates what that model calls real.
A 0.163 Hz tone is five cycles and safe; a genuine signal near the band floor is
not.

**If the carrier fix works, the sensitivity floor should drop by roughly the
square root of 2,000** -- the amplitude at which signal matches artefact scales
with the residual - which would put it near 0.003 mm before some other floor
takes over. That is a prediction, and it is the reason to do it.


## 52. Quadratic carrier removal: the floor falls eightfold

Item 51 diagnosed the sensitivity floor as a quadratic phase residual the linear
carrier removal leaves behind. Implemented.
`runs/giza/2026-08-04-quadratic-floor/`.

```
  static bright scatterer, zero motion, REL 20
    linear carrier only            12,060.1x at 0.092 Hz
    quadratic, UNCENTRED basis     10,697.4x    -- 1.13x, a failure
    quadratic, CENTRED basis          317.7x    -- 38x

      amp mm   local peak    freq          verdict
       2.0     1,311,807   0.153           signal
       0.125      32,502   0.153           signal
       0.0625      7,773   0.153           signal
       0.03125     1,866   0.153           signal
       0.015625      472   0.153           signal    <- last correct point
       0.0078125     348   0.092   THE ARTEFACT (317.7)
       0.00390625    347   0.092   THE ARTEFACT

  floor 0.0078-0.0156 mm, against 0.0625-0.125 mm before -- EIGHTFOLD
```

The 2 mm signal is unchanged (1,311,807 against 1,207,566), so this removes
artefact without touching signal, and discrimination improves 41x.

### The bug, which is the useful part

The first implementation used `nu*k + mu*k*k` directly and reduced the artefact
by **1.13x**. `k` and `k*k` are strongly correlated over a finite record: adding
`mu*k*k` shifts the mean rate by about `mu*N`, which at the curvature actually
present is 0.04 rad per look against a coarse step of 0.049 -- nearly a whole
step. Scanning `mu` while holding `nu` at the linear-only optimum therefore made
every trial WORSE, and the search returned `mu = 0` in every window.

Centring makes the terms orthogonal -- `kc = k - (N-1)/2`,
`q = kc^2 - (N^2-1)/12`, with `sum(kc*q) = 0` by symmetry -- so a staged search
is valid. That is the entire difference between 1.13x and 38x, and it was found
by measuring the residual curvature still in the output rather than by reading
the code.

### The prediction was right and its number was wrong

Item 51 predicted "toward 0.003 mm" from the offline fit's 2,000x reduction. The
phasor search achieves 38x. `sqrt(38) = 6.2` gives `0.125/6.2 = 0.020 mm`
against a measured 0.0156 -- so the reasoning held and the reduction it was
applied to did not. **An offline least-squares fit to the displacement is not
what maximising the phasor sum does**, and the gap between 2,000x and 38x is
exactly that difference. A cubic term would presumably close some of the rest;
the offline cubic gave a further 3.6x.

### It now beats the literature by nine times, which is a caution

0.0156 mm zero-to-peak is 0.011 mm RMS against Vattulainen et al.'s smallest
confirmed 0.10 mm RMS. Item 50 had just brought this project into agreement with
that figure and this puts it nine times below.

Their 0.10 mm is a REAL vibrating object with synchronous ground truth, carrying
every real-world effect. Ours is a synthetic tone injected into real clutter as a
perfectly coherent point target -- brighter and cleaner than any real structure,
at a frequency known in advance. The honest reading is that the INSTRUMENT's
floor on this collect is 0.011 mm RMS, and that this says nothing about what a
real structure would give. Item 39 flagged beating the published floor as a
reason for suspicion and that still applies.


## 53. The cubic term: 171x in total, and the returns are falling

Item 51 measured that a cubic fit removed a further 3.6x of the static-scatterer
artefact after the quadratic. Item 52 implemented the quadratic. This adds the
cubic. `runs/giza/2026-08-04-cubic-floor/`.

```
  kc = k - (N-1)/2
  q  = kc^2 - (N^2-1)/12          orthogonal to kc by parity
  c  = kc^3 - kc*(3N^2-7)/20      orthogonal to BOTH
```

**The cubic's constant is not decoration.** `c` and `kc` are both ODD, so parity
does not separate them the way it separates `q` from `kc`. The coefficient is
`sum(kc^4)/sum(kc^2) = (3N^2-7)/20` for `k = 0..N-1`, verified to machine
precision at N = 16, 64, 128 and 512. Without it the cubic stage would have
failed exactly as item 52's uncentred quadratic did.

```
  static bright scatterer, zero motion, REL 20
    linear only                 12,060.1x
    + quadratic                    317.7x     38x
    + cubic                         70.7x     a further 4.5x, 171x in total

  2 mm signal   1,207,566 -> 1,311,807 -> 1,277,488   unchanged within 3%

      amp mm   local peak    freq          verdict
       0.015625      556   0.153           signal
       0.0078125     144   0.153           signal    <- last correct point
       0.00390625     72   0.092   THE ARTEFACT (70.7)
       0.001953125    71   0.092   THE ARTEFACT

  floor 0.0039-0.0078 mm against 0.0078-0.0156 with the quadratic alone
```

Item 51's offline cubic predicted 3.6x; the search delivers 4.5x, and
`sqrt(4.5) = 2.1` predicts the floor halving, which is what happens. Unlike item
52, where the offline number was 50x optimistic, the offline cubic transferred.

### Where this stops being worth doing

```
                floor mm          artefact   gain
  linear      0.0625 - 0.125      12,060.1     --
  quadratic   0.0078 - 0.0156        317.7    38x
  cubic       0.0039 - 0.0078         70.7   4.5x
```

Each stage is another O(N^2) pass per window and the returns are falling fast.
A quartic looks worth about a factor of two for a fourth pass. **Not added**, and
the sequence above is the reason rather than a preference.

### Eighteen times below the published floor, which is now a real concern

0.0078 mm zero-to-peak is 0.0055 mm RMS against Vattulainen et al.'s smallest
confirmed 0.10 mm RMS. Item 50 had this project in agreement with that figure,
item 52 put it 9x below, and this puts it 18x.

**That gap is not evidence of a better instrument. It is evidence that the
injected target is not like a real one.** `rs_simulate_inject_vibrator()` writes
a perfectly coherent point scatterer with analytically exact phase, present in
every pulse, at a frequency known in advance. A real structure decorrelates, is
aspect-dependent, and is never a point -- and items 24-25 measured what aspect
dependence alone does to this estimator, which was not small.

The defensible claim is narrow: **the instrument's own floor on this collect is
0.0055 mm RMS**, measured against the only control that applies to an injected
run. Anything about real structures needs a real structure, which remains the
thing this project does not have.


## 54. The carrier fix against aspect dependence: recovery yes, safety no

Items 52-53 improved the carrier removal by 171x against a static bright
scatterer on real data. Item 25 is where the phase route breaks on a FIXTURE --
aspect-dependent scattering, the mechanism real structures have and
`rs_sim_scene()` lacks, and the reason item 14's recovery was bounded rather than
believed. `runs/fixtures/2026-08-04-aspect-carrier/`.

```
 lobe_frac |    slope   rms Hz | verdict  | static controls (3 seeds)
      1.00 |   1.0417   0.0333 | RECOVERS |  0.25  0.58  0.25  IN BAND
      0.50 |   1.0417   0.0289 | RECOVERS |  0.25  0.25  0.25  outside
      0.25 |   1.2083   1.1252 | fails    |  0.33  0.25  0.33  IN BAND
      0.12 |   0.9583   0.0236 | RECOVERS |  1.83  1.75  1.67  outside
 isotropic |   0.9583   0.0236 | RECOVERS
```

Item 25 measured `rs_spectrum_best_window()` failing at ALL FOUR lobe widths with
slope negative at three. **Three of four now recover.**

**Three of twelve static controls still come back inside the swept band**,
against item 25's two of twelve. That failure is untouched.

### The split is the finding

The two halves of item 25 had different causes and only one was a carrier
problem.

The slope-and-rms failure WAS the carrier residual. Aspect dependence makes a
scatterer's amplitude vary across the aperture, which perturbs a linear-only
carrier fit, and the leftover curvature swamped the tone. Fitting the curvature
removes it.

**The static false positives are a POLICY failure and item 25 said so at the
time**: "aspect dependence gives amplitude per-look structure the isotropic model
never had, and the prominence policy reads that structure as a frequency."
Nothing about the carrier changes what prominence does with a genuine amplitude
modulation on a scene where nothing moves. `rs_spectrum_ps_window()` was the only
policy that behaved there and still is.

### Not a like-for-like reproduction, stated because the numbers invite it

- **5 frequencies over 1 seed** against item 25's 6 over 3 -- 5 points against
  18, so slope and rms are far less constrained here.
- **12 s dwell at 64 looks** gives `df = 0.0833 Hz` and a half-bin bound of
  0.0417 against item 25's 0.0252. **A looser bar.**
- Same fixture, same lobe fractions, same estimator and window geometry.

So "three of four recover" is directional against item 25's "none of four", not a
withdrawal of it. The static count, 3 of 12 against 2 of 12, should be read as
unchanged rather than worse given the different seed counts.

**Item 25's headline was two-part -- the reported policy fails, and it fails
unsafely. The first part weakens; the second stands.**


## 55. All three policies on the aspect fixture, and where the local peak does not reach

Item 54 left the static false positives as a POLICY failure. So all three
selection policies were scored on identical spectra.
`runs/fixtures/2026-08-04-aspect-carrier/policy_run.log`.

```
           | PROMINENCE (best_window)  | LOCAL PEAK (item 47)      | PS SELECTOR
 lobe_frac |   slope     rms verdict   |   slope     rms verdict   |
      1.00 |  1.0417  0.0333 RECOVERS  |  0.7500  0.2555 fails     | answered 0/5
           | static in band 1/3        | static in band 2/3        | static 0/3
      0.50 |  1.0417  0.0289 RECOVERS  |  1.0833  0.0527 fails     | answered 0/5
           | static in band 0/3        | static in band 0/3        | static 0/3
      0.25 |  1.2083  1.1252 fails     |  2.0833  0.8528 fails     | answered 0/5
           | static in band 2/3        | static in band 0/3        | static 0/3
      0.12 |  0.9583  0.0236 RECOVERS  |  0.9583  0.0289 RECOVERS  | answered 0/5
           | static in band 0/3        | static in band 1/3        | static 0/3
```

**None of the three is both safe and useful.** Prominence recovers at three of
four and is in-band on 3 of 12 statics. The local peak recovers at ONE of four
and is equally unsafe at 3 of 12 -- strictly worse than prominence here. The PS
selector answers nothing at all, so it has no false positives and no recall.

### This pins item 47's scope, and it is narrower than it looked

The local peak was a large win on the real Giza collect -- injected-versus-control
separation from 3.3x to 37x -- because THAT failure was a red noise floor, and a
local background is the right correction for one.

**It is a loss on this fixture.** Aspect dependence does not produce a coloured
noise floor. It produces genuine spectral content: a facet lit over part of the
aperture amplitude-modulates the return, and a modulation has real sidebands. A
local background cannot tell a real sideband from a real tone -- both stand above
their neighbours -- so normalising by it removes the low-frequency bias without
touching this failure, and costs recall doing it.

**The two failures look identical in the output and are not the same failure.**
The statistic built for one does not transfer, and item 47 should be read as
addressing coloured noise specifically rather than spurious peaks generally.

### And the PS selector's recall is configuration-dependent

Item 25 measured it answering 6 of 18 points, all correct, at `lobe_frac` 0.12.
Here it answers nothing at any lobe width: no window meets `D_A <= 0.25`. Its
safety is real and comes from refusing; how often it refuses depends on the
configuration, and this one gets zero recall.

**No policy in this codebase is fit for an aspect-dependent scene**, which is the
scene type real structures produce. That is the same conclusion item 25 reached,
now with the carrier residual eliminated as an explanation and the newest
statistic tested and excluded as a remedy.


## 56. The AM/PM discriminator does not work, and what that says about the mechanism

Item 55 isolated the failure to the selection stage. The physics suggested a
discriminator, and it is a good idea that does not survive measurement.

A VIBRATION modulates PHASE -- the target's range changes, its brightness does
not. ASPECT DEPENDENCE modulates AMPLITUDE -- a facet lit over part of the
aperture fades, its range never changes. Both put a peak in the displacement
spectrum, because amplitude modulation corrupts a phase estimate. So a frequency
present in BOTH spectra should be brightness and one present only in displacement
should be motion. `rs_spectrum_am_check()` implements exactly that.

```
  lobe   injected AM ratio    static AM ratio    in-band statics rejected
  1.00        0.4 - 36.8          8.7 - 12.7                1 of 1
  0.50        1.1 - 97.5         26.8 - 444.6               0 of 0
  0.25        0.6 - 28.1          0.7 - 7.8                 0 of 2
  0.12        0.3 - 12.2          0.7 - 13.2                0 of 0
```

**It does not separate them.** Ranges overlap at every lobe width, the static
ratios EXCEED the injected ones at 0.50, and one of three in-band false
positives is caught. No threshold on this quantity works, so the threshold and
the rejection flag were removed and it reports a ratio and gates nothing.

### The mechanism is not the one assumed

**The false positives are not amplitude modulation at the reported frequency.**
At lobe 0.25 the two in-band statics have AM ratios of 0.7 to 7.8 -- no amplitude
peak at all where the displacement peaked.

What the aspect lobe does is make the tracked pixel FADE across part of the
aperture. The pixel is chosen ONCE from the reference look, so during the fade
its phase is noise-dominated: the series is non-stationary, good phase for part
of the record and noise for the rest, and the spectrum of that has structure at
no particular frequency. The amplitude signature is a smooth ENVELOPE at the
bottom of the band, not a tone where the displacement peaked.

That is a different failure from amplitude modulation and it needs a different
test.

### It re-derives item 25's conclusion the long way

The right question is **"does this window's brightness vary at all"** rather than
"does it vary at this frequency". That is amplitude dispersion, which
`rs_spectrum_ps_window()` already applies, and it is why item 25 found the PS
selector the only safe policy.

Two ideas have now been tried against item 25's unsafe failure and both have
failed: item 47's local peak (item 55) and this. Both failed for the same
underlying reason -- they test the SPECTRUM, and the failure is not in the
spectrum but in the tracked series being non-stationary. **A test for
non-stationarity is what this needs**, and none exists here.


## 57. KSTL, and the difference between a proxy and a measurement

`docs/KSTL_ADSB_POSITIVE_TEST.md` paired a Capella stripmap collect with
ADSB.lol and found two aircraft inside the footprint during the aperture -- the
first collect here with independently confirmed MOTION. The CPHD was not
downloaded. It is now, and it reads. `runs/kstl/2026-08-04-first/`.

```
CAPELLA_C11_SM_CPHD_HH_20250319191515_20250319191532   6.77 GB
  172997 pulses, 9.55 GHz, PRF 10174 Hz, dwell 17.003 s, near range 679.7 km
  footprint 10 x 100 km; KSTL, both aircraft and the scene reference all inside
```

**The footprint check needed the corners ordered.** The five `<Lat>/<Lon>` pairs
in the header are the scene reference then four corners in an order that is not
a ring; taken as given, point-in-polygon says KSTL is OUTSIDE its own scene.
Sorted by angle about the centroid everything is inside. A trap for anything
reading these footprints, this project included.

### ADS-B is a proxy, and that is a category difference

```
  N510CN   1394 m altitude,  69.8 m/s,  moved  975 m across the in-aperture reports
  N707VM    914 m altitude, 126.5 m/s,  moved 1386 m
```

Both are **airborne and translating**. The focusing grid sits at height 0, so a
target 1.4 km above it is mislocated and defocused before anything else; and a
moving target is displaced in azimuth by `R*v_r/V`, up to **6.3 km** at this
range and platform speed. These are moving-target signatures, which is `ccd.c`'s
question rather than the vibration chain's.

**ADS-B establishes presence and motion and supplies no displacement waveform.**
It cannot validate a frequency or an amplitude, which is what every open item
here needs. The doc said so; this sharpens why.

### No ground aircraft, and the tool said so for the wrong reason

A parked aircraft would be a much better target: ground level, not translating,
bright, and vibrating if its APU runs. Scanned the day's archive against this
footprint -- 200 reports in the box, 16 in aperture, **0 on the ground at any
time**.

**`adsblol_cphd_crossmatch.py` read `on_ground` from `point[6]`, the flags
word.** readsb puts the STRING `"ground"` in the ALTITUDE field for a surface
aircraft, so the column read 0 for everything -- which looks like a scan finding
no ground traffic rather than a bug. Fixed. The corrected scan still finds zero,
so the conclusion survives and the reason for believing it is now sound.

### The sensor join: object identification is not measurement

`tools/footprint_sensor_join.py` classifies a footprint's contents as
MEASUREMENT (an instrument with data across the aperture), OBJECT (a structure
known from a map) or PROXY (ADS-B, AIS, METAR).

```
scene                       box  in  rec  verdict
GIZA_C13_SP_20241004          0   0    0  nothing in the box
ICEYE_X47_HOUSTON             0   0    0  nothing in the box
KSTL_C11_SM_20250319         10   0    0  nearby but outside
```

**Eleven FDSN stations fall in the KSTL bounding box and NONE in the 10 km
strip**, including three building-mounted strong-motion instruments -- one at
One Bell Center, a downtown tower. Comparing scene centres, or boxes, would have
claimed a match that does not exist.

### What would actually change the answer

A MEASUREMENT-class hit needs a collect whose footprint contains an instrument
recording across the aperture. Three CPHDs is not a search; the catalogue has
thousands and the tool is written to be pointed at them.

Two cautions for when it is:

- **Strong-motion instruments in buildings are usually TRIGGERED.** They are
  open for decades and hold data for minutes of it. Station metadata says
  "operating"; only the availability service says "recorded".
- **Ambient ground motion is far below this instrument's floor.** A vault
  seismometer sees roughly 0.1 um at 1 Hz against item 53's 5.5 um RMS floor. A
  building's response to wind or traffic reaches tens of microns and is the
  plausible target; bare ground is not.


## 58. Stripmap cannot support this measurement, and that is arithmetic

The KSTL focus does not produce a recognisable airport -- 1024 x 1024 at 2 m from
all 172997 pulses, 21 min 55 s, and the result is speckle with faint smeared
streaks. `runs/kstl/2026-08-04-first/kstl.png`.

**It is the first STRIPMAP collect this project has processed.** The
backprojector integrates every pulse onto every grid cell, which is correct for
spotlight and wrong here: in stripmap the beam sweeps, and a point is lit for
`lambda*R/(2*rho*V)` and no longer.

```
  2 m azimuth resolution at 680 km needs 5335 m of aperture = 0.71 s = 7238
  pulses of 172997, so 95.8% of pulses have the target OUTSIDE the beam
```

Those pulses carry random phase; signal grows as `N_sig` and noise as
`sqrt(N_noise)`, so integrating the full 17 s is worse than using the right 4%.

### The requirement is per-target observation time

```
mode                            collect  per-target   df = 1/T  usable
Capella KSTL   (stripmap)         17.0s       0.71s    1.406Hz  NO
Capella Giza   (spotlight)        32.9s      32.90s    0.030Hz  yes
ICEYE Houston  (dwell-precise)    15.3s      15.30s    0.065Hz  yes
Umbra Panama   (spotlight)         2.0s       1.99s    0.503Hz  NO
```

At KSTL the frequency resolution is **1.4 Hz**, coarser than the entire 0.3-3 Hz
band this project targets, and 0.71 s split into 128 sub-looks gives 5.5 ms each
with no useful azimuth resolution. **No processing fixes that.** The target was
not observed long enough, and the sub-aperture stack has nothing to stack.

So KSTL was never a candidate for the vibration chain, aircraft or not -- it is a
moving-target scene and nothing else, which is what item 57 concluded from the
kinematics and this confirms from the geometry. The same table disqualifies
Umbra's 2 s spotlight at 0.50 Hz resolution, which item 36 called coarse without
quantifying.

**A collect is usable here only if one point stays in the beam for seconds** --
spotlight or a dwell mode. That should be the FIRST thing checked about any
candidate, before the footprint and before the sensors, because it is cheap and
it disqualifies outright.

### A flag ordering defect found on the way, not fixed

`focus --pulse-start 96000 --max-pulses 16000` fails with "pulse window
[96000, 112000) outside available 16000 pulses". `--max-pulses` truncates the
READ to the first 16000 and `--pulse-start` then indexes into that truncated
buffer, so the pair cannot select a window late in a collect -- exactly what a
stripmap sub-aperture would need. `USER_GUIDE` describes `--pulse-start` as
"first pulse to read", which is not what it does.

Left unfixed deliberately: the mode it would serve is disqualified above, and a
fix should be made when something needs it rather than on the way past.


## 59. The data problem, opened: 315 synchronised measurements in the archive

Every open item here ends at the same place -- no collect has independently
confirmed motion, so detection is bounded by signals this project injected
itself. That is a data problem, and this is the search.
`runs/screens/sensor-join/`.

```
  1174  CPHD in the Capella open archive
   939  spotlight or sliding spotlight              <- item 58's filter, applied first
        dwell: median 22.5 s, 646 at >= 15 s, 156 at >= 30 s
 70490  FDSN stations worldwide after dropping network SY, which is SYNTHETIC
  3877  polygon hits over 257 distinct collects
   553  at dwell >= 25 s
   315  WITH A REAL WAVEFORM across the aperture
```

**315 station-collect pairs where an instrument was recording inside the
footprint during the aperture.** This project has never had one.

### An instrument ON a structure, which is the case that matters

```
  BK.ORV   167,936 B   dwell 30.4 s   Oroville Dam, Oroville, CA
           CAPELLA_C10_SP_CPHD_HH_20240803004650_20240803004720   32.13 GB
  NN.CC12   23,552 B   dwell 25.7 s   Clark County Firehouse 12, ANSS strong motion
           CAPELLA_C09_SP_CPHD_HH_20231119132416_20231119132442   22.19 GB
```

Oroville Dam is the strongest candidate in the archive: a large concrete
structure, an instrument on it, a 30.4 s spotlight collect over it, and 168 KB of
waveform spanning the aperture.

### And forty repeats of Kilauea

`HV.KKO`, `HV.OTLD`, `HV.RIMD`, `HV.UWE`, `HV.WRM` each appear in **40 separate
collects**, with six or more stations recording per collect. That is a
repeatability experiment rather than one shot. Mount Etna appears with four INGV
stations in a single collect.

### What it does not establish

**A waveform existing is not a waveform showing anything.** Ambient ground
motion is of order 0.1 um at 1 Hz against item 53's 5.5 um RMS instrument floor
-- fifty times below it. A vault seismometer on quiet ground will show nothing
this radar could have seen, and the comparison would be a null on both sides.
What could clear the floor is a dam or building responding to wind, traffic or
machinery, where displacement reaches tens of microns, or any of these sites
during an actual earthquake.

**Co-located is not co-measured.** A seismometer records ground velocity at one
point; this pipeline records line-of-sight displacement of scatterers across a
window. The station's own structure has to be a scatterer the radar resolves.

**Nothing has been downloaded or run.** The two structural candidates are 32 GB
and 22 GB. This is a search result.

### A dead endpoint that looked like a clean negative

The first availability pass used `service.iris.edu/fdsnws/availability` and
reported **0 of 553 hits with data**. That is a plausible-looking null and it was
an artefact: the host now returns an HTML page, whose lines all fail the "not a
comment" test, so every query parsed as empty. `service.earthscope.org/fdsnws/
dataselect` answers correctly and gives 315.

The same shape as item 12's zsh word-splitting and item 33's defocused null: **a
negative result from an unverified harness is not a negative result.** Assert a
known-good control before believing a zero.


## 60. Oroville Dam: 7 to 11 times below the floor, and worth having anyway

Item 59 found `BK.ORV`, a seismic station on Oroville Dam, inside a 30.4 s
Capella spotlight footprint with 168 KB of waveform spanning the aperture -- the
strongest candidate in the archive. The waveform was measured BEFORE spending
32 GB on the CPHD. `runs/oroville/2026-08-04-first/`.

```
  ground displacement, 0.03-3 Hz, instrument response removed

   channel          rms   peak-to-peak
       HHE    0.7024 um      2.1772 um
       HHN    0.3349 um      1.1421 um
       HHZ    0.7837 um      2.4332 um
       HNE    0.0132 um      0.0653 um     (accelerometer, less sensitive at low f)
```

Cross-checked on HHZ by a second route -- stage-zero sensitivity plus
frequency-domain integration instead of full response removal -- giving
0.5050 um against 0.7837 um. Two different treatments of the response agree to
36%, which settles the order of magnitude, and the order of magnitude is what
the conclusion rests on.

**Item 53's floor is 5.5 um RMS. The dam moved 7 to 11 times below it.** The SAR
could not have seen this, so Oroville is not a positive control and does not test
detection.

### What it is instead, which this project has never had

`USER_GUIDE` section 7 item 0 states the problem plainly: a null on real data
means nothing, because "nothing moved" and "this chain cannot see motion in this
data" produce identical output. Every real-data null in items 17 through 50 is
uninterpretable for that reason.

**Here an independent instrument separates them.**

```
  pipeline reports NO frequency  ->  a null that is CORRECT, and provably so
  pipeline reports A frequency   ->  a FALSE POSITIVE, and provably so
```

Items 25 and 55 measured static aspect-dependent FIXTURES returning confident
in-band frequencies, and item 55 concluded that no policy here is fit for that
scene type. Oroville is the same test on real data, on a real structure, with
independent proof that the structure was static -- which is the only way this
project has ever been able to interpret a real-data null.

### Two service notes

**`service.iris.edu/irisws/timeseries` does not hold BK data.** It returned 404
for every `BK.ORV` request including uncorrected ones, while the documented
`IU.ANMO` example returns 29000 bytes through the identical syntax. The control
is what separates "wrong request" from "no such data", and running it turned a
suspected syntax error into a holdings fact: BK is Berkeley, archived at NCEDC,
and `service.ncedc.org/fdsnws/dataselect` serves it.

**The location code is `00`, not empty.** `loc=--` returns 404 at this station.
Read it from the station service rather than assuming.

### Open

The CPHD download is running. The waveform was measured first because it decides
what the eventual result will MEAN, and it changed this from a positive-control
attempt into a negative-control one before any of the 32 GB was spent.

The wider search should now look for a hit where the instrument shows motion
ABOVE 5.5 um -- a site during an earthquake, or a structure under load. The 315
hits in `runs/screens/sensor-join/measurement_hits.csv` have not been screened
that way, and screening them costs one waveform request each.


## 61. All 315 screened: none has motion above the floor

Item 60 asked the obvious question of the other 314 hits. Every one screened:
response removed, 0.03-3 Hz, RMS displacement over its own aperture.
`runs/screens/sensor-join/MOTION_SCREEN.md`.

```
  315  synchronised hits
  305  returned a waveform that survived response removal
  303  on an actual seismometer or accelerometer channel
   52  above the 5.5 um floor -- before auditing
   14  above it once HV.UWB is excluded
    0  that survive the neighbour test
```

### Three layers of artefact, each caught by a different check

**Non-seismic channels.** The two largest readings in the whole screen were
`IU.RAR` `LWD` at 317 METRES and `BK.ORV` `LCE` at 34 metres. `LWD` is WIND
DIRECTION, `LCE` is CLOCK ERROR. The SEED channel code's middle letter gives the
instrument -- `H`/`L` seismometer, `N` accelerometer, `W` wind, `C` clock -- and
filtering on it removes both. Anything reading a waveform archive blind will hit
this.

**Chronically high stations.**

```
  HV.UWB    39 collects   median   38.23 um   max 1918.28   INSTRUMENT
  HV.BYL     5            median    9.03      max   64.35   INSTRUMENT
  HV.WRM    31            median    3.50      max  109.96
  HV.UWE    40            median    1.93      max    3.68   quiet
  HV.RIMD   40            median    1.73      max   14.98
  HV.OTLD   40            median    1.22      max    2.45   quiet
```

`HV.UWB` reads 1-1.9 mm on collect after collect for six weeks. Neither ambient
ground nor earthquakes arrive at every satellite overpass.

**The neighbour test, which is decisive.** Kilauea puts six to eight stations
inside one footprint, so every reading has controls on the same ground at the
same instant:

```
  2024-07-09T20:22:25   UWB 1918.28 um
     WRM 4.73  UWE 2.24  RIMD 1.88  BYL 1.52  KKO 1.49  SDH 1.18  OTLD 1.11
```

UWB reads **400x its neighbours**. That settles it without needing to know what
is wrong with the station, and it is only possible because the same footprint
holds several instruments -- which is an argument for preferring dense networks
in any future search.

### The one candidate that looked real, and does not hold

`CAPELLA_C10_SP_CPHD_HH_20240609091921`, 2024-06-09T09:19:21Z: WRM 44.50 um and
RIMD 14.98 um with the rest at 2.3-3.7 um against a 1.6 um median. Spatially
coherent, which is what a real source looks like.

- **No catalogued event** in the aperture. USGS lists nothing; the nearest is
  M1.8 at 09:25:53, six and a half minutes after the radar had gone.
- **WRM is chronically high**, median 3.50 and above the floor on eight separate
  collects, so its 44.5 um is WRM being WRM.
- What is left is RIMD at 14.98 um against its own median of 1.73, with others at
  about twice background -- consistent with weak volcanic tremor during Kilauea's
  June 2024 activity, and too weak and too unattributed to build a control on.

### What this establishes, and what to do next

**No collect in the open Capella archive has independently confirmed motion
clearly above this instrument's floor.** That is stronger than `DATASETS.md` has
been able to say, which was that no such collect is KNOWN.

What would count: an instrument above **5.5 um RMS in 0.03-3 Hz**, on a
seismometer channel, at a station that is not chronically high, corroborated by a
neighbour or a catalogued event, inside a spotlight or dwell collect. The screen
applies all of those and re-runs against any new catalogue.

**Invert the search.** The likely place is a site DURING AN EARTHQUAKE, so start
from the earthquake catalogue and look for a collect over the epicentre within
the aperture, rather than starting from collects and hoping. There are 939
spotlight collects and rather more earthquakes.


## 62. The earthquake route fails on duty cycle, and here is the number

Item 61 proposed starting from the earthquake catalogue rather than from
collects. It does not work either, and the reason is arithmetic rather than bad
luck. `runs/screens/sensor-join/QUAKE_SEARCH.md`.

11,977 events of M >= 5.0 worldwide 2020-2026 against 939 spotlight and
sliding-spotlight collects, with P (8 km/s), S (4.5) and Rayleigh (3.5) arrivals
computed at each footprint centre.

```
  7 collect-earthquake pairs with a phase arriving during the aperture
  ALL teleseismic, 8,000-18,000 km, M5.0-5.8, all P or S -- no Rayleigh
```

P-wave displacement at 8,000 km from an M5.4 is well under 1 um, an order of
magnitude below the 5.5 um floor. None is usable, and the one phase that would
have carried real displacement -- Rayleigh -- never coincided.

### Why looking harder cannot fix it

```
  total aperture      20,013 s across 2,036 days
  duty cycle          1.14e-04
  M>=5.0 within 200 km of a collect site, any time:   1,242
  EXPECTED number coinciding with an aperture:        0.14
```

**Finding zero is what the arithmetic predicts.** An expectation of one needs
about seven times this archive, roughly 39 hours of spotlight over the same
sites. And 0.14 is generous: it counts M>=5 out to 200 km where the ground motion
is already below the floor, so the true expectation is lower.

**The duty cycle is the whole problem.** A satellite that stares for thirty
seconds cannot be expected to be looking when the ground moves. That is a
property of the observation, not of this archive, and no amount of catalogue
mining changes it.

### What is left, and it is not a search

1. **A structure that moves all the time.** Bridges under traffic, tall buildings
   in wind, wind turbines -- tens of microns continuously, no coincidence needed.
   Their instruments live in structural-health-monitoring archives (NTNU open
   data, Zenodo) rather than FDSN, so the join needs a SAR collect over that
   specific structure during its monitoring period.
2. **A tasked collect**, which is commercial rather than open.
3. **A lower floor.** Items 51-53 took it from 0.125 mm to 0.0055 mm by removing
   carrier residual. The quiet sites screened in item 61 sit at 1-2 um ambient,
   so roughly another factor of four would bring ORDINARY GROUND into range --
   turning all 315 hits into candidates instead of none.

**Route 3 is the only one this project can pursue alone**, and item 53 recorded
the returns falling fast: 38x then 4.5x from successive carrier terms, with a
quartic worth perhaps two.

That reframes the whole data problem. It has been stated throughout as "no
collect has confirmed motion". The truer statement after items 59-62 is that
**the instrument's floor sits a factor of three to four above ordinary ground
motion**, and closing that gap would supply hundreds of controls from data
already indexed.


## 63. Chasing the factor of four: finer cells help a little, Umbra not at all

Item 62 reframed the data problem -- the floor sits 3-4x above ordinary ground
motion, and closing that gap makes all 315 synchronised hits usable. Two routes
tested. `runs/giza/2026-08-04-cell-sweep/`.

### Finer grid cells: real, and worth 1.4x

The carrier the estimator removes is `(4*pi/lambda) * dX * dx / R` where **`dx`
is the scatterer's offset from its pixel centre** (`microm.c:623`). Giza runs
1.0 m cells on a collect with 0.051 m azimuth resolution -- twenty times coarser
-- so a scatterer sits up to half a metre from its pixel centre, and that offset
IS the carrier. Predicted 0.553 rad/look at 1.0 m against a measured 1.1-1.9,
same order.

```
  cell 1.000 m   artefact  70.7x
  cell 0.250 m   artefact  36.3x     halved
  cell 0.125 m   artefact 159.0x     WORSE
```

**Not monotone.** 0.25 m halves the artefact, which is 1.4x in the amplitude
floor since the floor goes as its square root. 0.125 m is four times worse.

The likely cause is scene extent rather than sampling: at 96 cells, 0.125 m spans
12 m and a 32-pixel window is 4 m, which holds too little scene for the
surrounding statistics the estimator leans on. Hypothesis; the non-monotonicity
is the measurement.

### Umbra: disqualified on dwell, with 4048 products

Umbra's open archive holds **4048 CPHD** against Capella's 707 spotlight, at
finer resolution. Dwell, sampled from 250:

```
   median 3.50 s, 90th 7.62 s, max 23.50 s
   dwell >= 10 s   2.9%   ~116 of 4048
   dwell >= 15 s   0.4%   ~ 16
   dwell >= 30 s   0.0%      0
```

Capella spotlight: median 22.5 s, 646 at >= 15 s, 156 at >= 30 s.

Umbra's median gives `df` = 0.29 Hz, coarser than most of the 0.3-3 Hz target
band, and 128 looks over 3.5 s is 27 ms each. **Disqualified by exactly item 58's
arithmetic** -- per-target observation time in seconds -- and six times the
products does not change it. Item 36 called the 1.99 s Panama collect coarse
without quantifying; this quantifies it and generalises it to the archive. Umbra
is built for resolution and revisit, not for staring.

### The arithmetic of the remaining factor

Finer cells 1.4x. A quartic carrier term is worth about 2x in artefact by item
53's trend, so 1.4x in amplitude. Together roughly 2x against the 4x needed.

**The untried lever is that the estimator reads ONE pixel per window and
discards the other 1023.** Combining the K brightest coherently gives up to
sqrt(K) if they share the motion -- 3x at K=9, which is the missing factor.
Whether they share it is item 15's precondition, and it has never been tested.


## 64. Multi-pixel combination fails, and the literature said so first

Item 63 named the missing factor: the phase estimator reads ONE pixel per window
and discards the other 1023. Combining the K brightest should give sqrt(K) --
3x at K = 9, exactly the gap. `runs/giza/2026-08-04-multipixel/`.

```
 pixels   artefact  signal @0.03125mm  signal/artefact  vs K=1
      1       70.7             2310.4             32.7   1.00x
      4      181.9             4340.7             23.9   0.73x
      9      159.9             4363.1             27.3   0.83x
```

**The artefact grows with K faster than the signal does.** Signal rises 2310 to
4341; artefact rises 70.7 to 182. The ratio that decides detection gets WORSE.

### The literature has done this properly for fifteen years

**SqueeSAR** (Ferretti et al. 2011) and the phase-linking family combine pixels
in exactly this situation. Two differences from what was implemented here, and
they are the whole result:

**Which pixels.** SqueeSAR selects STATISTICALLY HOMOGENEOUS PIXELS by a
two-sample Kolmogorov-Smirnov test on their amplitude distributions -- pixels
drawn from the same scattering population. This took the K BRIGHTEST, which is a
different set and generally the wrong one: the second and third brightest pixels
in a window are usually DIFFERENT SCATTERERS, each with its own sub-pixel offset,
its own carrier, and its own carrier residual. Averaging them adds artefacts
instead of averaging noise down, which is what the numbers show.

**How to combine.** Phase linking estimates the phase history from the sample
COVARIANCE MATRIX by maximum likelihood, not by averaging phases estimated
independently per pixel. Independent estimation discards the cross-terms that
carry most of the information.

### And this project already had the estimator

`src/core/phaselink.c` implements the maximum-likelihood phase estimate over a
stack -- the coherence matrix and the fixed-point iteration
`x_n <- exp(j*arg(sum_m Gamma_nm x_m))`, which is the standard phase-linking
solver. Written for the split-band route, never applied to the sub-aperture
stack.

So the right experiment is not the one run here: select statistically homogeneous
pixels in the window, form the covariance across SUB-LOOKS rather than across
passes, and hand it to the existing linker. The physics differs from multi-pass
InSAR -- sub-looks differ in squint rather than in time and baseline -- but the
estimator structure is identical.

**This is the third time this project has built something the field already
had** (item 32's argmax, item 56's AM check reaching item 25's conclusion, and
now this). CLAUDE.md's rule to search first exists for exactly this, and the
search was run after writing the code rather than before.

### What the measurement is worth

It bounds the naive version, which is what anyone tries first and which looked
like free sqrt(K). It does NOT test item 15's precondition -- whether several
pixels share the motion -- because a brightest-K construction cannot answer that.

`--pixels` stays, defaulting to 1, which reproduces every earlier measurement.


## 65. SqueeSAR done properly is worse, because homogeneous pixels share the artefact

Item 64 found brightest-K combination failing and identified the literature's
method -- KS-test selection of statistically homogeneous pixels, then
maximum-likelihood phase linking over the covariance, already half-implemented in
`phaselink.c`. Implemented properly. `runs/giza/2026-08-04-shp/`.

```
 pixels   artefact  signal @0.03125mm  signal/artefact
      1       70.7             2310.4             32.7
      9     1326.3             5391.8              4.1
     25     1326.3             5391.8              4.1

  item 64's brightest-K at K=9, for comparison:      27.3
```

**Worse than the naive version and eightfold worse than one pixel.** The signal
does rise, 2310 to 5392 -- phase linking genuinely extracts more from the stack --
but the artefact rises NINETEENFOLD and the ratio that decides detection falls.

K=25 is identical to K=9, so the KS test selects about nine homogeneous pixels
per window and the cap is not binding.

### Why, and it is item 47's argument in a new place

Averaging beats down noise that is INDEPENDENT between the things averaged. The
carrier residual is not independent between neighbouring pixels: they sit at
similar sub-pixel offsets, so they carry similar carriers and similar residuals
after the cubic fit of item 53.

**Statistically homogeneous pixels are homogeneous in the artefact too.** The KS
test selects pixels from the same scattering population, which is precisely the
set that shares a residual, and a maximum-likelihood estimator over the whole
covariance then estimates that shared term very precisely. It amplifies the
common mode because it is doing its job well.

This is the same shape as item 47 (a local background cannot separate a real
sideband from a real tone) and item 55 (the local peak addresses coloured noise,
not spurious peaks generally): **a null, a normalisation or an average only helps
against variation across the things being combined.**

### What it closes

Items 62-63 named a missing factor of four and identified multi-pixel combination
as the one untried lever with the headroom. **It is not a lever.** Both its naive
and its literature forms make detection worse here, structurally rather than by
mis-tuning.

The remaining routes are already measured and insufficient: finer cells 1.4x
(item 63), a quartic carrier term perhaps 1.4x by item 53's trend. Roughly 2x
against the 4x needed, with nothing else identified.

**The honest position: this instrument's floor is within about a factor of two of
what this approach can reach.** Closing the gap to ordinary ground motion, which
items 60-62 showed is what would supply hundreds of controls, needs something not
yet identified rather than more of what is here.

`--pixels` defaults to 1, reproducing every earlier measurement.


## 66. The ground-truth survey corroborates items 59-62, and uses the wrong floor

`docs/GROUND_TRUTH_DATASETS.md` surveys public structural-motion datasets against
SAR collects. It reaches items 61 and 62's conclusion independently and by a
different route -- **no publicly downloadable positive benchmark that is both
synchronised and above the motion floor and native to this pipeline** -- which is
worth more than either finding alone.

It also adds candidates this project did not have: South Portland Bridge (ten
2048 Hz accelerometers, 1.5-2 mm/s peak LOS, matched modal peaks), the Trento
Capella shaker (15 mm at 2 Hz), Hardanger Bridge with Sentinel-1 during two named
storms, and a Mexico City ICEYE pair.

### The floor it compares against is the per-look one

`validate` reports a per-look CRLB -- 0.2017 mm/look on ICEYE. Item 53 measured
the END-TO-END detection floor, after 128 looks and a periodogram, at **0.0055 mm
RMS**. That is **37x lower**, and the difference is coherent averaging doing what
it is supposed to.

```
                                          measured    vs 5.5 um floor
  Kilauea HV stations, median (item 61)      1.70 um       0.31x  below
  Kilauea HV.RIMD, 2024-06-09 (item 61)     15.00 um       2.73x  ABOVE
  Mexico City G.UNM.00 (doc)                  0.60 um       0.11x  below
  Oroville BK.ORV (item 60)                   0.78 um       0.14x  below
```

The page calls Kilauea "about 700 times below the phase floor". Against the
measured floor it sits AT it: 3x below at the median, and one station 2.7x ABOVE
on 2024-06-09. **Item 61 rejected that reading on ATTRIBUTION, not amplitude** --
no catalogued event in the aperture, and `HV.RIMD` is only intermittently high.

Oroville and Mexico City remain genuinely below by an order of magnitude, so the
page's conclusion stands. What changes is the margin, and therefore what a factor
of two would buy: items 63-65 put roughly 2x within reach, which does not rescue
Oroville or Mexico City but would put ordinary Kilauea ground within range.

### What is actually blocking, restated

The page's strongest entry, South Portland Bridge, is native to `mmotion --cphd`,
independently synchronised, AND demonstrably above the floor at 1.5-2 mm/s -- some
300x the detection floor. **It is blocked by ACCESS, not by physics or by
sensitivity.** That is a different problem from the one items 59-65 have been
solving, and it is the one worth solving: an author request rather than another
factor of two.


## 67. Public sources only: one pairing survives, and it changes the experiment

Item 66's survey lists the strongest candidate as South Portland Bridge --
native, synchronised, 300x above the floor -- behind an author request. With
public sources only that is out, and so is most of the list. What survives:

**Hardanger Bridge fails on dwell, before the missing reader matters.** It needs
Sentinel-1, which this project cannot read; but adding a reader would not help.
Sentinel-1 IW sweeps its beam electronically, so a point is lit for a fraction of
a second whatever the product length:

```
  IW (TOPS)            per-target dwell 0.16 s    df = 6.31 Hz
  IW, single burst                      0.63 s    df = 1.58 Hz
  SM stripmap                           0.63 s    df = 1.58 Hz
```

That is item 58's arithmetic again, and `df` around 1 Hz is coarser than the
bridge modes of interest. **A bridge moving centimetres in a storm is still
unmeasurable if the radar only watches it for six tenths of a second.**

**Mexico City and Oroville are genuinely below the floor** by an order of
magnitude (item 66).

**That leaves Kilauea, and only Kilauea**: public on both sides, native to
`mmotion --cphd`, and dwell-adequate at 25-40 s.

### What Kilauea actually supports, which is not a detection test

```
  51 collects, 249 station-readings, HV network, UWB excluded
  station RMS: median 1.64 um, 90th percentile 3.60 um, max 109.96 um
  the 5.5 um floor sits at the 95th percentile
```

Ninety-five percent of readings are below the floor, so no single collect is a
positive control -- which is what items 60 and 61 already concluded one collect at
a time.

**But 51 collects with 6-8 recording stations each is not one experiment, it is
51.** The test that ensemble supports is a CORRELATION: does the pipeline's
reported per-window displacement track the seismometer's RMS across 51
independent acquisitions of the same ground? That needs no single collect above
the floor. It asks whether the instrument responds to real ground motion at all,
which is a weaker claim than detection and a stronger one than anything this
project currently has.

It would also be the first test here whose truth varies. Every injection
experiment has one known answer per run; this has 51 known answers spanning two
orders of magnitude in amplitude, and a pipeline that is measuring nothing would
show no correlation with them.

**Not attempted.** It needs the 51 CPHDs, which at Capella spotlight sizes is of
order a terabyte, and that is the real cost rather than any missing method.


## 68. The additional public SHM datasets: no pairing, but a better injection

`GROUND_TRUTH_DATASETS.md` lists public structural-motion repositories and names
the ETH Aventa AV-7 research wind turbine as "the strongest new spatially
joinable candidate", asking for a CPHD polygon search at 47.520056 N, 8.682139 E
before downloading multi-gigabyte packages. Item 59's harvest already holds 939
Capella spotlight footprints, so the search is free.

```
  ETH Aventa AV-7 turbine, Winterthur     0 footprints contain it; nearest centre 57 km
  Hell Bridge Test Arena, Norway          0;  nearest 1076 km
  Route 345 Bridge, New York              0;  nearest   66 km
```

**None is inside any Capella spotlight footprint.** Umbra has 56 named task sites
of which only *Jeddah Tower* and *Port of Rotterdam* are structural at all, and
neither is one of these; and item 63 disqualified Umbra on dwell regardless. That
matches the doc's own screen, which found no simultaneous pair for Hell Bridge.

So the spatial join is closed for these, and cheaply -- the polygon search cost
nothing because the footprints were already harvested. **Do the join before the
download**, which is what the doc advised and what item 57 built the tool for.

### What they ARE good for, which the doc half-says

These are real structural response records: eleven accelerometers plus two strain
gauges at 200 Hz on a wind turbine, 100 Hz acceleration and strain on a
full-scale steel bridge with known damage states, dual-axis accelerometers under
vehicle loading on a road bridge.

**Every injection this project has ever run was a pure sinusoid.**
`rs_simulate_inject_vibrator()` takes one frequency and one amplitude. A real
structure is multi-modal, non-stationary, and amplitude-modulated by whatever is
exciting it -- which is precisely the regime items 25, 55 and 56 found the
selection policies failing in, on a fixture built to imitate it.

Injecting a MEASURED waveform instead of a sine would test:

- whether `rs_spectrum_best_window()` reports the dominant mode when there are
  several, rather than the loudest artefact;
- whether the band floor of item 37 removes a real structure's lowest mode;
- whether the local peak of item 47 survives a spectrum that is genuinely
  multi-line rather than one tone plus noise.

None of that needs a co-located collect. It needs the waveform, the existing
injection path taking a series instead of a frequency, and the fixtures already
here. **That is the cheapest untried experiment left**, and unlike items 63-65 it
is not chasing a factor of two -- it is asking whether the reporting stage works
on realistic motion at all.


## 69. A real waveform instead of a sine: the reported answer goes wrong

Item 68 said this was the cheapest untried experiment left. It is done, and it
is the most damaging result in this file.

### The path

`rs_simulate_inject_waveform()` drives the injected scatterer from a measured
displacement record; `sim_cphd --wave FILE[,RATE_HZ]` drives the whole
`--clutter-vib` patch from one, which is the fixture that actually recovers.
`mmotion --inject-wave` exposes the first. Both normalise the record to unit
peak, so `AMP_MM` keeps meaning peak vertical displacement and a waveform run is
directly comparable with a sine run at the same amplitude.
`rs_simulate_inject_vibrator()` is unchanged in behaviour -- it is now a wrapper
passing no waveform -- so every earlier measurement stands.

### The waveform

Oroville Dam, `BK.ORV.00.HNZ`, the M5.5 Lake Almanor earthquake of 2023-05-11,
from NCEDC, response removed to displacement, highest-energy 20 s slice:

```
  1.200Hz(1.00) 1.150Hz(0.80) 1.250Hz(0.78) 0.750Hz(0.77) 1.400Hz(0.54) 1.100Hz(0.48)
```

Six lines within a factor of two, no dominant mode. A sine has one line at 1.00
and nothing else; that is the whole difference under test. Oroville was taken
over the Zenodo SHM packages of `GROUND_TRUTH_DATASETS.md` because it is free,
public, already this project's nominated static negative control (item 60), and
a real structure -- and item 68 showed those sites have no collect over them
either, so the download buys shape and nothing else.

### The result

Identical scene, seed, amplitude and processing. Only the shape of the
displacement in time differs.

| | injected | reported | consensus |
|---|---|---|---|
| sine | 0.500 Hz | **0.504 Hz** correct | 0.504 Hz, 4/49 |
| real record | six modes, 0.31-0.58 Hz | **1.966 Hz**, no mode within 1.3 Hz | 0.605 Hz, 4/49 |

Window-level, counting a window as a hit if its dominant frequency is within one
bin of ANY of the six modes: **20% for the sine, 10% for the record.** The
comparison is generous to the waveform by construction -- the sine is scored
against a target six bins wide when it has one line in it -- and it still scores
twice as well.

The record is replayed at 1/2.4 speed so its cluster lands at 0.46-0.58 Hz. That
is necessary, not cosmetic: a 1.20 Hz SINE does not recover in this fixture
either (2.520 Hz reported), by item 13's response ceiling, so comparing a
recoverable sine against an unrecoverable waveform would have measured the
ceiling rather than the waveform. Time-scaling preserves every ratio between the
modes and the entire envelope. The sine run reproduces the 2026-08-01 e2e result
to three decimals, so the fixture and operating point are unchanged.

### What it means

The tracker is NOT blind to the record: consensus lands at 0.605 Hz against a
true mode at 0.583, within half a bin of the fifth-strongest line. It is
`rs_spectrum_best_window()` -- prominence, the reported answer -- that picks
1.966 Hz over all six.

That is items 7-9 again and worse. There the selection policy discarded a
carrier the tracker had recovered in most windows. Here the recovered energy is
**split across six modes**, so no single one is ever prominent enough to win and
prominence goes to whatever noise line is tallest. **A statistic that reports one
frequency is the wrong shape of answer for a structure**, and no threshold on it
fixes that. What a structure needs reported is a modal SET, scored as a set.

The honest reading of every earlier synthetic recovery in this project is now
that it was measured on the easiest possible motion. Item 25 found item 14's
recovery not surviving aspect-dependent scattering; this finds the reported
policy not surviving realistic motion. The two gaps are independent and both
sit between the fixtures and a real collect.

### Amendment from item 70: the ground truth above is at the WRONG RESOLUTION

The six-mode table is the accelerometer's own spectrum at 100 Hz over 20 s. What
the DWELL can see is coarser -- `df` = 0.0500 Hz, and the record replayed at
1/2.4 speed and sampled at 128 looks gives:

```
    0.550 Hz  (bin 11)  rel power 1.00
    0.600 Hz  (bin 12)  rel power 0.43
    0.300 Hz  (bin  6)  rel power 0.28
```

The four modes at 1.10-1.25 Hz are separated by 0.021-0.026 Hz after scaling,
inside one Hann main lobe, so they are ONE peak here. **"Six modes, no dominant
line" is true of the instrument and false of the measurement**: at the dwell's
resolution the record has a dominant at 0.550 Hz, 2.3x the next feature.

That makes this result cleaner rather than weaker. There IS a single correct
answer, 0.550 Hz, and the reported 1.966 Hz is not near it. Score against the
record processed to the SAME resolution, which is what Lotti et al. do (item 70)
and what "within one bin of any true mode" above does not.

Run: `runs/synthetic/2026-08-04-real-waveform/`.


## 70. A modal set is the right shape of answer and does not fix item 69

Item 69 concluded that a statistic reporting one frequency is the wrong shape for
a structure. `rs_spectrum_modal_set()` reports a set instead: each window
nominates its RS_MODAL_PER_WINDOW strongest peaks against their own local
background (item 47), nominations are separated by RS_SPECTRUM_LEAKAGE_BINS so a
Hann skirt cannot be nominated twice, and a bin is reported when its SUPPORT --
the number of windows nominating it -- clears a threshold derived from a
binomial null with a family-wise budget of half a bin over the band. Nothing is
tuned; `support_min` is computed FROM the nomination count, so raising the count
widens what can be found without loosening what is believed.

### What the literature says, read before the write-up and after the code

**The SAR micro-motion field already reports more than one frequency, and does it
with no acceptance criterion at all.** Lotti et al., *Vibration-based Structural
Health Monitoring from single-pass SAR images*, EVACES 2025 -- South Portland
Street Suspension Bridge, Glasgow, Umbra-04 spotlight:

> "The two highest peak frequencies -- in descending order -- extracted from the
> m-m time history are compared with those from the ground signal in Table 3."

Top two peaks by height. No threshold, no support test, no clustering. It half
fails on their own data: *"the second peak is not detectable for pixels 1 and 2,
as its magnitude is much lower than the peak observed for pixels 3 and 4"* --
two of four pixels yield one mode.

Their operating point, worth having on the record:

| | |
|---|---|
| Umbra-04 spotlight, 9.6 GHz | effective acquisition 9.252 s |
| sub-aperture 0.271 s, aperture fraction 0.375 | **overlap 0.17-0.20** |
| N = 34-36 sub-looks | f_s 4.67-4.95 Hz, **df 0.138 Hz** |
| mode 1 residual | 0.016, 0.054 Hz on two pixels; 0.154, 0.151 Hz on the other two |
| mode 2 residual | 0.013, 0.080 Hz; undetectable on two pixels |
| Pearson rho against accelerometers | **0.46, 0.47, 0.39, 0.33** |

Three things there bear on this project. That overlap is 0.17-0.20, a second
source contradicting item 13's "the published campaigns use ~99% overlap", now
with exact figures. Their rho tops out at 0.47, so "validated against
accelerometers" means moderate correlation and not a match -- a useful
calibration for what success looks like in this field. And they track four bright
pixels SEPARATELY and never combine them, quietly consistent with items 64-65.
The site is the South Portland Street Bridge, which item 66 called the best entry
in `GROUND_TRUTH_DATASETS.md` and blocked by ACCESS: the pairing exists and is
published, and what is missing is their data, not their target.

**The OMA field has the mature machinery and it is the same principle.**
Automated operational modal analysis settled "which peaks are real modes" with
the STABILIZATION DIAGRAM: identify modes at many model orders, and physical
modes REPEAT across orders while spurious ones scatter. Modern versions cluster
the diagram, explicitly designed to need no user-specified thresholds (Reynders
et al., three-stage clustering; later hierarchical-density variants). What
`rs_spectrum_modal_set()` does is repetition-as-evidence with the SPATIAL WINDOW
substituted for model order. That is a legitimate analogue and it is not new;
the derived-threshold instinct matches the field's stated goal, but the idea is
theirs.

### The measurement

Same two scenes as item 69, at the resolution that item's amendment establishes.

| policy | sine, true 0.500 | record, true 0.550 |
|---|---|---|
| `rs_spectrum_best_window()` | 0.504 correct | **1.966** |
| `rs_spectrum_consensus()` | 0.504 correct | **0.605** -- bin 12, the second feature |
| `rs_spectrum_modal_set()` | 0.504, exactly one mode, correct | **2.671** |

On the sine the new policy behaves exactly as it should: it finds ONE mode,
because there is one, and gets it right. On the record it reports one mode and
that mode is wrong.

The threshold came out at 11 of 49 voting windows, with 0.39 bins expected to
clear it by chance. The reported bin had support 12. **The true bin also had
support around 12, and so did a noise bin at 2.671 Hz** -- support alone does not
separate them, and the tie was broken by local ratio, which went the wrong way.

### What this establishes

Reporting a set is the right shape and it is not sufficient. The discriminator
has to be something other than cross-window support, and item 11 said so before
this was built: agreement is blind to anything the processing puts in every
window identically. A modal set inherits that blindness whole.

What separates a real mode from a noise line in the OMA literature is not merely
that it recurs but that it recurs WITH CONSISTENT PROPERTIES -- damping, mode
shape, and the MAC between shapes at different orders. This project has a spatial
analogue available and unused: a real mode's amplitude across the window grid is
a mode SHAPE, smooth and contiguous, where a noise line's is not. That is
`rs_spectrum_centroid()`'s clustering applied per candidate mode rather than to
the single reported peak, and it is the next thing to try. It is also the same
answer item 69 pointed at from the other side.

The policy is committed because it gates nothing, it is correct where a correct
answer exists, and it makes the modal structure visible in the output. It is not
committed as a fix.


## 71. Rank the set by shape, and the failure moves upstream of selection

Item 70 left the discriminator named but unbuilt: what separates a real mode in
the stabilization-diagram literature is not that it recurs but that it recurs
with a consistent MODE SHAPE. The spatial analogue is that a vibrating structure
occupies contiguous ground, so the windows nominating a real mode form a BLOCK
while a noise line's are scattered.

`rs_mode_t` now carries `n_contiguous`, the largest 4-connected block of
nominating windows, and it is the ranking key -- support and local ratio are only
tie-breaks. The geometric floor of `rs_spectrum_consensus()` applies and is
ENFORCED rather than warned about, because this function selects: windows are
laid at half their width, so a resolvable target falls in a 2x2 block at minimum
and a largest block below four cannot be a spatially resolved mode.

### It fixes the safety half

| policy | sine, true 0.500 | record, true 0.550 |
|---|---|---|
| support-ranked (item 70) | 0.504, correct | **2.671**, wrong |
| **contiguity-ranked** | **0.504, block 4, correct** | **refuses** |

The noise bin at 2.671 Hz had the same support as the true bin and lost on shape.
That is the first policy here to turn a confident wrong answer into a refusal on
the multi-modal case -- the failure mode items 25, 55 and 56 all recorded and
none fixed.

`RS_MODAL_PER_WINDOW` is the recall knob and it is NOT free, which contradicts
what item 70's header claimed for it. At 12 nominations the sine gains a second
spurious mode (1.159 Hz, block 5) and the record gains a CONFIDENT WRONG ANSWER,
1.008 Hz at block 14. The threshold adapts to the nomination count as designed,
and a scattered artefact still becomes contiguous once enough bins are nominated
per window. **Six is the measured safe setting**; the header claim that raising
it "widens what can be found without loosening what is believed" is now known to
be false and has been corrected.

### And it shows the recall half is not a selection problem at all

`--probe-hz` (item 39) at each scene's own true dominant, prominence per window:

```
  record, probe 0.550 Hz            sine, probe 0.504 Hz
   1.7 0.8 0.4 0.9 0.9 0.4 2.0       3.3 0.5 1.7 1.7 3.3 2.5 1.2
   0.7 0.9 1.1 0.4 0.0 1.5 0.1       1.8 0.3 0.1 1.6 0.0 2.5 0.6
   4.0 1.5 0.1 2.2 0.1 0.2 1.1       0.5 0.4 1.5 0.4 0.4 0.5 4.8
   1.1 0.1 0.0 0.8 1.0 0.4 0.4       1.4 0.5 0.9 2.0 1.3 2.5 1.4
   0.5 0.4 4.7 0.2 4.7 2.7 1.2       4.4 0.0 3.4 2.0 0.5 1.0 0.3
   1.3 0.8 2.0 0.2 1.7 0.2 0.0       2.6 2.1 2.0 1.4 0.0 1.4 0.5
   6.0 1.4 2.6 0.2 0.3 1.8 0.7       0.9 6.2 3.0 0.9 2.9 7.4 9.5
  median 0.84   max 6.0             median 1.44   max 9.5
```

**Neither is a patch.** The record is about 1.7x weaker than the sine at its own
dominant, and both are scattered. So the true mode is not sitting in the tracking
waiting to be selected: a whole-scene `--clutter-vib` fixture, where every
scatterer moves together, does not produce a contiguous field of windows carrying
the driving frequency even for a SINE. The sine's reported answer is right
because a handful of windows carry it strongly enough to win, not because the
scene agrees.

That relocates item 69 and reverses part of its conclusion. Item 69 said the
tracker was not blind to the record because consensus landed near a true mode --
but that was bin 12, the SECOND feature, and at the dominant there is nothing
spatially coherent to find. **The reporting stage is not what loses a real
structure's motion. The per-window spectrum is.**

### Where that points

Splitting energy across modes costs ~1.7x in per-window evidence at the dominant,
and the record is also NON-STATIONARY across the dwell -- an earthquake's envelope
rises and decays inside the 20 s. A periodogram over the whole dwell is the wrong
estimator for that, and it spreads the energy exactly as observed. This is item
56 arriving from the other direction: there the non-stationarity was in the
SCATTERER, faded by the aspect lobe; here it is in the SIGNAL. Item 56 concluded
"a test for non-stationarity is what this needs and none exists here", and that
is still true and is now the blocking item for realistic motion, not a refinement
of it.

A 20 s dwell at 128 looks leaves little room to subdivide -- halving the record
halves `df` to 0.10 Hz, which is coarser than the mode spacing this record has.
So the honest statement is that single-periodogram micro-motion has a
stationarity precondition nothing in this project has ever stated, every
synthetic recovery here satisfied it by construction, and a real structure under
transient excitation does not.


## 72. The short-time estimator: built, and it fails its controls

Item 71 named the blocking problem -- a whole-dwell periodogram assumes the
motion lasts the dwell -- so `rs_spectrum_maxhold()` cuts the tracked series into
overlapping segments, transforms each, and takes the per-bin MAXIMUM. Exposed as
`mmotion --stft L`.

The design follows the literature rather than invention. A short-time transform
preserves transients that a multitaper estimator smears, and pays in frequency
resolution and variance; multitaper is the better estimator when the signal is
stationary and weak, and the wrong one when it is strong but brief. The maximum
rather than Welch's mean is the standard analyser mode for transient vibration
for the reason that matters here -- linear averaging suppresses intermittent
events, which is the dilution being escaped. **Peak-hold is not a linear
operation**: it is valid for auto-power spectra, which this is, and NOT for FRF
or coherence, so nothing derived from `--coherence` should be read from a
max-hold run.

### The result, with the controls that decide it

| | whole dwell | `--stft 64` |
|---|---|---|
| sine, true 0.500 | **0.504 correct** | **3.175 WRONG** |
| record, true 0.550 | 1.966 wrong | 0.504 |
| **static, nothing moving** | -- | **2.974, prominence 4.9** |

The record's 0.504 Hz looks like a recovery and **is a coincidence**. The static
control returns an equally confident in-band answer from a motionless scene, and
the estimator BREAKS the sine that the whole-dwell periodogram recovers. One
frequency matched once is what this project's own bar exists to reject.

`L` is not a free parameter either, and it fails the same way
`RS_MODAL_PER_WINDOW` did at 12. At `L = 32` the record reports 3.175 Hz and the
modal set -- which refuses at 64 -- confidently returns THREE modes, 0.907, 1.915
and 3.175 Hz, none of them true, at blocks 4-5. Shorter segments manufacture
exactly the contiguous support item 71's shape test was built to demand.

### Why it fails, and what that costs

Two effects, both predicted in the header and both underestimated.

Resolution halves at `L = 64`, from 0.0504 to 0.1008 Hz. And a maximum over
segments sits above their mean, so the floor rises everywhere -- prominence at
the sine's own peak falls from 8.3 to 4.4. The signal is diluted by segmentation
faster than the transient is recovered by max-hold, because **this fixture's
motion is not actually transient enough to pay for it**: the injected record is
20 s of continuous shaking, not a burst with a low duty cycle. The estimator is
built for a case the fixture does not contain.

So item 71's diagnosis stands and this is not its remedy. It also does not
refute the diagnosis: what was measured here is that segmenting a 128-look
series costs more than non-stationarity does at THIS duty cycle. A record with a
genuinely short burst -- an impulse response, a vehicle crossing -- is the case
`--stft` was written for and has not been tried.

The function is kept because it is correct, documented, opt-in, prints its own
resolution cost, and is the only time-frequency estimator here. It is NOT a
result, and no run should quote it without the static control beside it.

**The honest state after items 69-72: nothing in this project recovers a real
structure's motion, and the reason is now located rather than guessed.**


## 73. A real burst: the shape-ranked modal set answers, and the control refuses

Item 72 ended with one hypothesis left for `--stft`: it was built for a
low-duty-cycle burst and the fixture contained 20 s of continuous shaking. So a
burst was built from the same Oroville record -- the window of the Lake Almanor
earthquake containing the ARRIVAL rather than the coda, chosen by searching all
20 s slices for the one concentrating most energy in a quarter of its length.

```
  burst_20s   start 17.2 s   76% of energy in the strongest 5 s of 20
              peak 91.00 um  rms 20.97 um
              at the dwell's resolution: 0.300Hz(1.00) 0.350Hz(0.65) 0.200Hz(0.38)
              tracker-side: 38% of samples above a quarter of peak
```

### The result, with its control

| scene | `best_window` | **modal set (whole dwell)** |
|---|---|---|
| **burst**, true 0.300 / 0.350 | 2.571 Hz wrong | **0.353 Hz, block 6, support 12/49** |
| sine, true 0.500 | 0.504 correct | 0.504 Hz, block 4 |
| record (continuous), true 0.550 | 1.966 wrong | refuses |
| **static, nothing moving** | 2.974 Hz, prominence **8.6** | **refuses** |

0.353 Hz is the record's second mode to three decimals and is 1.05 bins from its
dominant. **This is the first policy in this project that returns a true
frequency from a real structural waveform and refuses on a motionless control
through the identical processing.** Prominence, on the same four runs, is wrong
on the burst and confidently wrong on the static scene at a HIGHER prominence
(8.6) than it gave the burst (8.5) -- item 38's failure again, unchanged.

### And it kills `--stft`

The burst is the case `--stft` was written for, and it makes it worse: the
short-time run reports 2.520 Hz and the modal set, which answers correctly on the
whole-dwell spectrum, refuses. Segmentation costs more than the transient
recovers even at 76% energy concentration. Item 72's remaining hypothesis is
disproved rather than untested. `rs_spectrum_maxhold()` stays because it is
correct, opt-in and documented, but nothing here has ever been improved by it.

### What this is NOT

One burst, one seed, one placement, one amplitude. **The project's own bar is
`rs_track_fit()` -- slope near 1 and rms under half a bin across a sweep of
injected frequencies, pooled over independent clutter realisations -- and that
has not been run.** A single frequency matched once is what item 2 records
several of. Item 72's `--stft 64` reporting 0.504 Hz against a true 0.550 looked
exactly this good and was a coincidence, caught only by its control; this has its
control, which is the difference, and it still has no sweep.

The sweep is the next thing, and it is now a well-defined experiment rather than
a direction: inject the burst at a range of time-scalings so its modes land at
known different frequencies, and score the modal set's leading mode by slope and
rms with a static control at every point.


## 74. The sweep: item 73 was one lucky point

Item 73 reported the shape-ranked modal set returning 0.353 Hz against a true
0.350 on a real burst while the static control refused, and said plainly that it
was not a recovery until it survived a sweep. It has not.

Twelve injection points -- six playback rates placing the burst's dominant from
0.300 to 0.850 Hz, two independent clutter seeds -- plus a zero-amplitude static
control per seed, all through identical processing.

| true | seed 7 | seed 11 |
|---|---|---|
| 0.300 | **0.353** | refused |
| 0.400 | refused | refused |
| 0.450 | 1.260 | refused |
| 0.550 | 1.159 | refused |
| 0.750 | refused | **1.865** |
| 0.850 | refused | **2.571** |
| **static** | **refused** | **refused** |

**Six answers over twelve points, and one of the six is near the truth.** The
other five are wrong by 0.7 to 1.7 Hz, and they are not scattered randomly --
they sit ABOVE the injection at every point, which is the signature of a
processing artefact being selected rather than the target. Slope is nowhere near
1 and rms is two orders above half a bin. `rs_track_fit()`'s bar is not met and
is not close.

Item 73's 0.353 Hz was the one point that worked. Nothing distinguished it from
the other five answers at the time -- same block size, same support, same local
ratio range -- which is exactly why a single matched frequency is not a result
here, and why item 2 records several of them.

### What survives

**Both static controls refuse.** That is not nothing: prominence reports a
confident in-band frequency on those same motionless scenes at prominence 8.6.
The shape test does keep a motionless scene silent, across two seeds.

But it is not the precision half of a precision/recall trade either, because the
policy ANSWERS WRONGLY on five of six moving scenes. The honest description is
that a moving scene sometimes manufactures a contiguous spurious mode and a
motionless one does not -- which is a statement about what motion does to the
noise, not about the policy recovering anything.

### What this closes

Items 69-74 are one arc, and it ends here. A real structure's motion goes in; no
policy in this project reports it correctly across a sweep; the short-time
estimator makes it worse (item 72) including on the burst it was written for
(item 73); and ranking by spatial shape buys silence on static scenes and not
accuracy on moving ones. **The reporting stage remains the blocking failure for
realistic motion, and none of the four things tried here fixes it.**

The one measurement worth keeping from the whole arc is item 71's: the true mode
is not in the per-window spectra to be selected. Everything since has been an
attempt to select better from evidence that is not there.


## 75. Federating the screen: 2.7x the search, none of the motion

Item 59's screen queried `service.iris.edu` alone, and the result showed it:
289 of its 315 hits came from IRISDMC, five from INGV, none from Japan or most
of EIDA. **"315 synchronised measurements" meant 315 reachable from one
archive.** `tools/footprint_sensor_join.py` now queries the FDSN federator,
which resolves across every data centre and reports which one holds each
channel.

### The join

```
410 hits over 648 footprints      against 315 over 939 before
 76 distinct stations             against 27
 new networks: CE ES CH BW HL CA CI CO TX Z2 1I
```

Footprints were filtered to dwell >= 15 s (item 58) BEFORE the expensive stage,
which is why 648 and not 939.

**Read the shape, not the total. 324 of 410 are HV** -- Kilauea, which item 66
already characterised as 95% below the floor -- and **1 of 410 has a structural
site name.** Europe contributed about twenty hits. Federation found more
sensors; they are still ground vaults rather than instruments on loaded
structures. The three-legged requirement of item 68 is unchanged: a continuous
public record, on a structure, under a >=15 s dwell.

### The audit, and its funnel

1716 channels over 72 of 76 stations, with failures itemised: 136 non-motion
channels, 58 below Nyquist, 10 genuine no-data.

```
1. above 5.5 um, motion channels, Nyquist > 3 Hz : 115   (11 stations)
2. after chronic-station check                    :  70   (dropped 6 AM Raspberry Shakes)
3. after neighbour test                           :   8   (4 distinct stations)
4. after the neighbour test ACTUALLY RAN          :   0 credible
```

Step 4 exists because steps 1-3 lied. Two survivors, both on La Palma, were
marked UNTESTED -- no second station sat within 30 km IN THE SAME SCENE, so the
check that eliminated 62 of 70 never fired on them. **They survived by not being
tested.** Asking the archive directly for every station within 50 km during the
same aperture settles it:

| station | date | target | neighbour median | ratio |
|---|---|---|---|---|
| CDLV | 2021-09-18 | 0.26 um | 0.31 um | 0.8 |
| EXILP | 2021-09-23 | 0.68 | 0.74 | 0.9 |
| CENR | 2021-09-24 | 5.87 | 0.79 | 7.4 |
| CENR | 2021-09-26 | 5.14 | 0.92 | 5.6 |
| CENR | 2021-09-29 | 2.01 | 0.79 | 2.5 |
| CENR | 2024-08-02 | 5.01 | 0.36 | **13.9** |
| **CJED** | **2024-08-03** | **55.62** | **0.42** | **131.2** |

**ES.CJED reads 131x the stations on the same island at the same instant.** That
is HV.UWB's signature, rejected at 400x in item 61, and it fails the 10x cut by
an order of magnitude. The 55.62 um headline was the instrument.

### The one thing that looked like ground

CENR during the September 2021 eruption, at 2.5-7.4x. The circumstantial case is
coherent: Cumbre Vieja erupted on 19 September, CDLV reads 0.8x the day BEFORE,
CENR is nearest the vent, and its readings DECAY 5.87 -> 5.14 -> 2.01 across 24,
26 and 29 September in step with the tremor. An instrument fault does not taper
with an eruption sequence.

But 5.87 um is **1.07x** the 5.5 um floor, and it is volcanic GROUND motion, not
a structure under load. It is the closest this project has come to an
above-floor reading that is not hardware, and it is not close enough to be one.

### What it establishes

**115 raw exceedances to zero credible**, the same outcome as item 61's 52 to
zero, on a screen 2.7x larger reaching an entire continent the first could not
see. That is item 62's arithmetic tested rather than assumed: widening the net
multiplies pairings, not amplitude, because ambient ground motion is below the
floor everywhere and not merely in the United States.

**The data problem is not a search problem, and this is the second independent
confirmation.** What would change the answer is a sensor on a loaded structure,
and 1 of 410 hits even has a structural site name. Federation was worth doing --
it was cheap, it was wrong not to have done it, and it closes the question --
but the answer it returns is the one item 62 predicted.

Run: `runs/screens/sensor-join-federated/`.


## 76. Items 69-74 measured a tracker that could not see. On phase, the burst recovers.

Item 71 concluded the true mode was not in the per-window spectra, and items
72-74 then built three ways to select better from it. All failed. The reason was
upstream of all of them and was visible in the evidence file the whole time.

**Every run in items 69-75 used the DEFAULT estimator, `correlation`.** Bamler &
Eineder's bound for correlation shift estimation scales with the sub-look
RESOLUTION CELL, which at 128 looks is 8.26 m here. The measured `sigma_px`:

```
  sigma_px  median 93.4 px = 46.7 m per look   (CRB at coherence 0.32 predicts 4.92 m)
  excursion median 31.5 px = 15.7 m
  injected  20 mm          =  0.040 px
```

**The tracked series was noise at 787x the signal.** No spectrum of it contains
the injected mode except by chance -- which is exactly item 74's 1 correct answer
in 6. Item 66 already recorded the two routes' per-look precisions, ~0.2 mm for
phase against a resolution-cell fraction for correlation; nobody compared them to
the injected amplitude.

### The re-run, at 2 mm so the phase observable does not wrap

| scene | true | `best_window` | modal set (leading) |
|---|---|---|---|
| sine | 0.500 | 0.504 correct | **0.504 Hz, block 30, support 35/49, ratio 25.0** |
| **burst** | **0.300** | 2.268 wrong | **0.302 Hz, block 31, support 34/49, ratio 7.1** |
| static | -- | 2.268 wrong | 1.512 Hz, block 12 |

**A real structure's measured motion is recovered**: 0.302 Hz against a true
0.300, from an Oroville accelerometer record of the Lake Almanor earthquake.

**The block size separates signal from artefact with no overlap**: 30-31 for the
true modes, at most 12 for anything the static control produces. Item 71's shape
test was right and was starved of evidence, not wrong.

`best_window` fails on the burst and on the static scene alike, at prominence
23-32. Prominence remains what the tool reports and remains the wrong statistic.

**1.512 Hz appears in all three scenes including static** -- a common-mode
artefact, item 11 exactly, and identifiable ONLY because the static control ran
beside them.

### What this withdraws and what it leaves

Withdrawn: item 71's "the true mode is not in the per-window spectra to be
selected", as a statement about the method. It is true of the CORRELATION route
at these settings and false of the phase route. Items 72-74's negative results
stand as measurements of the correlation route and say nothing about the chain.

Not withdrawn: item 74's sweep discipline. One burst recovering at one amplitude
and one seed is what item 73 was, and that did not survive. **The phase re-run
needs the same sweep before it is a recovery**, with the added question of where
the block threshold sits between 12 and 30.


## 77. The block threshold is a property of the look count, not a geometric bound

Item 76 recovered a real burst on `--estimator phase` and found the modal set's
BLOCK separating signal from artefact with no overlap: 30-31 for true modes,
at most 12 for anything a static scene produced. Two sweeps now bound that.

### At 128 looks, the separation holds and recall is low

Twelve points, two seeds, plus a static control each:

```
  true    seed 7            seed 11
  0.300   0.302 blk 31 OK   0.302 blk 14 OK
  0.400   0.403 blk 18 OK   1.663 blk  9
  0.450   1.512 blk 12      0.605 blk 15
  0.550   1.512 blk 11      1.210 blk 12
  0.750   0.151 blk  8      2.419 blk 11
  0.850   1.512 blk 12      0.605 blk  6
  static  1.512 blk 12      1.210 blk  7
```

**3 of 12 correct** against item 74's 1 of 12 on the correlation route, and all
three recoveries are at 0.300-0.400 Hz. The block separates: correct answers at
14-31, both static controls at 7-12.

### At 48 looks the response improves and the discriminator DIES

Fewer looks raises the sub-aperture response, 0.7586 to **0.9654** -- so the
recall ceiling really is item 13's and the setting moves it. It buys nothing:

```
  true    seed 7            seed 11
  0.300   0.301 blk 21 OK   0.151 blk 39
  0.400   0.251 blk 25      0.351 blk 13
  0.450   0.301 blk 31      0.602 blk 30
  0.550   0.853 blk 26      0.151 blk 21
  0.750   1.004 blk 24      0.753 blk 31 OK
  0.850   0.853 blk 19      0.151 blk 21
  static  0.301 blk 21      0.151 blk 23
```

**2 of 12 correct, and BOTH static controls return confident modes at block
21-23** -- inside the range that flagged true modes at 128 looks. Static seed 7
returns **0.301 Hz**, the same frequency that reads as a recovery of the true
0.300. A wrong answer reaches block 31, the highest in either sweep.

### What this settles

**The block threshold is contingent on the look count.** It is not the geometric
2x2 bound `rs_spectrum_modal_set()` derives, which is a floor and not a
separator. Quoting "block 30 means real" is quoting a tuned constant, and item
76 must be read with its look count attached or not at all.

**Raising the response does not raise recall.** More response gave more
confident answers and fewer correct ones. Whatever limits recovery above 0.450
Hz at 128 looks, it is not only the sub-aperture response.

**The best result in this project remains 3 of 12**, phase route, 128 looks,
0.300-0.400 Hz, with static controls refusing at block <= 12. That is not
`rs_track_fit()`'s bar and should not be quoted as a recovery.

Run: `runs/synthetic/2026-08-05-phase-sweep/`.


## 78. Null-calibrating the block does not fix it. The block needs a CHANCE model.

Item 77 showed the block threshold is contingent on the look count. The obvious
fix is this project's own doctrine -- calibrate against a null rather than a
constant -- so compare the modal set's leading block against a MATCHED STATIC
run's block at identical settings. Both sweeps' static runs were already on
disk, so the rule was tested before it was implemented:

```
128 looks   threshold = max static block = 12
   accepted & correct 3    false positives 1    missed 0

48 looks    threshold = max static block = 23
   accepted & correct 1    false positives 6    missed 3
```

**It does not work.** Better than a fixed constant, which would have accepted
everything at 48 looks, and still six false positives.

### Why, and it is the useful part

At 48 looks the spectrum has **25 bins**; at 128 it has **65**. Fewer bins means
more windows land on the same bin by chance, so blocks inflate EVERYWHERE --
signal and noise together. The wrong answers at 48 looks reach 39, 31, 30, 26,
25 and 24; the static's 23 is not an outlier in that distribution, it is typical
of it. A null threshold cannot separate two things drawn from the same widened
distribution.

**The block statistic is not comparable across configurations, because the
number of admissible bins changes with the look count and chance agreement
scales with it.**

### The gap this names in rs_spectrum_modal_set()

`support_min` is derived from a binomial null over `n_bin` -- properly, with a
family-wise budget and no tuned constant. The BLOCK is then gated by a fixed
floor of 4, taken from the 2x2 window geometry. **Support is normalised for
chance; the block is not.**

What the block needs is the same treatment: given this bin count, this many
voting windows and this nomination count, what is the largest 4-connected block
CHANCE produces? A bin should be reported when its block exceeds that, not when
it exceeds 4 and not when it exceeds a null run's.

That is a percolation question on the window grid rather than a binomial one, so
it likely wants a small Monte Carlo over the same null the support threshold
already assumes -- shuffle nominations at random across bins, take the largest
block, repeat. It is configuration-free by construction, which neither the
constant nor the null-calibrated version is.

NOT IMPLEMENTED. Recorded because the negative result is what makes the
requirement precise: two thresholds have now failed for the same reason, and the
third has to normalise for bin count or it will fail identically.


## 79. The literature does not threshold a spectral statistic. It reports a posterior.

Items 76-78 chased a configuration-free threshold on the modal set's block: a
constant (item 76), contingent on look count (item 77), then a null-calibrated
version that failed for the same reason (item 78), ending with "the block needs
a chance model". A search of the operational-modal-analysis literature says the
premise is wrong.

### Uncertainty instead of a gate

**Bayesian operational modal analysis** fits a modal model and returns a
POSTERIOR PER MODE, quantifying identification uncertainty from measurement
noise and data limitations. Other frequency-domain lines estimate CONFIDENCE
BOUNDS simultaneously with the modal parameters.

Neither gates a spectral statistic. A mode with a wide posterior is reported as
uncertain; it is not refused by a threshold. That dissolves item 78's problem
rather than solving it -- there is no configuration-free constant to find,
because the field does not use a constant. **The successor to
`rs_spectrum_modal_set()` is per-mode uncertainty, not a better block null.**

### Short records: model the transient, do not window it

The most directly applicable finding, and it is a critique of code this project
has had for years: frequency-domain methods for SHORT DATA RECORDS estimate
TRANSIENT EFFECTS SIMULTANEOUSLY WITH THE MODAL PARAMETERS, *instead of*
applying Hann windows to the record.

`rs_spectrum_compute_opts()` applies a Hann window to a 128-sample series and
`rs_microm_track()` fits carrier polynomials in a separate stage (items 51-53).
The literature says that decomposition is wrong for records this short: the
transient and the modes should be estimated JOINTLY. This is independent of
everything items 69-78 tried and was never considered.

### Two of our measurements are known properties, not defects

- *"Dominant modes are reliably estimated with minimal decay data, while
  challenging modes need multiple free decays."* That is item 77 exactly:
  0.300-0.400 Hz recovered from one burst, nothing above 0.450 Hz. **The recall
  limit is a property of single-record modal identification**, not of this
  chain, and the remedy in the literature is MORE RECORDS -- which for a
  satellite means more collects over the same structure.
- Non-stationarity has established machinery: moving-window SOBI, wavelet
  adaptive filtering, multivariate VMD. None resembles item 72's max-hold STFT.
  Item 72's negative result stands and its cause is now clearer -- a
  spectrum-analyser tool was used where the field uses time-varying SYSTEM
  IDENTIFICATION.

### What this changes

Item 78's Monte Carlo block null is no longer the obvious next step. The larger
and better-supported direction is per-mode uncertainty, and the cheapest real
improvement available before that is joint transient-and-mode estimation on the
short record.

Recorded, not implemented. **This is the fourth time a search has found the
field already had what was being invented here** -- after `RS_MICROM_EST_ARGMAX`,
the overlap figure of item 13, and `phaselink.c`'s ML solver in items 64-65.
Search first.


---

## 80. The chance model is built. It prices the block correctly and detects nothing.

Item 78 specified this and item 79 named the better direction. Both are now
implemented as far as this project's evidence supports: `rs_spectrum_modal_set()`
reports a **per-mode `p_chance`** from a Monte Carlo over its own null, and a
**per-mode frequency uncertainty** from the spread of the sub-bin estimate across
the nominating windows.

### What was built

**The chance model.** Under the null a window's nominations are unrelated to any
bin, so they are reshuffled: each voting window re-draws `RS_MODAL_PER_WINDOW`
bins uniformly over the admissible band under the same leakage separation, over
the same grid positions that actually voted, 1000 times. The trial statistic is
the **largest 4-connected block reached by any bin clearing `support_min`** -- a
max over the band, so the look-elsewhere cost items 49 and 55 were caught by is
inside it. `p_chance = (1 + trials at least this large) / (1 + trials)`.

**Per-mode frequency uncertainty.** Each nominating window's peak is interpolated
parabolically on the log-power; `freq_mean` and `freq_sd` are the mean and spread
over those windows. The reported leading figure is still the BIN CENTRE, so every
number in this file stays comparable.

### The threshold is configuration-dependent, and by a factor of three

This is item 77's finding, now measured directly rather than inferred:

| looks | admissible bins | block chance reaches (p <= 0.05) | worst trial |
|---|---|---|---|
| 128 | 62 | **6** | 7 |
| 48 | 22 | **20** | 28 |

A fixed floor of 4 cannot know that, and item 77's warning -- *quote the block
with its look count attached or not at all* -- is now enforced by the code rather
than by the reader. A block of 21 is overwhelming at 128 looks and is **what
chance produces** at 48.

### It changes almost no verdict, and no verdict that matters

Both sweeps re-run, 12 injected points and 2 static controls at each look count,
`--estimator phase` at 2 mm, seeds 7 and 11:

- **128 looks.** 3 of 12 correct (0.302, 0.403, 0.302 against 0.300/0.400/0.300),
  identical to item 77. Nothing is refused that was not refused before: the
  lowest block among the twelve answers is 6, and the threshold is 6.
- **48 looks.** 2 of 12 correct. Two answers are newly REFUSED (0.850/seed 7 at
  block 19, 0.400/seed 11 at block 13) and **both were wrong**, so precision
  improves from 2-of-12 to 2-of-10. That is the entire gain.
- **Both static controls still pass, at both look counts.** 128: 1.512 Hz at
  block 12 `p 0.001`, 1.210 Hz at block 7 `p 0.012`. 48: 0.301 Hz at block 21
  `p 0.033`, 0.151 Hz at block 23 `p 0.019`.

### Why it cannot refuse them, which is item 11 for the third time

The statics are not agreeing *by chance*. They are agreeing because the
processing put the same thing in every window: **1.512 Hz is the common-mode
artefact** item 76 identified in all three scenes, and 0.151 Hz is the first
admissible bin -- the trend field this project has documented since item 47. A
null built by shuffling nominations across bins asks *"could this much agreement
arise from independent windows?"*, and the answer for a common-mode artefact is
correctly **no**, because it did not arise that way. It arose from a real feature
of the processing.

**A chance model over nominations cannot substitute for a null over scenes.**
That is exactly what item 11 says and it is now demonstrated a third time, on the
statistic built specifically to fix it. `--null-static` remains the only thing
that adjudicates.

### One diagnostic fell out that was not designed

`freq_sd` is **exactly 0.000** on four of the 48-look answers -- 0.300/s11,
0.550/s11, 0.850/s11 and STATIC/s11, all reporting 0.151 Hz. Every window's
sub-bin estimate agrees to the last digit, which is not what independent
estimates of a real mode do; it is what windows pinned to the band floor do.
**A zero spread is the signature of the trend field, visible in the output for
the first time.** The largest block anywhere in either sweep, **39 of 49**, is one
of these -- a pure artefact. Block size is not evidence, and here is the proof in
one number.

### Status

Implemented, tested, ASAN-clean, and it does not detect. What it fixes is real
but narrow: the block threshold is no longer a constant that silently means
different things at different look counts. What it does not fix is the thing
items 69-79 have been circling, and item 79's diagnosis stands unaltered --
the remaining direction is a **modal model fitted to the data with a genuine
posterior**, and before that, **joint transient-and-mode estimation** on the
short record. Neither is attempted here.


---

## 81. Joint transient-and-mode estimation is built. It changes nothing.

Item 79's second open direction, and it closes the same way item 80 closed the
first. `rs_transient_fit()` / `mmotion --tfit N` fits the transient and the modes
TOGETHER instead of Hann-windowing the transient away.

### What was built

The model is a sum of exponentially damped sinusoids with onsets,

    y(t) = sum_k A_k exp(-alpha_k (t - t0_k)) cos(2 pi f_k (t - t0_k) + phi_k)

for t >= t0_k, fitted to the UNWINDOWED series by separable (variable-projection)
least squares: for fixed (f, alpha, t0) the model is linear in two coefficients,
so only the three nonlinear parameters are searched and the rest is a 2x2 solve.
Modes are taken greedily. That is the standard free-decay model of the
operational-modal-analysis literature -- what a structure struck by a seismic
arrival actually does -- and unlike a periodogram it reports DAMPING.

`tests/test_modalfit.c` is the unit test, scored the way this project scores
everything: a sweep, never a point.

- frequency sweep, six points with noise: **slope 0.9929, rms 0.0072 Hz**
  against a half-bin bound of 0.0250
- damping sweep, six points: **slope 1.1016, rms 0.0027** against a grid step
  of 0.0088
- a sustained tone returns **zeta ~ 0**, an onset at 5.0 s is located to within
  one grid step, two simultaneous modes are separated

**The damping ceiling is frequency-dependent and documented**:
`zeta_max = RS_TFIT_DECAY_MAX / (2 pi f T)` -- 0.080 at 0.8 Hz over a 20 s dwell,
0.021 at 3 Hz. Past it the fit SATURATES, so a zeta on the ceiling means "at
least", not "equal to". Pinned by a test.

### At the chain level it is not better, and on the sweep it is worse

Twelve injected burst points and two static controls at 128 looks,
`--estimator phase` at 2 mm, seeds 7 and 11, otherwise identical to items 74/77/80:

| estimator | correct within half a bin | statics answering |
|---|---|---|
| periodogram | **3 of 12** | 2 of 2 |
| joint transient fit | **2 of 12** | 2 of 2 |

### Two hypotheses formed on this data, both killed by their own controls

**"The fitted damping separates driven from motionless."** On the sweep's two
static controls it looked clean and with no overlap -- driven 0.0047-0.0079,
static 0.0026-0.0028. Run against **TEN** static realisations instead of two it
dies: statics reach **0.0044** and bursts fall to **0.0023**. The n=2 separation
was the coincidence this project's own bar exists to catch.

What DID survive is a physics check worth keeping: a **sustained sine returns
zeta ~ 0.0000 on five of six seeds**, sitting with the static scenes rather than
with the bursts. So zeta measures TRANSIENCE and not the presence of signal,
which is what it is supposed to do. It is simply not separating at this SNR.

**"It is better on sustained tones."** The sine arm returns the correct 0.504 Hz
on 6 of 6 seeds at blocks 36-47 against statics at 6-14 -- a clean separation,
and it is **not the estimator's**. The paired periodogram run on the same six
seeds gives blocks **43, 47, 30, 44, 47, 47** against the fit's 43, 45, 36, 47,
47, 47. Identical. The sine's large block is a property of a sine being easy.

### What this means

**The limiting factor is not the spectral estimator.** Item 71 located the loss
in the per-window spectrum rather than in the reporting stage; item 81 narrows
it further -- it is not in the CHOICE OF ESTIMATOR for that spectrum either.
Windowed periodogram, max-hold STFT (item 72) and a fitted modal model with the
transient in it all give the same answer, because they are all estimating the
same tracked series and the mode is not reliably in it. Items 79's two
directions are now both closed, and neither was where the problem is.

### The honest limit of the integration

`rs_transient_fit_windows()` expresses the fit AS A SPECTRUM so every existing
policy reads it unchanged -- residual periodogram for the floor, each fitted
mode's power at its nearest bin. That deliberately discards the fit's own
sub-bin frequency and its damping before the selection policies see it. It does
not explain the negative result, since the sweep's errors are whole bins and
more, but a fuller integration would aggregate the fitted PARAMETERS across
windows rather than re-binning them. Not attempted.

Kept because it is correct, opt-in, unit-tested and prints its own caveats --
the same grounds item 72 was kept on. It is not a result.


---

## 82. The literature explains item 81, and names two things it is not.

Searched after building item 81, per this project's standing rule. Three
findings, and the first predicts item 81's result rather than merely agreeing
with it.

### The THRESHOLD EFFECT predicts that estimator choice stops mattering

Frequency estimation has a documented **threshold SNR**: as SNR falls below it
with the record length fixed, the estimate's variance departs sharply from the
Cramer-Rao bound and **no estimator attains the bound any more**. Above
threshold, estimators separate by their efficiency; below it they all fail
together, and which one you chose stops being the question.

That is item 81's result stated in advance. A windowed periodogram, a max-hold
STFT (item 72) and a fitted damped-sinusoid model with the transient in it give
the same answers on this chain because the tracked series is BELOW THRESHOLD --
not because the three are equally good. **Item 81's conclusion should be quoted
in that form**: the limit is not the estimator, and the reason is a known
property of estimation rather than an observation about this code. It also says
where the gain is if there is one -- SNR on the tracked series, which is items
51-53 and 64-65 territory, not the spectrum stage.

The damped case sharpens it further: the CRB for a damped sinusoid degrades as
the damping approaches zero and as the observation shortens relative to the
signal's time constant, and the achievable accuracy DECOUPLES into
(amplitude, damping) against (frequency, phase). So the damping is not merely
harder to get than the frequency -- it is bounded by a different quantity.

### What was built is the WEAKER member of its own family

`rs_transient_fit()` is nonlinear least squares over a damped-sinusoid model,
which is the Prony family. The comparative literature is consistent that the
**matrix pencil** method has **lower estimate variance than Prony, is more
robust to noise, and is computationally cheaper**, with the eigensystem
realization algorithm the third standard member. Prony specifically "has
difficulty extracting the modes of noisy signals".

Since noise is precisely the regime this chain sits in, **item 81 has not tested
the best available time-domain estimator**, and its conclusion is correspondingly
weaker than it reads. That said, the threshold-effect finding above predicts a
lower-variance estimator does not rescue a below-threshold record -- so this is
worth stating as untried rather than as the obvious next experiment.

### And it is NOT the mechanism item 79 was describing

Item 79 recorded that the field "estimates the TRANSIENT JOINTLY with the modal
parameters instead of Hann-windowing". That is real, but the mechanism is
**frequency-domain**: Cauberghe and Guillaume generalise the least-squares
complex frequency estimator (LSCF, the basis of PolyMAX) by modelling the
**initial and final conditions of each data block** as extra transient terms in
the model. The purpose is to remove **LEAKAGE** without an exponential window,
and the paper reports it improves the modal estimates and **the damping
estimates in particular**.

Fitting decaying envelopes in the time domain, which is what item 81 built, is a
different thing that shares a description. **The frequency-domain transient-term
approach remains untried here.** It is also the one aimed at the defect this
project actually has a name for -- item 47's leakage and red floor.

### Status

Item 81 stands as a measurement and its explanation is now external rather than
local. Two named, unbuilt alternatives: **matrix pencil / ERA** in place of the
Prony-family fit, and **LSCF with initial-and-final-condition transient terms**
in place of windowing. Neither is predicted to help while the series is below
threshold.

**Fifth time a search has found the field already had what was being built or
puzzled over here** -- after `RS_MICROM_EST_ARGMAX`, item 13's overlap figure,
`phaselink.c` in items 64-65, and item 79's posterior. Search first.


---

## 83. Instrumented STRUCTURES are above the floor and are never observed moving.

Every sensor screen before this one joined GROUND stations. CESMD classifies by
what the instrument is mounted on, so the screen was re-run over `sttype` **B**
(building), **Br** (bridge) and **D** (dam) -- the structures that RESPOND where
ground does not. Each CESMD record carries `pgd`, so a candidate's displacement
is known before anything is downloaded.

### The harvest

288 structures -- **239 buildings, 35 bridges, 14 dams** -- and 1063 records over
the Capella archive's own span, with **0 failed queries**. The type filter was
verified rather than assumed: `sttype=Br` returns 7 stations all typed Bridge,
`sttype=D` returns 6 all typed Dam, against 873 unfiltered.

### The amplitude question is ANSWERED, and favourably

1015 records carry `pgd`. **437 of them -- 43% -- are above item 53's 5.5 um
end-to-end floor**, and the largest is **10.9 cm, which is 19,800x the floor.**

That is the first time in this project that a real, instrumented, public
measurement of a real structure has been shown to sit far above the sensitivity
this chain has. Item 62's "another factor of ~4 in sensitivity would turn all 315
synchronised hits into candidates" was a statement about quiet GROUND. For
structures the factor needed is **less than one**.

### And the time question kills it anyway

- **12 distinct structures have ever been inside a Capella footprint.** All
  twelve are BUILDINGS, all in the Los Angeles basin. **No bridge and no dam has
  ever been inside one.**
- 50 records exist at those twelve.
- **0 of them had shaking overlapping an aperture.** The nearest miss is
  **122 days**.
- Total aperture over covered structures is **1325 s** against an archive span of
  **2031 days**. Expected coincidences: **0.0040**. An expectation of one needs
  **252x this archive**.

### What it changes

**It splits a failure this project had been treating as one.** Item 62's is an
AMPLITUDE failure -- quiet ground is 50x below the floor and no search fixes
that. Item 83's is a TIME failure -- structures are 100x to 20,000x ABOVE the
floor and are simply never watched while they move. Targeting structures was the
right correction and it disposes of the amplitude half entirely.

It also disposes of a hope item 68 left open. The reason `GROUND_TRUTH_DATASETS`
sites are in no footprint is not bad luck about which structures were
instrumented: **not one of the 35 instrumented bridges or 14 dams in the entire
CESMD catalogue has ever been inside a Capella spotlight footprint.**

### What is left

Not an archive query. **A TASKED collect over a known instrumented structure** --
which is the third independent confirmation that the data problem is not a search
problem (items 62, 75, 83). An earthquake cannot be scheduled, so either the
collect is tasked to a structure whose ambient response clears the floor, or the
pairing waits on a vendor.

Umbra and ICEYE cannot rescue the spatial half. Umbra holds 4048 CPHD but item 63
disqualified it on dwell -- median 3.50 s, none at >= 30 s -- so multiplying the
footprint count by four multiplies dwell-inadequate scenes. ICEYE publishes no
bulk archive with footprints to join at all. **Neither was queried here**, and
that is a limit of this entry rather than a finding: what is established is that
the CAPELLA archive contains no such pairing.


---

## 84. Item 63's Umbra dwell verdict does not survive the full archive.

Item 83 left the spatial question open for a second archive: if Umbra covers
structures Capella does not, the blocker is dwell alone. Harvested Umbra's whole
open catalogue from its STAC sidecars -- **12,539 sidecars, 12,469 read, failures
counted by reason** (56 HTTP, 14 unparseable times) -- and joined it through the
identical code and the identical control as item 83.

### The dwell figures were taken from a sample with no power in the tail

Item 63 sampled **250** products. The tail it was trying to measure is under 2%,
so ~250 draws expect to see four or fewer of them; the sample could not have
resolved what it was quoted for. Over **12,404** collects (65 excluded for
implausible timestamps -- one claims to end in 2046):

| | item 63 (n=250) | actual (n=12,404) |
|---|---|---|
| median | 3.50 s | **5.80 s** |
| max | 23.50 s | **496 s** |
| >= 15 s | 0.4% (~16) | **8.6% (1071)** |
| >= 30 s | **NONE** | **1.8% (228)** |

**Umbra has MORE collects at >= 30 s than Capella does** -- 228 against 156.
Item 63's "disqualified on dwell" is **withdrawn as a statement about the
archive**. What stands is the statement about the MEDIAN: most of Umbra is short,
5.80 s giving `df` = 0.17 Hz, so 98% of it is unusable for this band. The
usable tail is real and was reported as empty.

### Umbra covers instrumented BRIDGES; Capella never has

- **81 structures** inside an Umbra footprint: 77 buildings, **4 bridges**.
- **19** of them inside a DWELL-ADEQUATE (>= 15 s) collect: 17 buildings and
  **2 bridges** -- **San Francisco Bay Bridge/West** (`CE.58632`, best dwell
  **24.8 s**, three collects) and **Fremont Channel B Bridge** (`CE.57595`,
  **27.4 s**).

Against item 83's Capella result -- 12 structures, all buildings, no bridge and
no dam ever. So the spatial half is not a fact about the sky; it is a fact about
where one operator points.

### The catch, and it is a real one

**Not one of the 19 carries a CPHD.** 332 of the 1071 dwell-adequate Umbra
collects do, but zero of the structure-covering ones; the bridge products are
SICD, CSI and SIDD only. That does not disqualify them -- a SICD is the focused
full aperture, which is exactly what `--subap paper` and `--subap uniform` split
spectrally -- but it forces the image-domain route, with item 13's sub-aperture
response ceiling and item 15's one-dominant-per-sub-look-cell precondition, and
rules out the `pulse` route this project's real-data recoveries were measured on.

### Earthquakes: still zero, on both archives

0 records whose shaking overlapped an aperture, Umbra as well as Capella. Item
83's duty-cycle wall stands and this doubles the archive it stands over.

### What actually opens

**The Bay Bridge is under continuous traffic load**, which is item 68's target
class and needs no earthquake to coincide with anything. The pairing that does
not exist for the earthquake route may exist for the AMBIENT route, and this is
the first named, public, dwell-adequate collect over an instrumented structure
this project has found. Whether a bridge's traffic response clears item 53's
5.5 um floor is untested and is the next measurement, not a conclusion.


---

## 85. All three public long-dwell archives, joined. One bridge pairing exists.

Completes items 83-84 with ICEYE, so every public archive this project can reach
has now been put through the same join, the same CESMD structure list and the
same scene-centre-in-its-own-ring control.

ICEYE's open STAC holds **404 items, 391 harvested** -- 13 skipped for
MultiPolygon geometry, which the containment test does not handle and which are
scan-mode strips rather than stares. Its dwell is the best of the three by
median.

| | collects | median dwell | >= 15 s | >= 30 s | structures inside | bridges | dams |
|---|---|---|---|---|---|---|---|
| Capella | 940 | 22.5 s | 648 (69%) | 156 | 12 | **0** | **0** |
| Umbra | 12,404 | 5.80 s | 1071 (8.6%) | 228 | 81 | **4** | 0 |
| ICEYE | 391 | 14.86 s | 197 (50%) | 8 | **0** | 0 | 0 |

**ICEYE covers no instrumented CESMD structure at all.** Its control passes
391 of 391, so that is a measurement and not a broken join. The cause is size and
aim: 391 collects, pointed by an open-data programme at global CATASTROPHES,
which is a different target set from instrumented civil structures in California.
Its value to this project remains the one already recorded in
`runs/screens/iceye/RUN.md` -- six CPHD in `dwell-precise` and `dwell-fine`
modes, the only open provider labelling long-stare modes as such, and the best
per-look phase floor screened anywhere here at 0.2017 mm.

### The state of the search, entire

- **Earthquake coincidences across all three archives: 0.** Item 83's duty-cycle
  arithmetic -- 0.0040 expected, 252x the archive for an expectation of one --
  now stands over 13,735 collects rather than 940.
- **Exactly one class of pairing exists**: Umbra over the **San Francisco Bay
  Bridge/West** (24.8 s, three collects) and the **Fremont Channel B Bridge**
  (27.4 s), SICD only, no CPHD.
- **No dam has ever been inside any footprint of any of the three.**

So the search across public archives is finished, and it returns one candidate.
Whether that candidate is worth anything turns on a question no archive can
answer: whether a bridge under traffic clears item 53's 5.5 um floor. That is a
measurement on data already identified, not another search.


---

## 86. RESIF joined: one pairing in Europe, and it is another proven-static scene.

Item 83's screen was CESMD, which is US-only, and said so. `ws.resif.fr` is the
French FDSN node and the first non-US network joined here: **5932 stations over
118 networks**, put through the same containment test and the same control.

### RESIF has the structures CESMD's typing would have found, and no field to find them by

There is no `sttype`, so structures must be found by SITE NAME. That works and it
is a heuristic, not a classification:

- **dams**: `FR.OGGM` Grand'Maison, `FR.SMPL` Sampolo, and `RA.CGMB`/`RA.CGMH` --
  the Barrage de la Manzo instrumented at **BASE and TOP**, a genuine structural
  array of exactly the kind this project has never had.
- **buildings**: `RA.PYTO` "Tour Ophite Terrasse 01-02", `RA.CGCO`, `RA.OCLD`,
  `FR.CURIE`, plus schools and town halls.
- **bridges: every match is a FALSE POSITIVE.** French communes routinely contain
  "Pont" -- Pont-de-Claix, Vallon-Pont-d'Arc, Pont de Salars. The heuristic finds
  the word and not the structure, which is the cost of having no type field.

### The operating-period filter is where this one turns

Many RESIF networks are TEMPORARY experiments that were decommissioned before the
SAR archives existed, so a space-only join badly overstates the coverage:

| archive | inside a footprint | station OPERATING then | and dwell >= 15 s |
|---|---|---|---|
| Capella | 1 | 1 | **1** |
| Umbra | 45 | 5 | **0** |
| ICEYE | **292** | 48 | **0** |

ICEYE's 292 is genuine coverage of the Piton de la Fournaise network -- its open
programme points at catastrophes and a volcano network is what sits under one --
and it dies on dwell, every collect being 10.0 s. Umbra's apparent 243-collect
Yasur coverage does not survive the operating filter at all. **Quote a
footprint-join count with its time filter attached or not at all**; this is the
same shape as item 84's tail-from-a-sample.

### The one pairing, measured

`FR.CURIE`, bâtiment Curie, Paris, inside
`CAPELLA_C02_SP_CPHD_HH_20210212074546_20210212074612`, **dwell 26.6 s**. HH
channels at 100 Hz, so the SEED band code is right. **40,960 bytes during the
aperture**, against a control window returning 430,080.

Displacement, response removed, 0.3-3 Hz:

```
  during the aperture   3-component 0.5101 um   ->  10.8x BELOW the 5.5 um floor
  midday control        3-component 0.4132 um   ->  13.3x below
```

**A second Oroville** (item 60): a PROVEN-STATIC scene, where a null is
interpretable and a reported frequency would be a proven false positive. It is
not a positive control and cannot become one. The midday traffic window is no
higher, so nothing about the hour rescues it.

### And no dam, again

Not one of RESIF's four dam stations is inside any footprint of any of the three
archives. Item 83 found the same across CESMD's 14 dams. **Two independent
sensor networks on two continents, three SAR archives, and no instrumented dam
has ever been imaged by a long-dwell collect.**

### What it does not do

The join tests space and the station's operating period. It does NOT test
earthquake coincidence: RESIF station metadata carries no event list, unlike
CESMD's, so items 83's shaking-overlap question would need a separate catalogue
query. Nothing here contradicts item 83's duty-cycle arithmetic; it simply was
not re-asked.


---

## 87. INGV joined. A third network, a third continent-scale negative, same floor.

`webservices.ingv.it` is the Italian FDSN node and the third sensor network put
through this join: **1719 stations over 66 networks**, same containment test,
same control, same operating-period filter item 86 showed is decisive.

### Italy instruments its structures, and INGV publishes them

- **dam** — `IV.ME05` Diga Del Menta
- **bridge** — `7C.SPAOL` **Catania, Viadotto S. Paolo (ANAS)**, a genuine
  instrumented road viaduct, which is the target class item 68 named and item 84
  found only two of anywhere
- **buildings** — the `XO` array at Amandola is a real structural set:
  `AM01` Torre del Podestà, `AM02` Istituto Tecnico, `AM03` Ospedale and
  **`AM04` Base Costone Ospedale**, a hospital with its own base station; plus
  `6N.CMA14` Palazzo Raffaello and `6N.CMA05` Scuola Piaget in Ancona.

**Not one of them is inside any footprint of any of the three archives.** The
"Ponte" false-positive class recurs in Italian exactly as in French -- `IV.PONC`
is Pontechianale, a commune.

### The funnel

| archive | inside a footprint | operating then | and dwell >= 15 s |
|---|---|---|---|
| Capella | 15 | 6 | **5** |
| Umbra | 33 | 17 | **1** |
| ICEYE | 0 | 0 | **0** |

The five are four **Etna summit** stations plus Venezia Lido, and the four Etna
stations share **one** dwell-adequate collect:
`CAPELLA_C14_SP_CPHD_HH_20240709040329`, **29.1 s, 2024-07-09 04:03:57 UTC**,
inside the Voragine paroxysm sequence of early July 2024. Umbra's single hit is
`IV.FIR`, Firenze, 18.4 s.

### Measured, and it is the same answer

Displacement, response removed, 0.3-3 Hz:

```
  during the aperture   EBCN 0.589   ECPN 0.803   EPLC 0.589   ECNE 0.620 um RMS
  control, 6 h earlier  EBCN 0.308   ECPN 0.422   EPLC 0.300   ECNE 0.275 um RMS
```

**6.9x to 9.3x BELOW the 5.5 um floor.** The tremor is REAL -- every station reads
about twice its own six-hours-earlier control, so Etna was genuinely more active
during the collect -- and it is still an order of magnitude too small.

### What items 83-87 now say together

Three sensor networks on two continents, three SAR archives, and the pattern does
not move:

- **Motion above the floor exists and is documented**: 43% of CESMD structural
  records, up to 10.9 cm (item 83).
- **It is never inside an aperture.** Zero earthquake coincidences over 13,735
  collects.
- **Everything that IS inside an aperture is 7x to 20x below the floor** --
  Oroville 0.5-0.8 um (item 60), Kilauea's median (item 67), FR.CURIE 0.510 um
  (item 86), Etna summit 0.589-0.803 um during a paroxysm sequence.
- **No instrumented dam, bridge or structural array has ever been imaged**, now
  across CESMD, RESIF and INGV.

The consistency is the finding. Ambient and volcanic ground motion sits about an
order of magnitude below this chain's floor everywhere it has been checked, and
that is not a sampling accident.

### What was NOT queried

**ESM (Engineering Strong Motion, `esm-db.eu`) refused every request with HTTP
413**, including `level=network`. It is the European strong-motion archive and
would be the natural place to find structural arrays with event-triggered
records, the way CESMD was. That is an unqueried source, not a negative result.
ITACA (`itaca.mi.ingv.it`) was likewise not queried.


---

## 88. ESM queried. It WITHDRAWS "no dam has ever been in a footprint", and the wall moves to the instrument.

Item 87 recorded ESM as refused with HTTP 413 and therefore unqueried. Retried
properly, it is the richest source in this whole search -- and it corrects a
claim items 83, 86 and 87 all made.

### How to query it

Two things blocked it. `net=` is **ambiguous** on this server ("could match
--network, --network-code") and must be spelled `network=`; and any wide
geographic query returns **413**. A ~8 degree tile succeeds, so the harvest tiles
the world and **subdivides recursively on 413**, which is what stops a dense
region from vanishing silently. **3890 stations**, 69 empty tiles counted by
reason. Largest networks: `IT` 931, `TK` 497, `IV` 391, `CH` 236.

### It withdraws the "no dam" claim, and the cause is my own heuristic

Items 83, 86 and 87 each concluded that **no instrumented dam has ever been
inside any footprint**. That is **WRONG**, and it was wrong because every one of
those screens found structures by SITE NAME:

**`CH.DIX` and `CH.SDIF` sit at GRANDE DIXENCE** -- the tallest gravity dam in
the world, 285 m -- **inside a 34.7 s Capella footprint**
(`CAPELLA_C14_SP_CPHD_HH_20250117040427`, 2025-01-17). The keyword search missed
it because the site names are "Grande Dixence VS" and "GRANDE DIXENCE-SDIF",
containing no "diga", "dam" or "barrage". `SDIF` sits at 2222 m and `DIX` at
2370 m against a crest at about 2365 m.

**A name heuristic finds the word, not the structure -- in both directions.** It
invented bridges out of French and Italian communes containing "Pont", and it
lost the largest dam in Europe. Any structural count in items 83-87 is a lower
bound on coverage and an upper bound on precision.

### ESM also gives far more pairings than any other network

| archive | space | operating | dwell >= 15 s |
|---|---|---|---|
| Capella | 28 | 26 | **24** |
| Umbra | 47 | 45 | **9** |
| ICEYE | 6 | 6 | **4** |

against RESIF's 1 and INGV's 5. Among them: **`A.FAT` Fatih-Tomb, Istanbul, at
60.0 s dwell** -- the longest pairing found anywhere in this project -- Grande
Dixence at 34.7 s, **Roma Palazzo Valentini / Palazzo Spada / DPC Ulpiano** at
33.2 s, Wien-Palais Festetics, Zermatt Kirche.

### And the wall simply moves to the instrument

Measured during the apertures:

```
  CH.DIX    Grande Dixence, continuous broadband   0.113 um   48.7x below floor
            control -6 h                           0.050 um
  CH.SDIF   Grande Dixence strong-motion           NO DATA
  IT.RMPV   Roma Palazzo Valentini                 NO DATA
  IT.RMPS   Roma Palazzo Spada                     NO DATA
  IT.RMUL   DPC Ulpiano                            NO DATA
```

**The structural stations are EVENT-TRIGGERED, not continuous.** They hold a
record only when something shook them, which is why ESM is an "Engineering Strong
Motion" database rather than a continuous archive. So a footprint over an
instrumented building yields a waveform only if an earthquake happened during the
aperture -- **item 83's duty-cycle wall, reappearing from the instrument side
rather than the catalogue side.** The one continuous instrument at these sites is
a bedrock vault station reading 0.113 um.

### Where this leaves items 83-88

The correction is real and narrow. **Instrumented structures ARE inside
long-dwell footprints -- more of them than any previous screen found** -- and the
obstacle was never purely spatial. It is that the instruments which would witness
above-floor motion only record when the motion happens, and no such moment has
coincided with an aperture in 13,735 collects. Every continuously-recording
instrument checked, on four networks, reads 7x to 50x below the floor:
Oroville 0.5-0.8, FR.CURIE 0.510, Etna 0.589-0.803, Grande Dixence 0.113 um.


---

## 89. A continuous SHM record of a real building exists. Its modes are above the band this project uses.

`zenodo.org/records/17358241` -- **Hospital Real, Granada**, the 16th-century
University of Granada rectorate: **six KB12VD accelerometers at 200 Hz,
continuous, 27 January to 2 February 2025**, CC BY 4.0, 8.0 GB.

### No pairing

No collect over the building in any of the three archives during the window, and
none outside it either. Nearest scene centres: Capella 197.8 km, Umbra 91.0 km,
ICEYE 92.1 km. So this is not the coincidence item 88 said was missing.

### It is nonetheless the best INJECTION source this project has had

Item 88 established that the structural instruments inside footprints are
**event-triggered** and hold nothing unless something shook them. This record is
the opposite in every respect that matters:

- **continuous**, so there is no duty cycle on the sensor side at all
- a real building under real **AMBIENT** load -- item 68's target class, and a
  different regime from item 69's Oroville EARTHQUAKE transient
- **six sensors**, so a mode SHAPE exists, which is what items 70-71's spatial
  test wants and has never had from real data
- **seven days at 200 Hz**, so a sweep can pool over thousands of INDEPENDENT
  real-structure realisations. `rs_track_fit`'s bar demands exactly that and item
  69's single record could not supply it.

### Its modes were measured without downloading it

The Zenodo object supports range requests; the zip's central directory gives
seven daily members, and a 40 MB slice of the first one inflates through BOTH
layers (zip deflate, then gzip) to **564,272 lines -- 47 minutes at 200 Hz**.

```
  dominant structural mode   3.78 Hz   (4 of 6 sensors)
  mode family                3.27 - 4.32 Hz
  also 5.05, 5.93, 6.08 Hz
  0.22 Hz on 5 of 6 sensors -- NOT a mode: that is the secondary ocean
      microseism, and a 37 m masonry building cannot be that flexible
```

### And that is the problem

The observable band is `n_looks/(2T)` at the top and the leakage floor at the
bottom:

| dwell | looks | band |
|---|---|---|
| 30 s | **128** | 0.100 - **2.13 Hz** |
| 30 s | 256 | 0.100 - 4.27 Hz |
| 20 s | **128** | 0.150 - **3.20 Hz** |
| 20 s | 256 | 0.150 - 6.40 Hz |

**3.78 Hz is OUTSIDE the band at 128 looks**, which is the look count every
measurement in this file uses. Using this record means moving to **256 looks**,
halving the sub-aperture length and changing an operating point everything else
here is calibrated against -- items 13, 76 and 77 are all quoted at 128.

That is a real cost and it is not a reason to discard the dataset; it is the
reason to quote any result from it with its look count attached, which is item
77's rule. It also says something about the target class: **a stiff masonry
building is a poor match for a long dwell**, because long dwell buys frequency
resolution and spends Nyquist, and item 58's arithmetic runs the other way for
stiff structures than for the 0.3-1 Hz motions this project was built around.


---

## 90. An independent refutation of the DEEP claim endorses the FRONT END, and its controls are this project's.

Foreman, *No Reproducible Evidence for Deep Subsurface Structures Beneath the
Giza Plateau* (June 2026 preprint; code at
`github.com/Hassanforeman/subsurface-sar-tomo`, Zenodo 10.5281/zenodo.21065675,
MIT). A faithful reproduction of Biondi & Malanga's method from the papers and
patent WO2024008365A1, with controls added.

**What it does NOT dispute is exactly what this project builds.** Verbatim: *"the
surface-vibration front-end is legitimate and well-precedented... their
peer-reviewed monitoring of ships, bridges, and the Mosul Dam measures real,
millimetric surface deformation, and that work stands."* The critique is confined
to the DEPTH INVERSION -- the "steering matrix" that this project does not
implement and has never claimed. `--subap paper` is the sub-aperture sweep, block
7 and earlier; blocks 8-11 are the disputed part.

### Its added controls are the ones this project arrived at separately

- **look-order-shuffle null** -- `rs_shuffle_looks()` and `--null-static`. Note
  this project's own extra caveat, which the paper does not need: shuffling is
  NOT valid for `RS_MICROM_EST_PHASE`, because a phase series is unwrapped ACROSS
  looks.
- **in-data positive control** -- `--inject-vib`. Item 38 goes further with the
  ZERO-AMPLITUDE TWIN, which the paper does not have and which is the control
  that caught prominence endorsing a motionless scatterer at 56.3.
- **surface-leakage and depth-of-peak guards** -- specific to the depth stage,
  no analogue here.

### Three findings that confirm this project's, from outside it

1. **"Stacking manufactures agreement, not evidence."** Five same-geometry Umbra
   passes over Bingham Canyon, a bare open-pit mine with no subsurface void:
   each pass gives a peak at 117.8x its own null, the stack preserves it at 96.7x
   against a stacked null of 1.5x, and the reinforced feature sits at ~3 m depth
   over ground known to contain nothing. **That is item 11 exactly** -- agreement
   is blind to whatever the operator puts in every scene -- and items 64-65 from
   the other side, where statistically homogeneous pixels turned out to be
   homogeneous in the ARTEFACT.
2. **A contrast-vs-null ratio of 1720x can be an artefact.** The Butte tomogram
   passes a naive contrast test and is surface-pinned, matching none of the
   documented workings. This project has the same hazard in its own units --
   item 38's prominence 56.3 for a target that does not move, item 49's real
   desert reaching 34.4. **A large ratio against a null is not a detection**, and
   here is a fourth independent demonstration.
3. **Free archives are in the wrong MODE.** The repository's own data map:
   Sentinel-1 and NISAR are systematic and free but stripmap, while the vibration
   method needs spotlight tasking. **That is item 58**, and it is the same
   conclusion items 83-89 reached from the sensor side -- what is left is a
   TASKED collect.

### And one caution aimed straight at this project

The paper's mechanism for a false positive is *"a DFT returns a structured,
peaked spectrum from any input vector -- signal, noise, or a residual processing
trend alike."* Strip the depth axis and that is a statement about
`rs_spectrum_compute_opts()`: a periodogram of a tracked residual always has a
peak. Everything in items 11, 37, 38, 47 and 80 is this project discovering the
same thing in its own domain. **The front end is endorsed; the habit of reading
a peak as a detection is what both bodies of work keep having to unlearn.**

Not yet read: 16 further documents under `docs/` in that repository, including
`VALIDATION_PROTOCOL.md`, `TECHNICAL_BIBLE.md` and `BUTTE_GROUND_TRUTH.md`.

---

## 91. A real building's AMBIENT motion goes in, and the answer is the static scene's.

Item 89's Hospital Real record, injected and swept: **six target frequencies x
two independent segments x two clutter seeds, 128 looks, `--estimator phase` at
2 mm** -- settings identical to items 74/77/80/81 so the answer is comparable.
Each segment's playback rate was computed from ITS OWN dominant displacement
frequency, so the injected mode lands on the target in every run.

### The result

```
  24 injected points, 21 answered, 3 refused
  correct within half a bin:  1 of 24
  most common answer: 1.512 Hz, SEVEN times
  static controls report:      1.512 and 1.210 Hz
  injected runs returning a value a STATIC control also returns:  9 of 21
```

**Nine of twenty-one answers are literally the number a motionless scene
produces**, and 1.512 Hz is the common-mode artefact item 76 identified in all
three scenes. One correct answer in 24 is chance.

### It is WORSE than the earthquake burst, and that is the point

| injected motion | correct |
|---|---|
| pure sine (item 80) | 6 of 6 seeds |
| earthquake burst, item 77 | 3 of 12 |
| **building AMBIENT, this item** | **1 of 24** |

The ordering is not an accident. A sine is one line with all the energy; a burst
is a transient with a dominant arrival; **ambient response is stationary,
broadband and multi-modal, with no single line ever prominent** -- so
`rs_spectrum_best_window()` and the modal set both fall through to the loudest
thing in the band, which is the processing's own artefact.

**Every synthetic recovery in this project used the easiest possible motion, and
the ordering above measures how much that mattered.** Item 68 said every
injection before item 69 was a pure sinusoid; item 69 added a real earthquake;
this adds the case a structure actually spends its life in, and the chain does
not recover it at all.

It also converges with item 90 from the inside: the reported frequency is the
shared operator's, not the ground's, and it is identical on a driven scene and a
motionless one.

### What it does not say

The injected amplitude was 2 mm, far above the building's real 0.68-1.97 um.
This is a failure of the SELECTION and the estimator at generous amplitude, not a
sensitivity result -- the real building is 3-8x below the floor before any of
this applies.


---

## 92. Reading that repository's 16 documents: four things transfer, one of them a rule this project has now written three times.

Item 90 recorded the preprint. The `docs/` tree behind it is the more useful
half, because it is a validation practice rather than a result.

### Their erratum is this project's recurring rule, from outside

`ERRATUM_KOMATI.md`: a results-table row was computed at `n_sub = 128` while
every other row used the default `n_sub = 11`, **undisclosed**, and the published
50x / 10x sat *"precisely on the manuscript's > 5x decision rule"* -- the most
vulnerable number in the paper. Re-run at the standard setting it is **2.8x /
1.3x**. Found only because an external criticism prompted a full re-run, and two
sites reproduced while one did not.

This project has independently written the same rule three times:
- item 77 -- *quote item 76's block with its LOOK COUNT attached or not at all*
- item 84 -- *quote a tail figure with the SAMPLE SIZE that measured it*
- item 86 -- *quote a footprint-join count with its TIME FILTER attached*

Four instances now, two codebases, one failure: **a number carries its
configuration or it means nothing.** That it took an outside prompt to find theirs
is the part worth remembering.

### The axis-extent confound, which this project has in its own units

Sweeping `n_sub` from 11 to 128 on a null site produced a **17.8x spread** in
contrast on the NATIVE depth axis, collapsing to **1.9x** when scored on a fixed
0-11.6 m window. The spread was the axis changing extent, not the signal changing
strength.

That is item 47's finding in another domain: **prominence is not comparable
across `--fmin` settings**, because it is measured against the admissible band and
changing the floor changes the denominator. Same defect, same cure -- score on a
fixed support.

### Their threshold is derived the way item 80's is

`5.0x` contrast against null, **calibrated on 400 synthetic null runs**:
p95 = 4.35, p99 = 5.03, so 5x is about a 2% false-positive rate. Not chosen --
measured from a null distribution. That is exactly the reasoning behind item 80's
`p_chance` (1000 reshuffles, family-wise by construction), arrived at
independently. **Two projects, same conclusion: a threshold on a contrast
statistic has to come from the null's own distribution.**

### A sensitivity axis this project has never swept, and can now deprioritise

`SENSITIVITY_RESPONSE_BIONDI.md`: a full cross-product of **window (Blackman,
Hann, Hamming, rectangular) x precision (float32/64) x coregistrator (phase
correlation + parabolic, upsampled DFT, normalised cross-correlation)** over four
sites -- **96 configurations, 200 runs each**, positive control passing in all 96.

**No verdict moved.** float32 and float64 agreed to three significant figures with
a maximum paired divergence of 0.012.

This project has never swept the window or the coregistrator and forbids
`-ffast-math` on precision grounds. Their result does not license adding
`-ffast-math` -- that changes reassociation and denormals, not word size -- but it
does say **the window function and the coregistrator are not where the answers
live**, so effort spent there is unlikely to pay. Useful negative.

### The practice worth adopting

`VALIDATION_PROTOCOL.md` is a **pre-registration template filled in before any
real scene is processed**: falsifiable H1-H4 (detection above null at alpha,
localisation within one resolution cell, false-positive rate in known-solid
regions, cross-sensor agreement), a frozen pipeline and null model, a confound
checklist, **blinding** (tomogram generated before truth is overlaid, no tuning
after unblinding), **written kill criteria**, and *"report all slices, including
misses"*.

This project pre-registers exactly one thing -- `runs/kilauea/.../analyse.py`.
Everything else was scored after the fact. The template is directly reusable and
costs nothing.

### Their strongest negative, for the record

Butte, MT: ~15 levels at 100 ft (30.5 m) spacing, three shafts, a mine pool at
**155-160 m**, the best registration quality of their four sites (0.82), positive
control passing -- and **REAL 3.3x against a null of 1.4x, leakage 0.28: null.**
A densely-mapped shallow void network, undetected, at the site most likely to
succeed. Their `docs/BUTTE_GROUND_TRUTH.md` cites NMMR, MBMG's depth-coded 3-D
model and USGS I-2050-C, which is where the user's citation [8] came from.


---

## 93. The Butte ground-truth chain audited: geometry, no motion, and two "dead" links that are not dead.

The user's citation [8] traces to `BUTTE_GROUND_TRUTH.md` in the external
repository. Its nine links were fetched and checked rather than assumed.

### Link audit

| status | source |
|---|---|
| 200 | `mmr.osmre.gov` (National Mine Map Repository), `osmre.gov` NMMR programme page |
| 200 | `mbmg627_Butte-miningdistrict.pdf` (2.4 MB), `pubs.usgs.gov/imap/2050c/report.pdf` (7.7 MB) |
| 200 | `pitwatch.org` 3-D tunnel model, `storyofbutte.org` |
| **429** | `mtstandard.com` -- RATE LIMITED |
| **403** x2 | `mindat.org` loc-6850, loc-27746 -- BOT-BLOCKED |

**The 403s and the 429 are refusals to serve, not missing pages**, and this
project has now been caught by that distinction twice in one session -- the
`MB` station query returned HTTP 307 with 0 bytes and would have been recorded as
"0 stations" (item 87's addendum). A non-200 is a reason to look again, not a
result.

### What the sources actually contain

Both primary PDFs were text-extracted and searched:

```
  MBMG 627 (Geology of the Butte mining district), 31,743 chars
      "seismic" x2   "monitor" x0   "vibrat" x0   "accelerom" x0
  USGS I-2050-C (mines and prospects, Butte 1x2 quadrangle), 446,661 chars
      "monitor" x6   "seismic" x0   "vibrat" x0   "accelerom" x0
```

**Neither contains a displacement time series, an instrument, or a monitoring
record.** They are void geometry and geology: where the workings are, at what
depth, in what rock.

### The conclusion, now verified rather than asserted

Item 57's rule holds against the actual sources: **a proxy is not a
measurement.** Mine maps establish that voids exist and where -- which is exactly
what the external work needed, because its hypothesis was about STRUCTURE. They
supply nothing this project needs, because its hypothesis is about MOTION, and no
frequency or amplitude can be validated against a map.

The one motion-bearing source in the Butte area is the **MBMG seismic network
(`MB`)**, joined earlier in this session: 61 stations, two of them in Butte
(`MB.BUT`, `MB.NQBU`), both inside a single Umbra collect of **10.8 s dwell with
no CPHD** -- below item 58's dwell bar and unprocessable by the phase-history
route. Butte is therefore a good ground truth for the question the external work
asked and not for this one.


---

## 94. A second building record, and this one's mode is INSIDE the band. Its loudest peak is not a mode.

`zenodo.org/records/20667124` -- **former military hospital, Naples** (Chisari,
Zizi, Pantò, Kheirollahi; June 2026). A multi-setup ambient vibration test:
**4 tri-axial + 2 bi-axial accelerometers + temperature, 4 setups of 20 minutes
each at 200 Hz**, two reference sensors fixed and four roving. 51.8 MB,
**CC-BY-NC-4.0** -- note the NC, where item 89's Granada record is CC-BY.

`setupM.csv` gives every sensor's **x, y, z coordinates**, so mode SHAPES are
reconstructable. That is what items 70-71's spatial test wants and has never had
from a real structure.

### No pairing, again

0 collects over the Naples historic centre in any of the three archives; nearest
scene centres 14.1 km (Umbra, the Vesuvius scene), 187.8 km (Capella), 533.1 km
(ICEYE). **Caveat: the dataset gives only LOCAL coordinates, so this is the city
and not the building** -- but a 14.1 km nearest centre is not a near miss.

### The loudest peak is setup-dependent, therefore not modal

Averaged over 16 channels, the dominant ACCELERATION peaks are:

```
  setup 1   14.11 - 14.50 Hz
  setup 2   12.40 - 13.04 Hz
  setup 3   11.77 - 11.87 Hz
  setup 4   15.48 - 20.00 Hz
```

**A building's modes do not move 60% between measurement setups.** What changes
between setups is which sensors were remounted where, so this is mounting or
instrument resonance. Reading the tallest acceleration peak as a mode would have
been wrong four times out of four, and the multi-setup design is what exposes it
-- a single-setup campaign gives no way to tell.

This is the same lesson as item 89's 0.22 Hz microseism, which appeared on 5 of 6
Granada sensors and was not a mode either. **The tallest line in an SHM record is
routinely not the structure.**

### In displacement it is 2.64 Hz, and that is INSIDE the band

The corner test of item 91, reapplied and again necessary:

```
  corner 0.3 Hz -> 0.39 Hz   tracks the corner
  corner 0.5 Hz -> 0.59 Hz   tracks the corner
  corner 0.8 Hz -> 2.64 Hz   stable
  corner 1.0 Hz -> 2.64 Hz   stable
  corner 1.2 Hz -> 2.64 Hz   stable
```

| record | dominant displacement | in band at 128 looks? |
|---|---|---|
| Granada (item 89) | 3.78 Hz | **no** -- 0.100-2.13 Hz at 30 s, needs 256 looks |
| **Naples** | **2.64 Hz** | **yes at a 20 s dwell** -- 0.150-3.20 Hz |

**So Naples is the better injection source**: it needs no change to the operating
point every measurement in this file is quoted at, and it carries mode-shape
geometry that Granada's single-column record does not.

Its real peak displacement is **0.80-1.23 um**, so like every instrument measured
in items 83-91 it sits below item 53's 5.5 um floor and would not be detected
even by a perfect collect over it.


---

## 95. Item 91 replicates on a second building — and the artefact turns out to be SEED-BOUND.

The first run in this project with **hypotheses pre-registered before it ran**
(`runs/synthetic/2026-08-05-naples-ambient/PREREG.md`, commit `cb47b06`). The
prediction recorded there -- that H1 would fail -- was correct.

Item 94's Naples record injected through settings identical to items
74/77/80/81/91; only the waveform differs. Two segments from **different setups**,
so independent in time and in sensor mounting.

| | Naples | Granada (item 91) |
|---|---|---|
| correct within half a bin | **7 of 24** | 1 of 24 |
| `rs_track_fit` slope | **+0.779** | **-0.948** |
| rms | 0.7295 Hz | 0.9315 Hz |
| answers equal to a static control's | 7 of 24 | 9 of 21 |

**H1 fails on both** (bar: slope within 0.10 of 1, rms < 0.0252 Hz). **H3 fails on
both.** So the pre-registered kill criterion applies: item 91 is not a property of
one record, and *this chain does not recover ambient structural motion* stands at
the bar.

### But the failure is structured, and one part of it revises item 76

- **THE ARTEFACT IS SEED-BOUND.** All seven 1.512 Hz answers are **seed 7**;
  **seed 11 returns it not once.** Seed 7's own static control reports 1.512 and
  seed 11's reports 1.210. Item 76 recorded 1.512 Hz as appearing "in all three
  scenes including static" and called it common-mode; **it is common to one
  CLUTTER REALISATION, not to the processing.** Every sweep in items 80-91 used
  seeds 7 and 11, so the 1.512 Hz that recurs throughout this file is seed 7's
  signature. **Quote it as such.**
- **Source modal prominence predicts recovery.** Segment 02 scores 5 of 12,
  segment 13 scores 2 of 12. Both are normalised to unit peak before injection,
  so the difference is the record's spectral SHAPE -- how concentrated its energy
  is at its own dominant -- not its amplitude.
- **Recovery rises with target frequency**: 1, 0, 1, 1, 1, **3** of 4 at 0.30 to
  0.90 Hz. The low targets sit nearer the artefact-rich bins, which is item 47's
  red floor showing through.

### Where this leaves the ambient case

Granada gave slope **-0.948** -- anti-correlated, i.e. pure artefact. Naples gives
**+0.779** -- genuinely tracking, and failing the bar on scatter rather than on
direction. That is a real difference between two ambient records, and it points
at the source rather than the chain: **an ambient record with a sharper modal
peak is partially recoverable; one without is not.**

So the ordering of items 91 and 95 together is: sine 6/6, earthquake burst 3/12,
**ambient-with-sharp-mode 7/24, ambient-diffuse 1/24**. The chain degrades
smoothly with how concentrated the injected motion is, which is what a
prominence-based selection must do.

### What these runs may not claim

Pre-registered and unchanged: **no localisation claim** (target at the grid
origin, item 40) and **no detectability claim** (the controls are motionless
scenes, not item 38's zero-amplitude twins). Recoverability only.


---

## 96. It is not seed 7. TWELVE of twelve motionless scenes report a confident frequency, and every one is different.

Item 95 found that all seven `1.512 Hz` answers came from seed 7 and concluded
the artefact was **seed-bound**. That is true and it was reported in a way that
implied seed 11 was clean. **It is not.** Seed 11's static control returns
`1.210 Hz`, which is simply a different number.

Twelve motionless scenes, identical processing, `--estimator phase`, 128 looks:

```
  seed   3   0.756 Hz   block  9   p 0.001
  seed   5   2.319 Hz   block 10   p 0.001
  seed   7   1.512 Hz   block 12   p 0.001
  seed  11   1.210 Hz   block  7   p 0.012
  seed  13   1.714 Hz   block  8   p 0.001
  seed  17   1.210 Hz   block  8   p 0.001
  seed  19   2.218 Hz   block  8   p 0.001
  seed  23   1.714 Hz   block 11   p 0.001
  seed  29   2.369 Hz   block 16   p 0.001
  seed  31   0.151 Hz   block 14   p 0.001
  seed  37   1.613 Hz   block 10   p 0.001
  seed  41   2.218 Hz   block  9   p 0.001
```

**Nothing moved in any of these scenes.**

- **12 of 12 answered. NONE refused.** The false-positive rate of the reported
  modal set on motionless clutter is **100%** at this operating point.
- **9 distinct frequencies**, spread across 0.151-2.369 Hz. There is no clean
  seed and no single artefact to notch out.
- **Every one clears the chance model**, p <= 0.012, at blocks 7-16 against the 6
  that chance reaches at 128 looks (item 80). The Monte Carlo prices agreement
  and these scenes are not agreeing by chance -- each contains a real,
  scene-specific feature.
- **The modal set and `rs_spectrum_best_window()` agree on 0 of 12 scenes.** Two
  policies reading the same spectrum name different frequencies every single
  time, which is on its own a statement about how little either is tracking.

### So "can the fixture be fixed?" -- no, and the question dissolves

There is nothing wrong with seed 7. Each clutter realisation has its own
dominant scatterer geometry, so each carries its own residual carrier (item 63:
the carrier is `(4*pi/lambda)*dX*dx/R` with `dx` the offset from the pixel
centre, which is a property of where the scatterers landed). **A different seed
does not remove the artefact, it renames it.**

### What this does to items 91 and 95

It does not withdraw them, but it bounds them hard. Those sweeps scored an
injected frequency against answers drawn from a chain that returns a confident
wrong frequency on **every motionless scene**. So:

- item 95's "7 of 24 correct" is the injected line **winning against a
  scene-specific artefact of comparable strength**, not a chain that is quiet
  until given a signal.
- The 1.512 Hz that recurs through items 76-95 is seed 7's number. Every sweep
  in this file used seeds 7 and 11, so it also inherited seed 11's 1.210 --
  which is exactly the second-most-common wrong answer in item 91.

### And it is item 11 at full strength, measured

This is the oldest finding in this file, now with a number on it: **a per-scene
null control is not one option among several, it is the only thing standing
between this chain and a 100% false-positive rate.** `--null-static` on the SAME
scene is what item 91 and 95 had; `p_chance` is not a substitute and item 80
already said so.

**Bounds of this measurement:** one fixture family (`rs_sim_scene` clutter at 400
scatterers), one estimator, one operating point. It says nothing about real
collects, where item 17's Giza run DID return a null.


---

## 97. The paired same-scene twin FINDS the signal at the injected frequency in 76% of the runs whose REPORT is wrong.

Pre-registered at commit `8543974`, run afterwards. Item 96 had measured a 100%
false-positive rate on motionless clutter; this asks whether the remedy this
project already owns -- a zero-amplitude twin of the SAME scene, differenced at
the injected frequency with `--probe-hz` (items 38, 39) -- recovers what the
reported frequency loses.

24 injected points, each paired with a twin on the **same seed and same clutter,
differing only in whether the target moves**. Statistic: `D = probe_prominence
(injected) - probe_prominence(twin)` at the injected frequency.

### H1 passes, at exactly the pre-registered threshold

**`D > 0` in 20 of 24 points** against a threshold of 20. Median `D` = +0.832,
mean +4.466, range -0.319 to +28.342.

### The finding: the energy is there, the report is not

Of the **17 points whose REPORTED frequency was wrong** in item 95, **13 (76%)
have a POSITIVE paired difference at the frequency that was injected.** The
starkest cases are unambiguous:

```
  target 0.40, seg 02, seed 7   reported 1.512 Hz (seed 7's artefact)   D = +7.196
  target 0.60, seg 02, seed 7   reported 1.512 Hz                       D = +16.352
  target 0.75, seg 02, seed 7   reported 1.512 Hz                       D = +19.531
  target 0.90, seg 13, seed 7   reported 1.512 Hz                       D = +3.872
```

**The chain measures the injected frequency and reports the artefact instead.**

That is items 7-9 and item 30 restated with a paired statistic behind it -- *"the
tracker does recover the injected carrier in most windows and the selection policy
discards it"*, and *"the tracker meets the bar on real data; nothing that reports
does"*. Item 96's 100% false-positive rate is therefore **a property of the
SELECTION, not of the measurement**, and this is the first evidence that separates
those two cleanly on an ambient record.

### Two structures in D, both consistent with earlier items

- **`D` tracks source modal prominence.** Segment 02 gives +3.2 to +28.3;
  segment 13 gives +0.08 to +3.9. Same normalisation, different spectral shape --
  item 95's finding, now on a paired statistic instead of a hit rate.
- **`D` rises almost monotonically with target frequency** on segment 02: +3.2,
  +7.2, +11.8, +16.4, +19.5, +28.3 at 0.30 to 0.90 Hz. Item 47's red floor: the
  low bins are where the noise is.

### H2 was MIS-SPECIFIED, and that is my error, not a result

H2 required the median-window difference to be under a tenth of the centre
window's, so that a scene-wide gain could not pass. It came out at **0.518**,
which I would have reported as a failure of spatial specificity. **It is not a
failure. The test was wrong.**

This fixture uses `--clutter-vib`, which vibrates the **whole clutter patch**, so
every window contains moving ground and a scene-wide increment is the CORRECT
behaviour. H2's form was borrowed from item 39, whose setup was `--inject-vib` --
a single added scatterer at a point. Item 71 had already recorded that this
fixture is whole-scene.

**H2 is therefore inapplicable here and is not scored.** Testing spatial
specificity needs `--inject-vib` with `--inject-at`, which is a different run.
Recorded rather than quietly dropped, because a pre-registration that can be
reinterpreted after the fact is worth nothing.

### What follows

The pre-registered branch that fires is: *"the discriminator works and the failure
is in what is REPORTED, not in what is measurable -- which would make a
twin-differenced statistic the thing to report."*

That was the open work when this item was written. **It is now implemented**:
`mmotion --twin WINDOWS_CSV` reads a previous run's evidence file and reports the
per-window difference at the probed frequency, adding a `twin_delta` column. It
requires `--probe-hz` and refuses a twin probed at a different frequency, over a
different grid, or without probe columns. It reports the BEST window's excess over
the scene-wide median, which is the localised part. It gates nothing.

Verified against this item's own data: the 0.75/seg02/seed7 point, whose reported
frequency is the 1.512 Hz artefact, prints *"35 of 49 windows gained; median
+1.601 (scene-wide), best +21.025 at window 4; excess +19.425"* -- reproducing the
hand-computed +19.531 at the centre window.

What the tool cannot check is whether the twin is the RIGHT KIND of control.

**Bounds:** one fixture family, whole-scene motion, one estimator, 2 mm. `D > 0`
in 20 of 24 is not a detection either -- it is measured with the truth known, and
on a real collect there is no twin to pair against. That is exactly why
`--null-static` exists and why item 96 matters.


---

## 98. The twin difference is COHERENT CHANGE DETECTION, the peak search is the LOOK-ELSEWHERE EFFECT, and both fields have a better statistic than this one.

Searched after implementing `--twin`. **Sixth time the field already had what was
being built here** -- after `RS_MICROM_EST_ARGMAX`, item 13's overlap figure,
`phaselink.c` (items 64-65), item 79's posterior, and item 82's threshold effect.

### 1. `--twin` is Coherent Change Detection, and its best statistic is not a difference

CCD is a mature SAR discipline built on exactly this operation: two acquisitions
of the same scene, differenced. Its stated precondition is the one `--twin`
prints -- *"two images of the same scene collected from substantially different
imaging geometries will not work"* -- and the field has already ranked the
candidate statistics:

> **the log likelihood ratio change statistic is superior to the commonly used
> mean backscatter power ratio and sample coherence change statistics**

`--twin` reports a raw difference of prominences, which is the crude member of
that family. **The LLR change statistic is a direct, named improvement** and it
is derived rather than invented. Also standard there and absent here: a
three-image *double change map* to cut false alarms, i.e. two twins rather than
one.

### 2. Searching for the peak costs less than expected -- so it is not the whole story

For detecting a sinusoid, the GLRT over unknown frequency needs about **20% more
SNR than a known-frequency detector, at a 95% detection rate**. That is a real
penalty and a MODEST one.

**It therefore does not explain item 96.** A 20% SNR penalty does not turn a
working detector into a 100% false-positive rate on motionless clutter. What
`--probe-hz` buys over peak-picking is roughly that 20%; what item 96 measured is
something else, and this bounds the diagnosis rather than confirming it.

### 3. The peak search is the LOOK-ELSEWHERE EFFECT, and item 1 has been open since the start

This project's oldest unresolved entry, **item 1**, is *"the quantisation floor is
a per-window test with no multiplicity correction"*. That is the look-elsewhere
effect, named and standard in cosmology and particle physics:

> *when searching for the highest peak in each periodogram, one must account for
> the fact that the frequency is not known a priori*

Corrected by a **trials factor**, normally computed by simulation -- which is
exactly what item 80's `p_chance` does with 1000 reshuffles. So item 80
independently reinvented the standard remedy. **But there is a cheaper modern
one**: Bayer & Seljak (MNRAS 508, 1346, 2021) *self-calibrate the look-elsewhere
effect from the heights of the highest peaks*, avoiding the simulation entirely.
That is a direct replacement for `rs_modal_null()` and it applies per window,
where `p_chance` applies to the block.

### 4. The twin's failure mode is SHM's best-documented one

Baseline subtraction is standard in structural health monitoring, and its known
defect is precisely the caveat `--twin` prints:

> *operational and environmental variations tend to distort the signals and
> masquerade as damage, generating a significant number of false positive
> results*

Their mitigations are named -- **Optimal Baseline Subtraction**, which keeps a
library of baselines in small temperature steps, and **Optimal Signal Stretch**,
which interpolates between them by stretching in time. The analogue here is that
a twin must match the scene in everything but the motion, and this project
currently guarantees that only by construction in a simulator. **On a real
collect there is no baseline library, which is a sharper way of saying what item
97 said: there is no twin to pair against.**

### What is actionable

1. **Replace the difference with a likelihood ratio** -- named by CCD as superior,
   derived rather than tuned. **DONE**: `rs_twin_llr()` implements the two-sample
   GLRT for exponentially-distributed periodogram bins,
   `LLR = 2 log((1+r)/2) - log r` with the EXACT F(2,2) p-value `1/(1+r)`,
   one-sided and scale-free, verified against the direct likelihood to 1e-12 and
   pinned by four test cases. `--twin` reports it and the CSV carries `twin_llr`
   and `twin_p`. **Its ceiling is the finding**: two degrees of freedom per bin
   means the ratio must exceed **19** before `p < 0.05`, so a single-look pair
   cannot call a modest excess. A chi-squared asymptotic is WRONG here -- Wilks
   is asymptotic in sample size and there is one sample per mean; measured,
   `2*LLR` reaches 4.67 at p95 where the half-mass chi-squared predicts 2.71.
2. **Item 1 is the look-elsewhere effect** and has a modern self-calibrating
   solution that needs no Monte Carlo.
3. **A second twin** (CCD's double change map) is a known false-alarm reduction
   this project has never tried.

None is implemented.


---

## 99. ITEM 1 ANSWERED, NEGATIVELY: multiplicity does not explain the false positives. The artefact is a real coherent line.

Item 1 -- *"the quantisation floor is a per-window test with no multiplicity
correction"* -- is this project's oldest entry, open since before everything else
in this file. Item 98 gave it its name, the LOOK-ELSEWHERE EFFECT, and item 96
gave it a target: a 100% false-positive rate on motionless clutter. This tests
whether the one explains the other.

**It does not.**

### The modelled trials factor is not nearly enough

Prominence of a single bin against a white-noise band is approximately Exp(1), so
`P(prom > x) = exp(-x)` and a family-wise 5% threshold over `W x K` tests is
`-log(0.05/(W*K))`:

| tests | threshold |
|---|---|
| one bin | 3.00 |
| one window, 62 bins | 7.12 |
| **49 windows x 62 bins = 3038** | **11.01** |

Item 96's twelve motionless scenes reported prominence **20.1 to 36.9**. **All
twelve clear the 3038-trial threshold**, with implied family-wise p from 5.7e-6
down to 2.9e-13. No trials factor over this many tests can excuse them.

### And the empirical null shows why: the model is wrong by 14x

Re-run keeping every window: **588 per-window prominences over 12 motionless
scenes** (`runs/synthetic/2026-08-06-null-distribution/`).

```
  measured   mean 14.21   median 12.53   p95 27.06   p99 32.86   max 36.87
  Exp(1)     mean  1.00   median  0.69   p95  3.00   p99  4.61
```

**The null is 14x too large in the mean.** Prominence on a motionless scene is
not a chance maximum at all -- it is a systematic property of every window.

### What a prominence of 14 actually means

Prominence is the peak over the mean of `K = 62` admissible bins, so the peak's
share of the band's power is `prominence / K`:

```
  null mean          14.2  ->  the peak holds 23% of the band's power
  worst static scene 36.9  ->                 60%
```

**On a scene where nothing moves, one bin routinely holds a quarter of the band
and sometimes three fifths.** That is a coherent line, not noise -- consistent
with item 63's mechanism, where each clutter realisation's scatterer geometry
sets its own residual carrier `(4*pi/lambda)*dX*dx/R`.

### Both candidate explanations are now eliminated

- **the frequency search**: item 98 measured the GLRT-over-unknown-frequency
  penalty at ~20% in SNR. Too small.
- **multiplicity**: this item. The peaks are 20-37 where 11 would be generous.

**What is left is that the artefact is REAL SIGNAL in the tracked series**, and
the place to attack it is the CARRIER REMOVAL (items 51-53, 63), not the
statistics. That is a different half of the codebase from everything items 91-98
touched.

### The empirical null does buy one usable thing, and it is not detection

A threshold above all twelve motionless scenes must sit above **36.9**. Against
item 38's numbers:

```
  real injections                  38-47  ADMITTED
  MOTIONLESS bright scatterer       56.3  ADMITTED TOO
```

So a self-calibrated prominence threshold **separates motionless CLUTTER from an
injection, and still cannot separate a motionless SCATTERER from a moving one**.
Item 38 is unchanged, and recoverability and detectability stay apart.

**Bounds:** one fixture family, `--estimator phase`, 128 looks, 12 realisations.
The 95th percentile of a 12-sample maximum is itself uncertain; 12 controls give
`p_min = 1/13`, which is item 49's arithmetic.


---

## 100. The motionless-scene artefact is NOT the offset-driven carrier: it does not scale with the analysis grid at all.

Item 99 closed item 1 and pointed the work at the carrier removal, on the
reasoning that a coherent line holding 23% of the band must be item 63's residual
carrier `(4*pi/lambda)*dX*dx/R`, whose `dx` is the scatterer's offset from its
pixel centre. **That pointer is wrong, and this is the test that says so.**

If the artefact were that carrier, shrinking the analysis cell must move it,
because `dx` is bounded by half a cell. Item 63 measured exactly that on real
Giza data -- artefact **70.7 at 1.0 m, 36.3 at 0.25 m, 159.0 at 0.125 m**, a 4.4x
non-monotone swing.

Item 99's metric, on motionless clutter with the **scene held fixed** and only
the analysis grid varied (two seeds):

| cell | windows | share of band power in the peak bin |
|---|---|---|
| 1.0 m | 9 | 25.6% |
| 0.5 m | 49 | 18.0% |
| 0.25 m | 225 | 19.9% |
| 0.125 m | 961 | 19.2% |

**Flat.** A 1.42x total spread, and from 0.5 m down it moves by under 10% while
the window count changes by 20x. Where item 63's artefact is **4.4x worse** at
0.125 m than at 0.25 m, this is unchanged.

### The first attempt was confounded and is recorded as such

The first run varied `sim_cphd`'s cell as well as `mmotion`'s, which re-places
every scatterer and destroys the comparison -- the thing being tested is `dx` at
fixed scatterer positions. Only the corrected sweep above is scored.

### What this eliminates and what it narrows to

The artefact on this fixture is **independent of the imaging grid**, so it is not
produced by where scatterers fall within pixels. That puts it **upstream of the
windowing** -- in the sub-aperture formation or in the tracked series itself --
and it means the three candidate explanations for item 96's 100% false-positive
rate are now all eliminated:

1. the frequency search -- ~20% SNR, too small (item 98)
2. multiplicity -- cannot bridge prominence 11 to 37 (item 99)
3. **the offset-driven residual carrier -- does not scale with the grid (this item)**

### The bound that matters

Item 63 was measured on the **real Giza collect with an injected target**; this is
**synthetic clutter with nothing moving**. Different fixture and different metric,
so this does NOT disprove item 63's mechanism where item 63 measured it. What it
establishes is narrower and is the thing needed: **on the fixture where items 96
and 99 measured the false-positive rate, the artefact does not come from the
analysis grid**, so refining cells or the polynomial fit will not remove it.

The next thing to interrogate is the tracked series itself -- what a motionless
scene's per-window phase series actually looks like before any spectrum is taken.
That has never been plotted here.


---

## 101. The artefact is just PHASE NOISE, and this fixture's real floor is 0.29 mm, not 0.0055 mm.

Items 98, 99 and 100 eliminated the frequency search, multiplicity and the
offset-driven carrier. Item 100 said the remaining place to look was the tracked
series itself, which had never been examined. `--shifts` already dumps it. This
is what it contains on a motionless scene.

### The series

`--estimator phase`, 128 looks, seed 7, nothing moving:

```
  phase, measured CIRCULARLY (a wrapped series has no linear sd):
      resultant length R = 0.643,  circular sd = 0.940 rad
  apparent displacement = phase * lambda/(4*pi)  ->  2.32 mm rms
  item 53's quoted end-to-end floor              ->  0.0055 mm
  the noise is 421x the floor, on a scene where nothing moves
```

A circular sd of 0.940 rad corresponds to a sub-look coherence of about **0.70**
via `sd ~ sqrt((1-g^2)/(2g^2))`, which is the right order for this fixture family
-- item 12f measured its coherence topping out at 0.323.

### That is the whole artefact

The periodogram of a 128-sample noise series **has a peak by construction**, and
each window's is at its own frequency: measured, peak frequencies scattered over
**0.302 to 3.024 Hz**, median 1.663, with no preferred value. The peak's share of
band power is **14.8% median against 4.8% for white noise** -- concentrated
enough to give the prominence 12-37 that items 96 and 99 measured.

**No selection statistic can fix a 421x noise-to-floor ratio.** Items 91-100 were
all arguing about how to choose among peaks in a series that is 2.3 mm of noise.

### I overstated the wrapping, and the correction matters

Two steps earlier I read a 14.27 mm peak-to-peak against a `lambda/4` = 7.75 mm
ambiguity and called the phase saturated. **It is not.** Measured circularly the
resultant length is 0.643 and the sd is 0.940 rad, well inside the circle -- the
full-range peak-to-peak is the extremes of 6272 samples, not saturation. A
wrapped quantity must be summarised circularly and I summarised it linearly.

### The floor this project quotes does not apply to this fixture

Item 53's **0.0055 mm** was measured on an **injected bright coherent point
target** -- CLAUDE.md already flags it as "cleaner than any real structure". The
clutter these fixtures are made of gives 2.32 mm per look, and averaging 128
looks buys `sqrt(N)` = 11.3x, not the 422x the two figures differ by. The
amplitude detectable at SNR 1 after 128 looks is

```
  2.32 mm * sqrt(2/128) = 0.29 mm
```

**53x worse than the quoted floor.** So:

| injection | vs this fixture's 0.29 mm floor |
|---|---|
| 2.0 mm (items 91, 95, 97) | 6.9x above |
| 0.5 mm | 1.7x above |
| 0.3 mm | at it |
| 0.1 mm | below it |

That is why the 2 mm injections were partially recoverable at all, and it
retrospectively explains the recall in items 91 and 95 without any appeal to
selection policy.

**TWO FLOORS WERE ALREADY DISTINGUISHED IN ITEM 66 AND THIS IS A THIRD.** Item 66
separated the per-look CRLB (~0.2 mm) from the end-to-end floor (0.0055 mm).
The missing one is the **per-fixture** floor: what the clutter's own coherence
allows, which here is 0.29 mm and is 53x the number this project quotes.
**Quote a floor with the scatterer it was measured on attached.**

### What follows

The lever is **sub-look coherence**, not statistics. Raising it lowers the phase
noise directly, and everything downstream follows. Item 12f already established
that this fixture family cannot exceed 0.323 by construction, so the artefact is
a property of `rs_sim_scene()` rather than of the chain -- which means **items 96
and 99's false-positive rates are bounded to this fixture and should not be
quoted as properties of the method.**


---

## 102. The floor can now be PREDICTED before a run, and it kills the Kilauea experiment.

Item 101 showed the tracked series' phase noise sets the floor, and gave the
arithmetic: circular phase sd -> `sd * lambda/(4*pi)` per look ->
`* sqrt(2/N)` after N looks. That is a **prediction that can be made from an
uninjected run**, which this project has never done. Applied to real data it
answers a question that 450 GB of download was being spent on.

### Real Kilauea collects, measured

Three complete Capella spotlights, 6.0 s matched dwell, 128 looks, 225 windows:

| collect | circular sd | implied coherence | mm per look | predicted floor |
|---|---|---|---|---|
| C10 2024-06-09 | 1.716 rad | 0.381 | 4.233 | **0.529 mm** |
| C10 2024-06-11 | 1.709 rad | 0.382 | 4.215 | **0.527 mm** |
| C14 2024-06-10 | 1.686 rad | 0.387 | 4.159 | **0.520 mm** |

Remarkably consistent -- this is a property of the scene type and the processing,
not of any one acquisition.

### The real collect is WORSE than the synthetic fixture, which inverts the premise

I expected real data to beat the fixtures, because item 12f records "a real
collect's 0.85" against the fixture family's 0.323. **That 0.85 is the
CORRELATION peak, not the sub-look PHASE coherence**, and they are different
quantities. Measured:

```
  synthetic fixture (item 101)  coherence ~0.70  floor 0.290 mm
  real Kilauea                  coherence  0.38  floor 0.529 mm   1.8x WORSE
```

The cause is visible in the same run: **amplitude dispersion best 0.478, median
0.567, and 0 of 225 windows meet D_A <= 0.25.** Kilauea lava fields have no
persistent scatterers, so item 15's precondition -- one dominant scatterer per
sub-look cell -- is unmet across the whole scene, and low phase coherence
follows. This is item 19's Giza finding on a second real scene.

### That ends the Kilauea correlation test

The sixteen selected collects have seismometer truth of **0.137 to 1.728 um**.
The predicted floor is **529 um**.

**The strongest scene in the set is 306x below the floor; the weakest is 3900x
below.**

Item 67 scoped this as a CORRELATION test rather than a detection test, on the
reasoning that it needs no collect above the floor -- only that reported
displacement should track the seismometer's RMS. **That reasoning fails at this
ratio.** Item 82's threshold effect is explicit: below threshold the estimator
decouples from the truth, so the reported amplitude is noise and the correlation
is zero by construction. A null would measure nothing.

### What this cost and what it saves

Item 67 scoped the test against item 53's **0.0055 mm** floor, where 95% of
Kilauea's readings sit below but the best sits 2.7x above. The floor that
actually applies to that scene is **96x larger**, because 0.0055 mm was measured
on an injected bright coherent point target and Kilauea is distributed lava.
**Item 101's third floor -- the per-FIXTURE one -- is exactly what was missing,
and no one had measured a real collect's phase noise until now.**

**Recommendation: stop the download.** 5 of 16 collects are complete and
byte-verified; the remaining 11 cost ~285 GB and ~9 hours and cannot change the
answer. The 5 completed ones keep their value as REAL-CLUTTER SUBSTRATE for
injection, which is the experiment worth running.

### The framework this implies

Everything needed already exists -- `--inject-wave`, `--inject-at`, `--shifts`,
`--probe-hz`, `--twin`. What was missing is the PROTOCOL, and it is now three
steps:

1. run the collect **uninjected** with `--shifts`, take the circular phase sd,
   and predict the floor;
2. inject a real sensor waveform at amplitudes **bracketing that prediction**;
3. report recovery against the prediction rather than against a constant.

**Predicting the floor before spending the run is the change.** Every measurement
in this file so far chose an amplitude and discovered afterwards whether it was
above the noise.


---

## 103. The floor is PER-TARGET, not per-scene. Three distinct floors, and conflating them caused items 67 and 102.

Pre-registered at commit `201d897`, run afterwards on real Kilauea clutter. A
real building's ambient record (Naples segment 02) injected into real Capella
phase history at 0.25x to 4x item 102's predicted 0.53 mm floor, two collects,
each differenced against its own uninjected run.

### H1 FAILED, in the direction the pre-registration named

Injected at 1.00 Hz; half a bin is 0.083 Hz.

| amplitude | vs predicted floor | C10 2024-06-09 | C14 2024-06-10 |
|---|---|---|---|
| 0.00 (control) | -- | 0.665 Hz | 0.499 Hz |
| 0.13 mm | 0.25x | 0.665 -- missed | **0.997 -- recovered** |
| 0.26 mm | 0.5x | **0.998 -- recovered** | **0.997** |
| 0.53 mm | 1x | **0.998** | **0.997** |
| 1.06 mm | 2x | **0.998** | **0.997** |
| 2.12 mm | 4x | **0.998** | **0.997** |

H1 required recovery at and above 1x and **failure below it**. Recovery occurs at
**0.5x on one collect and 0.25x on the other**, so the prediction is not the
boundary. The pre-registered branch fires: *"if recovery occurs BELOW the
predicted floor, item 101's arithmetic is too pessimistic."*

**H3 PASSES**: neither control reports 1.00 Hz -- they report 0.665 and 0.499,
their own artefacts.

**H4 is weak**: the twin LLR is non-decreasing with amplitude (1.09, 1.09, 1.09,
1.54, 1.55) but plateaus and **never reaches p < 0.05**, which is item 98's
ceiling doing exactly what it said -- a single-look pair needs a power ratio
above 19.

### Why: the prediction and the injection describe different scatterers

`--inject-wave` defaults to **`rel = 20.0`** -- the injected target is 20x the
scene's mean brightness. Measured on the injected run with `--shifts`:

```
  scene MEDIAN circular sd  1.693 rad  ->  floor 0.522 mm
  TARGET window       sd    0.050 rad  ->  floor 0.0154 mm
  the target window is 34x quieter than the scene median
```

**Item 102 predicted the floor from the scene's clutter and then measured a
bright point target.** That is precisely the conflation item 101 identified in
item 53's 0.0055 mm, recurring in my own prediction one item later.

### There are THREE floors, and they differ by 34x

| floor | value here | what it describes |
|---|---|---|
| **target** | 0.015 mm | phase noise at a bright coherent scatterer. Item 53's 0.0055 mm is the same quantity, same order. |
| **clutter** | 0.52 mm | phase noise of the scene's own distributed return (item 102). |
| **competition** | 0.13-0.26 mm | what an injection must reach to BEAT THE SCENE'S OWN ARTEFACT, which on these collects peaks at prominence 9-10. |

**The competition floor is the operative one** and it is neither of the other
two. A target 8-17x above its own noise floor still loses to the scene's
strongest artefact until it clears it.

### What this fixes

- **Item 67's Kilauea scoping** used 0.0055 mm, a TARGET floor, for a
  CLUTTER question. Item 102 corrected it to 0.52 mm and killed the test --
  **that conclusion still stands**, because the seismometer measures the ground,
  which is clutter, at 0.137-1.728 um against a 0.52 mm clutter floor.
- **Item 102's protocol was right in form and wrong in the scatterer.** Predict
  the floor **at the window the target occupies**, not from the scene median --
  and state which of the three floors a number is.

**Quote a floor with the scatterer it was measured on attached** (item 101) is
now sharpened: quote it with the WINDOW, because the same scene carries both.

### Implemented

`rs_microm_floor()` computes it per window and every phase-route run prints the
floor at the strongest-prominence window beside the scene median and the
quietest; `floor_mm` joins the per-window CSV. Verified against this item's hand
computation -- 0.0157 mm at phase sd 0.049 rad against 0.0154 and 0.050.

**And it produced a measurement this item missed.** On the UNINJECTED Kilauea
collect **no window is quiet**: floors run **0.4237 to 0.6491 mm, a ratio of
1.3x**. The 34x quiet window in the injected run IS the injected target. So that
scene contains **no naturally bright scatterer anywhere**, and item 102's verdict
is stronger than it was stated -- not that the scene's median is bad, but that
it has no good window at all.


---

## 104. Recovery needs 20-26 dB of signal-to-clutter, and the field's own validated results put a CORNER REFLECTOR on the structure.

Pre-registered at `c0de7f5`. Only the target's brightness varies: amplitude fixed
at 0.26 mm, frequency at 1.00 Hz, waveform, geometry and collect all held.

### H1 and H2 both pass, and the transition is sharp

| REL | SCR | C10 2024-06-09 | its target floor | C14 2024-06-10 | its target floor |
|---|---|---|---|---|---|
| 20 | 26 dB | **0.998 recovered** | 0.0022 mm | **0.997 recovered** | 0.0023 mm |
| 10 | 20 dB | 0.665 -- the artefact | 0.486 mm | **0.997 recovered** | 0.0025 mm |
| 5 | 14 dB | 0.665 | 0.486 mm | 0.499 -- the artefact | 0.499 mm |
| 2 | 6 dB | 0.665 | 0.486 mm | 0.499 | 0.499 mm |
| 1 | 0 dB | 0.665 | 0.486 mm | 0.499 | 0.499 mm |

**The transition is between 14 and 26 dB** and it is not gradual. The target
window's floor is **bimodal**: 0.002 mm when the target dominates its pixel, and
0.486-0.499 mm -- the clutter floor -- the moment it does not. There is nothing
in between. Below the transition the reported frequency is exactly the
uninjected scene's own artefact, 0.665 and 0.499 Hz.

**My prediction was wrong.** I recorded "transition between REL 2 and 5"; it is
between 5 and 20. The method needs a far brighter target than I expected.

### This does NOT contradict item 51, and the difference is the whole point

Item 51 swept REL on Giza and concluded *"a brighter target does not make smaller
motion detectable"*, because there the competition was the target's OWN quadratic
phase residual, which scales as REL^2 exactly as the signal does. **Here the
competition is the SCENE's clutter artefact, which does not scale with the target
at all**, so the ratio improves with brightness until the target dominates its
pixel. Both are true; each must be quoted with the competition named.

### The literature says the same thing and has already acted on it

Vattulainen et al., *Assessment of spaceborne SAR micro-motion measurement for
vibration-based SHM* (IEEE, 2026; Strathclyde / Trento / Houston / DLR), assesses
this exact technique against synchronous accelerometer ground truth on Umbra-04
and ICEYE data. Three things in it matter here:

- **The targets are CORNER REFLECTORS on linear actuators.** *"The vibrating
  'shaker' targets were set up by mounting a corner reflector (CR) target on a
  linear actuator. CRs are commonly used in radar measurements to provide a
  point-like high intensity reflection."* That is the field buying the 20-26 dB
  this item measures as necessary, by hardware.
- **Their demonstrated floor is 0.10 mm RMS radial displacement** (0.81 mm/s RMS
  velocity), over a range of 10.43 down to 0.10 mm. Item 103's competition floor
  on real clutter was **0.13-0.26 mm** -- the same order, arrived at
  independently.
- **They have not measured a structure.** *"These results pave the way for future
  measurements of infrastructure."* And theirs are, to their knowledge, the first
  synchronous SAR-and-accelerometer measurements of vibration at all.

They also report needing to **raise the observation ratio** to reach the low
amplitudes, which is item 13's quantity, and conclude the method is *"better
suited to frequency-domain SHM approaches"* than to time-domain modal analysis --
which is what this project reports.

### What it means for items 83-89

Items 83-89 searched three SAR archives and four sensor networks for a structure
under an aperture and found essentially nothing. **That is not a shortfall
against the field: the field has not done it either.** The published validations
are corner reflectors on actuators, and infrastructure is stated as future work.
The gap this project could not close is one nobody has closed.

**And item 104 says what would close it**: a target presenting 20-26 dB of
signal-to-clutter. On a real structure that means a corner reflector, or a
structure that happens to contain one -- which is what item 15's
one-dominant-scatterer-per-cell precondition has been saying in other words since
the beginning.


---

## 105. The predicted sub-look SCR penalty is NOT THERE, and more looks lowers the floor.

Item 104 measured that recovery needs 20-26 dB of signal-to-clutter, against
PS-InSAR's standard `D_A <= 0.25` which corresponds to about 9 dB. I proposed a
mechanism for the 11-17 dB gap from the literature -- *"at higher resolution and
shorter wavelength, small scatterers show higher signal-to-clutter ratio"* -- on
the reasoning that N sub-looks each carry `B/N` of the bandwidth, so each
resolution cell is N times larger and holds N times more clutter, costing ~21 dB
at 128 looks.

**The measurement refutes it.** One real collect, one fixed target (REL 20,
0.26 mm, 1.00 Hz), look count swept, everything else held:

| N | bins | target floor | phase sd | scene median floor | response |
|---|---|---|---|---|---|
| 16 | 9 | 1.2206 mm | 1.353 rad | 1.0063 mm | -0.5 dB |
| 32 | 17 | 0.0036 mm | 0.006 rad | 0.8705 mm | -0.5 dB |
| 64 | 33 | 0.0028 mm | 0.006 rad | 0.6968 mm | -0.1 dB |
| 128 | 65 | 0.0022 mm | 0.007 rad | 0.5400 mm | -0.0 dB |
| 256 | 129 | **0.0017 mm** | 0.008 rad | **0.3980 mm** | -0.0 dB |

**The target's phase sd is FLAT at 0.006-0.008 rad from 32 looks to 256.** If the
sub-look SCR penalty existed at this brightness it would rise by 9 dB over that
range and it does not move at all. The floor instead FALLS as `N^-0.36`, against
`N^-0.50` for pure averaging -- so there is a small penalty, far too small to be
the 21 dB I predicted, and more looks is straightforwardly better here.

**The scene's clutter floor falls too**, 1.01 to 0.40 mm, which is the same
averaging gain acting on the clutter.

### N = 16 fails for an unrelated reason and must not be read as the penalty

At 16 looks the spectrum has **9 bins, of which 3 are unreportable leakage**, so
prominence is bounded near 6 and the statistic is starved before any question of
noise arises. Its phase sd of 1.353 rad is the *scene's*, because the target was
never selected -- the reported frequency is the uninjected artefact at 0.499 Hz.
That is a bin-count failure, not an SCR failure.

### What this leaves, and what it costs to say honestly

The 11-17 dB gap between this method's requirement and PS-InSAR's is **not
explained**. The sub-look resolution argument was the natural candidate and it is
measurably absent at REL 20. Two possibilities remain and neither is tested:

- the penalty exists but only bites at LOW REL, where the target no longer
  dominates its enlarged cell -- item 104's transition would then move with N,
  which is one sweep away;
- the gap is not about SCR at all but about what the two methods measure: PS-InSAR
  estimates a slow deformation over many passes, this estimates a spectrum inside
  one 6 s dwell, and the comparison of their thresholds may be meaningless.

**The practical finding stands on its own**: on this collect, more looks lowers
the floor monotonically to 256, the sub-aperture response is already 0.999 there,
and the only cost paid is Nyquist rising past anything of interest. **Item 13's
advice concerns OVERLAP, not look count, and nothing here contradicts it** -- but
the look count has been fixed at 128 throughout this file for reasons that were
never measured, and at 256 the floor is 23% lower.


---

## 106. The transition does NOT move with look count, and a real bridge HAS been measured without corner reflectors.

Two results. The first closes item 105's last open possibility; the second
corrects item 104.

### The brightness transition is identical at 256 looks

Pre-registered at `833c160`, identical to item 104 except `--n 256`:

| REL | C10 at 128 | C10 at 256 | C14 at 128 | C14 at 256 |
|---|---|---|---|---|
| 20 | recovered | **recovered** | recovered | **recovered** |
| 10 | artefact | **artefact** | recovered | **recovered** |
| 5 | artefact | **artefact** | artefact | **artefact** |
| 2 | artefact | **artefact** | artefact | **artefact** |
| 1 | artefact | **artefact** | artefact | **artefact** |

**Unchanged, collect for collect.** H1 fails exactly as pre-registered, so the
sub-look SCR penalty does not bite at low REL either. **It is dead in every
regime tested**, and the 11-17 dB gap against PS-InSAR's threshold is not about
sub-look resolution.

### The artefact is not at a fixed frequency

A detail worth keeping: the uninjected control's reported frequency **moves with
the look count**, differently per collect.

```
  C10   0.665 Hz at 128 looks -> 1.331 Hz at 256   -- exactly 2x, the same
                                                      FRACTION of Nyquist (1/16)
  C14   0.499 Hz at 128 looks -> 20.486 Hz at 256  -- band floor to near Nyquist
```

C10's artefact is periodic in LOOK INDEX rather than in time -- sixteen looks per
cycle at both settings. C14's is not, and moves somewhere else entirely. **Two
motionless scenes, two different mechanisms**, which is consistent with item 96's
finding that each realisation invents its own frequency and adds that the
invention is not even of one kind.

### The literature correction: item 104 is out of date

Item 104 said the field's validated results put a corner reflector on the
structure, and that infrastructure measurement remained future work. That was
true of Vattulainen et al.'s metrological assessment. **It is no longer true of
the field.**

Lotti et al., *Monitoring Bridge Vibrations via Spaceborne SAR Micro-Doppler*
(Structural Control and Health Monitoring, accepted 4 Dec 2025, published
13 Jan 2026, open access) measures a **real full-scale structure**:

- **South Portland Street Suspension Bridge, Glasgow**, two Umbra-04 spotlight
  acquisitions, validated against **synchronous** ground truth.
- **No corner reflectors.** The paper's reason is the answer to the question item
  104 left open: *"the bridge is highly reflective in SAR images due to its steel
  structure and the presence of sharp features and corners, enhancing its
  visibility in the reflectivity map."*
- Peak LOS velocities **0.5-2 mm/s**, dominant frequencies **1.5-2 Hz**,
  **velocity error of order 1 mm/s**, **spectral correlation with ground truth up
  to 0.88**, and **frequency resolution 0.06 Hz from a 16 s acquisition**.
- It positions the corner-reflector-on-a-shaker work as the PRIOR art it goes
  beyond.

**So a real structure can supply the 20-26 dB itself, if it is steel with sharp
corners.** Item 104's requirement stands; its pessimism about who can meet it
does not.

### This also updates item 70

Item 70 cites the EVACES 2025 conference version of the same campaign at
`df` 0.138 Hz and Pearson **0.33-0.47**. The journal version reports **0.06 Hz**
and **up to 0.88**. **Quote the journal figures, not the conference ones** -- and
item 70's use of 0.47 as "what validated means there" is no longer the best
available number.


---

## 107. A BLIND DISCRIMINATOR THAT WORKS: require the frequency to survive a change of LOOK COUNT. False positives 12/12 -> 1/12, recall 6/6.

Pre-registered at `f6ad9f4`, with the prediction recorded before running. **Both
hypotheses pass.**

### The idea, and why it is not new but was aimed at the wrong axis

The operational-modal-analysis STABILIZATION DIAGRAM: *"spurious modes will vary
and physical modes will remain quite constant at different model orders"*. Item
70 identified that principle and substituted the SPATIAL WINDOW for model order.
**That is the wrong axis.** The analogue of model order is a PROCESSING parameter
that changes what the estimator fits, and in this chain that is the **number of
sub-looks**: a real vibration sits at f Hz however the aperture is sliced, while
an artefact produced by the slicing need not.

Item 106 measured this by accident -- a motionless scene's artefact moved from
0.665 to 1.331 Hz, exactly twice, the same fraction of Nyquist.

### The result

Twelve motionless scenes and six injected ones, each processed at **128 and 256
looks**, nothing else changed. A scene reports only if the two answers agree
within half a bin.

```
  H1  motionless scenes still reporting:   1 of 12    (item 96 measured 12 of 12)
  H2  injected scenes still reporting:     6 of 6
```

Of the eleven rejected statics, **seven moved within the band** (by 0.15 to
2.07 Hz) and **four moved out of it entirely** -- 0.756 to 4.284 Hz, 1.714 to
5.544, 2.369 to 3.730, 0.151 to 5.292. Every injected scene reported **0.504 Hz
at both look counts, to three decimals**.

### Why this matters more than anything else in items 96-106

**It needs no twin, no null control and no ground truth.** Every discriminator
that has worked here needed something a single real collect cannot supply:

- `--null-static` needs a simulated motionless realisation of the same scene;
- item 97's `--twin` needs a run differing in nothing but the motion, which on
  real data does not exist;
- item 38's zero-amplitude control needs the injection machinery.

**This needs only the collect, processed twice.** That is the first blind test in
this project, and it is the shape the problem demanded since item 11.

### What it does not establish

- **The injected fixture is the easy case.** `--clutter-vib` at 2 mm moves the
  WHOLE scene coherently. A real structure is weaker, localised, and item 104
  says it must also clear 20-26 dB. H2's 6 of 6 is not a claim about real
  structures.
- **One static survived** -- seed 17 at 1.210 Hz on both. The false-positive rate
  is 1 in 12, not zero, and 12 realisations bound it no better than that.
- **`df` was identical at both look counts** because `df = 1/T` and the dwell was
  fixed; only Nyquist changed. A comparison across different DWELLS is a
  different test and is untested.
- Restricting to the common band 0.151-3.2 Hz is part of the rule, not a
  convenience: a frequency reported above 128 looks' Nyquist has no counterpart
  to be stable against.

### Implemented

`mmotion --stable OTHER_WINDOWS_CSV`. Reproduces this item's values on seed 7 --
motionless 1.512 -> 0.806 MOVED, injected 0.504 -> 0.504 STABLE.

**One thing the implementation got wrong first, and it is the subtle part.** The
verdict must be on the MODAL SET's leading frequency, not on the
strongest-prominence window's `dominant_hz`. Those are different statistics:
verified, the injected seed-7 scene whose modal answer is 0.504 Hz at both look
counts has that window reporting 0.504 at 128 and **2.571 at 256**, so a
window-level comparison rejects a true recovery. The first build did exactly
that and called an injected scene unstable. `modal_lead_hz` is now written into
the evidence file for the comparison to use.

It refuses equal look counts as vacuous, a differing window grid, and an
unreadable file; it warns when `df` differs, since that compares across DWELLS,
which this item records as untested.


---

## 108. The stabilization test caught a false positive sitting exactly on the frequency being sought. Recall on a weak OFF-CENTRE target is another matter.

Pre-registered at `25f7351`. Item 107 measured `--stable` on a whole-scene 2 mm
synthetic injection; this puts it on a **localised point target 20 m off the grid
origin, at 0.13-0.53 mm, on REAL Kilauea clutter**. The injection deposited into
56645 of 56645 pulses, so nothing here is a placement failure.

| collect | amp | modal @128 | modal @256 | verdict |
|---|---|---|---|---|
| C10 | 0.00 control | 10.148 | 15.971 | not comparable |
| C10 | 0.13 | 0.499 | 15.971 | not comparable |
| C10 | 0.26 | 0.499 | **0.998** | MOVED -> reject |
| C10 | 0.53 | 0.499 | **0.998** | MOVED -> reject |
| C14 | 0.00 control | **0.997** | 7.828 | MOVED -> reject |
| C14 | 0.13 | 0.997 | 11.992 | not comparable |
| C14 | 0.26 | **0.997** | **0.999** | **STABLE -> report** |
| C14 | 0.53 | **0.997** | **0.999** | **STABLE -> report** |

### The result that matters

**C14's MOTIONLESS control reports 0.997 Hz at 128 looks.** The injected
frequency is 1.00 Hz. On a real collect, at the look count this whole project has
used throughout, a scene with **nothing in it** produced an answer
indistinguishable from the signal being looked for -- 0.003 Hz away, well inside
half a bin.

**`--stable` rejected it**, because at 256 looks the same motionless scene says
7.828 Hz.

That is the clearest demonstration this project has of why a blind discriminator
was needed. Every statistic in items 38-99 -- prominence, consensus, contiguity,
the chance model, the modal set -- would have endorsed 0.997 Hz on that scene.

**H3 passes**: both motionless controls are refused.

### H1 passes as written, and the recall is still poor

H1 required that no case recovered at BOTH look counts be called unstable. **No
case was**, so H1 passes. But only **2 of 6 injected runs are reported**, and the
reason is not the stabilization test:

**C10 recovers at 256 looks and not at 128** (0.998 against 0.499) at both
0.26 and 0.53 mm. Item 103 recovered the same amplitudes on the same collect at
128 looks with the target at the GRID ORIGIN. The only change is
`--inject-at 20,20`. That is items 40-41 again -- an off-centre target falls
across four overlapping windows and its energy is split -- and it costs recall
before `--stable` is consulted.

### The abstention rate is a real weakness

**Three of eight comparisons returned "not comparable"** because the 256-look
answer landed above the 128-look Nyquist of 10.7 Hz. The test abstains rather
than deciding whenever the higher-look run wanders out of the common band, and on
real clutter it wanders there often. A pair of look counts closer together --
128 and 192, say -- would share more band and abstain less; untested.

### What transfers and what does not

- **Specificity transfers to real clutter**, and does so on the hardest possible
  example.
- **Recall was never tested here**, because the off-centre placement removed the
  signal at 128 looks before stability could be assessed. Item 107's 6-of-6
  stands only for a centred, scene-wide, 2 mm injection.
- The honest summary is that `--stable` is a **specificity instrument**. Nothing
  measured so far says it preserves recall on a weak localised target, and item
  108's own design prevented that question from being answered.


---

## 109. The selection loses a localised target that its OWN evidence carries best. Both of my explanations were wrong.

Pre-registered at `c3fb48d`. Item 108's target at +20 m sat exactly on a window
boundary; this repeats it at **+24 m, an exact window centre** (index 10; centres
fall every 8 m), changing nothing else.

| collect | amp | @128 | @256 | verdict |
|---|---|---|---|---|
| C10 | 0.00 control | 10.148 | 15.971 | not comparable |
| C10 | 0.13 | 0.499 | 0.998 | MOVED -> reject |
| C10 | 0.26 | 0.499 | 0.998 | MOVED -> reject |
| C10 | 0.53 | 0.499 | 0.998 | MOVED -> reject |
| C14 | 0.00 control | 0.997 | 7.828 | MOVED -> reject |
| C14 | 0.13 | **0.997** | **0.999** | **STABLE -> report** |
| C14 | 0.26 | **0.997** | **0.999** | **STABLE -> report** |
| C14 | 0.53 | **0.997** | **0.999** | **STABLE -> report** |

**H1 FAILS** (3 of 6 recover at 128 looks, needed 4). **H3 passes** -- both
controls refused. **H4 passes** -- every case recovered at both look counts is
reported STABLE, so `--stable` does not discard true positives. C14 now recovers
**down to 0.13 mm**, where at +20 m it could not be compared at all.

### Both of my explanations were wrong, in order

**Item 108 blamed the window boundary** (items 40-41's split across four
overlapping windows). Wrong: C10 fails identically on an exact window centre.

**I then blamed local clutter**, on the CFAR reasoning that detection depends on
LOCAL clutter power rather than the scene mean, so a `rel` defined against the
scene mean means nothing at a particular spot. **Also wrong**, and measured:

```
  C10, injected 0.53 mm at +24,+24
     target window (10,10):  floor 0.0044 mm   quality 0.949   d_a 0.056
     same window uninjected: floor 0.5210 mm   quality 0.769   d_a 0.574
```

The target **dominates its window completely** -- a floor 118x below the
clutter's, `d_a` 0.056 against 0.574. Local clutter is not the problem.

### What is actually happening

At 128 looks, per-window `dominant_hz` across all 225 windows:

```
  injected 1.00 Hz    15 windows   largest 4-connected block 13
  artefact 0.665 Hz   11 windows   largest block  4
  REPORTED 0.499 Hz    7 windows   largest block  6
```

**The injected frequency wins on BOTH of the modal set's stated criteria** --
most support and much the largest contiguous block, centred on the target -- and
the modal set reports a frequency with half the support and half the block.

So the loss is not in the ranking. It is in the **NOMINATION**:
`rs_spectrum_modal_set()` does not nominate on `dominant_hz` but on
`rs_local_ratio()`, each window's top peaks scored against their own **spectral
neighbourhood**. A strong isolated line raises the very background it is measured
against, through its own Hann skirt, if `RS_LOCAL_GUARD_BINS` does not exclude
enough of it -- so the cleaner the target, the worse it scores.

**That is a specific, testable defect and it is where the next work belongs.**
Items 7-9 and 30 said "the tracker recovers it and the selection discards it";
this is the first time the discarding step has been named.

### What stands

- `--stable` is vindicated on recall as far as it was tested: **H4 passes**, and
  it kept every true positive that reached it, down to 0.13 mm.
- Its specificity holds again, including C14's motionless 0.997 Hz.
- **The recall problem is upstream of it**, in the modal set's nomination, and no
  stabilization test can recover a frequency the selection never nominates.

---

## 110. Item 109's mechanism was wrong too. The localised target is refused by the SUPPORT gate and then out-ranked on EXTENT, and both are now fixed.

Pre-registered at `fce5869`, with the pipeline frozen and the kill criterion
written before the first of the sixteen runs. Item 109's exact configuration,
re-run verbatim; only the binary differs.
`runs/kilauea/2026-08-07-nomination-fix/`.

### Item 109 named the guard band. It is not the guard band.

The tracked series was dumped with `--shifts` and `rs_spectrum_compute_opts()`,
`rs_local_ratio()` and `rs_spectrum_modal_set()` replicated offline. The replica
reproduces the binary exactly -- 0.499 Hz, block 14, support 42 -- and sweeping
`RS_LOCAL_GUARD_BINS` from 2 to 8 never recovers the injected line:

| guard | 2 | 3 | 4 | 5 | 6 | 8 |
|---|---|---|---|---|---|---|
| reported | 0.499 | 0.499 | 0.499 | 0.499 | 0.499 | 1.497 |

The injected bin holds block 13 and support 26-32 at every width. **The
Hann-skirt argument does not apply on this collect at all**: at `--overlap 0`
the floor is flat, median psd varying 1.3x across the band, so a strong isolated
line is not what sets the background near it. **Third wrong explanation of the
same failure** -- item 108 blamed the window boundary, item 109 blamed local
clutter and then the guard band.

### The target is lost twice over

**First, at the SUPPORT gate.** `support_min` was 34 of 225 voting windows and
the injected bin's support is **28**. It was refused before block, ratio or
ranking were consulted. That threshold is a binomial family-wise budget and it is
correct about chance -- it is a *fraction of the whole window grid*, so a mode
occupying a handful of windows cannot reach it however strong it is. **This is
CLAUDE.md's standing rule about localised targets, in the one gate it had never
been checked against**, and it is the same shape as items 30-31: a scene-wide
criterion calibrated on fixtures where the whole scene vibrates.

**Second, on EXTENT.** Had it been admitted, block-first ranking put it fourth,
behind three artefacts beating it by exactly **one window**, while it led every
rival on strength: median local ratio **8.98 against 4.5-5.5**, max **73.7
against 24**. The target's own window nominates it at ratio **41.5**, its top
pick, on a psd peak **107x** that window's median. The evidence was in the struct
and the sort threw it away.

### What changed

**Admission** is now `RS_MODAL_BLOCK_MIN` (4) on support -- the 2x2 block floor
restated, so it refuses only what the block gate refuses anyway -- plus the
unchanged block gate and `p_chance`. The family-wise correction is not lost with
the threshold: `rs_modal_null()` is drawn under the SAME rule, so the chance
block rose from 7 to 9 on this collect and compensates automatically. The
binomial `support_min` is still computed and printed, as a description of how
much ground carries a mode.

**Ranking** is by `evidence = n_contiguous * log(median_ratio)`. A periodogram
bin is exponentially distributed about its background, so the log-likelihood
ratio for a line at r times the background is monotone in `log r` -- the model
`rs_twin_llr()` already states -- and the windows carrying a mode contribute
additively. It is an ORDERING statistic and not a calibrated evidence value:
overlapping windows are not independent looks.

**Both keys were measured alone and each fails on the other's case.** Block alone
loses the localised target above. **Ratio alone is worse**: on item 107's 2 mm
injected synthetic fixture it reports seed 7's **1.512 Hz artefact at ratio 38.8
over block 11**, ahead of the true **0.504 Hz at ratio 25.0 over block 30** --
six of item 107's recoveries traded for one. A sharp artefact on a little ground
and a moderate line on a lot of ground are the two failure modes and each key has
exactly one of them.

### Measured

**H1 passes at 5 of 6** against item 109's 3, **H3 at 2 of 2**, and the kill
criterion **H3b at 1 of 12** -- item 107's false-positive rate exactly, with
injected recall held at **6 of 6** (`runs/synthetic/2026-08-07-nomination-fix-null/`).

| collect | amp | @128 | @256 | verdict | item 109 |
|---|---|---|---|---|---|
| C10 | 0.00 control | 10.148 | 15.971 | not comparable | not comparable |
| C10 | 0.13 | 10.148 | 20.630 | not comparable | MOVED |
| C10 | 0.26 | **0.998** | **0.998** | **STABLE** | MOVED |
| C10 | 0.53 | **0.998** | **0.998** | **STABLE** | MOVED |
| C14 | 0.00 control | 0.997 | 5.996 | MOVED | MOVED |
| C14 | 0.13-0.53 | **0.997** | **0.999** | **STABLE** | STABLE |

**The recovery is now a threshold that can be read off the report.** The
injected line's `ev` against its own scene's competition at 128 looks:

| amp | C10 injected | C10 competition | C14 injected | C14 competition |
|---|---|---|---|---|
| 0.00 | -- | **25.0** | -- | **28.3** |
| 0.13 | 16.6 (5th) | 23.8 | **40.5** | 15.5 |
| 0.26 | **24.0** | 23.8 | **51.6** | 15.5 |
| 0.53 | **28.5** | 23.8 | **55.1** | 17.5 |

C10 crosses between 0.13 and 0.26 mm and C14 is across at 0.13 -- **item 103's
competition floor of 0.13-0.26 mm, reached independently through a different
statistic.** The line is now admitted at every amplitude including 0.13 mm where
it sits fifth; before this it was admitted at none.

### Bounds, and what is emphatically NOT fixed

- **C14's motionless control still leads with 0.997 Hz at `ev` 28.3** against an
  injected 1.00 Hz. Item 108 stands untouched: a scene with nothing in it
  answers 0.003 Hz from the frequency being sought, and `--stable` is still the
  only thing that rejects it.
- **My pre-registered prediction of 6 of 6 was wrong**; C10 at 0.13 mm misses.
  It abstains rather than answering wrongly, which the `ev` table explains.
- The surviving static is **seed 31 at 0.958 Hz** where item 107's was seed 17 at
  1.210 -- item 96 again, a change to the selection renames the survivor.
- One fixture family, one operating point, two real collects, and the target was
  put where it was found. **This is a selection result and not a detection.**

### A defect found on the way and deliberately NOT fixed

`rs_local_ratio()` clips its neighbourhood at the band edges, so the first
admissible bin is scored against **10 reference bins against a mid-band bin's
20** -- and a median over half as many draws is about twice as variable, on a
statistic that is then maximised over the band. Both frequencies item 109
reported sit in that starved zone, and making the count constant drops bin 3's
block from 14 to 9. **The obvious fix is wrong and `test_tracking` caught it**:
growing the neighbourhood outward reaches past a red floor's first null into the
deep tail and inflates the low bins it was meant to demote. What a real fix needs
is in `docs/CODE-REVIEW.md`; it is a narrower neighbourhood or a fitted slope,
which is what `rs_spectrum_local_window()`'s header already said and nobody had
needed until now.

---

## 111. The band-edge bias was real, the fix is to NARROW the neighbourhood, and a third defect is now named: the ranking's strength term is a median over chance nominators.

Pre-registered at `8885312` with item 110's thresholds restated unchanged,
because this is a re-measurement of the same claim under a changed estimator and
moving the bar would make the comparison meaningless.
`runs/kilauea/2026-08-07-refbins/`, `runs/synthetic/2026-08-07-refbins-null/`.

### The bias, measured for the first time on scenes containing nothing

Item 110 asserted the band-edge starvation mattered and left it. It does. On
**200 realisations of flat unit-mean noise with nothing planted at all**,
recording which bin won `rs_local_ratio()`'s maximum each time:

| | starved bins (24 of 62, 39% of the band) | full-neighbourhood bins |
|---|---|---|
| before | **72% of maxima**, 2.98% per bin | 0.75% per bin |
| after | 36% of maxima, 1.50% per bin | 1.68% per bin |

**4.0x over-representation becomes 0.89x.** The mechanism is variance, not level:
the background is a MEDIAN, the median of 10 draws is about twice as variable as
the median of 20, and the statistic is then MAXIMISED over the band, so the
noisier denominators win the maximum out of proportion to their content. The
measurement is `runs/kilauea/2026-08-07-refbins/measure_edge_bias.c`.

### The fix is the OPPOSITE of the one item 110 tried

Item 110 widened -- grow the neighbourhood outward until every bin has the 20
references mid-band has -- and `test_tracking`'s red-floor case killed it,
because on a floor rolling off as sinc^2 the extension reaches past the first
null into the deep tail and inflates the low bins it was meant to demote.
**Levelling the count DOWN works**: every bin takes the
`RS_LOCAL_HALF_BINS - RS_LOCAL_GUARD_BINS` = 10 NEAREST references outside the
guard, which is what the bin at the band floor can supply. The span then never
exceeds what it was anywhere and mid-band shrinks to about +-7. A narrower
neighbourhood is what `rs_spectrum_local_window()`'s header has said a steep
floor needs since item 47; widening was the opposite of its own advice.

### Measured: nothing lost, one artefact gone

**H1 5 of 6, H3 2 of 2, and the kill criterion H3b 1 of 12 with injected recall
6 of 6** -- item 110's numbers exactly. **All three pre-registered predictions
were correct**, the first time that has happened here; items 103, 105, 109 and
110 each recorded one that was wrong.

**What visibly changed is an artefact disappearing.** The 10.148 Hz answer that
led C10's motionless control in item 110 is **bin 61, inside the starved zone**,
and it is gone: the control now leads with 5.823 Hz, a mid-band bin, and the
competition fell with it.

| | item 110 | item 111 |
|---|---|---|
| C10 competition `ev` | 23.8 - 25.0 | **17.1 - 18.8** |
| C10 leading control frequency | 10.148 Hz (bin 61, starved) | 5.823 Hz (mid-band) |
| real-arm abstentions | 2 of 8 | **1 of 8** |
| synthetic abstentions | 5 of 12 | **3 of 12** |

The abstentions are the practical gain: fewer answers land on band-edge bins, so
fewer 256-look answers fall above the 128-look Nyquist and `--stable` decides on
9 of 12 motionless scenes where it decided on 7. **A discriminator that abstains
less at the same false-positive rate is strictly better**, and that -- not the
rate -- is what this bought.

C10 still crosses between 0.13 and 0.26 mm, so item 110's agreement with item
103's competition floor stands. **The 0.26 mm margin is thin in both runs**, 19.6
against 18.7 here and 24.0 against 23.8 there; do not quote it as comfortable.

### THE THIRD DEFECT, FOUND BY THE TEST THAT DID NOT EXIST

`tests/test_modalset.c` now covers `rs_spectrum_modal_set()` -- until this, **not
one test in the suite contained the word "modal"**, and `test_modalfit.c` is a
different estimator despite the name. It reproduces item 109's geometry (a
localised block of 16 at ratio 68 beating a broad block of 20 at ratio 16) and
item 107's (a broad block of 30 at ratio 27 beating a sharp block of 9 at ratio
48), so reverting to either ranking key alone is caught **in both directions**.

Writing it exposed something none of items 70-110 had noticed. **`median_ratio`,
the strength term in the ranking key, is a median over CHANCE nominators.** Every
window nominates `RS_MODAL_PER_WINDOW` bins wherever they fall, so each bin of a
K-bin band collects about `n_win * M / K` nominations from noise alone --
**22 of 225 at the 65-bin operating point everything in this file is quoted at**,
against reported supports of 28 to 46 on the item 109 collect. Measured: planting
a line on 16 windows at gain 40 and at gain 200 -- a factor of five in signal --
moves `median_ratio` from **5.97 to 6.39**. The plant is nearly invisible to the
statistic that ranks it. The test had to move to a 257-bin band, where the chance
floor is about 5, before a plant dominated its own statistics.

**The fix is to take the median over the windows in the LARGEST BLOCK** rather
than over every nominator: the block is already computed and is the mode's
measured footprint, so scattered chance nominators are excluded by construction.
It changes `evidence` and needs these two arms run again behind a
pre-registration. Not done; `docs/CODE-REVIEW.md` carries it.

**It is not a reason to doubt items 109-111.** The dilution applies to every
candidate in a scene equally, so it costs the ranking RESOLUTION rather than
biasing it toward a frequency, and the arms measured the ranking end to end at
the operating point in use. It is a reason to expect better separation once
fixed, not a different answer.

### What still does not move

**C14's motionless control leads with 0.997 Hz at `ev` 28.0** against an injected
1.00 Hz. Three items have now changed the selection around it and item 108 is
exactly where it was: only `--stable` rejects it, on the strength of a 256-look
answer of 9.327 Hz.

---

## 112. The ranking's strength term was measuring the background. Over the mode's own block instead, recall reaches 6 of 6 and the false positives fall to 0 of 12.

Pre-registered at `eb0775f` with items 110 and 111's thresholds restated
unchanged, and with a prediction that was a real one rather than a restatement --
including the failure mode that was expected and did not happen.
`runs/kilauea/2026-08-07-blockmedian/`,
`runs/synthetic/2026-08-07-blockmedian-null/`.

### The defect item 111 named

`median_ratio` -- the strength term in `rs_mode_t.evidence` -- was the median
over EVERY window that nominated the bin. Most of them nominated by chance. Each
window picks `RS_MODAL_PER_WINDOW` bins wherever they fall, so every bin of a
K-bin band collects about `n_win * M / K` nominations from noise alone: **22 of
225 at the 65-bin operating point every figure in this file is quoted at**,
against reported supports of 28 to 46. The statistic was therefore mostly
background. Measured on that operating point, planting a line on 16 windows and
varying only its amplitude:

| plant gain | before | after |
|---|---|---|
| 40 | 5.97 | **14.48** |
| 200 | 6.39 | **67.74** |

**A factor of five in signal moved the old statistic by seven percent.**

The fix is to take the median over the windows of the mode's LARGEST BLOCK,
which is its measured footprint, so scattered chance nominators are excluded by
construction. `nom[k]` is indexed by window rather than by nomination order, and
`rs_largest_block()` returns the winning component's membership via one extra
flood fill; the null's inner loop passes NULL and is unchanged.

### Measured: the first 6 of 6

**H1 passes at 6 of 6**, where items 110 and 111 both scored 5. **H3 passes
2 of 2.** **H3b passes and improves: 0 of 12** motionless synthetic scenes
survive against a bar of 1, with injected recall held at 6 of 6.

| | before 110 | 110 | 111 | 112 |
|---|---|---|---|---|
| H1, 128-look modal answer correct | 3 of 6 | 5 of 6 | 5 of 6 | **6 of 6** |
| H3 real controls refused | 2 of 2 | 2 of 2 | 2 of 2 | **2 of 2** |
| H3b motionless synthetic | 1 of 12 | 1 of 12 | 1 of 12 | **0 of 12** |
| injected synthetic recall | 6 of 6 | 6 of 6 | 6 of 6 | **6 of 6** |

**H1 AND `--stable` NOW DIFFER AND MUST BE QUOTED SEPARATELY.** H1 is on the
128-look modal answer. `--stable` then adjudicates it against the 256-look run
and **abstains** on C10 at 0.13 mm, because that run's 256-look answer of
19.631 Hz is above the 128-look Nyquist. So the SELECTION recovers 6 of 6 and
the STABILIZATION TEST reports 5 of 6, and neither figure is the other. Every
earlier item could quote one number because they agreed.

### The predicted risk was the right one to name, and it did not happen

The pre-registration recorded that removing the dilution raises EVERY
candidate's strength, artefacts included, and named item 108's C14 motionless
control -- a small, internally clean block -- as what could gain more than the
injected line and fail H3. Measured, it gained **+0.4** where the injected runs
gained **+7.6 to +14.9**:

| | item 111 `ev` | item 112 `ev` |
|---|---|---|
| C14 motionless control 0.997 Hz | 28.0 | **28.4** |
| C14 injected 0.13 / 0.26 / 0.53 mm | 42.3 / 46.7 / 46.7 | **49.9 / 58.6 / 61.6** |

**Because that control's block is not internally clean** -- ratio 5.3, barely
above the scene -- it gains nothing from a statistic that rewards a clean block.
That is the intended mechanism visible in the one place it was predicted to
misfire: an injected line's block is pure and an artefact's is not, and the old
statistic could not separate them because it averaged both against the same
chance background.

### What moved, and the floor that moves with it

The target's own ratio is what changed; the competition barely did. C10's
injected line at 0.53 mm went from ratio **9.0 to 40.3**, at 0.26 mm from 4.5 to
21.2, and **C10 now recovers at 0.13 mm** where it failed in items 109, 110 and
111.

| amp | C10 injected `ev` | C10 competition | C14 injected `ev` | C14 competition |
|---|---|---|---|---|
| 0.00 | -- | **18.6** | -- | **28.4** |
| 0.13 | **21.6** | 18.6 | **49.9** | 26.9 |
| 0.26 | **39.7** | 18.6 | **58.6** | 18.9 |
| 0.53 | **48.1** | 19.2 | **61.6** | 23.7 |

**THIS PUTS THE COMPETITION FLOOR BELOW ITEM 103's 0.13-0.26 mm, AND THAT
REVISES WHAT THE FLOOR IS.** It is a property of the scene AND THE SELECTION
together, not of the scene alone: item 103 measured it with the selection of the
time, and three items of work on the selection moved it. **Do not quote
0.13-0.26 mm as a scene property.** Item 103's three floors -- target, clutter,
competition -- keep their meanings, but only the first two are scene properties.

### Bounds

- **C10 at 0.13 mm is recovered by the modal set and NOT adjudicated**, because
  `--stable` abstains. It is not a detection.
- **0 of 12 is not a zero false-positive rate.** Twelve realisations, wide
  interval; one survivor would have been consistent with the same rate, and
  three of the four measurements of this fixture found exactly one. What is fair
  is that the rate did not rise and that seed 31, which survived three times, no
  longer does.
- Two real collects, one placement, one operating point, and the target was put
  where it was found. **Still a selection result, not a detection.**
- **Item 108 is untouched by four items of work.** C14's motionless control
  still leads with 0.997 Hz against an injected 1.00 Hz, now at `ev` 28.4, and
  `--stable` is still the only thing that rejects it.

---

## 113. Item 108's cause is a null that assumed independence where window overlap makes neighbours correlated. It is a published failure mode, and the fix makes the reports go quiet.

Pre-registered at `87d27eb`, with four predictions recorded in advance and
scored below -- including one stating that the primary aim would NOT be met.
`runs/kilauea/2026-08-07-clustermass/`,
`runs/synthetic/2026-08-07-clustermass-null/`.

### The diagnosis, and two wrong explanations killed by their own controls

`--shifts` on the motionless C14 collect, nomination replicated offline (the
replica reproduces the binary exactly: support 35, block 17).

**Phase coherence** at the artefact's bin across its 17-window block is 0.673 --
which looks like a coherent oscillation until the controls run. Random
contiguous 17-window blocks elsewhere reach **up to 0.703**, and bin 6 ranks only
**5th of 62 bins** on those same windows. **Amplitude** is **1.54 mm**, the
clutter noise level and ~1000x the seismometer truth. It is noise.

**What is real is the CORRELATION.** Windows are laid at half their width, so
neighbours share half their pixels and track the same dominant scatterer -- item
41 measured overlapping windows returning BIT-IDENTICAL series. Measured: an
adjacent pair shares **2.37 of 6** nominations against **0.71** for a random
pair. Three nulls on the same observed block of 17:

| null | median | p95 | p(block >= 17) |
|---|---|---|---|
| independent draw (shipped until now) | 6 | 8 | **0.001**, above its own 300-trial max of 9 |
| 2x2 tile-shift (observed correlation kept) | 11 | 15 | **0.013** |
| 2x2 dilated (neighbours assumed identical) | 16 | 24 | ~0.5 |

And that one motionless scene had **EIGHT bins clearing p <= 0.05** where the
family-wise design intends 0.05 in total.

### THE SEVENTH TIME A SEARCH FOUND THE FIELD ALREADY THERE

Eklund, Nichols & Knutsson, **PNAS 2016**, *Cluster failure: why fMRI inferences
for spatial extent have inflated false-positive rates*: three million analyses
across the major fMRI packages found parametric **cluster-extent inference
invalid** at a nominal 5% family-wise rate, the cause being *"spatial
autocorrelation functions that do not follow the assumed shape"*, while a
**nonparametric permutation gives nominal rates**. The window grid is their voxel
grid, window overlap is their smoothing, `n_contiguous` is their cluster extent,
and `p_chance` was their invalid parametric p.

Their remedy for the power cost is theirs too: **cluster MASS** is reported as
more powerful than extent and specifically better for **small but intense**
clusters (Zhang, Nichols & Johnson 2009; Hayasaka & Nichols 2004) -- which is
exactly the injected target, block 13 at ratio 40, that an extent threshold
refuses. **Item 108's false positive is the LARGEST block in its scene and
nearly the WEAKEST mass**, so no extent threshold separates it and mass can.
That is why both halves of this change are one idea.

### What was built

`rs_modal_null()` keeps every window's own observed nominations and ratios and
gives each **2x2 TILE** an independent circular shift of the band -- correlation
preserved within a tile, destroyed across tiles. The **2x2 tile is geometry**:
the same half-width stride the block floor and `freq_se`'s `n_eff = n/4` already
come from. Admission then tests **`evidence`** against that null instead of block
size, and `p_chance` becomes P(a chance run reaches this evidence).

### Measured: the reports went quiet

**H1 6 of 6. H2 passes. H3 2 of 2. H3b 0 of 12 reporting with recall 6 of 6.**

The gate change did not move answers; it removed the competition:

| run | modes admitted, item 112 | item 113 |
|---|---|---|
| C10 motionless | 4 | **0** |
| C14 motionless | 8 | **2** |
| every injected run, both collects | 8-10 | **1** |

**Every injected run now admits exactly one mode and it is the injected
frequency.** There is no longer a list to rank.

**AND ITEM 96 IS BROKEN FOR THE FIRST TIME.** That item measured twelve of twelve
motionless synthetic scenes returning a confident frequency -- a 100% answer rate
-- and called a per-scene null the only thing standing between this chain and it.
**Four of those twelve now return NO MODAL ANSWER AT ALL** before `--stable` is
consulted, and **C10's motionless real collect refuses outright**, the first real
motionless collect ever to do so.

### The four pre-registered predictions, scored

| prediction | outcome |
|---|---|
| H2 passes, admitted modes at least halve | **RIGHT** -- 12 across the two motionless collects becomes 2 |
| C10's motionless control refused entirely | **RIGHT** |
| **Item 108's own false positive is NOT refused** | **RIGHT, and it is the honest half of this item** |
| H1 falls to 5 of 6 | **WRONG**, in the better direction -- 6 of 6 |

### Item 108: an order of magnitude, not a solution

C14's motionless control still leads with **0.997 Hz** against a sought 1.00 Hz.
What changed is the number attached to it: **p 0.001 becomes 0.010**, and the
scene admits 2 modes rather than 8. The offline analysis predicted 0.013 for this
block under the same null design, so the implementation agrees with the analysis.
**The residual is most likely that correlation extends beyond the 2x2 tile this
null decorrelates** -- the dilated null puts the same block at p ~ 0.5, which
brackets the truth between. `--stable` is still what rejects it.

### Bounds

- **C10 at 0.13 mm sits exactly on p = 0.050**, `ev` 21.6 against a critical
  21.6. One Monte Carlo trial either way moves it across. It is the threshold,
  not a recovery, and `--stable` abstains on it anyway.
- 0 of 12 and 4 of 12 are rates on twelve realisations with wide intervals.
- The abstention tally on the synthetic arm rose to 7 of 12, but **six of those
  are REFUSALS** -- no modal set at either look count -- which is a different
  and better failure than an answer outside the common band. The scorer lumps
  them; read the `REFUSED` column.
- Two real collects, one placement, one operating point. **Still a selection
  result, not a detection.**

---

## 114. A permutation null on the nominations CANNOT be made exact. Bracket it instead, gate on the conservative end, and item 108 is finally refused.

Pre-registered at `6fb1cb4`. **Every prediction recorded there was correct**,
including the exact recall figure and which point would be lost.
`runs/kilauea/2026-08-07-bracket/`, `runs/synthetic/2026-08-07-bracket-null/`.

### Item 113's stated residual was WRONG, and measuring first is what caught it

Item 113 supposed the correlation between windows reached beyond the 2x2 tile its
null decorrelates. Measured on the motionless C14 collect, shared nominations by
window separation:

| separation | 0 | 1 | 2 | 3-7 | random pair |
|---|---|---|---|---|---|
| shared of 6 | 6.00 | **2.16** | 0.59 | 0.57-0.63 | 0.63 |

**The correlation is confined to ADJACENT windows and is at the random-pair
baseline by separation 2** -- exactly the pixel-sharing range of a 32 px window
at stride 16. The 2x2 tile was the right scale all along. That is the fourth
wrong explanation caught in this arc, and the first one caught *before* anything
was built on it.

### The real residual, and why it cannot be removed

**Any FIXED partition destroys the correlation across its own boundaries.** Half
of all adjacent pairs straddle one, so item 113's tile-shift null reproduces only
**66%** of the observed adjacent sharing and is still anti-conservative.
Measured attempts to close it:

| construction | reproduces observed d=1 sharing |
|---|---|
| fixed 2x2 tiling (item 113) | 66% |
| jittered tile origin | 63% |
| boundary-free field, copy probability 0.8 | 67% |
| ... 0.95 | 86% |
| ... 0.99 | 95% |

A boundary-free shift field only reproduces the observed correlation as it
becomes **globally constant** -- which preserves the very structure the null
exists to destroy. **A permutation null on the nominations cannot be made exact**,
because at short range the mechanical correlation and the structure under test
are the same thing.

### So bracket it

Two nulls bound the truth from both sides. The **shift** draw keeps each window's
own nominations and under-correlates at 66% of observed, giving an optimistic p.
The **dilate** draw makes the whole 2x2 tile share one member's nomination
pattern -- each window keeping its own typical ratio, because borrowing the
source's would manufacture a uniformly clean block no real scene produces -- and
over-correlates at 152%, giving a conservative one. Both are reported; **admission
gates on the conservative end**, so the family-wise rate is controlled at or
below nominal by construction.

### Measured: item 108 is refused

**H4 passes.** The collect that has led with 0.997 Hz against a sought 1.00 Hz
since item 108 now returns **nothing recurs across the windows**:

```
the closest was 0.997 Hz, nominated by 35 of 225 windows in a
largest block of 17 at evidence 28.4 (p 0.342 against chance,
which reaches 37.7 here).
```

That artefact's p across four items: **0.001 -> 0.010 -> 0.342.** C10's
motionless control is refused at both look counts, its best candidate at p 0.999.

**H3 2 of 2, both by the CHAIN rather than by `--stable`. H1 5 of 6 against a
bar of 4. H3b 0 of 12 with injected recall 6 of 6**, and **8 of 12 motionless
synthetic scenes now refuse outright** against item 113's 4 and item 96's 0.
**Item 96 measured 12 of 12 motionless scenes returning a confident frequency;
two thirds are now silent, with no null control.**

| p bracket | verdict |
|---|---|
| C14 motionless (item 108) 0.342 | refused |
| C10 motionless 0.999 | refused |
| C10 injected 0.13 mm 0.942 | refused |
| C10 injected 0.26 mm **0.001-0.043** | admitted, and the width is the honest statement |
| C10 injected 0.53 mm 0.001-0.012 | admitted |
| C14 injected 0.13-0.53 mm 0.001-0.006 | admitted |

### Costs, and one of them was not predicted

- **H1 is 5 of 6.** C10 at 0.13 mm is refused at p 0.942. Item 113 had it at
  exactly p = 0.050 and called it "the threshold, not a recovery"; the
  conservative null puts it firmly on the chance side. Predicted, and priced.
- **NOT PREDICTED: `--stable` reportable falls from 5 of 6 to 3 of 6.** It
  rejects nothing -- the **256-look runs now refuse**, so there is no second
  answer to compare against. C10 at 0.26 and C14 at 0.13 both recover correctly
  at 128 and lose their partner. A more specific chain gives the stabilization
  test less to work with, and that trade is now on the record.
- The synthetic arm shows **no** recall cost and the real arm does. Read them
  together; the synthetic injection is 2 mm, 6.9x that fixture's floor.
- Two real collects, one placement, one operating point, target put where it was
  found. **Still a selection result, not a detection.**
