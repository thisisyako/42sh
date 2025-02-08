#define _POSIX_C_SOURCE 200112L
#include "ast_utils.h"

#include <err.h>
#include <stdio.h>
#include <stdlib.h>

void assign_variable(struct internal_state *global, char *str)
{
    size_t i = 0;
    while (str[i] != '=')
        ++i;
    str[i] = '\0';

    set_variable(global, str, str + i + 1);
    str[i] = '=';
}
