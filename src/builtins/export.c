#define _POSIX_C_SOURCE 200809L

#include <err.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "builtins.h"
#include "lexer/lexer.h"

int builtins_export(struct ast_builtin *command)
{
    if (!command->args[0])
    {
        return 0;
    }
    int i = 0;
    while (command->args[i])
    {
        char *equals = strchr(command->args[i], '=');
        if (equals)
        {
            *equals = '\0';
            char *name = command->args[i];
            if (!is_valid_name(name))
            {
                fprintf(stderr, "export: not a valid identifier '%s'\n", name);
                return 1;
            }
            char *value = equals + 1;
            if (setenv(name, value, 1) != 0)
                return 1;
            *equals = '=';
        }
        else
        {
            char *existing = getenv(command->args[i]);
            if (existing && setenv(command->args[i], existing, 1) != 0)
            {
                return 1;
            }
        }
        i++;
    }
    return 0;
}
