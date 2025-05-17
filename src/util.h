/*
 * Copyright (C) 2025 therealblue24.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Utilities header.
 */

#ifndef UTIL_H_
#define UTIL_H_

/* Includes a bunch of utils. */

#include <errno.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* stringify macro is xstr -- AA_str is just a dummy macro */
/* AA_ is used as a namespace here because there is basically
 * zero probability that you will actually namespace
 * something with that */
#define AA_str(x) x
#define xstr(x) AA_str(#x)

/* calloc(1, size) with error checking */
void *zalloc(size_t size);
/* calloc(count, size) with error checking */
void *zcalloc(size_t count, size_t size);
/* realloc(ptr, size) with error checking */
void *zrealloc(void *ptr, size_t size);
/* zrealloc but calloc-like */
void *zcrealloc(void *ptr, size_t count, size_t size);

/* Error out with debug info. */
#define ERROR(reas, ...)                                       \
    do {                                                       \
        fprintf(stderr, "ERROR: %s: %s: " reas "\n", __FILE__, \
                __func__ __VA_OPT__(, ) __VA_ARGS__);          \
        exit(EXIT_FAILURE);                                    \
    } while(0);

/* Print a warning to console. */
#define WARN(reas, ...)                                       \
    do {                                                      \
        fprintf(stderr, "WARN: %s: %s: " reas "\n", __FILE__, \
                __func__ __VA_OPT__(, ) __VA_ARGS__);         \
    } while(0);

/* assert++ */
#define ASSERT(cond, reas, ...)                                              \
    if(!(cond)) {                                                            \
        fprintf(stderr, "ASSERT: condition `" xstr(cond) "' failed: " reas   \
                                                         "\n" __VA_OPT__(, ) \
                                                             __VA_ARGS__);   \
        exit(EXIT_FAILURE);                                                  \
    }

/* todo macro */
#define TODO(...)                              \
    do {                                       \
        fprintf(stderr, "TODO: " __VA_ARGS__); \
        putchar('\n');                         \
        exit(EXIT_FAILURE);                    \
    } while(0);

/* assert but warning */
#define WASSERT(cond, reas, ...)                                          \
    if(!(cond)) {                                                         \
        fprintf(stderr,                                                   \
                "WARN ASSERT: condition `" xstr(cond) "' failed: " reas   \
                                                      "\n" __VA_OPT__(, ) \
                                                          __VA_ARGS__);   \
    }

/* Simple max. */
static inline size_t size_max(size_t a, size_t b)
{
    return (a > b) ? a : b;
}

/* meant to be used like:
 * struct something {
 *     int foo;
 *     short bar;
 *     LIST(long) stuff;
 * };
 */
#define LIST(type)        \
    struct {              \
        type *elems;      \
        size_t size, cap; \
    }

#define LIST_INITIAL_CAP (256)

/* resize a list if list cap is smaller than `sz`. */
#define list_resize(l, sz)                                                    \
    do {                                                                      \
        if((sz) >= (l)->cap) {                                                \
            while((sz) >= (l)->cap) {                                         \
                l->cap = size_max(LIST_INITIAL_CAP, (l)->cap * 2);            \
            }                                                                 \
            (l)->elems = realloc((l)->elems, (l)->cap * sizeof(*(l)->elems)); \
            ASSERT((l)->elems, "failed to reallocate %zu %zu-byte list!",     \
                   (l)->cap, sizeof(*(l)->elems));                            \
        }                                                                     \
    } while(0);

/* append element `x` to list ref `l`. */
#define list_append(l, x)              \
    do {                               \
        list_resize((l), (l)->size);   \
        (l)->elems[(l)->size++] = (x); \
    } while(0);

/* internal macro - do not use */
#define _list_append_many(l, xs, count)                                      \
    do {                                                                     \
        list_resize((l), (l)->size + (count));                               \
        memcpy(&(l)->elems[(l)->size], (xs), (count) * sizeof(*(l)->elems)); \
        (l)->size += (count);                                                \
    } while(0);

/* append many elems to list */
#define list_append_many(l, ...)                                \
    do {                                                        \
        const typeof((l)->elems[0]) li[] = { __VA_ARGS__ };     \
        _list_append_many((l), li, sizeof(li) / sizeof(li[0])); \
    } while(0);

/* Print a generic compile info/warn/err/blah. */
void print_generic(const char *label, const char *file, size_t line, size_t col,
                   const uint8_t *src, size_t range, size_t hl_range,
                   const char *msg, ...);

#define COMP_ERR(file, line, col, src, range, hl_range, ...) \
    print_generic("error", file, line, col, src, range, hl_range, __VA_ARGS__)

#define COMP_WARN(file, line, col, src, range, hl_range, ...) \
    print_generic("warning", file, line, col, src, range, hl_range, __VA_ARGS__)

#endif /* UTIL_H_ */
