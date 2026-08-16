#ifndef CCOMPILER_UTIL_H
#define CCOMPILER_UTIL_H

#include <stdbool.h>
#include <stddef.h>

#include "ccompiler/lexer.h"

void *cc_reallocate_or_die(void *memory, size_t size);
char *cc_duplicate_string(const char *text);
char *cc_duplicate_range(const char *text, size_t length);
char *cc_format_string(const char *format, ...);

/* Cap on diagnostics per stage. Keep below 256: diagnostic buffers double
 * from 8 or 16 slots, so capacity always exceeds the cap when it is hit. */
#define CC_MAX_DIAGNOSTICS 200

/* Returns true if another diagnostic may be recorded. The first call past
 * the cap records a single overflow notice; later calls are dropped.
 * Guards against unbounded memory growth from error cascades on
 * pathological input (e.g. deeply nested or non-advancing parse errors). */
bool cc_diagnostic_buffer_can_add(CCDiagnosticBuffer *buffer, CCDiagnosticSeverity severity);

#endif
