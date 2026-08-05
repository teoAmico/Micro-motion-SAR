# CESMD structures against the Capella archive

Host: darwin. Commit: see `git log` for the commit adding this directory.

## Question

Every sensor screen before this one (items 59-62, 75) joined GROUND stations,
and the conclusion was that quiet ground sits ~50x below item 53's 5.5 um floor.
CESMD classifies its stations by what they are mounted on, so filter to
`sttype` **B** (building), **Br** (bridge) and **D** (dam) and ask the question
again for instrumented STRUCTURES, which respond to an earthquake where ground
does not. Each CESMD record carries `pgd` directly, so every candidate arrives
with its own displacement and can be compared with the floor before anything is
downloaded.

## Commands

```sh
python3 harvest_cesmd.py cesmd_structures.json      # 288 structures, 1063 records
python3 join.py footprints.json cesmd_structures.json hits.csv
```

`footprints.json` is the 940-collect Capella harvest from
`../sensor-join-federated/harvest_and_join.py`.

## The control that mattered

The first run reported **0 structures inside any footprint** and it was a BUG:
the harvested ring holds `[lat, lon]` pairs and the containment test read them as
`[lon, lat]`. The control -- a scene's own centre must fall inside its own ring --
returned **0 of 940**, which is what caught it. After the fix it returns 940 of
940 and the spatial join finds **94** pairs. `join.py` now runs that control
first and refuses to print counts if it fails.

This is the same lesson as the FDSN entries in CLAUDE.md, in a new place: a
negative from an unverified harness is not a negative.

## Result

- **288 structures** harvested: 239 buildings, 35 bridges, 14 dams. 0 failed
  queries.
- **1063 records**, 1015 carrying `pgd`. **437 of them (43%) are above the
  5.5 um floor**, the largest at **10.9 cm -- 19,800x the floor.**
- **12 distinct structures have ever been inside a Capella footprint**, all of
  them BUILDINGS in the Los Angeles basin. **No bridge and no dam, ever.**
- **0 records whose shaking overlapped an aperture.** Nearest miss **122 days**.
- Expected coincidences from the geometry: **0.0040**. An archive **252x** this
  one is what an expectation of 1 requires.

## What it changes

It separates two failures this project had been treating as one. Item 62's is an
AMPLITUDE failure -- quiet ground is below the floor. This is a TIME failure:
instrumented structures are comfortably above the floor and are simply never
observed while they are moving. Targeting structures was the right correction and
it fixes the amplitude half completely.

Third independent confirmation that the data problem is not a search problem
(items 62, 75, 83). What remains is a TASKED collect, not an archive query.
