/*
 * Copyright (C) 2025 therealblue24.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Main compiler front-end for stac.
 */

#include "lex.h"
#include "cg.h"
#include <stdio.h>

static void bar(void)
{
    printf("==========\n");
    return;
}

static void named_bar(const char *l)
{
    bar();
    puts(l);
    bar();
}

static void print_el(const char *src, view_t v)
{
    printf("%s:%zu:%zu: \"%.*s\" ", src, v.line_start, v.col_start,
           (int)v.range, v.src);
    if(v.cutoff) {
        printf("(cutoff)");
    }
    putchar('\n');
    return;
}

int main(int argc, char *argv[])
{
    (void)argc;
    FILE *in = fopen(argv[1], "rb");

    /* slurp file */

    size_t size = 0;
    fseek(in, 0, SEEK_END);
    size = (size_t)ftell(in);
    rewind(in);
    uint8_t *mem = calloc(1, size);

    fread(mem, 1, size, in);
    fclose(in);

    lex_t *l = lex_create();
    lex_supply_src(l, mem, size);
    lex_supply_name(l, argv[1]);

    if(lex_do(l)) {
        fprintf(stderr, "%s: failed to compile\n", argv[0]);
        lex_delete(l);
        free(mem);
        return 1;
    }

    named_bar("Lexing pt. 1");

    for(size_t i = 0; i < l->split.size; i++) {
        print_el(argv[1], l->split.elems[i]);
    }

    named_bar("Lexing pt. 2");

    for(size_t i = 0; i < l->toks.size; i++) {
        token_t tok = l->toks.elems[i];
        printf("%s:%zu:%zu: ", tok.filenam, tok.line, tok.col);
        switch(tok.langtype) {
        case TOKL_NUM:
            printf("(num) %llu\n", tok.tok_num.unsignd);
            break;
        case TOKL_STRLIT:
            printf("(strlit) `%s`\n", tok.tokl_strlit);
            break;
        case TOKL_KEYWORD:
            printf("(kw) %.*s\n", (int)(tok.range), tok.raw);
            break;
        case TOKL_LIT:
            printf("(lit) %.*s\n", (int)(tok.range), tok.tokl_lit);
            break;
        }
    }

    FILE *f = fopen("out.ssa", "w");
    cg_emit(l, f);

    fclose(f);

    free(mem);

    return 0;
}
