/*
 * Copyright (C) 2025 therealblue24.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Main lexing code for stac.
 */

#include "lex.h"
#include "util.h"
#include <ctype.h>

#define X(x) "uint" xstr(x) "_t", "int" xstr(x) "_t"

static const char *keywords[] = { "+",     "dump",   "do",       "end",
                                  "func",  "->",     "char",     "uchar",
                                  "short", "ushort", "int",      "uint",
                                  "long",  "ulong",  "str",      "ptr",
                                  "none",  "size_t", "intmax_t", "uintmax_t",
                                  X(8),    X(16),    X(32),      X(64) };

#undef X
#define X(x) TOK_UINT##x##_T, TOK_INT##x##_T

static const size_t keywordsmap[] = {
    TOK_ADD,      TOK_DUMP,      TOK_DO,    TOK_END,    TOK_FUNC, TOK_ARROW,
    TOK_CHAR,     TOK_UCHAR,     TOK_SHORT, TOK_USHORT, TOK_INT,  TOK_UINT,
    TOK_LONG,     TOK_ULONG,     TOK_STR,   TOK_PTR,    TOK_NONE, TOK_SIZE_T,
    TOK_INTMAX_T, TOK_UINTMAX_T, X(8),      X(16),      X(32),    X(64)
};
#undef X
static const size_t keyword_count = sizeof(keywords) / sizeof(keywords[0]);

static char def_name[] = "<unknown>";

/* Create a lexer. Returns NULL on failure. */
lex_t *lex_create(void)
{
    /* allocate lexer */
    lex_t *lex = zalloc(sizeof(lex_t));

    /* allocate `strlit` */
    lex->scap = 1024;
    lex->strlit = zalloc(lex->scap);

    /* allocate `split` list */
    lex->split.cap = 1024;
    lex->split.size = 0;
    lex->split.elems = zcalloc(lex->split.cap, sizeof(view_t));

    /* allocate `lines` list */
    lex->lines.cap = 1024;
    lex->lines.size = 0;
    lex->lines.elems = zcalloc(lex->lines.cap, sizeof(size_t));
    list_append(&lex->lines, -1); /* error val */

    /* don't allocate the `toks` list, we do that later */
    lex->toks.elems = NULL;
    lex->toks.size = lex->toks.cap = 0;

    /* init lexer state */

    lex->col = 0;
    lex->line = 1; /* lines are 1-indexed */
    lex->buf = NULL; /* don't have info */
    lex->name = NULL; /* don't have info */
    lex->pos = lex->len = lex->range = 0; /* don't have info */

    return lex;
}

/* Supply an input `src` into lexer `lx`.
 * Returns nonzero on failure. */
int lex_supply_src(lex_t *lex, const uint8_t *src, size_t len)
{
    if(!lex || !src) {
        return 1;
    }
    lex->buf = src;
    lex->len = len;
    return 0;
}

/* Supply an input name `name` into lexer `lx`.
 * Returns nonzero on failure. */
int lex_supply_name(lex_t *lex, const char *name)
{
    if(!lex || !name) {
        return 1;
    }
    lex->name = name;
    return 0;
}

/* Save lexing state.    ss = Store   State */
static void lex_ss(lex_t *lex)
{
    lex->ss_col = lex->col;
    lex->ss_line = lex->line;
    lex->ss_pos = lex->pos;
}

/* Restore lexing state. rs = Restore State */
static void lex_rs(lex_t *lex)
{
    lex->col = lex->ss_col;
    lex->line = lex->ss_line;
    lex->pos = lex->ss_pos;
}

/* Returns if `c` is whitespace. */
static int iswhitespace(int c)
{
    return isspace(c);
}

static int isnewline(int c)
{
    return c == '\n';
}

/* Get next character in lexer. */
static int lex_nextchar(lex_t *lex)
{
    /* If we hit EOF */
    if(lex->pos >= lex->len) {
        return EOF;
    }
    /* Increase column. */
    lex->col++;

    int c = lex->buf[lex->pos++]; /* get char */

    /* If newline, increase line and set col to zero. also
     * append line info */
    if(isnewline(c)) {
        list_append(&lex->lines, lex->col - 1);
        lex->line++;
        lex->col = 0;
    }

    return c;
}

/* Peek next character in lexer. */
static int lex_peekchar(lex_t *lex)
{
    /* If we hit EOF */
    if(lex->pos >= lex->len) {
        return EOF;
    }

    return lex->buf[lex->pos];
}

static int lex_eof(lex_t *lex)
{
    if(lex->pos >= lex->len) {
        return 1;
    } else {
        return 0;
    }
}

/* Skip until a non-whitespace character in `lex`. */
static void lex_trim_whitespace(lex_t *lex)
{
    lex_ss(lex);
    int c = ' ';
    while(iswhitespace(c) && c != EOF) {
        /* Next character, please. */
        c = lex_nextchar(lex);
        if(iswhitespace(c)) {
            lex_ss(lex);
        } else {
            lex_rs(lex);
            return;
        }
    }
    return; /* done */
}

static void lex_trim_singleline_comment(lex_t *lex)
{
    /* 1-liner in disguise */
    while(!lex_eof(lex) && lex_peekchar(lex) != '\n') {
        (void)lex_nextchar(lex);
    }
    return;
}

static void lex_trim_multiline_comment(lex_t *lex)
{
    int stack = 1;
    int c1 = lex_nextchar(lex);
    int c2 = lex_nextchar(lex);
    while(stack && !lex_eof(lex)) {
        if(c1 == '/' && c2 == '*')
            stack++;
        if(c1 == '*' && c2 == '/')
            stack--;
        c1 = c2;
        c2 = lex_nextchar(lex);
    }
    return;
}

static void lex_trim_comment(lex_t *lex, int type)
{
    if(type == '/')
        lex_trim_singleline_comment(lex);
    if(type == '*')
        lex_trim_multiline_comment(lex);
    return;
}

static bool lex_strlit_ending(int c1, int c2, int c3)
{
    const int quote = '\"';
    const int slash = '\\';
    int ending_naive = c3 == quote;
    int esc = (c2 == slash) && ending_naive && (c1 != slash);
    return ending_naive && !esc;
}

static void lex_split_by_whitespace(lex_t *lex)
{
#define RETURN_IF_EOF                               \
    do {                                            \
        if(lex_eof(lex)) {                          \
            list_append(&lex->lines, lex->col - 1); \
            return;                                 \
        }                                           \
    } while(0);
    /* While there is still data to process. */
    while(!lex_eof(lex)) {
        lex_trim_whitespace(lex);
        RETURN_IF_EOF; /* yeah nothing to split anymore */

        /* Store line start, column start, and position start. */

        size_t line_start = lex->line;
        size_t col_start = lex->col;
        size_t start = lex->pos;

        /* Check if there is a comment. */

        if(lex_peekchar(lex) == '/') {
            lex_ss(lex); /* save state */
            int c;
            (void)lex_nextchar(lex); /* dont care about the '/' */
            c = lex_nextchar(lex); /* the one we care about */
            if(c == '/' || c == '*') {
                /* found a comment! */
                lex_trim_comment(lex, c);
                continue;
            } else {
                /* too eager, go back */
                lex_rs(lex);
            }
        }

        size_t end = start;
        bool cutoff = false;

        /* Check if there is a string. */
        if(lex_peekchar(lex) == '\"') {
            /* get current, next, and next next char */
            int c1 = lex_nextchar(lex);
            int c2 = lex_nextchar(lex);
            int c3 = lex_nextchar(lex);
            end += 2;
            /* continue on until end of strlit or EOF */
            while(!lex_eof(lex) && !lex_strlit_ending(c1, c2, c3)) {
                c1 = c2;
                c2 = c3;
                c3 = lex_nextchar(lex);
                end++;
            }

            cutoff = lex_eof(lex) && !lex_strlit_ending(c1, c2, c3);
            end += !cutoff;
        }

        /* Go ahead until there is whitespace. */
        while(!lex_eof(lex) && !iswhitespace(lex_nextchar(lex))) {
            end++;
        }

        /* Ok there is whitespace now. Store line & column end. */
        size_t line_end = lex->line;
        size_t col_end = lex->col;

        view_t v; /* the view */
        /* store all info */
        v.start = start;
        v.end = end;
        v.line_start = line_start;
        v.line_end = line_end;
        v.col_start = col_start;
        v.col_end = col_end;
        v.cutoff = cutoff;

        /* qol */
        v.range = end - start; /* end >= start; this is safe */
        v.src = lex->buf + start;

        list_append(&lex->split, v);

        RETURN_IF_EOF; /* there is nothing we can do */
    }
    return;
}

/* Lex input.
 * Returns nonzero on failure. */
int lex_do(lex_t *lex)
{
    if(!lex) {
        return 1;
    }

    if(!lex->buf) {
        return 1;
    }

    if(!lex->name) {
        lex->name = def_name;
    }

    /* Spilt by whitespace; the main lexer.
     * Definitely not the reason why I decided to make
     * a stack based lang... definitely... */
    lex_split_by_whitespace(lex);

    /* For all the views we have lexed */
    for(size_t i = 0; i < lex->split.size; i++) {
        view_t v = lex->split.elems[i]; /* the view */
        if(v.cutoff) {
            COMP_ERR(lex->name, v.line_start, v.col_start, v.src,
                     lex->lines.elems[v.line_start] - v.col_start, v.range,
                     "cutoff string literal");
            return 1;
        }

        bool keyword_found = false;
        size_t keyword = 0;

        /* try to find keywords */
        for(size_t j = 0; j < keyword_count; j++) {
            if(v.range == strnlen(keywords[j], 100) &&
               strncmp((const char *)v.src, keywords[j], v.range) == 0) {
                keyword_found = true;
                keyword = j;
                break;
            }
        }

        /* found one? */
        if(keyword_found) {
            /* emit it */
            int type = keywordsmap[keyword];
            token_t tok;
            /* copy over fields */
            tok.langtype = TOKL_KEYWORD;
            tok.toktype = type;
            tok.line = v.line_start;
            tok.col = v.col_start;
            tok.line_end = v.line_end;
            tok.col_end = v.col_end;
            tok.filenam = lex->name;
            tok.raw = v.src;
            tok.range = v.range;
            /* emit the token now, nothing else to fill out */
            list_append(&lex->toks, tok);

            continue; /* next possible token, please */
        }

        /* number, literal, or string literal. */
        if(*v.src == '\"') {
            /* string literal it is! */
            token_t tok;
            /* copy over fields */
            tok.langtype = TOKL_STRLIT;
            tok.toktype = TOK_SPECIAL_STRLIT;
            tok.line = v.line_start;
            tok.col = v.col_start;
            tok.line_end = v.line_end;
            tok.col_end = v.col_end;
            tok.filenam = lex->name;
            tok.raw = v.src;
            tok.range = v.range;
            void *scratch = zalloc(v.range + 1);
            size_t sz = strl_parse(tok.raw, scratch, v.range);
            if(sz == 0) {
                COMP_ERR(lex->name, v.line_start, v.col_start, v.src,
                         lex->lines.elems[v.line_start] - v.col_start, v.range,
                         "malformed string literal");
                return 1;
            }
            tok.tokl_strlit = scratch;
            tok.tokl_strsz = sz;

            /* append the str */
            list_append(&lex->toks, tok);
            continue;
        }

        /* number */
        if(isdigit(*v.src)) {
            uint64_t num = 0;
            for(size_t k = 0; k < v.range; k++) {
                num *= 10;
                if(isdigit(v.src[k])) {
                    num += v.src[k] - '0';
                } else {
                    /* TODO: Numbered literals */
                    COMP_ERR(lex->name, v.line_start, v.col_start, v.src,
                             lex->lines.elems[v.line_start] - v.col_start,
                             v.range, "invalid numeral");
                    return 1;
                }
            }

            token_t tok;
            /* copy over fields */
            tok.langtype = TOKL_NUM;
            tok.toktype = TOK_NUM_INTU;
            tok.line = v.line_start;
            tok.col = v.col_start;
            tok.line_end = v.line_end;
            tok.col_end = v.col_end;
            tok.filenam = lex->name;
            tok.raw = v.src;
            tok.range = v.range;
            tok.tok_num.unsignd = num;
            list_append(&lex->toks, tok);

            continue; /* next possible token, please */
        }

        /* literal */
        token_t tok;
        tok.langtype = TOKL_LIT;
        tok.toktype = TOK_SPECIAL_LIT;
        tok.line = v.line_start;
        tok.col = v.col_start;
        tok.line_end = v.line_end;
        tok.col_end = v.col_end;
        tok.filenam = lex->name;
        tok.raw = v.src;
        tok.range = v.range;
        tok.tokl_lit = tok.raw;
        list_append(&lex->toks, tok);
    }

    return 0;
}

/* Delete/free a lexer. */
void lex_delete(lex_t *lex)
{
    /* According to free(3):
     * "If ptr is a NULL pointer, no operation is performed." */
    free(lex->strlit);
    lex->strlit = NULL;
    /* free all strlits */
    for(size_t i = 0; i < lex->toks.size; i++) {
        if(lex->toks.elems[i].langtype == TOKL_STRLIT) {
            free((void *)lex->toks.elems[i].tokl_strlit);
        }
    }
    free(lex->toks.elems);
    lex->toks.elems = NULL;
    free(lex->split.elems);
    lex->split.elems = NULL;
    free(lex->lines.elems);
    lex->lines.elems = NULL;
}
