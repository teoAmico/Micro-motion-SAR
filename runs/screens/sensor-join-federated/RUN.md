# Run: 2026-08-05 screens / sensor-join federated

**Question:** the 315-hit screen queried service.iris.edu alone. What does the
whole FDSN federation hold over the same archive, and does any of it move above
item 53's 5.5 um floor?

## Join

648 Capella spotlight/sliding footprints with dwell >= 15 s (item 58 applied up
front), joined against the FDSN federator rather than one archive.

```
410 hits over 648 footprints      against 315 over 939 before
76 distinct stations              against 27
new networks: CE ES CH BW HL CA CI CO TX Z2 1I
```

**Read the shape, not the total: 324 of 410 are HV (Kilauea)**, and 1 of 410 has
a structural site name. Federation found more sensors; they are still ground
vaults, not instruments on loaded structures. Europe contributed ~20 hits.

## Audit

```
measured: 1716 channels, 72 of 76 stations, 113 scenes
failures: 136 not a motion channel, 58 below Nyquist, 10 genuine no-data

1. above 5.5 um, motion channels, Nyquist > 3 Hz : 115  (11 stations)
2. after chronic-station check                    :  70  (dropped 6 AM Raspberry Shakes)
3. after neighbour test                           :   8  (4 distinct stations)
```

| station | rms | site | status |
|---|---|---|---|
| ES.CJED | **55.62 um** | Jedey (El Paso), La Palma, Spain | **UNTESTED - no neighbour** |
| ES.CENR | 5.87 um | El Paso, La Palma, Spain | UNTESTED - no neighbour |
| HV.UWB | 11.78 um | Uwekahuna Bluff, Hawaii | item 61's known chronically-high station |
| HV.WRM | 5.59 um | West Rim, Hawaii | ratio 3.6 vs 27 neighbours |

**ES.CJED is the first above-floor candidate here that is neither an instrument
artefact nor an already-known-bad station**, and it exists only because of the
federation -- network ES is unreachable from service.iris.edu. La Palma is Cumbre
Vieja, so volcanic tremor is the obvious reading: GROUND motion, not a structure.

**Both ES entries survived by NOT BEING TESTED.** No second station sat within
30 km in the same scene, so the check that eliminated 62 of 70 candidates never
ran on them. That is the caveat that matters, and it is why this is a candidate
and not a finding.

## Neighbour check: CJED IS THE INSTRUMENT

Run against every station within 50 km during the same aperture, clients built
from the federator's service URLs (hardcoding ODC returned 204 for everything
and looked exactly like "no neighbours exist").

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
is HV.UWB's signature -- item 61 rejected UWB at 400x -- and it fails the 10x
cut by an order of magnitude. The headline 55.62 um candidate is the instrument.
CENR's 2024 reading fails the same cut at 13.9x.

What survives is CENR during the September 2021 eruption at 2.5-7.4x, and that
is the one part of this that looks like GROUND: Cumbre Vieja erupted on 19
September, CDLV reads 0.8x the day BEFORE, CENR is nearest the vent, and its
readings DECAY 5.87 -> 5.14 -> 2.01 across 24, 26 and 29 September in step with
the tremor. An instrument fault does not taper with an eruption sequence.

But 5.87 um is **1.07x** the 5.5 um floor. Marginal, and volcanic ground motion
rather than a structure under load.

**115 raw exceedances -> 0 credible.** Identical to item 61's 52 -> 0, on a
screen 2.7x larger reaching an entire continent the first one could not see.

## Next, and it is narrow

Done, above: CJED reads 55 um while its neighbours read 0.42. It is the
instrument. Nothing in the federated screen is a candidate.

## Four swallowed failures, all caught only by a known-good control

The retired availability endpoint (410 + HTML), the federator's 400 on a purely
spatial query, a hand-written datacentre->client map that silently fell back to
IRIS, and a bare `except: pass` that hid 59 unmeasured stations. Each made
FAILURE look like ABSENCE, and the first audit's clean-looking zero rested on 17
of 76 stations. **An audit that cannot say what it did not measure is not an
audit** -- the failure counts above exist for that reason.
