#include <ctype.h>
#include <err.h>
#include <errno.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "ast/ast.h"
#include "builtins/builtins.h"
#include "internal_state/internal_state.h"

static int unset_variables(struct ast_builtin *command,
                           struct internal_state *global, size_t index)
{
    while (command->args[index])
    {
        set_variable(global, command->args[index], "");
        index++;
    }

    return 0;
}

static int unset_functions(struct ast_builtin *command,
                           struct internal_state *global, size_t index)
{
    while (command->args[index])
    {
        remove_function(global, command->args[index]);
        index++;
    }

    return 0;
}

int builtins_unset(struct ast_builtin *command, struct internal_state *global)
{
    bool v_flag = false;
    bool f_flag = false;
    size_t i = 0;

    // Parse options
    while (command->args[i] && command->args[i][0] == '-')
    {
        for (int j = 1; command->args[i][j]; j++)
        {
            switch (command->args[i][j])
            {
            case 'f':
                f_flag = true;
                break;
            case 'v':
                v_flag = true;
                break;
            default:
                fprintf(stderr, "unset: invalid option '-%c'\n",
                        command->args[i][j]);
                return 2;
            }
        }
        i++;
    }
    if (v_flag && f_flag)
    {
        // Both set, error.
        fprintf(
            stderr,
            "unset: cannot simultaneously unset a function and a variable\n");
        return 1;
    }

    // default is variable unset.
    if (!v_flag && !f_flag)
        v_flag = true;

    if (v_flag)
        return unset_variables(command, global, i);
    else // f_flag
        return unset_functions(command, global, i);
}
