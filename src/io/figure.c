/* Minimal drawing canvas: pixels, rectangles, lines and fixed-font text. */

#include "figure.h"

#include <stdlib.h>
#include <string.h>

/* A 5x7 fixed font covering ASCII 32 ('space') through 95 ('_').
 *
 * Column-major: each glyph is five bytes, one per column left to right, and
 * within a byte bit 0 is the top row and bit 6 the bottom. This is the layout
 * every small character LCD uses, which is why the encoding looks arbitrary and
 * is not -- it is the one form of this table that can be checked against a
 * published reference rather than redrawn by eye.
 *
 * The range runs past 'Z' to '_' for one reason: axis units. A power spectral
 * density is (M/S)^2/HZ, and with the table stopping at 'Z' the caret rendered
 * as the missing-glyph box, which left the only two honest ways to write the
 * label as an ASCII approximation or no units at all. See rs_cmd_mmotion().
 *
 * Index with (c - 32) after the range check in rs_fig_glyph(). */
static const unsigned char rs_font5x7[64][RS_FIG_GLYPH_W] = {
    { 0x00, 0x00, 0x00, 0x00, 0x00 },   /* space */
    { 0x00, 0x00, 0x5F, 0x00, 0x00 },   /* ! */
    { 0x00, 0x07, 0x00, 0x07, 0x00 },   /* " */
    { 0x14, 0x7F, 0x14, 0x7F, 0x14 },   /* # */
    { 0x24, 0x2A, 0x7F, 0x2A, 0x12 },   /* $ */
    { 0x23, 0x13, 0x08, 0x64, 0x62 },   /* % */
    { 0x36, 0x49, 0x55, 0x22, 0x50 },   /* & */
    { 0x00, 0x05, 0x03, 0x00, 0x00 },   /* ' */
    { 0x00, 0x1C, 0x22, 0x41, 0x00 },   /* ( */
    { 0x00, 0x41, 0x22, 0x1C, 0x00 },   /* ) */
    { 0x08, 0x2A, 0x1C, 0x2A, 0x08 },   /* * */
    { 0x08, 0x08, 0x3E, 0x08, 0x08 },   /* + */
    { 0x00, 0x50, 0x30, 0x00, 0x00 },   /* , */
    { 0x08, 0x08, 0x08, 0x08, 0x08 },   /* - */
    { 0x00, 0x60, 0x60, 0x00, 0x00 },   /* . */
    { 0x20, 0x10, 0x08, 0x04, 0x02 },   /* / */
    { 0x3E, 0x51, 0x49, 0x45, 0x3E },   /* 0 */
    { 0x00, 0x42, 0x7F, 0x40, 0x00 },   /* 1 */
    { 0x42, 0x61, 0x51, 0x49, 0x46 },   /* 2 */
    { 0x21, 0x41, 0x45, 0x4B, 0x31 },   /* 3 */
    { 0x18, 0x14, 0x12, 0x7F, 0x10 },   /* 4 */
    { 0x27, 0x45, 0x45, 0x45, 0x39 },   /* 5 */
    { 0x3C, 0x4A, 0x49, 0x49, 0x30 },   /* 6 */
    { 0x01, 0x71, 0x09, 0x05, 0x03 },   /* 7 */
    { 0x36, 0x49, 0x49, 0x49, 0x36 },   /* 8 */
    { 0x06, 0x49, 0x49, 0x29, 0x1E },   /* 9 */
    { 0x00, 0x36, 0x36, 0x00, 0x00 },   /* : */
    { 0x00, 0x56, 0x36, 0x00, 0x00 },   /* ; */
    { 0x08, 0x14, 0x22, 0x41, 0x00 },   /* < */
    { 0x14, 0x14, 0x14, 0x14, 0x14 },   /* = */
    { 0x00, 0x41, 0x22, 0x14, 0x08 },   /* > */
    { 0x02, 0x01, 0x51, 0x09, 0x06 },   /* ? */
    { 0x32, 0x49, 0x79, 0x41, 0x3E },   /* @ */
    { 0x7E, 0x11, 0x11, 0x11, 0x7E },   /* A */
    { 0x7F, 0x49, 0x49, 0x49, 0x36 },   /* B */
    { 0x3E, 0x41, 0x41, 0x41, 0x22 },   /* C */
    { 0x7F, 0x41, 0x41, 0x22, 0x1C },   /* D */
    { 0x7F, 0x49, 0x49, 0x49, 0x41 },   /* E */
    { 0x7F, 0x09, 0x09, 0x09, 0x01 },   /* F */
    { 0x3E, 0x41, 0x49, 0x49, 0x7A },   /* G */
    { 0x7F, 0x08, 0x08, 0x08, 0x7F },   /* H */
    { 0x00, 0x41, 0x7F, 0x41, 0x00 },   /* I */
    { 0x20, 0x40, 0x41, 0x3F, 0x01 },   /* J */
    { 0x7F, 0x08, 0x14, 0x22, 0x41 },   /* K */
    { 0x7F, 0x40, 0x40, 0x40, 0x40 },   /* L */
    { 0x7F, 0x02, 0x0C, 0x02, 0x7F },   /* M */
    { 0x7F, 0x04, 0x08, 0x10, 0x7F },   /* N */
    { 0x3E, 0x41, 0x41, 0x41, 0x3E },   /* O */
    { 0x7F, 0x09, 0x09, 0x09, 0x06 },   /* P */
    { 0x3E, 0x41, 0x51, 0x21, 0x5E },   /* Q */
    { 0x7F, 0x09, 0x19, 0x29, 0x46 },   /* R */
    { 0x46, 0x49, 0x49, 0x49, 0x31 },   /* S */
    { 0x01, 0x01, 0x7F, 0x01, 0x01 },   /* T */
    { 0x3F, 0x40, 0x40, 0x40, 0x3F },   /* U */
    { 0x1F, 0x20, 0x40, 0x20, 0x1F },   /* V */
    { 0x3F, 0x40, 0x38, 0x40, 0x3F },   /* W */
    { 0x63, 0x14, 0x08, 0x14, 0x63 },   /* X */
    { 0x07, 0x08, 0x70, 0x08, 0x07 },   /* Y */
    { 0x61, 0x51, 0x49, 0x45, 0x43 },   /* Z */
    { 0x00, 0x7F, 0x41, 0x41, 0x00 },   /* [ */
    { 0x02, 0x04, 0x08, 0x10, 0x20 },   /* backslash */
    { 0x00, 0x41, 0x41, 0x7F, 0x00 },   /* ] */
    { 0x04, 0x02, 0x01, 0x02, 0x04 },   /* ^ */
    { 0x40, 0x40, 0x40, 0x40, 0x40 }    /* _ */
};

/* The glyph a character renders as, after upcasing.
 *
 * Anything the table does not cover returns a filled box rather than NULL or a
 * blank. A missing glyph is a defect in the caller's label, and a visible box
 * says so where a space would leave the figure looking correct and wrong. */
static const unsigned char *rs_fig_glyph(char c)
{
    static const unsigned char box[RS_FIG_GLYPH_W] = { 0x7F, 0x7F, 0x7F, 0x7F, 0x7F };

    if (c >= 'a' && c <= 'z') c = (char)(c - ('a' - 'A'));
    if (c < 32 || c > 95) return box;
    return rs_font5x7[(unsigned char)c - 32u];
}

/* Allocate a canvas and flood it with one colour. See figure.h. */
resonarsat_status_t rs_fig_create(rs_fig_t *f, size_t w, size_t h,
                                  const unsigned char rgb[3])
{
    if (!f || !rgb) return RS_ERR_ARG;
    f->px = NULL;
    f->w = f->h = 0;
    if (w == 0 || h == 0) return RS_ERR_ARG;

    unsigned char *px = malloc(w * h * 3u);
    if (!px) {
        rs_set_error("figure: cannot allocate a %zux%zu RGB canvas", w, h);
        return RS_ERR_ALLOC;
    }

    for (size_t i = 0; i < w * h; i++) {
        px[i * 3u + 0] = rgb[0];
        px[i * 3u + 1] = rgb[1];
        px[i * 3u + 2] = rgb[2];
    }
    f->px = px;
    f->w = w;
    f->h = h;
    return RS_OK;
}

/* Release the canvas buffer. See figure.h. */
void rs_fig_free(rs_fig_t *f)
{
    if (!f) return;
    free(f->px);
    f->px = NULL;
    f->w = f->h = 0;
}

/* Set one pixel, dropping anything outside the canvas. See figure.h. */
void rs_fig_pixel(rs_fig_t *f, long x, long y, const unsigned char rgb[3])
{
    if (!f || !f->px || !rgb) return;
    if (x < 0 || y < 0) return;
    if ((size_t)x >= f->w || (size_t)y >= f->h) return;

    unsigned char *p = f->px + ((size_t)y * f->w + (size_t)x) * 3u;
    p[0] = rgb[0];
    p[1] = rgb[1];
    p[2] = rgb[2];
}

/* Fill an axis-aligned rectangle. See figure.h. */
void rs_fig_rect(rs_fig_t *f, long x, long y, long w, long h,
                 const unsigned char rgb[3])
{
    if (w <= 0 || h <= 0) return;
    for (long j = 0; j < h; j++) {
        for (long i = 0; i < w; i++) rs_fig_pixel(f, x + i, y + j, rgb);
    }
}

/* Draw a one-pixel line by Bresenham. See figure.h. */
void rs_fig_line(rs_fig_t *f, long x0, long y0, long x1, long y1,
                 const unsigned char rgb[3])
{
    const long dx = (x1 > x0) ? x1 - x0 : x0 - x1;
    const long dy = (y1 > y0) ? y1 - y0 : y0 - y1;
    const long sx = (x0 < x1) ? 1 : -1;
    const long sy = (y0 < y1) ? 1 : -1;
    long err = dx - dy;

    for (;;) {
        rs_fig_pixel(f, x0, y0, rgb);
        if (x0 == x1 && y0 == y1) break;
        const long e2 = 2 * err;
        if (e2 > -dy) { err -= dy; x0 += sx; }
        if (e2 <  dx) { err += dx; y0 += sy; }
    }
}

/* Draw a string, each glyph pixel becoming a scale-by-scale block. See figure.h. */
void rs_fig_text(rs_fig_t *f, long x, long y, const char *s, int scale,
                 const unsigned char rgb[3])
{
    if (!s) return;
    if (scale < 1) scale = 1;

    long pen = x;
    for (const char *p = s; *p; p++) {
        const unsigned char *g = rs_fig_glyph(*p);
        for (int col = 0; col < RS_FIG_GLYPH_W; col++) {
            for (int row = 0; row < RS_FIG_GLYPH_H; row++) {
                if (!(g[col] & (1u << row))) continue;
                rs_fig_rect(f, pen + (long)col * scale, y + (long)row * scale,
                            scale, scale, rgb);
            }
        }
        pen += (long)RS_FIG_ADVANCE * scale;
    }
}

/* Width a string will occupy when drawn. See figure.h. */
size_t rs_fig_text_width(const char *s, int scale)
{
    if (!s) return 0;
    if (scale < 1) scale = 1;
    return strlen(s) * (size_t)RS_FIG_ADVANCE * (size_t)scale;
}

/* Height of one line of text. See figure.h. */
size_t rs_fig_text_height(int scale)
{
    if (scale < 1) scale = 1;
    return (size_t)RS_FIG_GLYPH_H * (size_t)scale;
}
