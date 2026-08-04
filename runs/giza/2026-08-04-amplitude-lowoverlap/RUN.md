# Run: 2026-08-04 giza / amplitude-lowoverlap

**Question:** every sensitivity figure in items 37 and 43 was measured at
`--overlap 0.90`, which item 48 has just shown to be the worst setting tested.
Re-run the amplitude sweep at 0.5 and go further down, since the 2 mm point now
sits five orders of magnitude above an empty-desert control.

**Answer: the floor is bracketed between 0.0625 and 0.125 mm, and it is the
first properly controlled bracket this project has.** Recorded as item 50.

## Result

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

`df` is 0.0306 here, so bin 5 is 0.153 Hz and the injected 0.163 lands within
half a bin of it. 0.092 Hz is bin 3, the band floor.

**Below 0.125 mm the reported answer stops being the injection and becomes the
zero-amplitude twin.** The ratio plateaus at 10,400-12,100 and stops responding
to amplitude at all -- across a sixteen-fold change -- which is the signature of
an artefact, and the twin sits at 12,060 in the middle of it.

## The control that matters is not the one item 49 measured

Item 49 calibrated empty desert at 15.1-34.4 over nine disjoint grids. Every
point above -- including the ones that are pure machinery -- clears that by two
to three orders of magnitude.

**Empty desert is the right control for an uninjected scene and the wrong one
for an injected experiment.** An injected run contains a scatterer at 20x the
median sample magnitude that empty desert does not, and that scatterer alone
produces 12,060x. The only valid control for an injected run is its own
zero-amplitude twin, which is what item 38 established and what this measures
again at a different overlap.

Both numbers are real and they answer different questions. Quoting item 49's 34.4
against an injected run would have called 0.0078 mm a detection.

## Against the earlier claims and the literature

Items 37 and 43 reported recovery to 0.0625 mm at overlap 0.90 **with no
zero-amplitude twin at that setting**. Item 38 had already shown the twin
outscoring real injections there, so that figure was never safe; this is the
first sweep with the twin at its own operating point.

The bracket also lands where the literature is. 0.125 mm zero-to-peak is
0.088 mm RMS, against Vattulainen et al.'s smallest confirmed radial RMS
displacement of 0.10 mm. Item 39 flagged the earlier 0.0625 mm as a reason for
suspicion precisely because it beat the published floor by 2x. It no longer does.

## What this is not

The frequency was known in advance, the target was ours, and the scene contains
nothing known to move. This bounds RECOVERABILITY with a proper control. It says
nothing about detecting motion that was not put there.
