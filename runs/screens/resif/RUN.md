# RESIF (Epos-France) against the three SAR archives

`https://ws.resif.fr/fdsnws/` — the French FDSN node, and the first non-US sensor
network joined here. Item 83's CESMD screen is US-only, which was a stated limit
of it.

## What RESIF has that CESMD's typing would miss

**5932 stations over 118 networks.** There is no `sttype` field, so structures
have to be found by SITE NAME, which is a heuristic and not a classification:

- **dams** — `FR.OGGM` Barrage de Grand'Maison, `FR.SMPL` Barrage de Sampolo,
  and `RA.CGMB` / `RA.CGMH`, the Barrage de la Manzo instrumented at **BASE and
  TOP**, which is a genuine structural array
- **buildings** — `RA.PYTO` "Tour Ophite Terrasse 01-02", `RA.CGCO` immeuble
  Concorde, `RA.OCLD` bâtiment Ledru, `FR.CURIE` bâtiment Curie, schools and
  mairies

**The bridge matches are all false positives** and the reason is worth recording:
French communes routinely contain "Pont" — Pont-de-Claix, Vallon-Pont-d'Arc,
Pont de Salars are places, not bridges. A name heuristic finds the word, not the
structure.

## The funnel, and why the middle stage matters

Space alone is misleading here, because many of these networks are TEMPORARY
experiments that were decommissioned before the SAR collects existed:

| archive | inside a footprint | station OPERATING at collect time | and dwell >= 15 s |
|---|---|---|---|
| Capella | 1 | 1 | **1** |
| Umbra | 45 | 5 | **0** |
| ICEYE | 292 | 48 | **0** |

ICEYE's 292 is real coverage of the Piton de la Fournaise network on La Réunion —
its open programme points at catastrophes, and a volcano network is what sits
under one. It survives the operating-period filter at 48 and dies on dwell: every
one of those collects is 10.0 s. Umbra's apparent Yasur coverage (243 collects)
does not survive the operating-period filter at all.

## The one pairing, measured

`FR.CURIE`, bâtiment Curie, Paris, inside
`CAPELLA_C02_SP_CPHD_HH_20210212074546_20210212074612` — **dwell 26.6 s**,
2021-02-12 07:46:12–07:46:38 UTC. HH channels at 100 Hz, so the SEED band code
is right (item 61) and Nyquist is far above the target band.

Data exists during the aperture: **40,960 bytes**, against a control window
returning 430,080.

**Displacement in the 0.3–3 Hz band, response removed:**

```
  during the aperture   HHE 0.2841  HHN 0.3325  HHZ 0.2624 um RMS
                        3-component 0.5101 um   ->  10.8x BELOW the 5.5 um floor
  midday control        3-component 0.4132 um   ->  13.3x below
```

**Another Oroville.** A proven-static scene: valuable because a null there is
interpretable, useless as a positive control. The midday traffic control is no
higher, so nothing about the time of day rescues it.

## Notes

`obspy` is not in the system Python; it was installed into a throwaway venv under
the session scratchpad rather than into the user's environment.

The join tests SPACE and the station's OPERATING PERIOD. It does not test
earthquake coincidence, because RESIF's station metadata carries no event list —
that comparison would need a separate catalogue query.
