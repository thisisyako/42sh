#include "ast/ast_structs.h"
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "ast/ast.h"
#include "internal_state/internal_state.h"
#include "io_backend/io_backend.h"
#include "lexer/lexer.h"
#include "parser/parser.h"

int main(int argc, char *argv[])
{
    int return_code = 2;
    bool command_line_res = false;
    int is_interactive = 0;
    FILE *input_file;
    int mode = -1;

    // stdin
    if (argc == 1)
    {
        command_line_res = get_stdin(&input_file);
        is_interactive = isatty(STDIN_FILENO);
        mode = 1;
    }
    else if (argc == 2)
    {
        command_line_res = get_file(argv[1], &input_file);
        if (command_line_res == 0)
            errx(127, "'%s': No such file or directory", argv[1]);
        mode = 2;
    }
    else if (argc >= 3 && !strcmp(argv[1], "-c"))
    {
        command_line_res = get_string(argv[2], &input_file);
        mode = 4;
    }
    else
    {
        command_line_res = get_file(argv[1], &input_file);
        mode = 2;
    }

    if (!command_line_res)
        return 2;

    // Set up the internal state
    struct internal_state *global = internal_state_new();

    set_params(global, argc, argv, mode);

    // Evaluating.
    struct lexer *lexer = lexer_new(input_file, is_interactive);
    lexer->global = global;
    while (lexer->current_tok.type != TOKEN_EOF)
    {
        if (is_interactive)
        {
            printf("42sh$ ");
            fflush(stdout);
        }
        return_code = evaluate_input(lexer, global);
    }
    if (is_interactive)
        printf("\n");

    lexer_free(lexer);
    internal_state_free(global);

    return return_code;
}
