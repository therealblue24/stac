#include "cg.h"
#include "lex.h"

static void prelude(FILE *to)
{
    fprintf(to, "data $fmt = { b \"%%llu\\n\", b 0 }\n");
    fprintf(to, "export function w $main() {\n");
    fprintf(to, "@start\n");
}

static void end(FILE *to)
{
    fprintf(to, "\n\tret 0\n}\n");
}

static int uflowcheck(int stack_index, lex_t *lex, token_t it, int min)
{
    if(stack_index < min) {
        COMP_ERR(lex->name, it.line, it.col, it.raw - it.col,
                 lex->lines.elems[it.line], it.range, "stack underflow");
        return 1;
    }
    return 0;
}

#define returnif(x)   \
    do {              \
        if((x)) {     \
            return 1; \
        }             \
    } while(0);

#define ufcheck(si, l, x, n) returnif(uflowcheck(si, l, x, n));

/* emit code to file `to` */
int cg_emit(lex_t *lex, FILE *to)
{
    prelude(to);
    (void)to;
    int stack_index = 0;
    list_foreach(&lex->toks) {
        switch(it.toktype) {
        case TOK_ADD:
            ufcheck(stack_index, lex, it, 1);
            fprintf(to, "%%s%d =l add %%s%d, %%s%d\n", stack_index - 2,
                    stack_index - 1, stack_index - 2);
            stack_index--;
            break;
        case TOK_SUB:
            ufcheck(stack_index, lex, it, 1);
            fprintf(to, "%%s%d =l sub %%s%d, %%s%d\n", stack_index - 2,
                    stack_index - 1, stack_index - 2);
            stack_index--;
            break;
        case TOK_MUL:
            ufcheck(stack_index, lex, it, 1);
            fprintf(to, "%%s%d =l mul %%s%d, %%s%d\n", stack_index - 2,
                    stack_index - 1, stack_index - 2);
            stack_index--;
            break;
        case TOK_DIV:
            ufcheck(stack_index, lex, it, 1);
            fprintf(to, "%%s%d =l div %%s%d, %%s%d\n", stack_index - 2,
                    stack_index - 1, stack_index - 2);
            stack_index--;
            break;
        case TOK_DUMP:
            ufcheck(stack_index, lex, it, 1);
            fprintf(to, "call $printf(l $fmt, ..., l %%s%d)\n",
                    stack_index - 1);
            stack_index--;
            break;
        case TOK_DUP:
            fprintf(to, "%%s%d =l copy %%s%d\n", stack_index, stack_index - 1);
            stack_index++;
            break;
        case TOK_DROP:
            ufcheck(stack_index, lex, it, 1);
            stack_index--;
            break;
        case TOK_DROPALL:
            stack_index = 0;
            break;
        case TOK_RET:
            fprintf(to, "ret %%s%d\n@thing_stack_%d", stack_index - 1,
                    stack_index);
            stack_index = 0;
            break;
        case TOK_NUM_INTU:
            fprintf(to, "%%s%d =l copy %llu\n", stack_index,
                    it.tok_num.unsignd);
            stack_index++;
            break;
        case TOK_NUM_INT:
            fprintf(to, "%%s%d =l copy %lld\n", stack_index, it.tok_num.signd);
            stack_index++;
            break;
        case TOK_SPECIAL_LIT:
            fprintf(to, "%%s%d =l call $%.*s()\n", stack_index, (int)it.range,
                    it.tokl_lit);
            stack_index++;
            break;
        default:
            COMP_ERR(lex->name, it.line, it.col, it.raw - it.col,
                     lex->lines.elems[it.line], it.range, "unsupported op");
            return 1;
            break;
        }
    }
    end(to);
    return 0;
}
