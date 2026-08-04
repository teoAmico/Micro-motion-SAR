# Run: 2026-08-04 giza / overlap-sweep

**Question:** the overlap trade is three-sided and had never been measured whole.
Item 13 says higher overlap raises the sampling ceiling and spends it on
frequencies already averaged away. Item 14 says higher overlap buys sub-look
coherence, and recommends **0.90-0.95 for the phase route** -- which is what
every phase run in this project has used. Item 47 says higher overlap
manufactures the red noise that beats the signal.

**Answer: item 14's recommendation is wrong on real data, by four orders of
magnitude.** Recorded as item 48.

## Result

Injection of 0.163 Hz at 2 mm against a control at the same settings, scored on
the local peak, which is the only statistic here comparable across a coloured
floor. "correct" means within half a bin of the injected frequency at that
overlap's own `df`.

```
 overlap  sub-look res   injected local peak     control   separation
    0.00        6.59 m     816,997  (correct)      22.9      35,676x
    0.50        3.32 m   1,207,566  (correct)      12.6      95,838x
    0.75        1.69 m   1,856,891  (WRONG bin)    12.9     143,945x
    0.90        0.70 m         431.8 (correct)     39.2          11x
    0.95        0.38 m          46.7 (WRONG)       26.0         1.8x
```

**The separation collapses by four orders of magnitude between 0.75 and 0.90,
and at 0.95 neither policy finds the injection at all** -- `best_window` reports
0.105 Hz and the local peak agrees with it, both wrong.

The ordinary prominence tells the same story more quietly: the control scores
6.6-13.6 at overlap 0-0.75 and 17.9-20.7 at 0.90-0.95. The red floor of item 47
is what is growing.

## What this does to item 14

Item 14 measured that recovery "holds to 95% overlap" **on synthetic fixtures**,
and reasoned that high overlap is what buys sub-look coherence on a real collect.
The first half is not true of this real collect and the second was never the
binding constraint.

The mechanism is item 47's: overlap correlates the noise between adjacent
sub-looks, and correlated noise is red, and a red floor is where a spurious peak
lives. What overlap buys in coherence it spends on manufacturing the thing the
coherence was for.

**Sub-look resolution moves the opposite way and does not rescue it.** At
overlap 0 each sub-look is 6.59 m where at 0.95 it is 0.38 m -- seventeen times
coarser -- and item 15's precondition is one dominant scatterer per sub-look
resolution cell, so the coarse end should be the harder one. It is not. The noise
term dominates.

## Two cautions in the numbers

**The local peak named the wrong bin at overlap 0.75**, 0.187 Hz where bin 5 is
0.156 Hz, while `best_window` named the right one. It maximises a RATIO, so a bin
with a quieter neighbourhood can beat a bin with more signal. That is a real
failure mode of the statistic and it appears at its best-separating setting.

**Nothing here says 0.5 is optimal.** Five points, one collect, one frequency,
one amplitude. What is established is that the 0.90-0.95 band this project has
used throughout is far worse than anything below it, not where the maximum sits.
