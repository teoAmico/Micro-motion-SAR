# Joint transient-and-mode estimation

Host: darwin, Release build. Commit: the one adding `src/core/modalfit.c`.

## Question

Item 79 recorded that for short records the field estimates the transient
JOINTLY with the modal parameters instead of Hann-windowing it away, and called
that the cheapest real improvement available. Build it and measure: does it
recover a real structure's motion where the periodogram does not?

## Commands

- `sweep81.sh` -- 12 injected burst frequencies x 2 clutter seeds + 2 static
  controls at 128 looks, `--estimator phase --tfit 3` at 2 mm. Identical to
  items 74/77/80 in everything but the estimator.
- `zeta_control.sh` -- the control for the damping hypothesis: TEN static
  realisations, six SUSTAINED sine injections and six bursts, same amplitude and
  processing. The sine arm is what separates "zeta measures transience" from
  "zeta measures the presence of signal".

## Result

**No.** 2 of 12 correct against the periodogram's 3 of 12; both statics still
answer confidently.

Two hypotheses formed on the sweep and both died to their controls:

- *fitted damping separates driven from motionless* -- clean on n=2 statics,
  gone at n=10 (statics reach 0.0044, bursts fall to 0.0023).
- *better on sustained tones* -- the sine arm's blocks of 36-47 are matched by
  the paired periodogram run at 43, 47, 30, 44, 47, 47.

What survives is a physics check: a sustained sine returns zeta ~ 0.0000, so the
estimator is measuring transience and not signal presence. And the unit tests in
`tests/test_modalfit.c` establish the estimator itself -- frequency slope 0.9929
rms 0.0072 Hz, damping slope 1.1016.

The conclusion is about where the loss is: **not in the choice of spectral
estimator**. Full write-up in `docs/FOLLOW-UPS.md` item 81.
