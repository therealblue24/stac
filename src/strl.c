/*
 * Copyright (C) 2025 therealblue24.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * String literal parsing
 */
#include "strl.h"

/* Generated from `tools/escapetab.c`.
 * -1 = parse octal, -2 = parse hex   */
static const int escapetab[256] = {
    0,   1,   2,   3,   4,   5,   6,   7,   8,   9,   10,  11,  12,  13,  14,
    15,  16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,
    30,  31,  32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,
    45,  46,  47,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  56,  57,  58,  59,
    60,  61,  62,  63,  64,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,
    75,  76,  77,  78,  79,  80,  81,  82,  83,  84,  85,  86,  87,  88,  89,
    90,  91,  92,  93,  94,  95,  96,  7,   8,   99,  100, 27,  12,  103, 104,
    105, 106, 107, 108, 109, 10,  111, 112, 113, 13,  115, 9,   117, 11,  119,
    -2,  121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134,
    135, 136, 137, 138, 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149,
    150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 160, 161, 162, 163, 164,
    165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 176, 177, 178, 179,
    180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 192, 193, 194,
    195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207, 208, 209,
    210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223, 224,
    225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239,
    240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254,
    255
};

/* Parses a string literal `src` into an actual string `dst`,
 * given size of the string literal `size`. Assumes that
 * dst is sized enough to fit the output of `src`.
 * Returns zero on failure, else size of `dst`. */
size_t strl_parse(const uint8_t *src, uint8_t *dst, size_t size)
{
    size_t j = 0;
    size_t i = 0;
    /* skip beginning " */
    if(*src == '\"') {
        i++;
    }

    while(i < size) {
        /* don't want to emit the extra closing " */
        if(i == (size - 1) && src[i] == '\"')
            break;

        /* If we don't encounter an escape sequence yet */
        if(src[i] != '\\') {
            dst[j++] = src[i++];
            continue; /* go on */
        }

        /* should not be possible as lexer would also trip up */
        if(i == (size - 1)) {
            return 0;
        }

        i++; /* get the actual char */
        int d = escapetab[src[i]]; /* get its value */
        /* if we dont have to parse number */
        if(d >= 0) {
            /* emit it, continue on */
            dst[j++] = d;
            i++;
            continue;
        }

        /* else we have to parse the number */
        TODO("Number parsing in string literals");
    }

    return j;
}
