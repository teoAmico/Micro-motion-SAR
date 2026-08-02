/* Status code names and thread-local error message storage. */

#include "resonarsat/resonarsat.h"

#include <stdarg.h>
#include <stdio.h>

/* Per-thread error message. Sized so that a path plus a sentence fits without
 * truncation in the common case; vsnprintf truncates rather than overflowing
 * if it does not. _Thread_local is C11 and keeps this safe to call from inside
 * an OpenMP parallel region, where several threads may fail at once. */
static _Thread_local char rs_error_buf[512];

/* Stable short names for the status codes. */
const char *rs_status_str(resonarsat_status_t st)
{
    switch (st) {
    case RS_OK:               return "ok";
    case RS_ERR_ARG:          return "invalid argument";
    case RS_ERR_ALLOC:        return "out of memory";
    case RS_ERR_IO:           return "I/O error";
    case RS_ERR_FORMAT:       return "format error";
    case RS_ERR_MISSING_META: return "missing metadata";
    case RS_ERR_UNSUPPORTED:  return "unsupported";
    case RS_ERR_SINGULAR:     return "singular system";
    case RS_ERR_RANGE:        return "value out of range";
    }
    return "unknown status";
}

/* Record a descriptive message for the current thread's pending failure. */
void rs_set_error(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(rs_error_buf, sizeof rs_error_buf, fmt, ap);
    va_end(ap);
}

/* Most recent message recorded on this thread, or "" if none. */
const char *rs_last_error(void)
{
    return rs_error_buf;
}

/* Discard this thread's pending message.
 *
 * THE FAILURE THIS PREVENTS IS A MISLEADING MESSAGE, NOT A MISSING ONE. The
 * buffer is never overwritten except by rs_set_error(), so a function that
 * returns non-OK without calling it does not leave rs_report_error() with
 * nothing to print -- it leaves it with the DETAIL OF AN UNRELATED EARLIER
 * FAILURE, which is then printed in parentheses beside the new status as though
 * it belonged to it. A truncated metadata block reporting "I/O error (cphd:
 * %s is not seekable)" about a different file is worse than one reporting
 * "I/O error" alone.
 *
 * The 2026-08-02 review found 28 runtime-reachable returns with no message, most
 * of them allocation failures on paths that had already logged something
 * (docs/CODE-REVIEW.md finding 6). Rather than rely on all 28 -- and every future
 * one -- being remembered, the CLI clears the buffer before each fallible
 * operation, so an uncovered site degrades to a bare status instead of a wrong
 * sentence. Covering the sites is still better and is done where the detail is
 * worth having; this is the floor under it. */
void rs_clear_error(void)
{
    rs_error_buf[0] = '\0';
}

/* Print a failure to stderr in the CLI's standard shape. */
void rs_report_error(const char *context, resonarsat_status_t st)
{
    const char *detail = rs_last_error();
    if (detail && detail[0]) {
        fprintf(stderr, "%s: %s (%s)\n", context, rs_status_str(st), detail);
    } else {
        fprintf(stderr, "%s: %s\n", context, rs_status_str(st));
    }
}
