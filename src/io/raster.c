/* Raster output: PNG and PGM images, and raw float cubes. */

#include "resonarsat/raster.h"

#include "figure.h"
#include "png.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Comparison for qsort over doubles, used to find a display percentile. */
static int rs_cmp_double(const void *a, const void *b)
{
    const double x = *(const double *)a, y = *(const double *)b;
    return (x < y) ? -1 : (x > y) ? 1 : 0;
}

/* True when the path names a PNG.
 *
 * Case-insensitive because '.PNG' is common on files that have been through a
 * Windows toolchain, and writing a PGM into a name ending in .PNG would produce
 * something no viewer opens. */
static int rs_path_is_png(const char *path)
{
    const size_t n = strlen(path);
    if (n < 4) return 0;
    const char *e = path + n - 4;
    return (e[0] == '.' &&
            (e[1] == 'p' || e[1] == 'P') &&
            (e[2] == 'n' || e[2] == 'N') &&
            (e[3] == 'g' || e[3] == 'G'));
}

/* Write an already-quantised 8-bit greyscale buffer in whichever container the
 * path asks for. Both writers take the same row-major, tightly packed bytes, so
 * every caller quantises once and stays out of the format question. */
static resonarsat_status_t rs_write_gray(const char *path, const unsigned char *px,
                                         size_t n_row, size_t n_col)
{
    if (rs_path_is_png(path)) return rs_png_write(path, px, n_col, n_row, 1);

    FILE *f = fopen(path, "wb");
    if (!f) {
        rs_set_error("raster: cannot open %s for writing", path);
        return RS_ERR_IO;
    }
    fprintf(f, "P5\n%zu %zu\n255\n", n_col, n_row);
    const int ok = (fwrite(px, 1, n_row * n_col, f) == n_row * n_col) && !ferror(f);
    if (fclose(f) != 0 || !ok) {
        rs_set_error("raster: write failed for %s", path);
        return RS_ERR_IO;
    }
    return RS_OK;
}

/* Colour ramps, as sixteen anchors each, interpolated linearly.
 *
 * The reference maps are 256-entry tables; carrying anchors instead keeps the
 * source readable and costs under one 8-bit level of error, which cannot matter
 * for a display product. Both are sampled at equal intervals from their
 * reference definitions. See raster.h on when each is appropriate. */
static const unsigned char rs_anchor_viridis[16][3] = {
    {  68,   1,  84 }, {  72,  26, 108 }, {  71,  47, 125 }, {  65,  68, 135 },
    {  57,  86, 140 }, {  49, 104, 142 }, {  42, 120, 142 }, {  35, 136, 142 },
    {  31, 152, 139 }, {  34, 168, 132 }, {  53, 183, 121 }, {  85, 198, 103 },
    { 122, 209,  81 }, { 165, 219,  54 }, { 210, 226,  27 }, { 253, 231,  37 }
};

/* Turbo: blue for low, green for intermediate, red for high. */
static const unsigned char rs_anchor_energy[16][3] = {
    {  48,  18,  59 }, {  65,  69, 171 }, {  74, 117, 240 }, {  70, 160, 253 },
    {  49, 198, 223 }, {  34, 225, 190 }, {  48, 243, 140 }, {  96, 253,  89 },
    { 146, 255,  52 }, { 188, 243,  42 }, { 222, 220,  49 }, { 247, 187,  49 },
    { 254, 145,  38 }, { 241,  99,  22 }, { 216,  60,   9 }, { 165,  20,   2 }
};

/* MATLAB jet, the ramp the published tomograms use. See raster.h. */
static const unsigned char rs_anchor_jet[16][3] = {
    {   0,   0, 143 }, {   0,   0, 207 }, {   0,   0, 255 }, {   0,  63, 255 },
    {   0, 127, 255 }, {   0, 191, 255 }, {  38, 255, 214 }, {  93, 255, 159 },
    { 159, 255,  93 }, { 214, 255,  38 }, { 255, 208,   0 }, { 255, 145,   0 },
    { 255,  81,   0 }, { 255,  18,   0 }, { 207,   0,   0 }, { 143,   0,   0 }
};

/* Resolve a palette name for the command line. See raster.h. */
rs_palette_t rs_palette_from_name(const char *name, rs_palette_t fallback)
{
    if (!name || !*name) return fallback;
    if (strcmp(name, "gray") == 0 || strcmp(name, "grey") == 0) return RS_PALETTE_GRAY;
    if (strcmp(name, "viridis") == 0) return RS_PALETTE_VIRIDIS;
    if (strcmp(name, "energy") == 0)  return RS_PALETTE_ENERGY;
    if (strcmp(name, "jet") == 0)     return RS_PALETTE_JET;
    fprintf(stderr, "warning: unknown palette '%s'; using the default. "
                    "Known: gray, viridis, energy, jet\n", name);
    return fallback;
}

/* Map a normalised value to a colour from one of the ramps above.
 *
 * 't' is clamped to [0,1]; the comparison is written so that NaN clamps to zero
 * rather than falling through to an out-of-range table index. A NaN reaching
 * here is not hypothetical -- a fully masked tomogram is all NaN, and it must
 * still render. */
static void rs_colourise(double t, rs_palette_t palette, unsigned char rgb[3])
{
    const unsigned char (*anchor)[3] =
        (palette == RS_PALETTE_ENERGY) ? rs_anchor_energy :
        (palette == RS_PALETTE_JET)    ? rs_anchor_jet    : rs_anchor_viridis;

    if (!(t >= 0.0)) t = 0.0;      /* also catches NaN, which would index wildly */
    if (t > 1.0) t = 1.0;

    const double s = t * 15.0;
    size_t i = (size_t)s;
    if (i > 14) i = 14;
    const double f = s - (double)i;

    for (int k = 0; k < 3; k++) {
        const double v = (double)anchor[i][k] +
                         f * ((double)anchor[i + 1][k] - (double)anchor[i][k]);
        rgb[k] = (unsigned char)(v + 0.5);
    }
}

/* Write an amplitude quicklook on a decibel scale. See raster.h. */
resonarsat_status_t rs_raster_write_quicklook(const rs_slc_t *img, const char *path,
                                              double dyn_range_db)
{
    if (!img || !img->data || !path) return RS_ERR_ARG;
    if (dyn_range_db <= 0.0) dyn_range_db = 40.0;

    const size_t n = img->n_az * img->n_rg;
    double *amp = malloc(n * sizeof *amp);
    double *sorted = malloc(n * sizeof *sorted);
    unsigned char *px = malloc(n);
    if (!amp || !sorted || !px) { free(amp); free(sorted); free(px); return RS_ERR_ALLOC; }

    for (size_t i = 0; i < n; i++) amp[i] = (double)cabsf(img->data[i]);
    memcpy(sorted, amp, n * sizeof *sorted);
    qsort(sorted, n, sizeof *sorted, rs_cmp_double);

    const double hi = sorted[(size_t)((double)(n - 1) * 0.99)];
    free(sorted);

    const double hi_db = (hi > 0.0) ? 20.0 * log10(hi) : 0.0;
    const double lo_db = hi_db - dyn_range_db;

    for (size_t i = 0; i < n; i++) {
        /* A uniformly zero image is legitimate output from a null test; emit
         * black rather than failing, so the test still produces a figure. */
        double v = 0.0;
        if (hi > 0.0 && amp[i] > 0.0) {
            v = (20.0 * log10(amp[i]) - lo_db) / (hi_db - lo_db);
        }
        if (v < 0.0) v = 0.0;
        if (v > 1.0) v = 1.0;
        px[i] = (unsigned char)(v * 255.0 + 0.5);
    }
    free(amp);

    const resonarsat_status_t st = rs_write_gray(path, px, img->n_az, img->n_rg);
    free(px);
    return st;
}

/* Write a real map with a linear stretch, autoscaling when lo == hi. */
resonarsat_status_t rs_raster_write_map(const double *map, size_t n_row, size_t n_col,
                                        const char *path, double lo, double hi,
                                        rs_palette_t palette)
{
    if (!map || !path || n_row == 0 || n_col == 0) return RS_ERR_ARG;

    const size_t n = n_row * n_col;

    if (lo == hi) {
        /* Autoscale over the finite samples only. A single NaN would otherwise
         * poison both limits through the comparisons and blank the image. */
        int seen = 0;
        for (size_t i = 0; i < n; i++) {
            if (!isfinite(map[i])) continue;
            if (!seen) { lo = hi = map[i]; seen = 1; continue; }
            if (map[i] < lo) lo = map[i];
            if (map[i] > hi) hi = map[i];
        }
        if (!seen) { lo = 0.0; hi = 1.0; }
    }
    if (hi <= lo) hi = lo + 1.0;

    const int colour = (palette != RS_PALETTE_GRAY) && rs_path_is_png(path);
    unsigned char *px = malloc(n * (colour ? 3u : 1u));
    if (!px) return RS_ERR_ALLOC;

    for (size_t i = 0; i < n; i++) {
        double v = isfinite(map[i]) ? (map[i] - lo) / (hi - lo) : 0.0;
        if (v < 0.0) v = 0.0;
        if (v > 1.0) v = 1.0;
        if (colour) rs_colourise(v, palette, px + i * 3);
        else        px[i] = (unsigned char)(v * 255.0 + 0.5);
    }

    resonarsat_status_t st;
    if (colour) {
        st = rs_png_write(path, px, n_col, n_row, 3);
    } else {
        st = rs_write_gray(path, px, n_row, n_col);
    }
    free(px);
    return st;
}

/* Format a number for an axis or colour-bar tick.
 *
 * Three significant figures, which is more than a colour bar can be read to and
 * enough that two adjacent ticks never print identically at the ranges these
 * figures cover. Written into a caller's buffer so the callers can hold several
 * labels at once while sizing a margin. */
static void rs_fig_fmt(char *buf, size_t cap, double v)
{
    snprintf(buf, cap, "%.3g", v);
}

/* Find the finite extremes of a map, or fall back to [0,1].
 *
 * Shared by the two figure writers and by nothing else, because both have the
 * same NaN problem: a single non-finite sample poisons a naive min/max through
 * the comparisons and blanks the whole figure. */
static void rs_fig_limits(const double *v, size_t n, double *lo, double *hi)
{
    int seen = 0;
    for (size_t i = 0; i < n; i++) {
        if (!isfinite(v[i])) continue;
        if (!seen) { *lo = *hi = v[i]; seen = 1; continue; }
        if (v[i] < *lo) *lo = v[i];
        if (v[i] > *hi) *hi = v[i];
    }
    if (!seen) { *lo = 0.0; *hi = 1.0; }
}

/* Write an upscaled, titled map with a labelled colour bar. See raster.h. */
resonarsat_status_t rs_raster_write_map_figure(const double *map,
                                               size_t n_row, size_t n_col,
                                               const char *path,
                                               double lo, double hi,
                                               rs_palette_t palette,
                                               const char *title,
                                               const char *unit,
                                               size_t min_px)
{
    if (!map || !path || n_row == 0 || n_col == 0) return RS_ERR_ARG;
    if (min_px == 0) min_px = 360;

    if (lo == hi) rs_fig_limits(map, n_row * n_col, &lo, &hi);
    if (hi <= lo) hi = lo + 1.0;

    /* Integer zoom, so every block of colour is exactly one window. */
    const size_t longer = (n_row > n_col) ? n_row : n_col;
    size_t zoom = (min_px + longer - 1) / longer;
    if (zoom < 1) zoom = 1;
    if (zoom > 64) zoom = 64;

    const int ts = 2;                                   /* text scale */
    const size_t ch = rs_fig_text_height(ts);
    const size_t map_w = n_col * zoom, map_h = n_row * zoom;

    /* Margins sized to the longest label each region will actually draw. */
    char lab_hi[32], lab_lo[32];
    rs_fig_fmt(lab_hi, sizeof lab_hi, hi);
    rs_fig_fmt(lab_lo, sizeof lab_lo, lo);
    size_t lab_w = rs_fig_text_width(lab_hi, ts);
    if (rs_fig_text_width(lab_lo, ts) > lab_w) lab_w = rs_fig_text_width(lab_lo, ts);
    if (unit && rs_fig_text_width(unit, ts) > lab_w) lab_w = rs_fig_text_width(unit, ts);

    const size_t pad = 12;
    const size_t bar_w = 18, bar_gap = 14;
    const size_t top = pad + (title ? ch + 10 : 0);
    const size_t left = pad;
    const size_t right = bar_gap + bar_w + 6 + lab_w + pad;
    const size_t bottom = pad + ch + 6;                 /* extent caption */

    rs_fig_t f;
    static const unsigned char white[3] = { 255, 255, 255 };
    static const unsigned char black[3] = { 20, 20, 20 };
    static const unsigned char grey[3]  = { 130, 130, 130 };

    resonarsat_status_t st = rs_fig_create(&f, left + map_w + right,
                                           top + map_h + bottom, white);
    if (st != RS_OK) return st;

    /* The map itself, nearest-neighbour so a window stays a solid block. */
    for (size_t r = 0; r < map_h; r++) {
        for (size_t c = 0; c < map_w; c++) {
            const double m = map[(r / zoom) * n_col + (c / zoom)];
            unsigned char rgb[3];
            double t = isfinite(m) ? (m - lo) / (hi - lo) : 0.0;
            if (t < 0.0) t = 0.0;
            if (t > 1.0) t = 1.0;
            if (palette == RS_PALETTE_GRAY) {
                const unsigned char g = (unsigned char)(t * 255.0 + 0.5);
                rgb[0] = rgb[1] = rgb[2] = g;
            } else {
                rs_colourise(t, palette, rgb);
            }
            rs_fig_pixel(&f, (long)(left + c), (long)(top + r), rgb);
        }
    }
    rs_fig_rect(&f, (long)left - 1, (long)top - 1, (long)map_w + 2, 1, black);
    rs_fig_rect(&f, (long)left - 1, (long)(top + map_h), (long)map_w + 2, 1, black);
    rs_fig_rect(&f, (long)left - 1, (long)top - 1, 1, (long)map_h + 2, black);
    rs_fig_rect(&f, (long)(left + map_w), (long)top - 1, 1, (long)map_h + 2, black);

    /* Colour bar, high at the top, with ticks from the same limits. */
    const size_t bar_x = left + map_w + bar_gap;
    for (size_t r = 0; r < map_h; r++) {
        unsigned char rgb[3];
        const double t = 1.0 - (double)r / (double)(map_h > 1 ? map_h - 1 : 1);
        if (palette == RS_PALETTE_GRAY) {
            const unsigned char g = (unsigned char)(t * 255.0 + 0.5);
            rgb[0] = rgb[1] = rgb[2] = g;
        } else {
            rs_colourise(t, palette, rgb);
        }
        rs_fig_rect(&f, (long)bar_x, (long)(top + r), (long)bar_w, 1, rgb);
    }
    rs_fig_rect(&f, (long)bar_x - 1, (long)top - 1, (long)bar_w + 2, 1, black);
    rs_fig_rect(&f, (long)bar_x - 1, (long)(top + map_h), (long)bar_w + 2, 1, black);
    rs_fig_rect(&f, (long)bar_x - 1, (long)top - 1, 1, (long)map_h + 2, black);
    rs_fig_rect(&f, (long)(bar_x + bar_w), (long)top - 1, 1, (long)map_h + 2, black);

    for (int k = 0; k <= 4; k++) {
        const double frac = (double)k / 4.0;
        const double v = hi - frac * (hi - lo);
        const long y = (long)top + (long)(frac * (double)(map_h - 1));
        char lab[32];
        rs_fig_fmt(lab, sizeof lab, v);
        rs_fig_rect(&f, (long)(bar_x + bar_w), y, 4, 1, black);
        rs_fig_text(&f, (long)(bar_x + bar_w + 6), y - (long)(ch / 2), lab, ts, black);
    }
    if (unit) {
        rs_fig_text(&f, (long)bar_x, (long)(top + map_h) + 8, unit, ts, black);
    }

    if (title) rs_fig_text(&f, (long)left, (long)pad, title, ts, black);

    /* The grid extent, so the block size is not mistaken for resolution. */
    {
        char cap[96];
        snprintf(cap, sizeof cap, "%zu X %zu WINDOWS, %zu PX EACH",
                 n_row, n_col, zoom);
        rs_fig_text(&f, (long)left, (long)(top + map_h) + 8, cap, ts, grey);
    }

    st = rs_png_write(path, f.px, f.w, f.h, 3);
    rs_fig_free(&f);
    return st;
}

/* Write an XY line plot with axes and an optional marker. See raster.h. */
resonarsat_status_t rs_raster_write_plot(const double *x, const double *y, size_t n,
                                         const char *path,
                                         const char *title,
                                         const char *x_label,
                                         const char *y_label,
                                         double marker_x)
{
    if (!x || !y || !path || n < 2) return RS_ERR_ARG;

    double x_lo = 0.0, x_hi = 0.0, y_lo = 0.0, y_hi = 0.0;
    rs_fig_limits(x, n, &x_lo, &x_hi);
    rs_fig_limits(y, n, &y_lo, &y_hi);
    if (x_hi <= x_lo) x_hi = x_lo + 1.0;

    /* Zero-based y: a power spectrum's zero is meaningful, and starting the axis
     * at min(y) manufactures prominence the data does not contain. */
    y_lo = 0.0;
    if (y_hi <= y_lo) y_hi = 1.0;

    const int ts = 2;
    const size_t ch = rs_fig_text_height(ts);

    char lab[32];
    rs_fig_fmt(lab, sizeof lab, y_hi);
    const size_t y_lab_w = rs_fig_text_width(lab, ts);

    const size_t pad = 12;
    const size_t top = pad + (title ? ch + 10 : 0) + (y_label ? ch + 6 : 0);
    const size_t left = pad + y_lab_w + 8;
    const size_t right = pad + 24;
    const size_t bottom = pad + ch + 8 + (x_label ? ch + 6 : 0);
    const size_t plot_w = 620, plot_h = 300;

    rs_fig_t f;
    static const unsigned char white[3]  = { 255, 255, 255 };
    static const unsigned char black[3]  = { 20, 20, 20 };
    static const unsigned char grey[3]   = { 190, 190, 190 };
    static const unsigned char blue[3]   = { 40, 90, 200 };
    static const unsigned char red[3]    = { 200, 50, 40 };

    resonarsat_status_t st = rs_fig_create(&f, left + plot_w + right,
                                           top + plot_h + bottom, white);
    if (st != RS_OK) return st;

    /* Gridlines first, so data and axes draw over them. */
    for (int k = 1; k < 5; k++) {
        const long gy = (long)top + (long)((double)k / 5.0 * (double)plot_h);
        rs_fig_rect(&f, (long)left, gy, (long)plot_w, 1, grey);
    }

    /* Axes. */
    rs_fig_rect(&f, (long)left, (long)(top + plot_h), (long)plot_w + 1, 1, black);
    rs_fig_rect(&f, (long)left, (long)top, 1, (long)plot_h + 1, black);

    /* Ticks and labels. */
    for (int k = 0; k <= 4; k++) {
        const double frac = (double)k / 4.0;
        const double vx = x_lo + frac * (x_hi - x_lo);
        const long px = (long)left + (long)(frac * (double)plot_w);
        rs_fig_fmt(lab, sizeof lab, vx);
        rs_fig_rect(&f, px, (long)(top + plot_h) + 1, 1, 5, black);
        rs_fig_text(&f, px - (long)(rs_fig_text_width(lab, ts) / 2),
                    (long)(top + plot_h) + 9, lab, ts, black);

        const double vy = y_lo + frac * (y_hi - y_lo);
        const long py = (long)(top + plot_h) - (long)(frac * (double)plot_h);
        rs_fig_fmt(lab, sizeof lab, vy);
        rs_fig_rect(&f, (long)left - 5, py, 5, 1, black);
        rs_fig_text(&f, (long)left - 8 - (long)rs_fig_text_width(lab, ts),
                    py - (long)(ch / 2), lab, ts, black);
    }

    /* The marker goes under the series, so a peak is never hidden by it. */
    if (isfinite(marker_x) && marker_x >= x_lo && marker_x <= x_hi) {
        const long mx = (long)left +
                        (long)((marker_x - x_lo) / (x_hi - x_lo) * (double)plot_w);
        for (long yy = 0; yy < (long)plot_h; yy += 6) {
            rs_fig_rect(&f, mx, (long)top + yy, 1, 3, red);
        }
    }

    /* The series. */
    long prev_x = 0, prev_y = 0;
    int have_prev = 0;
    for (size_t i = 0; i < n; i++) {
        if (!isfinite(x[i]) || !isfinite(y[i])) { have_prev = 0; continue; }
        double tx = (x[i] - x_lo) / (x_hi - x_lo);
        double ty = (y[i] - y_lo) / (y_hi - y_lo);
        if (tx < 0.0) tx = 0.0;
        if (tx > 1.0) tx = 1.0;
        if (ty < 0.0) ty = 0.0;
        if (ty > 1.0) ty = 1.0;
        const long px = (long)left + (long)(tx * (double)plot_w);
        const long py = (long)(top + plot_h) - (long)(ty * (double)plot_h);
        if (have_prev) rs_fig_line(&f, prev_x, prev_y, px, py, blue);
        prev_x = px;
        prev_y = py;
        have_prev = 1;
    }

    if (title)   rs_fig_text(&f, (long)left, (long)pad, title, ts, black);
    if (y_label) rs_fig_text(&f, (long)left, (long)pad + (title ? (long)ch + 10 : 0),
                             y_label, ts, black);
    if (x_label) {
        rs_fig_text(&f, (long)left + (long)(plot_w / 2) -
                        (long)(rs_fig_text_width(x_label, ts) / 2),
                    (long)(top + plot_h) + 9 + (long)ch + 6, x_label, ts, black);
    }

    st = rs_png_write(path, f.px, f.w, f.h, 3);
    rs_fig_free(&f);
    return st;
}

/* Write a float32 cube plus a sidecar describing its shape and axes. */
resonarsat_status_t rs_raster_write_cube(const double *data, size_t n_plane, size_t n_row,
                                         size_t n_col, const char *path,
                                         const char *axis_desc)
{
    if (!data || !path || n_plane == 0 || n_row == 0 || n_col == 0) return RS_ERR_ARG;

    FILE *f = fopen(path, "wb");
    if (!f) {
        rs_set_error("raster: cannot open %s for writing", path);
        return RS_ERR_IO;
    }

    const size_t n = n_plane * n_row * n_col;
    float *buf = malloc(n * sizeof *buf);
    if (!buf) { fclose(f); return RS_ERR_ALLOC; }
    for (size_t i = 0; i < n; i++) buf[i] = (float)data[i];

    const int ok = fwrite(buf, sizeof *buf, n, f) == n;
    free(buf);
    fclose(f);

    if (!ok) {
        rs_set_error("raster: short write to %s", path);
        return RS_ERR_IO;
    }

    char side[512];
    snprintf(side, sizeof side, "%s.hdr", path);
    FILE *h = fopen(side, "w");
    if (h) {
        fprintf(h, "format float32\nbyte_order little_endian\n");
        fprintf(h, "n_plane %zu\nn_row %zu\nn_col %zu\n", n_plane, n_row, n_col);
        fprintf(h, "layout plane-major, then row, then column\n");
        if (axis_desc) fprintf(h, "axes %s\n", axis_desc);
        fclose(h);
    }

    return RS_OK;
}
