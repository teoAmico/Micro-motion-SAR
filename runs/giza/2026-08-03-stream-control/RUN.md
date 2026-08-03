# Run: 2026-08-03 giza / stream-control

**Question this run is meant to answer:** item 22 verified `mmotion --stream`
against the resident path at 16 looks over a 64 m grid. Does streaming reproduce
the resident result at the operating point the project actually runs — 128 looks,
0.90 overlap, 256 m grid, `--rbins 4096` — and what does it cost?

**Answer: exact, 10x less memory, and 5.3x the wall time on this hardware.**

## Scope, and why it is one run and not six

Only the CONTROL run of item 30's sweep can be streamed. The five injected runs
use `--inject-vib`, which `mmotion` refuses to combine with `--stream`: the
injection is written into the resident phase history and a streamed read would
re-read it from disc and never see it. That guard exists because item 28 is the
story of an injected positive control that silently was not there.

## Provenance

- commit: `e680739`
- host: 8 cores, 24 GB RAM, macOS; collect on external USB (exFAT)
- collect: `CAPELLA_C13_SP_CPHD_HH_20241004001939_20241004002012.cphd`, 39 GB

## Command

```sh
./build/micromotion mmotion --cphd "$C" --at 29.979175,31.134186 \
    --estimator phase --n 128 --overlap 0.90 \
    --size 256 --cell 1.0 --win 32 --rbins 4096 --coherence 0 \
    --stream 16384 --out control
```

## Correctness: exact

Against the committed resident control, `sweep_CONTROL_windows.csv`, all twelve
substantive per-window columns are **identical** — `dominant_hz`, `prominence`,
`quality`, `d_a` and the rest, digit for digit — and the cull's own header line
matches byte for byte:

```
# cull_hz=0.0651340991323 agree=21 input=171 survivors=65 snr_cull=0
  sigma_cull=0 neigh_cull=106 snr_gate=0 snr_null=0 sigma_gate_px=0 gates=0
```

That extends item 22's verification from 16 looks over a 64 m grid to the
configuration the Giza runs use.

**The thirteenth column differs, and that is a second result rather than a
discrepancy.** `passed_cull` sums to 171 in the resident file and 65 in this one.
The resident file was written before `docs/CODE-REVIEW.md` finding 2 was fixed,
when the column was recomputed from the recorded thresholds and could not express
gate 3 — so it reported every gate-1-and-2 survivor, overstating by exactly the
106 windows `neigh_cull` removed. This run carries the selector's own verdict and
sums to `survivors=65`, as its `passed_cull_source=selector expected_sum=65`
header says. The defect and its fix, on the same scene.

## Cost: 10x less memory, 5.3x the wall time

```
                    wall        user     mean CPU   peak RSS
  resident      44 min 55 s        --        442%     ~11 GB of phase history
  streamed       4 h 00 min    9626 s         67%       1.05 GB
```

Memory is what `--stream` was built for and it delivers: 1.05 GB against the
11 GB of resident phase history `--rbins 4096` implies.

**The wall-clock cost is far larger than item 22's 6% for `focus --stream`, and
the CPU figure says why it is not extra work.** Total user time went *down*
(9626 s against the resident run's implied ~11900 s), while wall time went up
5.3x — so the process was waiting, not computing. Mean CPU fell from 442% to 67%
on an 8-core machine. The streaming path reads the collect once in blocks and
re-parses the header and PVP per block, and this collect is on an external USB
drive with `--rbins 4096` of 29160 samples, so every pulse is a strided read.

Not isolated further here: distinguishing drive bandwidth from seek cost from
per-block re-parsing needs the same run against local storage, which is a
different experiment. What is established is that on this hardware `--stream` at
this operating point is an I/O-bound trade, not a 6% one, and a run budget should
assume hours rather than minutes.

## Note on the headline

This run prints `NOT ADJUDICATED` where the 2026-08-02 runs printed
`NO FREQUENCY REPORTED`. That is the gate change of item 31, not a streaming
effect — the underlying per-window numbers are identical.

```
strongest peak in window 182: 0.130 Hz, prominence 17.5, quality 0.519
  backed by 31 windows, largest touching block 8
  NOT ADJUDICATED
```

Prominence 17.5 is the control value item 31 measured, against 25.77–35.35 for
the five injected runs.
