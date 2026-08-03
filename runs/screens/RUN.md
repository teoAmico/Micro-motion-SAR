# Metadata screens

Extracted CPHD XML blocks and the `validate --xml` verdicts they produced. Each
is ~11 KB against a download of tens of gigabytes; see `docs/DATASETS.md` for the
two range requests that fetch one.

Kept because a screen is a result: it decides whether a collect is worth the
download, and the reasoning behind a "no" is worth as much as behind a "yes".

## CAPELLA_C11_SP_CPHD_HH_20250112202415_20250112202441

Chosen from the survey in `DATASETS.md` as the best-geometry daylight spotlight
CPHD in the archive: incidence 14.2 deg against Istanbul's 19.5, squint 0.1,
12:24 local.

```
PASS  metadata     dwell 25.390 s, lambda 0.0319 m, slant range 651.2 km,
                   platform 7245 m/s, incidence 14.2 deg, 259315 pulses
PASS  freq res     0.0394 Hz from a 25.390 s dwell; 2.000 Hz is 50.8 bins
PASS  band         alpha 0.670%, overlap 0 -> 149 sub-apertures, band 2.939 Hz
WARN  obs ratio    eta 0.340 at 2 Hz
WARN  aperture     alpha 0.670% against a published 4.5-7.6%
PASS  ambiguity    lambda/4 fold caps 8.226 mm vertical; 3 mm is 0.36x
PASS  phase floor  0.3730 mm per look
PASS  memory       259315 x 25001 = 51.9 GB as complex float
VERDICT: WARN, no failures
```

16.0 GB on disk. Geometry is the best in the archive and the screen is clean.

**But the scene is wrong for this project's target class.** The footprint is a
~7 km square centred on 34.0694 N, 118.5598 W — the Santa Monica Mountains above
Pacific Palisades, collected 2025-01-12, five days into the Palisades Fire. It is
a fire-response acquisition over terrain, not built infrastructure. Nothing here
resembles a bridge deck, and the target class this project settled on is
structures whose modes project onto the line of sight.

So: **screened clean, not downloaded.** Excellent geometry over the wrong scene
is not a candidate, and the survey's timezone grouping is what made it look like
one — `America/Los_Angeles` spans the whole US Pacific coast, and of its 26
collects only this one is near Los Angeles at all. The rest are Eugene, Las
Vegas, Fresno and the Mojave.

### And it exposed a gap in the tool

Screened again at the published operating point -- Suppi et al. use aperture
fractions of 1.8-4.9 percent -- the same collect FAILS:

```
FAIL  observable band   alpha 3.600% and overlap 0.30 give 39 sub-apertures.
                        Each averages over 0.9141 s, so the averaging ceiling is
                        0.547 Hz ... The target is ABOVE the band.
```

That is not a defect of the collect. Their aperture fraction is a fraction of a
**5.2-6.1 s observation**, and applying it to a 25.4 s dwell makes each sub-look
0.91 s long, which averages a 2 Hz target away. Reaching their configuration
requires processing about six seconds of this collect.

**`mmotion` cannot do that.** It hard-codes `max_pulses = 0` and `pulse_first = 0`
in its read options; `--max-pulses` and `--pulse-start` are parsed by `focus`
only. `rs_cphd_read_opts_t` already carries both fields, so this is an exposure
gap rather than a missing capability -- but it means `FOLLOW-UPS.md` item 4's
"long dwells may need to be deliberately truncated, and never have been" is
currently **not possible from the CLI**, not merely untried.
