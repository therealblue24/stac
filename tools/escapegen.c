/*
 * Copyright (C) 2025 therealblue24.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Generator for escape character tables
 */
#include <stddef.h>
#include <stdio.h>
#include <stdint.h>

int one(uint8_t byte)
{
    /* ref: https://en.wikipedia.org/wiki/Escape_sequences_in_C */
    switch(byte) {
    /* normal escape codes */
    case 'a':
        return 0x07;
    case 'b':
        return 0x08;
    case 'e':
        return 0x1b;
    case 'f':
        return 0x0c;
    case 'n':
        return 0x0a;
    case 'r':
        return 0x0d;
    case 't':
        return 0x09;
    case 'v':
        return 0x0b;
    case '\\':
        return 0x5c;
    case '\'':
        return 0x27;
    case '\"':
        return 0x22;
    case '?':
        return 0x3f;
    /* special ones */
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
        return -1;
    case 'x':
        return -2;
        /* no unicode support yet */
    default:
        return (int)byte;
    }
}

int main(void)
{
    printf("static int escapetab[256] = { ");
    for(int i = 0; i < 255; i++) {
        printf("%d, ", one(i));
    }
    printf("%d };", one(255));
    return 0;
}
