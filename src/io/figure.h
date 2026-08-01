/* Minimal drawing canvas for annotated figures, private to src/io.
 *
 * Not in include/resonarsat/ for the same reason png.h is not: this is an
 * implementation detail of raster.c. Callers ask for a figure and do not learn
 * that a rasteriser exists.
 *
 * WHY THIS EXISTS AT ALL. rs_raster_write_map() writes one pixel per datum,
 * which for a window map is a 7x7 or 15x15 image -- a coloured speck that no
 * viewer will show at a useful size, carrying no indication of what any colour
 * means. A figure that cannot be read is not output. Upscaling and a labelled
 * colour bar are the whole difference, and both need somewhere to draw text.
 *
 * WHY HAND-ROLLED. The project builds with no external libraries and that is
 * worth keeping; a plotting dependency would be by far the largest thing in the
 * tree. What is needed here is small: filled rectangles, lines, and a fixed
 * font. The cost is that this is a rasteriser with no anti-aliasing, no
 * kerning, and no glyph beyond the ones tabulated below.
 *
 * TEXT IS UPPERCASE. The font covers ASCII 32 to 95, space through '_' -- the
 * digits, the punctuation that appears in a number and in a unit, and the
 * capitals. Lowercase input is upcased rather than dropped, so a caller may pass
 * "quality" and get "QUALITY"; anything outside the range renders as a filled
 * box, which is visible as a defect instead of silently vanishing. Restricting
 * the range still halves the table and costs nothing a scientific figure needs.
 * It stops at '_' rather than 'Z' so that '^' exists: an axis whose unit is
 * (M/S)^2/HZ cannot be labelled without it. */

#ifndef RS_IO_FIGURE_H
#define RS_IO_FIGURE_H

#include <stddef.h>

#include "resonarsat/resonarsat.h"

/* Glyph cell, before scaling. Advance is one column wider than the glyph, which
 * is where inter-character spacing comes from -- there is no kerning. */
#define RS_FIG_GLYPH_W 5
#define RS_FIG_GLYPH_H 7
#define RS_FIG_ADVANCE 6

/* An RGB canvas. 'px' is row-major, three bytes per pixel, tightly packed, which
 * is exactly what rs_png_write() takes for a three-channel image. */
typedef struct {
    unsigned char *px;
    size_t w, h;
} rs_fig_t;

/* Allocate a canvas and fill it with one colour.
 *
 * Returns RS_ERR_ARG for a zero dimension and RS_ERR_ALLOC if the buffer cannot
 * be sized. On failure the canvas is left zeroed, so rs_fig_free() is safe on
 * it. */
resonarsat_status_t rs_fig_create(rs_fig_t *f, size_t w, size_t h,
                                  const unsigned char rgb[3]);

/* Release the canvas buffer. Safe on a zeroed or already-freed canvas. */
void rs_fig_free(rs_fig_t *f);

/* Set one pixel. Coordinates outside the canvas are dropped rather than
 * wrapped or refused: every drawing primitive below clips through this, so a
 * label that runs past the edge loses its tail instead of corrupting the row
 * above. Signed coordinates so that clipping works on the low side too. */
void rs_fig_pixel(rs_fig_t *f, long x, long y, const unsigned char rgb[3]);

/* Fill an axis-aligned rectangle, 'w' by 'h' from the top-left corner (x,y).
 * Non-positive extents draw nothing. Clipped per pixel. */
void rs_fig_rect(rs_fig_t *f, long x, long y, long w, long h,
                 const unsigned char rgb[3]);

/* Draw a one-pixel line by Bresenham. Endpoints inclusive, clipped per pixel.
 *
 * One pixel wide and unsmoothed. For a spectrum with more bins than the plot has
 * columns this is what makes consecutive samples join up rather than appearing
 * as isolated dots. */
void rs_fig_line(rs_fig_t *f, long x0, long y0, long x1, long y1,
                 const unsigned char rgb[3]);

/* Draw a string with its top-left corner at (x,y), each glyph pixel becoming a
 * 'scale' by 'scale' block. A scale below 1 is treated as 1.
 *
 * See the note at the top of this file: the text is rendered uppercase, and
 * characters outside ASCII 32-95 render as a filled box. */
void rs_fig_text(rs_fig_t *f, long x, long y, const char *s, int scale,
                 const unsigned char rgb[3]);

/* Width in pixels that rs_fig_text() will occupy, so a caller can right-align
 * or centre a label, or size a margin to fit the longest one it will draw.
 * Counts the trailing inter-character gap, which keeps the arithmetic uniform;
 * a label is therefore one scaled column wider than its ink. */
size_t rs_fig_text_width(const char *s, int scale);

/* Height in pixels of a line of text at the given scale. */
size_t rs_fig_text_height(int scale);

#endif /* RS_IO_FIGURE_H */
