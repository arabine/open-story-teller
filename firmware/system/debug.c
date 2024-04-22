#include <stdlib.h>

#include "debug.h"
#include "ost_hal.h"

#define NANOPRINTF_USE_FIELD_WIDTH_FORMAT_SPECIFIERS 1
#define NANOPRINTF_USE_PRECISION_FORMAT_SPECIFIERS 1
#define NANOPRINTF_USE_LARGE_FORMAT_SPECIFIERS 0
#define NANOPRINTF_USE_FLOAT_FORMAT_SPECIFIERS 1
#define NANOPRINTF_USE_BINARY_FORMAT_SPECIFIERS 0
#define NANOPRINTF_USE_WRITEBACK_FORMAT_SPECIFIERS 0

// Compile nanoprintf in this translation unit.
#define NANOPRINTF_IMPLEMENTATION
#include "nanoprintf.h"

static void putc_wrapper(int c, void *ctx)
{
    (void) ctx;
    system_putc(c);
}

void debug_printf(char const *fmt, ...)
{
    va_list val;
    va_start(val, fmt);
    (void) npf_vpprintf(putc_wrapper, NULL, fmt, val);
    va_end(val);
}

