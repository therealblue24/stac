/*
 * Copyright (C) 2025 therealblue24.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Header file for lexing stac code.
 */

#ifndef LEX_H_
#define LEX_H_

#include "util.h"
#include "strl.h"
#include <ctype.h>
#include <stdlib.h>

#define X(x) TOK_UINT##x##_T, TOK_INT##x##_T

/* Contains token types */
enum toktype {
    TOKL_KEYWORD = 0, /* keyword */
    TOKL_LIT = 1, /* literal */
    TOKL_STRLIT = 2, /* str literal */
    TOKL_NUM = 3, /* number */

    TOK__START, /* token start */

    /* -- begin actual tokens --- */
    TOK_FUNC /* func */,
    TOK_ARROW, /* -> */
    TOK_DO, /* do */
    TOK_THEN, /* do but for if loops */
    TOK_END, /* end */

    /* -- types -- */
    TOK_CHAR, /* char */
    TOK_UCHAR, /* unsigned char */
    TOK_SHORT, /* short */
    TOK_USHORT, /* unsigned short */
    TOK_INT, /* int */
    TOK_UINT, /* unsigned int */
    /* long here is 64-bit.  windows users can use WSL */
    TOK_LONG, /* long */
    TOK_ULONG, /* unsigned long */

    TOK_STR, /* void* but fancy */
    TOK_PTR, /* void* */
    TOK_NONE, /* void */
    TOK_SIZE_T, /* size_t */
    TOK_INTMAX_T, /* intmax_t */
    TOK_UINTMAX_T, /* uintmax_t */

    /* -- *int*_t -- */

    X(8),
    X(16),
    X(32),
    X(64),

    TOK_ADD, /* addition */
    TOK_DUMP, /* dump to stdout */

    /* -- number token -- */
    TOK_NUM_INT, /* int64_t  */
    TOK_NUM_INTU, /* uint64_t */
    TOK_NUM_FLT, /* float    */
    TOK_NUM_FLTD, /* double   */

    /* -- type-tokens -- */

    TOK_SPECIAL_STRLIT, /* special token for string literals */
    TOK_SPECIAL_LIT, /* special token for normal literals */

    TOK__COUNTR,
    TOK_COUNT = (TOK__COUNTR - TOK__START) - 1, /* # of tokens */
};

#undef X

/* Token. */
typedef struct token {
    /* langtype is syntax type (TOKL_*).
     * toktype is actual token type (TOK_*). */
    uint32_t langtype, toktype;

    /* raw token text */
    const uint8_t *raw;

    /* compiler diagnostics */
    const char *filenam; /* file name */
    size_t line, line_end; /* file line */
    size_t col, col_end; /* file column */
    size_t range; /* range of raw */

    /* lang/toktype-specific stuff */

    union { /* -- langtype -- */
        /* TOKL_KEYWORD -- nothing */
        /* TOKL_LIT : literal */
        const uint8_t *tokl_lit;
        /* TOKL_STRLIT : string literal */
        struct {
            const uint8_t *tokl_strlit;
            size_t tokl_strsz;
        };
        /* TOKL_NUM -- nothing */
    };

    union { /* -- toktype -- */
        union {
            int64_t signd; /* TOK_NUM_INT: signed 64-bit int */
            uint64_t unsignd; /* TOK_NUM_INTU: unsigned 64-bit int */
            float flt; /* TOK_NUM_FLT: single precision float */
            double fltd; /* TOK_NUM_FLTD: double precision float */
        } tok_num;
        /* TOK_ADD -- nothing */
        /* TOK_DUMP -- nothing */
    };
} token_t;

typedef struct view {
    /* When it starts and ends in `buf`.
     * `range` is `end` - `start`. */
    size_t start, end, range;
    /* Shorthand for `buf + start`. */
    const uint8_t *src;

    /* -- file diagonstics -- */

    /* start line, end line */
    size_t line_start, line_end;
    /* start column, end column */
    size_t col_start, col_end;
    /* cutoff? */
    bool cutoff;
} view_t;

typedef struct lex {
    /* -- general -- */

    const uint8_t *buf; /* input to lex */
    const char *name; /* name of input */
    /* pos -- progress in `buf`;
     * range -- lexing internal used for lexing;
     * len -- size of `buf`. */
    size_t pos, range, len;

    /* tokens that we have lexed */
    LIST(token_t) toks;

    /* -- internal lexer -- */

    /* Used to split the source by whitespace into a list of text */
    LIST(view_t) split;

    /* Used to determine size of lines. */
    LIST(size_t) lines;

    /* Line & column for the splitting */
    size_t line, col;

    /* Save states */
    size_t ss_pos, ss_line, ss_col;

    /* String literal lexing.
     * Used to parse "This \"stuff\" \n" into `This "stuff" <\n>`.  */
    uint8_t *strlit;
    size_t ssize, scap; /* ssize -> string size, scap -> strlit alloc'd size */
} lex_t;

/* Create a lexer. Returns NULL on failure. */
lex_t *lex_create(void);

/* Supply an input `src` with length `len` into lexer `lex`.
 * Returns nonzero on failure. */
int lex_supply_src(lex_t *lex, const uint8_t *src, size_t len);

/* Supply an input name `name` into lexer `lex`.
 * Returns nonzero on failure. */
int lex_supply_name(lex_t *lex, const char *name);

/* Lex input.
 * Returns nonzero on failure. */
int lex_do(lex_t *lex);

/* Delete/free a lexer. */
void lex_delete(lex_t *lex);

#endif /* LEX_H_ */
