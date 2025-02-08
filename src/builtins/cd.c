#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <limits.h>
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "builtins/builtins.h"
#include "internal_state/internal_state.h"

static int handle_cd_no_args(struct internal_state *state, char **path)
{
    *path = get_variable(state, "HOME");
    if (!*path || **path == '\0')
    {
        fprintf(stderr, "cd: HOME not set\n");
        return 1;
    }
    return 0;
}

static int handle_cd_dash(struct internal_state *state, char **path)
{
    *path = get_variable(state, "OLDPWD");
    if (!*path)
    {
        fprintf(stderr, "cd: OLDPWD not set\n");
        return 1;
    }
    printf("%s\n", *path);
    return 0;
}

static int try_cdpath(const char *path, struct internal_state *state)
{
    char cwd[PATH_MAX];
    char *cdpath = get_variable(state, "CDPATH");
    if (!cdpath || !*cdpath)
        return -1;

    char *dir = strtok(cdpath, ":");
    while (dir)
    {
        char full_path[PATH_MAX];
        if (*dir)
            snprintf(full_path, sizeof(full_path), "%s/%s", dir, path);
        else
            snprintf(full_path, sizeof(full_path), "%s", path);

        if (chdir(full_path) == 0)
        {
            if (getcwd(cwd, sizeof(cwd)) != NULL)
                printf("%s\n", cwd);
            return 0;
        }
        dir = strtok(NULL, ":");
    }
    return -1;
}

static void update_pwd_vars(struct internal_state *state, const char *oldcwd)
{
    char cwd[PATH_MAX];
    char *oldpwd = strdup(oldcwd);

    if (getcwd(cwd, sizeof(cwd)) != NULL)
    {
        set_variable(state, "PWD", cwd);
        if (oldpwd)
        {
            set_variable(state, "OLDPWD", oldpwd);
            free(oldpwd);
        }
    }
}

int cd(struct ast_builtin *command, struct internal_state *internal_state)
{
    char *path = NULL;
    int p_flag = 0;
    char cwd[PATH_MAX];

    if (getcwd(cwd, sizeof(cwd)) == NULL)
        return 1;

    if (command->args && command->args[0])
    {
        if (strcmp(command->args[0], "-P") == 0)
            p_flag = 1;
        else if (strcmp(command->args[0], "-L") == 0)
            p_flag = 0;
    }

    if (!command->args || !command->args[0])
    {
        if (handle_cd_no_args(internal_state, &path))
            return 1;
    }
    else if (command->args[0][0] == '-' && command->args[0][1] == '\0')
    {
        if (handle_cd_dash(internal_state, &path))
            return 1;
    }
    else
    {
        path = command->args[p_flag ? 1 : 0];
        if (!path)
            return 1;

        if (path[0] != '/' && path[0] != '.'
            && !(path[0] == '.' && path[1] == '.'))
        {
            if (try_cdpath(path, internal_state) == 0)
            {
                update_pwd_vars(internal_state, cwd);
                return 0;
            }
        }
    }

    if (chdir(path) != 0)
    {
        fprintf(stderr, "cd: %s: %s\n", path, strerror(errno));
        return 1;
    }

    update_pwd_vars(internal_state, cwd);
    return 0;
}
