# Run: 2026-08-03 giza / policy-offline

**Question this run is meant to answer:** item 30 measured the tracker recovering
five injected frequencies from the real Giza collect while every run printed
`NO FREQUENCY REPORTED`, and concluded the selection policy discarded them. Which
stage actually discarded them, and is there any per-window statistic in the
recorded evidence that would have reported the injection while refusing the
control?

**Answer: the frequency was never lost.** `rs_spectrum_best_window()` names the
injected frequency in 5 of 5 runs. The scene-wide agreement *gate* is what
suppressed it. Recorded as item 31.

## Provenance

- commit: `5a2e43c`
- **no reprocessing.** Reads only the six `*_windows.csv` files committed under
  `runs/giza/2026-08-02-inject-sweep/`, which exist precisely so a question about
  the selection policy can be asked without rerunning anything.
- ground truth: window 112 of the 15x15 grid holds the injected scatterer; the
  injected frequency is in each filename; `CONTROL` had nothing injected.

## Command

```sh
python3 policy_test.py          # from the repository root
```

Output in `result.log`.

## Result

```
run       gated  inj bin   best(prom)   consensus   block-rank   block-diff
0.098       123        3   0.098 HIT    0.065  --   0.065  --    0.098 HIT
0.130       136        4   0.130 HIT    0.130 HIT   0.065  --    0.130 HIT
0.163       136        5   0.163 HIT    0.163 HIT   0.065  --    0.163 HIT
0.196       136        6   0.195 HIT    0.065  --   0.065  --    0.195 HIT
0.228       136        7   0.228 HIT    0.065  --   0.065  --    0.228 HIT
CONTROL     171        -   0.130        0.065       0.065        0.033
```

Prominence 5/5, consensus 2/5, contiguity alone 0/5, contiguity differenced
against the control 5/5.

**Contiguity alone fails because the desert wins it.** A 12-window block at
0.065 Hz — the lowest bins — is present in every run including the control. That
is item 11's common-mode artefact defeating a spatial statistic, which is what
item 11 says only a null control can catch.

## The missing piece is refusal, not recovery

```
run        max prom   2nd    ratio   blk@winner   margin vs control
0.098         35.35  35.35   1.000           11                   7
0.130         32.56  32.56   1.000           12                   4
0.163         31.97  31.97   1.000            9                   1
0.196         30.04  30.04   1.000           11                   8
0.228         25.77  25.60   1.007            9                   3
CONTROL       17.48  17.16   1.019            8                   0
```

Injected 25.77–35.35 against a control of 17.48 — a 1.47x gap with no overlap.

The `ratio` column matters: max over runner-up is 1.000 in four of five runs,
because the top two windows are **neighbours of the same target**. Any criterion
of the form "the peak stands clear of its rivals" is useless here — the rivals
are the signal.

## Why this is a lead and not a result

Five injected runs and one control; any threshold chosen after seeing this is
fitted to it. The target is 2 mm at 20x the median non-zero sample magnitude, so
the prominence gap may be a direct consequence of that amplitude. The
control-differenced block statistic is degenerate on the control, since
differencing a run against itself gives zero everywhere. And prominence is
anti-correlated with correctness everywhere else in `FOLLOW-UPS` — items 7–9 and
25.

See item 31 for the full caveats and the next step.
