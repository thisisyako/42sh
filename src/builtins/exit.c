#include <ctype.h>
#include <err.h>
#include <errno.h>
#include <stdlib.h>

#include "builtins/builtins.h"

int builtins_exit(struct ast_builtin *command)
{
    fprintf(stderr, "exit\n");
    if (command->args[0] == NULL)
    {
        exit(0);
    }
    else
    {
        size_t i = 0;
        while (command->args[0][i])
            if (!isdigit(command->args[0][i++]))
                errx(2, "'%s': numeric argument required", command->args[0]);
        int status = atoi(command->args[0]);
        exit(status);
    }
    return 0;
}
