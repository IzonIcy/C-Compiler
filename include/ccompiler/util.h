#ifndef CCOMPILER_UTIL_H
#define CCOMPILER_UTIL_H

#include <stddef.h>

void *cc_reallocate_or_die(void *memory, size_t size);
char *cc_duplicate_string(const char *text);
char *cc_duplicate_range(const char *text, size_t length);
char *cc_format_string(const char *format, ...);

#endif
