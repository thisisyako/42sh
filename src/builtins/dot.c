#include "ast/ast.h"
#include "builtins.h"
#include "io_backend/io_backend.h"
#include "parser/parser.h"
#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <limits.h>
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static char *resolve_path(const char *filename)
{
    char *path = malloc((strlen(filename) + 1) * sizeof(char));
    if (strchr(filename, '/'))
        return strcpy(path, filename);

    char *path_env = getenv("PATH");
    if (!path_env)
        return path;

    char *path_copy = malloc((strlen(path_env) + 1) * sizeof(char));
    strcpy(path_copy, path_env);
    char *dir = strtok(path_copy, ":");
    while (dir)
    {
        free(path);
        path = malloc(strlen(dir) + strlen(filename) + 2);
        sprintf(path, "%s/%s", dir, filename);
        if (access(path, R_OK) == 0)
            break;
        dir = strtok(NULL, ":");
    }
    free(path_copy);
    return path;
}

static int execute_script(FILE *file, struct ast_builtin *command)
{
    struct internal_state *global = internal_state_new();
    struct lexer *lexer = lexer_new(file, 0);

    size_t n = 0;
    while (command->args[n])
        n++;

    char **_argv = calloc(n + 2, sizeof(char *));
    _argv[0] = command->command_name;
    for (size_t i = 0; i < n; ++i)
        _argv[i + 1] = command->args[i];

    set_params(global, n + 1, _argv, 2);
    int status = 0;
    while (lexer->current_tok.type != TOKEN_EOF)
        status = evaluate_input(lexer, global);

    free(_argv);
    lexer_free(lexer);
    internal_state_free(global);
    return status;
}

int builtins_dot(struct ast_builtin *command)
{
    if (!command->args || !command->args[0])
    {
        fprintf(stderr, ".: usage: . filename\n");
        return 1;
    }

    char *path = resolve_path(command->args[0]);
    if (!path || access(path, R_OK) != 0)
    {
        fprintf(stderr, ".: %s: file not found\n", command->args[0]);
        free(path);
        return 1;
    }

    FILE *file = fopen(path, "r");
    if (!file)
    {
        fprintf(stderr, ".: cannot open %s\n", command->args[0]);
        free(path);
        return 1;
    }

    int status = execute_script(file, command);
    fclose(file);
    free(path);
    return status;
}
