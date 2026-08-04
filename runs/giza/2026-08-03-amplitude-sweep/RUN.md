# Run: 2026-08-03 giza / amplitude-sweep

**Question this run was meant to answer:** item 35 adjudicated a 2 mm injection at
p = 0.05. How far down does that go? Sweep the amplitude until adjudication fails
and report the first sensitivity bound this project has.

**It does not answer that question.** Below 2 mm the tool does not stop
answering — it answers *wrongly*, with a confidence that rises as the injection
weakens. Recorded as item 37.

## Design

Amplitudes 2.0, 1.0, 0.5, 0.25, 0.125 mm at fixed brightness (`rel 20`) and fixed
frequency (0.163 Hz), everything else as item 35: 96 m grid, 128 looks, 0.90
overlap, `--estimator phase`.

The null trials synthesise their own static scenes and never see the injection,
so **item 35's 19-trial distribution applies unchanged to every amplitude here**.
Reusing it turned a 5 x 90 min sweep into 5 x 5 min. Its maximum is 23.8, so
p = 0.05 requires a prominence above that.

## Result

```
amp mm | injected window 8   | selected      | vs null max 23.8
   2.0 |  0.163 Hz  prom 32.0 | win 8 @ 0.163 | ADJUDICATES   correct
   1.0 |  0.033 Hz  prom 31.4 | win13 @ 0.033 | ADJUDICATES   WRONG ANSWER
   0.5 |  0.033 Hz  prom 47.4 | win13 @ 0.033 | ADJUDICATES   WRONG ANSWER
  0.25 |  0.033 Hz  prom 54.0 | win13 @ 0.033 | ADJUDICATES   WRONG ANSWER
 0.125 |  0.033 Hz  prom 56.0 | win13 @ 0.033 | ADJUDICATES   WRONG ANSWER
```

**Prominence rises monotonically as the signal weakens**, 32.0 to 56.0, and every
one of those clears the null threshold that item 35 called an adjudication.

**0.033 Hz is bin 1.** 128 looks at dt = 0.2399 s is a 30.71 s record, so
df = 0.0326 Hz and the reported frequency is one bin — a single cycle across the
whole dwell. That is a trend, not a vibration.

Windows 8 and 13 agree to within 0.1 prominence at every amplitude, which is the
signature of a common-mode artefact rather than two findings.

## Every available gate endorses it, and more strongly the wronger it gets

```
amp mm | windows in bin 1 | quality | D_A at window 8
   2.0 |       1/25       |  0.419  |  0.581
   1.0 |      10/25       |  0.675  |  0.325
   0.5 |      10/25       |  0.806  |  0.194   <- passes Ferretti 0.25
  0.25 |      10/25       |  0.863  |  0.137   <- passes
 0.125 |      10/25       |  0.884  |  0.121   <- passes
```

`D_A` falls because a scatterer that barely moves has a stable amplitude. The
persistent-scatterer criterion of items 19-20 is satisfied *by the target
failing to vibrate*. Quality rises for the same reason. So the cull, the
PS policy and the null control all agree on a wrong answer, and item 35's null
cannot catch it: its trials are synthetically static, they never contain a bright
dominant scatterer, and their maximum is 23.8 against this artefact's 56.

## The code predicted this and stops one bin short

`src/core/spectrum.c:30` already argues the case:

> Removing the mean alone leaves any linear trend in the record ... Every window
> would then report the same spurious "dominant frequency" of one bin width,
> which looks like a measurement and is not.

Its defences are a least-squares **linear** detrend and exclusion of **bin 0**.
The artefact arrives at bin 1, so it is residual curvature the straight-line fit
does not remove. A `--fmin` exists and defaults to `0.0`.

## What this does and does not overturn

**Item 35's 2 mm result stands as measured.** The selected peak was the injected
one, in the injected window, at the injected frequency, and it beat 19 controls.

**What it loses is its generality.** The gate that passed it would have passed a
wrong answer at any smaller amplitude with a *higher* score, so p = 0.05 there
was a statement about that operating point and not about the method.

**No sensitivity bound is established.** The detection floor at these settings
lies between 1 and 2 mm, but the failure above is silent, so the number that
matters is not where detection stops — it is that nothing here reports when it has.

See `../2026-08-03-amplitude-sweep-fmin/` for the sweep repeated with bins 1-2
excluded, which is what a Hann window's main lobe demands regardless.
