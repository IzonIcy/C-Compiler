#include "ccompiler/util.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void *cc_reallocate_or_die(void *memory, size_t size) {
    void *result;

    if (size == 0) {
        size = 1;
    }

    result = realloc(memory, size);
    if (result == NULL) {
        fprintf(stderr, "fatal: out of memory\n");
        exit(EXIT_FAILURE);
    }

    return result;
}

char *cc_duplicate_string(const char *text) {
    size_t length;
    char *copy;

    length = strlen(text) + 1;
    copy = cc_reallocate_or_die(NULL, length);
    memcpy(copy, text, length);
    return copy;
}

char *cc_duplicate_range(const char *text, size_t length) {
    char *copy;

    copy = cc_reallocate_or_die(NULL, length + 1);
    memcpy(copy, text, length);
    copy[length] = '\0';
    return copy;
}

char *cc_format_string(const char *format, ...) {
    char stack_buffer[256];
    char *heap_buffer;
    int needed;
    va_list args;
    va_list copy;

    va_start(args, format);
    va_copy(copy, args);
    needed = vsnprintf(stack_buffer, sizeof(stack_buffer), format, args);
    va_end(args);

    if (needed < 0) {
        va_end(copy);
        return cc_duplicate_string("<format_error>");
    }

    if ((size_t)needed < sizeof(stack_buffer)) {
        va_end(copy);
        return cc_duplicate_string(stack_buffer);
    }

    heap_buffer = cc_reallocate_or_die(NULL, (size_t)needed + 1);
    vsnprintf(heap_buffer, (size_t)needed + 1, format, copy);
    va_end(copy);
    return heap_buffer;
}
