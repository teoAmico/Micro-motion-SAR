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

## Next, and it is narrow

Pull the surrounding ES stations for those apertures and run the neighbour
comparison; check whether the collect dates coincide with the 2021 eruption. If
CJED reads 55 um while its neighbours read 5, it is the instrument. If the flank
moves together, it is real.

## Four swallowed failures, all caught only by a known-good control

The retired availability endpoint (410 + HTML), the federator's 400 on a purely
spatial query, a hand-written datacentre->client map that silently fell back to
IRIS, and a bare `except: pass` that hid 59 unmeasured stations. Each made
FAILURE look like ABSENCE, and the first audit's clean-looking zero rested on 17
of 76 stations. **An audit that cannot say what it did not measure is not an
audit** -- the failure counts above exist for that reason.
