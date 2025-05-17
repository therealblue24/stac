/*
 * Copyright (C) 2025 therealblue24.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * A bunch of utilities.
 */

#include "util.h"

/* calloc(1, size) with error checking */
void *zalloc(size_t size)
{
    void *mem = malloc(size);
    ASSERT(mem, "failed to allocate %zu bytes of memory!", size);
    memset(mem, 0, size);
    return mem;
}

/* calloc(count, size) with error checking */
void *zcalloc(size_t count, size_t size)
{
    void *mem = calloc(count, size);
    ASSERT(mem, "failed to allocate %zux%zu bytes (%zu bytes) of memory!",
           count, size, count * size);
    return mem;
}

/* realloc(ptr, size) with error checking */
void *zrealloc(void *ptr, size_t size)
{
    void *newptr = realloc(ptr, size);
    ASSERT(newptr, "failed to reallocate %zu bytes of memory!", size);
    return newptr;
}

/* zrealloc but calloc-like */
void *zcrealloc(void *ptr, size_t count, size_t size)
{
    void *newptr = realloc(ptr, count * size);
    ASSERT(newptr, "failed to reallocate %zux%zu bytes (%zu bytes) of memory!",
           count, size, count * size);
    return newptr;
}

void print_generic(const char *label, const char *file, size_t line, size_t col,
                   const uint8_t *src, size_t range, size_t hl_range,
                   const char *msg, ...)
{
    fprintf(stderr, "%s:%zu:%zu: %s: ", file, line, col, label);

    va_list ap;
    va_start(ap, msg);
    vfprintf(stderr, msg, ap);
    va_end(ap);

    fputc('\n', stderr);

    fprintf(stderr, " %zu | %.*s\n ", line, (int)range, src);
    size_t lline = line;
    while(lline) {
        lline /= 10;
        fputc(' ', stderr);
    }
    fprintf(stderr, " | ");
    size_t lcol = col;
    while(lcol) {
        fputc(' ', stderr);
        lcol--;
    }
    size_t h = hl_range - 1;
    fputc('^', stderr);
    while(h) {
        fputc('~', stderr);
        h--;
    }
    fputc('\n', stderr);
    return;
}

void print_generic_add(const char *label, const char *file, size_t line,
                       size_t col, const char *src, size_t range,
                       size_t hl_begin, size_t hl_range, const char *msg, ...)
{
    fprintf(stderr, "%s:%zu:%zu: %s: ", file, line, col, label);

    va_list ap;
    va_start(ap, msg);
    vfprintf(stderr, msg, ap);
    va_end(ap);

    fputc('\n', stderr);

    fprintf(stderr, " %zu | %.*s\n ", line, (int)range, src);
    size_t lline = line;
    while(lline) {
        lline /= 10;
        fputc(' ', stderr);
    }
    fprintf(stderr, " | ");
    size_t lcol = hl_begin;
    while(lcol) {
        fputc(' ', stderr);
        lcol--;
    }
    size_t h = hl_range;
    while(h) {
        fputc('+', stderr);
        h--;
    }
    fputc('\n', stderr);
    return;
}
