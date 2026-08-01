# runs/

One directory per processing run, holding what was asked, what came back, and
enough provenance to repeat it.

```
runs/<scene>/<YYYY-MM-DD>-<suffix>/
  RUN.md          the question, the commit, the host, every command verbatim
  *_freq.png      dominant frequency per window
  *_quality.png   tracking quality per window
  *_spectrum.png  the spectrum the reported frequency was read from
  *_windows.csv   per-window evidence behind the selection
  *.log           stdout and stderr, one pair per configuration
```

Seed one with:

```sh
tools/new-run.sh <scene> <suffix> "what question this run answers"
```

which creates the directory and writes `RUN.md` with the commit, host and
timestamp already filled in. It exists so that recording provenance is easier
than not recording it — the alternative, remembering to write it afterwards, is
what produces figures whose processing route has to be reconstructed from shell
history.

## What a run is for

**State the question before the run, not after.** `RUN.md` asks for it in the
first line because a run without one cannot produce a null result — only a
disappointing one, which is how a negative gets quietly rerun with different
settings until it turns positive.

**A null result stays.** It is the more common outcome here and the more
informative one. Deleting the runs that found nothing is how a project ends up
believing its own best case.

**Keep the evidence, not just the answer.** `*_windows.csv` is the per-window
data the selection was made from. Every earlier result in this project kept the
reported frequency and discarded what produced it, so a later question about the
selection policy — and there has been one — could not be asked without
reprocessing the collect. That is why several documented results cannot now be
re-scored.

**Record the null control beside the measurement.** A result without one cannot
be distinguished from a common-mode artefact, because the consensus statistic is
blind to those by construction. `--null-static` is the check that matters; run
it in the same session, at the same settings, and keep its output here too.

## What is committed

Run outputs are exempted from the repository's ignore rules under this
directory, and only under it: PNG figures, CSV evidence and logs are the record
of what happened and belong in git. Two things stay out —

- **`.f32` cubes and other raw binaries.** Regenerable from the CSV and the
  commit, and large enough to dominate the repository if kept.
- **Collects.** `*.cphd` is ignored everywhere. Reference the product by its
  vendor name in `RUN.md` instead.

Check the size of a figure set before committing it. A run directory should be
kilobytes to a few megabytes; anything larger means a raw product has crept in.
