#define _POSIX_C_SOURCE 200809L
#include "io_backend.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ast/ast.h"
#include "ast/free_ast.h"
#include "ast/pretty_print.h"
#include "lexer/lexer.h"
#include "parser/parser.h"

bool get_file(const char *file_name, FILE **file)
{
    *file = fopen(file_name, "r");
    return *file != NULL;
}

bool get_string(const char *string, FILE **file)
{
    size_t len = strlen(string);
    *file = fmemopen((void *)string, len, "r");
    return *file != NULL;
}

bool get_stdin(FILE **file)
{
    *file = stdin;
    return true;
}

int evaluate_input(struct lexer *lexer, struct internal_state *global)
{
    int return_code;

    struct ast *ast = parse_input(lexer);

    // pretty_print(ast);
    return_code = evaluate_ast(ast, global);
    free_ast(ast);

    return return_code;
}
