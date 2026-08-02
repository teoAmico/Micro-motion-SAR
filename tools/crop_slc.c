/* Cut a small patch out of a large scene.
 *
 * All development happens on patches of a couple of thousand pixels centred on
 * the structure of interest, never on multi-gigabyte scenes: backprojection and
 * per-window tracking both scale with area, and a full scene turns a two-minute
 * experiment into an overnight one. */

#include "resonarsat/readers.h"
#include "resonarsat/raster.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Command-line entry point. */
int main(int argc, char **argv)
{
    if (argc < 7) {
        printf("usage: crop_slc SLC ANN AZ0 RG0 SIZE OUT.pgm\n");
        return 1;
    }

    rs_slc_t src;
    resonarsat_status_t st = rs_read_uavsar(argv[1], argv[2], &src);
    if (st != RS_OK) { rs_report_error("crop_slc", st); return 1; }

    const size_t az0 = (size_t)atol(argv[3]);
    const size_t rg0 = (size_t)atol(argv[4]);
    const size_t size = (size_t)atol(argv[5]);

    rs_slc_t dst;
    st = rs_slc_crop(&src, az0, rg0, size, size, &dst);
    if (st != RS_OK) { rs_report_error("crop_slc", st); rs_slc_free(&src); return 1; }

    st = rs_raster_write_quicklook(&dst, argv[6], 40.0);
    if (st != RS_OK) rs_report_error("crop_slc", st);
    else printf("wrote %s: %zux%zu\n", argv[6], dst.n_az, dst.n_rg);

    rs_slc_free(&dst);
    rs_slc_free(&src);
    return st == RS_OK ? 0 : 1;
}
