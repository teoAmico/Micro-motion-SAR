/* Raster output.
 *
 * Dependency-free by design: PNG and PGM are both written directly, and raw
 * float cubes carry a text sidecar. GeoTIFF export needs GDAL and is a later
 * addition; keeping it out means the whole project builds with no external
 * libraries at all, which matters more at this stage than georeferencing does.
 *
 * CONTAINER IS CHOSEN BY THE OUTPUT PATH. A path ending in '.png' writes PNG,
 * anything else writes PGM. Callers pass a filename and do not select a format,
 * which is what lets the command line accept either without a flag that could
 * disagree with the extension the user typed. */

#ifndef RESONARSAT_RASTER_H
#define RESONARSAT_RASTER_H

#include <complex.h>
#include <stddef.h>

#include "resonarsat/resonarsat.h"
#include "resonarsat/slc.h"

/* How a real-valued map is coloured.
 *
 * RS_PALETTE_GRAY is the honest default for anything whose absolute level
 * matters, because a reader can judge relative brightness without a key.
 *
 * RS_PALETTE_VIRIDIS exists for maps that are read as structure rather than as
 * level -- depth sections above all, where the question is where a feature sits
 * rather than how bright it is. Greyscale hides small differences in the middle
 * of the range, which is exactly where a weak depth feature would appear.
 * Viridis is perceptually uniform and keeps its ordering in greyscale print and
 * for the common forms of colour blindness, so it does not manufacture contrast
 * that the data does not contain.
 *
 * RS_PALETTE_ENERGY is the conventional energy ramp: blue for low, green for
 * intermediate, red for high. It is what most readers of a tomogram expect, and
 * being able to say "the red band sits at nine metres" without consulting a key
 * is worth a great deal when the figure is being discussed rather than measured.
 *
 * The implementation is Turbo rather than the older jet. Both run blue to red,
 * but jet's luminance rises and falls several times across the ramp, so it
 * creates bright bands at cyan and yellow that read as edges wherever they land
 * -- structure the data does not contain. Turbo was constructed to keep
 * luminance monotonic while preserving the blue-low, red-high ordering, which
 * removes the false banding without giving up the intuition. It is still a
 * rainbow: it is not safe for red-green colour blindness, and it is the wrong
 * choice for a figure that must survive greyscale reproduction. Prefer viridis
 * where the reader will measure from the image rather than read it.
 *
 * RS_PALETTE_JET is the MATLAB default that the SAR Doppler tomography
 * literature is drawn in, including the tomograms of Biondi & Malanga (2022),
 * whose colour bars run dark blue at the low end through cyan, green and yellow
 * to dark red at the high end over a NORMALISED amplitude axis. It exists here
 * for one purpose: putting our output beside a published figure without the
 * colours themselves being a difference. For that comparison it is the correct
 * choice and RS_PALETTE_ENERGY is the wrong one, because a reader matching two
 * images cannot tell a genuine disagreement from a change of ramp.
 *
 * It is otherwise the worst of the three. Jet's luminance is not monotonic, so
 * it invents bright bands at cyan and yellow that read as boundaries wherever
 * they fall, and a normalised colour bar makes every figure's red mean
 * something different. Use it to compare, not to conclude. */
typedef enum {
    RS_PALETTE_GRAY = 0,
    RS_PALETTE_VIRIDIS = 1,
    RS_PALETTE_ENERGY = 2,
    RS_PALETTE_JET = 3
} rs_palette_t;


/* Write an 8-bit greyscale amplitude quicklook.
 *
 * Amplitude is displayed on a decibel scale clipped to 'dyn_range_db' below the
 * 99th percentile, which is the convention that makes SAR imagery legible: a
 * linear stretch is dominated by a handful of bright scatterers and shows
 * nothing else. Percentile rather than maximum for the same reason.
 *
 * Format follows the extension of 'path'; see the note at the top of this file.
 * Returns RS_ERR_IO if the file cannot be written. */
resonarsat_status_t rs_raster_write_quicklook(const rs_slc_t *img, const char *path,
                                              double dyn_range_db);

/* Write a real-valued map with a linear stretch between the given limits.
 *
 * Values outside the limits clamp rather than wrap, so a single outlier cannot
 * alias into the middle of the range and read as signal. Pass lo == hi to
 * autoscale to the data's own extremes.
 *
 * Used for the dominant-frequency and quality maps and for tomographic depth
 * sections, where a decibel stretch would be meaningless. 'palette' is ignored
 * for PGM, which has no way to carry colour: asking for viridis while writing a
 * .pgm silently yields the greyscale the container supports rather than
 * failing, because the alternative is refusing to write an image over a
 * cosmetic request. */
resonarsat_status_t rs_raster_write_map(const double *map, size_t n_row, size_t n_col,
                                        const char *path, double lo, double hi,
                                        rs_palette_t palette);

/* Write a real-valued map as a READABLE FIGURE: upscaled, titled, and with a
 * labelled colour bar.
 *
 * WHY THIS IS SEPARATE FROM rs_raster_write_map(). That function writes one
 * pixel per datum, which is the right thing for an image and the wrong thing
 * for a window map. A 7 by 7 tracking grid becomes a 7 by 7 PNG: a coloured
 * speck that no viewer displays usefully and that carries no indication of what
 * any colour means. Both are kept because they answer different questions --
 * the raw map is data a program reads, this is a figure a person reads.
 *
 * The map is enlarged by an INTEGER factor with nearest-neighbour sampling,
 * chosen so the longer side reaches roughly 'min_px' pixels. Integer and
 * nearest-neighbour on purpose: interpolation would blur one window's value
 * into its neighbour's, and windows overlap already, so a smooth image would
 * imply a spatial resolution the data does not have. Every block of solid
 * colour is exactly one window.
 *
 * 'title' and 'unit' may be NULL. 'unit' labels the colour bar, whose ticks are
 * drawn from the same limits the pixels use, so a reader can convert colour to
 * number without consulting anything else. Pass lo == hi to autoscale, as with
 * rs_raster_write_map().
 *
 * Text renders uppercase and ASCII-only; see src/io/figure.h. Always writes PNG
 * -- the format is not chosen by the path here, because a figure with a colour
 * bar in greyscale PGM would defeat the point. Returns RS_ERR_ARG on a NULL or
 * zero-dimension argument. */
resonarsat_status_t rs_raster_write_map_figure(const double *map,
                                               size_t n_row, size_t n_col,
                                               const char *path,
                                               double lo, double hi,
                                               rs_palette_t palette,
                                               const char *title,
                                               const char *unit,
                                               size_t min_px);

/* The tracking grid, for drawing over the scene it was laid on.
 *
 * Every field is in image pixels of the sub-look the grid was built from, which
 * is the only frame in which the two can be drawn together. 'n_win_az' by
 * 'n_win_rg' windows of 'win_az' by 'win_rg' pixels start every 'stride_az' by
 * 'stride_rg' pixels, so window w -- the index used by the spectrum figure and
 * by the windows CSV -- covers rows (w / n_win_rg) * stride_az onward and
 * columns (w % n_win_rg) * stride_rg onward. 'mark' highlights one window, or
 * RS_WIN_GRID_NO_MARK for none. */
#define RS_WIN_GRID_NO_MARK ((size_t)-1)

typedef struct {
    size_t win_az, win_rg;
    size_t stride_az, stride_rg;
    size_t n_win_az, n_win_rg;
    size_t mark;
} rs_win_grid_t;

/* Write the scene a measurement was taken from, with the tracking grid on it.
 *
 * WHAT THIS IS FOR. Every other figure this tool writes is an abstraction over
 * a scene the reader never sees: a 7 by 7 map of frequencies, and one window's
 * spectrum labelled by an index. Nothing says whether the window that produced
 * the reported peak sits on a target, on distributed clutter, or on the edge of
 * the patch, and no amount of per-window evidence answers that.
 *
 * It also shows how the correlation patch compares to the sub-look's own
 * resolution, which is not the resolution 'validate' and the aliasing warning
 * talk about. Those concern the FULL aperture; a sub-look is coarser by roughly
 * the number of looks, so a grid cell that undersamples the full aperture can
 * still oversample every image the tracker actually sees. Whether a patch spans
 * many resolution cells or barely two is legible here and nowhere else.
 *
 * WHICH IMAGE TO PASS. The sub-look the tracker correlated against -- look 0 of
 * the stack -- and NOT a full-aperture focus. The window grid is defined on that
 * image's dimensions, so the overlay lands where the windows actually were, and
 * in the pulse route the stack is not a decomposition of a full-aperture image
 * at all. Note that a shuffle null permutes the stack in place, so a caller that
 * runs one must copy the reference look before it does.
 *
 * WHAT IS DRAWN. The amplitude on the same decibel stretch as
 * rs_raster_write_quicklook(), enlarged by an integer factor to roughly
 * 'min_px'; a lattice at the window STRIDE, which is finer than the window when
 * they overlap; and the marked window's full patch extent as a red box. The
 * lattice therefore shows how the grid was sampled and the box shows how much
 * ground one correlation actually covered -- the two differ, and the caption
 * states both.
 *
 * The stretch is for looking, not measuring. Bright scatterers saturate at the
 * 99th percentile, so a point-spread function measured off this comes out
 * broader than it is; 'focus --raw' is the route for that. The caption says so.
 *
 * 'grid' may be NULL for a bare enlarged scene. Always writes PNG. Returns
 * RS_ERR_ARG on a NULL image or path.
 *
 * 'd_az_m' and 'd_rg_m' are the ground spacing of one image pixel in azimuth
 * and range, in metres. Pass them and the figure gains metre axes -- ticks
 * measured from the grid centre, an AZIMUTH, M label under the x axis and a
 * RANGE, M label over the y -- plus a reflectivity bar keyed in dB below the
 * scene peak, which is the only thing that makes the grey levels readable as
 * anything. Pass 0 for either and the axes are omitted rather than guessed:
 * a scene whose spacing the caller does not know must not be given a metre
 * scale that looks authoritative.
 *
 * The dB bar is labelled RELATIVE, because that is what a log stretch leaves:
 * 0 dB is this image's own brightest cell, not a calibrated sigma-nought. */
resonarsat_status_t rs_raster_write_scene_figure(const rs_slc_t *img,
                                                 double dyn_range_db,
                                                 const rs_win_grid_t *grid,
                                                 const char *path,
                                                 const char *title,
                                                 double d_az_m, double d_rg_m,
                                                 size_t min_px);

/* Write an XY line plot with axes, tick labels and an optional marker.
 *
 * Written for the one picture this project's argument turns on and did not
 * have: the spectrum of the window a result was read from. The whole question
 * is which bin won and by how much over its neighbours, and that is invisible
 * in a number and in a per-window map alike. A reader who cannot see the
 * spectrum cannot judge whether a reported peak stands out of its
 * surroundings or merely happens to be the largest sample in noise.
 *
 * 'x' and 'y' are 'n' samples of a single series, joined by straight segments.
 * The y axis runs from zero rather than from min(y), because a power spectrum's
 * zero is meaningful and a stretched axis manufactures prominence the data does
 * not contain. The x axis spans the data.
 *
 * 'marker_x' draws a vertical line at that abscissa when it is finite, for
 * showing which bin was selected. Pass NAN for none. 'title', 'x_label' and
 * 'y_label' may be NULL; the y label is drawn horizontally above the axis
 * rather than rotated, there being no rotated text in this rasteriser.
 *
 * Always writes PNG. Returns RS_ERR_ARG on a NULL argument or fewer than two
 * samples -- a one-point plot has no line and no axis span, and refusing is
 * clearer than emitting an empty frame. */
resonarsat_status_t rs_raster_write_plot(const double *x, const double *y, size_t n,
                                         const char *path,
                                         const char *title,
                                         const char *x_label,
                                         const char *y_label,
                                         double marker_x);

/* Write a raw float32 cube with a text sidecar describing its shape.
 *
 * The sidecar carries the dimensions and axis descriptions, so the binary is
 * interpretable without reference to the code that wrote it. Written for the
 * tomographic profiles, which are three-dimensional and have no natural image
 * representation. 'axis_desc' is copied verbatim into the sidecar. */
resonarsat_status_t rs_raster_write_cube(const double *data, size_t n_plane, size_t n_row,
                                         size_t n_col, const char *path,
                                         const char *axis_desc);

#endif /* RESONARSAT_RASTER_H */
