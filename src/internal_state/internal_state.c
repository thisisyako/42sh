#define _POSIX_C_SOURCE 200809L

#include "internal_state.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const struct internal_state *g_global = NULL;

static int is_env_var(char *key)
{
    char *env_variables[] = { "PWD", "OLD_PWD" };
    for (size_t i = 0; i < sizeof(env_variables) / sizeof(char *); i++)
    {
        if (!strcmp(env_variables[i], key))
            return 1;
    }

    return 0;
}

struct internal_state *internal_state_new(void)
{
    struct internal_state *global = malloc(sizeof(struct internal_state));
    global->param_number = 0;
    global->return_code = 0;
    global->positional_params = NULL;
    global->variables = dict_create();
    global->functions = dict_create();
    g_global = global;
    return global;
}

struct internal_state *get_internal_state(void)
{
    return (struct internal_state *)g_global;
}

void internal_state_free(struct internal_state *global)
{
    if (global->positional_params)
    {
        for (int i = 0; global->positional_params[i]; ++i)
        {
            free(global->positional_params[i]);
        }
        free(global->positional_params);
    }
    dict_free(global->variables);
    dict_function_free(global->functions);
    free(global);
}

void set_return_code(int return_code, struct internal_state *global)
{
    if (global)
        global->return_code = return_code;
}

void set_params(struct internal_state *global, int argc, char *argv[], int mode)
{
    global->param_number = argc - mode;
    if (global->param_number >= 0)
    {
        size_t count = 0;
        int i = mode - 1;
        while (i < argc)
        {
            global->positional_params = realloc(global->positional_params,
                                                (count + 3) * sizeof(char *));
            char *str = strdup(argv[i]);
            str[strlen(str)] = '\0';
            global->positional_params[count] = str;
            ++i;
            ++count;
        }

        global->positional_params[count] = NULL;
    }
}

void set_variable(struct internal_state *global, char *key, char *value)
{
    // In case PWD is not set. We want to set it into env.
    if (getenv(key) == NULL && !is_env_var(key))
        dict_set(global->variables, key, value);
    else
        setenv(key, value, 1);
}

void set_function(struct internal_state *global, char *name,
                  struct ast_shell_command *function)
{
    dict_function_set(global->functions, name, function);
}

void remove_function(struct internal_state *global, char *name)
{
    dict_function_remove(global->functions, name);
}

char *get_variable(struct internal_state *global, char *key)
{
    char *res_maybe = getenv(key);
    if (res_maybe == NULL)
        return dict_get(global->variables, key);
    else
        return res_maybe;
}

struct ast_shell_command *get_function(struct internal_state *global,
                                       char *name)
{
    return dict_get(global->functions, name);
}
