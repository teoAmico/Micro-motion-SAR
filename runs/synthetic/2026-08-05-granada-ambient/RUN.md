# A real building's ambient motion, injected and swept

## Question

Item 89 found a continuous six-sensor SHM record of the Hospital Real, Granada
(Zenodo 17358241). Items 68-69 established that every injection before item 69
was a pure sinusoid and that a real EARTHQUAKE record is harder. Ambient response
is the case a structure actually spends its life in. Does the chain recover it?

## Waveform preparation

Sensor 6 acceleration (200 Hz, g) double-integrated to displacement by spectral
division, high-passed at **2.5 Hz**. That corner was chosen by MEASUREMENT, not
convention: below 2 Hz the dominant displacement frequency TRACKS THE CORNER,
because 1/omega^2 amplifies the broadband floor and the 0.22 Hz ocean microseism
by (3.9/0.22)^2 = 315x. At 2.0, 2.5 and 3.0 Hz it is stable at 3.91 Hz, so the
corner is not setting the answer.

15 independent segments at 190 s spacing; dominant displacement frequency
3.71-4.30 Hz, median 3.91. **Real peak displacement 0.68-1.97 um**, itself 3-8x
below item 53's floor. Playback rate is per segment, from that segment's own
dominant, so the injected mode lands on the target frequency in every run.

## Commands

`sweep_granada.sh` -- 6 targets x 2 segments x 2 seeds + 2 static controls,
`--n 128 --overlap 0 --estimator phase`, 2 mm. Identical to items 74/77/80/81.

## Result

**1 of 24 correct**, which is chance. 21 answered, 3 refused. The most common
answer is **1.512 Hz, seven times** -- the common-mode artefact of item 76 -- and
**9 of 21 answers are a value one of the STATIC controls also returns**.

Ordered against the earlier injections: sine 6/6, earthquake burst 3/12, ambient
1/24. Ambient response is stationary, broadband and multi-modal, so no line is
ever prominent and the selection falls through to the processing's own artefact.

Full write-up in `docs/FOLLOW-UPS.md` item 91. Item 90 records the independent
refutation this converges with.
