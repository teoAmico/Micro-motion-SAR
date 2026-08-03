# Glossary

Short definitions of radar and measurement terms used throughout Micromotion.
These definitions describe how the project uses each term; they are not a full
SAR reference.

## A

**Aliasing**  A false image structure or frequency produced when spatial or
temporal sampling is too coarse for the signal being sampled.

**AmpSF (amplitude scale factor)**  Per-vector CPHD metadata used to restore the
scale of stored signal samples.

**Aperture**  The portion of the radar flight path whose pulses are combined to
form an image. A longer aperture usually improves azimuth resolution but spans
more time.

**Aspect-dependent scattering**  A change in a target's radar brightness or
phase as the viewing angle changes. It can resemble motion across sub-apertures.

**Azimuth**  The along-track image direction, approximately parallel to the
radar platform's flight path.

**Backprojection**  The image-formation method used here to combine
phase-history samples at each output location using the acquisition geometry.

## C

**Coherence**  In Micromotion output, a measure of how consistently a spatial
window matches across sub-aperture images. High coherence supports tracking but
does not by itself prove motion.

**Collect**  One radar acquisition and its associated data product.

**Complex imagery**  Focused SAR pixels that retain both amplitude and phase,
unlike detected image products such as ordinary grayscale imagery.

**Consensus**  Agreement among spatial windows on a candidate frequency. It is
a selection diagnostic, not a substitute for a null control.

**CPHD (Compensated Phase History Data)**  A standard product containing
phase-preserving radar samples and the metadata needed to form an image. It is
the preferred input for Micromotion's full single-pass workflow.

## D–L

**Detected product**  SAR imagery containing intensity or magnitude but not the
complex phase needed by this workflow. Examples include GRD, GEC and GEO.

**Dwell**  The elapsed time during which the radar observes the scene. It limits
frequency resolution, but it is not the same as target illumination time within
one sub-aperture.

**Estimator**  The method used to infer motion from the sub-aperture image
sequence. Micromotion provides `correlation`, `phase`, `argmax` and experimental
`splitband` estimators.

**Frequency resolution**  The spacing between spectrum bins, determined mainly
by observation duration. A finer bin spacing does not imply greater accuracy.

**Incidence angle**  The angle between the radar line of sight and the local
vertical. It controls how strongly vertical motion projects into the measured
line-of-sight direction.

**Line of sight (LOS)**  The direction from the radar to the target. Radar phase
measures displacement projected along this direction, not full three-dimensional
motion.

**Look**  One image in the time-ordered sub-aperture sequence. In this project,
"look" and "sub-look" are often used informally for a sub-aperture image.

## N–P

**Null control**  A motionless or otherwise signal-absent case processed with
the same configuration as the candidate measurement. It estimates how often the
pipeline itself produces a convincing peak.

**Observable band**  The frequency range that a particular acquisition and
sub-aperture configuration can measure. Its ceiling depends on both sampling and
sub-aperture averaging, with estimator-specific limits.

**Periodogram**  The spectrum calculated from a tracked time series to locate
periodic components.

**Phase**  The angular component of a complex radar sample. Small changes in
line-of-sight distance change phase, making it sensitive to small motion but
also to wrapping, decorrelation and changing scatterers.

**Phase history**  Complex radar samples recorded pulse by pulse before image
formation. It preserves the timing and phase information needed to build
sub-aperture images.

**PRF (pulse repetition frequency)**  The rate at which radar pulses are
transmitted. It is not the vibration sampling rate; Micromotion's vibration
sampling interval is set by the step between sub-apertures.

**Prominence**  The height of a spectral peak relative to its surrounding
baseline. A prominent peak may still be a processing artefact or noise.

## R–S

**Range**  The image direction corresponding approximately to distance from the
radar.

**SAR (synthetic aperture radar)**  Radar imaging that combines measurements
made along a flight path to synthesize a much larger antenna aperture.

**Scatterer**  A point or area that reflects radar energy. A spatial window may
contain one dominant scatterer or many distributed scatterers.

**SGN**  CPHD metadata describing the sign convention used for the
frequency-to-delay transform. An incorrect convention can mirror a formed image
in range while leaving it apparently well focused.

**SICD (Sensor Independent Complex Data)**  A standard for focused complex SAR
imagery that retains phase.

**SLC (single-look complex)**  Focused complex SAR imagery retaining amplitude
and phase. Despite "single-look" in the name, it is a product type and is not a
Micromotion sub-aperture look.

**Spatial window**  A patch of image pixels tracked as one measurement unit.
Micromotion assigns the resulting time series and spectrum to that window, not
independently to every pixel.

**Spectrum bin**  One discrete frequency sample in a calculated spectrum.

**Squint angle**  The horizontal angular offset between the radar viewing
direction and broadside to the flight path.

**Sub-aperture**  A time-limited portion of the full aperture. Focusing a
sequence of overlapping or non-overlapping sub-apertures produces the image
sequence from which Micromotion estimates motion.

**Sub-aperture averaging**  Attenuation caused by combining motion over the
duration of each sub-aperture. A frequency may be sampled rapidly enough yet
still be averaged away within each look.

## V–W

**Vibration sampling rate**  The rate of the tracked sub-aperture time series,
equal to the inverse of the time step between looks. It must not be confused
with transmit PRF.

**Window quality**  An estimator-dependent confidence diagnostic for a tracked
spatial window. It is useful for screening, but it is not a probability that the
reported motion is real.
