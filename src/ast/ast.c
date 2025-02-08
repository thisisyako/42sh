#include "ast_structs.h"
#define _POSIX_C_SOURCE 200809L

#include <assert.h>
#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <wait.h>

#include "ast/ast.h"
#include "ast/ast_utils.h"
#include "expansion/expansion.h"

static int evaluate_list(struct ast_list *node, struct internal_state *global);
static int evaluate_shell_command(struct ast_shell_command *node,
                                  struct internal_state *global);

static int evaluate_sys_command(struct ast_builtin *node)
{
    size_t n = 0;
    while (node->args[n])
        n++;
    char **args = calloc(n + 2, sizeof(char *));
    args[0] = node->command_name;
    for (size_t i = 0; i < n; ++i)
        args[i + 1] = node->args[i];
    int id = fork();
    if (id)
    {
        int status;
        waitpid(id, &status, 0);
        if (WIFEXITED(status))
        {
            free(args);
            return WEXITSTATUS(status);
        }
        return 127;
    }
    else
    {
        if (execvp(node->command_name, args) == -1)
        {
            fprintf(stderr, "%s: command not found\n", node->command_name);
            _exit(127);
        }
    }
    return EXIT_SUCCESS;
}

static int expand_builtin(struct ast_builtin *node,
                          struct internal_state *global)
{
    if (!node->args)
        return 1;
    // Expansion for command name done during parsing.
    node->args = expand_args(node->args, global);

    return 1;
}

static char **save_builtin_args(struct ast_builtin *node)
{
    if (!node || !node->args)
        return NULL;

    size_t count = 0;
    while (node->args[count])
        count++;

    char **args_copy = malloc((count + 1) * sizeof(char *));
    if (!args_copy)
        return NULL;

    for (size_t i = 0; i < count; i++)
    {
        args_copy[i] = strdup(node->args[i]);
        if (!args_copy[i])
        {
            for (size_t j = 0; j < i; j++)
                free(args_copy[j]);
            free(args_copy);
            return NULL;
        }
    }
    args_copy[count] = NULL;

    return args_copy;
}

static void restore_args(struct ast_builtin *node, char **args_save)
{
    if (!node || !args_save)
        return;

    if (node->args)
    {
        for (size_t i = 0; node->args[i]; i++)
            free(node->args[i]);
        free(node->args);
    }

    node->args = args_save;
}

static char **save_positional_params(struct internal_state *global)
{
    if (!global->positional_params)
        return NULL;

    size_t i = 0;
    while (global->positional_params[i])
        ++i;
    char **res = calloc(i + 1, sizeof(char *));
    assert(res != NULL && "failed calloc");
    i = 0;
    while (global->positional_params[i])
    {
        res[i] = strdup(global->positional_params[i]);
        free(global->positional_params[i]);
        ++i;
    }
    free(global->positional_params);

    res[i] = NULL;
    return res;
}

static void change_positional_params(struct internal_state *global,
                                     struct ast_builtin *node)
{
    size_t i = 0;
    while (node->args[i])
        ++i;
    char **new_pos_params = calloc(i + 2, sizeof(char *));
    assert(new_pos_params != NULL && "failed calloc");
    i = 0;
    while (node->args[i])
    {
        new_pos_params[i + 1] = strdup(node->args[i]);
        ++i;
    }

    new_pos_params[i + 1] = NULL;
    new_pos_params[0] = strdup("42sh");
    global->param_number = i;
    global->positional_params = new_pos_params;
}

static void restore_positional_params(struct internal_state *global,
                                      char **params)
{
    size_t i = 0;
    while (global->positional_params[i])
        free(global->positional_params[i++]);
    free(global->positional_params);
    global->positional_params = params;
    global->param_number = 0;
    if (!global->positional_params)
        return;
    i = 0;
    while (global->positional_params[i])
        ++i;
    global->param_number = i - 1;
}

static int evaluate_loop_continue(struct ast_builtin *node, int is_continue)
{
    node->base.loop_continue = 1;
    if (node->args[0])
    {
        int itr_nb = atoi(node->args[0]);
        if (itr_nb <= 0)
            errx(2, "NaN after break or continue\n");
        node->base.loop_continue = itr_nb;
    }
    if (!is_continue)
        node->base.loop_continue = -node->base.loop_continue;
    return 0;
}

static void evaluate_builtin_handler1(struct ast_builtin *node, int *ret_code,
                                      struct internal_state *global)
{
    switch (node->type)
    {
    case ECHO:
        *ret_code = echo(node);
        break;
    case DOT:
        *ret_code = builtins_dot(node);
        break;
    case BREAK:
        *ret_code = evaluate_loop_continue(node, 0);
        break;
    case CONTINUE:
        *ret_code = evaluate_loop_continue(node, 1);
        break;
    case EXPORT:
        *ret_code = builtins_export(node);
        break;
    case EXIT:
        *ret_code = builtins_exit(node);
        break;
    case CD:
        *ret_code = cd(node, global);
        break;
    case UNSET:
        *ret_code = builtins_unset(node, global);
        break;
    default:
        *ret_code = 1;
        break;
    }
}

static int evaluate_builtin(struct ast_builtin *node,
                            struct internal_state *global)
{
    char **args_save = save_builtin_args(node);
    char **pos_params_save;
    expand_builtin(node, global);
    int ret_code = 0;
    struct ast_shell_command *f;
    switch (node->type)
    {
    case SYS_COMMAND:
        f = get_function(global, node->command_name);
        if (f != NULL)
        {
            pos_params_save = save_positional_params(global);
            change_positional_params(global, node);
            ret_code = evaluate_shell_command(f, global);
            restore_positional_params(global, pos_params_save);
        }
        else
            ret_code = evaluate_sys_command(node);
        break;
    case TRUE:
        ret_code = 0;
        break;
    case FALSE:
        ret_code = 1;
        break;
    default:
        evaluate_builtin_handler1(node, &ret_code, global);
        break;
    }
    restore_args(node, args_save);

    set_return_code(ret_code, global);
    return ret_code;
}

static int evaluate_redirection(struct ast_redirection *node)
{
    if (!node || !node->type || !node->word)
        return 1;

    int flags = 0;
    int fd = node->ionumber;
    mode_t mode = 0644;
    int newfd;

    if (strcmp(node->type, ">") == 0 || strcmp(node->type, ">|") == 0)
    {
        flags = O_WRONLY | O_CREAT | O_TRUNC;
        newfd = open(node->word, flags, mode);
    }
    else if (strcmp(node->type, ">>") == 0)
    {
        flags = O_WRONLY | O_CREAT | O_APPEND;
        newfd = open(node->word, flags, mode);
    }
    else if (strcmp(node->type, "<") == 0)
    {
        flags = O_RDONLY;
        newfd = open(node->word, flags, mode);
    }
    else if (strcmp(node->type, ">&") == 0)
    {
        newfd = atoi(node->word);
        if (newfd == 0 && errno == EINVAL)
            errx(2, "NaN after redirection\n");
    }
    else if (strcmp(node->type, "<&") == 0)
    {
        newfd = atoi(node->word);
        if (newfd == 0 && errno == EINVAL)
            errx(2, "NaN after redirection\n");
    }
    else
        return 1;

    if (newfd == -1)
        errx(1, "redirection error");

    // Perform the redirection
    if (dup2(newfd, fd) == -1)
        errx(1, "dup2 error");

    // Close the new file descriptor in all cases parce que berry t con
    close(newfd);

    return 0;
}

static int evaluate_prefix(struct ast_prefix *node,
                           struct internal_state *global)
{
    if (node->assignment)
    {
        char *before_expansion = malloc(strlen(node->assignment) + 1);
        strcpy(before_expansion, node->assignment);
        node->assignment = expand(node->assignment, global);
        assign_variable(global, node->assignment);
        free(node->assignment);
        node->assignment = before_expansion;
    }
    if (node->redir)
        evaluate_redirection((struct ast_redirection *)node->redir);
    return 0;
}

static int jump(struct ast *node, struct ast *child, int res)
{
    if (child->loop_continue)
    {
        node->loop_continue = child->loop_continue;
        return 0;
    }
    return res;
}

static int evaluate_simple_command(struct ast_simple_command *node,
                                   struct internal_state *global)
{
    for (size_t i = 0; node->prefs[i]; ++i)
        evaluate_prefix((struct ast_prefix *)node->prefs[i], global);
    for (size_t i = 0; node->redirs[i]; ++i)
        evaluate_redirection((struct ast_redirection *)node->redirs[i]);
    if (node->builtin)
    {
        int res = evaluate_builtin((struct ast_builtin *)node->builtin, global);
        return jump((struct ast *)node, node->builtin, res);
    }
    return 0;
}

static int evaluate_rule_if(struct ast_rule_if *node,
                            struct internal_state *global);
static int evaluate_list(struct ast_list *node, struct internal_state *global);

static int evaluate_else_clause(struct ast_else_clause *node,
                                struct internal_state *global)
{
    if (node->elif)
    {
        int res = evaluate_rule_if((struct ast_rule_if *)node->elif, global);
        return jump((struct ast *)node, node->elif, res);
    }
    int res = evaluate_list((struct ast_list *)node->otherwise, global);
    return jump((struct ast *)node, node->otherwise, res);
}

static int evaluate_rule_if(struct ast_rule_if *node,
                            struct internal_state *global)
{
    if (evaluate_list((struct ast_list *)node->cond, global) == 0)
    {
        int res = evaluate_list((struct ast_list *)node->then, global);
        return jump((struct ast *)node, node->then, res);
    }
    else if (node->otherwise)
    {
        return evaluate_else_clause((struct ast_else_clause *)node->otherwise,
                                    global);
    }
    return 0;
}

static int evaluate_rule_while(struct ast_rule_while *node,
                               struct internal_state *global)
{
    int res = 0;
    while (evaluate_list((struct ast_list *)node->cond, global) == 0)
    {
        res = evaluate_list((struct ast_list *)node->cmd, global);
        res = jump((struct ast *)node, node->cmd, res);
        if (node->base.loop_continue > 0)
            node->base.loop_continue--;
        else if (node->base.loop_continue < 0)
        {
            node->base.loop_continue++;
            break;
        }
    }
    return res;
}

static int evaluate_rule_until(struct ast_rule_until *node,
                               struct internal_state *global)
{
    int res = 0;
    while (evaluate_list((struct ast_list *)node->cond, global) != 0)
    {
        res = evaluate_list((struct ast_list *)node->cmd, global);
        res = jump((struct ast *)node, node->cmd, res);
        if (node->base.loop_continue > 0)
            node->base.loop_continue--;
        else if (node->base.loop_continue < 0)
        {
            node->base.loop_continue++;
            break;
        }
    }
    return res;
}

static int evaluate_rule_case(struct ast_rule_case *node,
                              struct internal_state *global)
{
    node->enumerator = expand(node->enumerator, global);
    for (size_t i = 0; node->items[i]; ++i)
    {
        struct ast_case_item *item = (struct ast_case_item *)node->items[i];
        for (size_t j = 0; item->item[j]; ++j)
        {
            item->item[j] = expand(item->item[j], global);
            if (!strcmp(item->item[j], "*")
                || !strcmp(item->item[j], node->enumerator))
            {
                if (item->list)
                    return evaluate_list((struct ast_list *)item->list, global);
                else
                    return 0;
            }
        }
    }
    return 0;
}

static int evaluate_rule_for(struct ast_rule_for *node,
                             struct internal_state *global)
{
    node->list = expand_args(node->list, global);
    for (size_t i = 0; node->list[i]; ++i)
    {
        set_variable(global, node->enumerator, node->list[i]);
        int res = evaluate_list((struct ast_list *)node->cmd, global);
        res = jump((struct ast *)node, node->cmd, res);
        if (node->base.loop_continue > 0)
            node->base.loop_continue--;
        else if (node->base.loop_continue < 0)
        {
            node->base.loop_continue++;
            break;
        }
        if (res != 0)
            return res;
    }
    return 0;
}

static int evaluate_function(struct ast_function *node,
                             struct internal_state *global)
{
    set_function(global, node->name, (struct ast_shell_command *)node->body);
    free(node);
    return 0;
}

static int evaluate_subshell(struct ast_list *node,
                             struct internal_state *global)
{
    int pid = fork();
    if (pid == 0)
    {
        int res = evaluate_list(node, global);
        _exit(res);
    }
    else
    {
        int status;
        waitpid(pid, &status, 0);
        // free_list(node);
        if (WIFEXITED(status))
        {
            return WEXITSTATUS(status);
        }
    }
    return EXIT_FAILURE;
}

static int evaluate_shell_command(struct ast_shell_command *node,
                                  struct internal_state *global)
{
    int res;
    switch (node->type)
    {
    case RULE_IF:
        res = evaluate_rule_if((struct ast_rule_if *)node->cmd, global);
        return jump((struct ast *)node, node->cmd, res);
    case RULE_WHILE:
        res = evaluate_rule_while((struct ast_rule_while *)node->cmd, global);
        return jump((struct ast *)node, node->cmd, res);
    case RULE_FOR:
        res = evaluate_rule_for((struct ast_rule_for *)node->cmd, global);
        return jump((struct ast *)node, node->cmd, res);
    case LIST:
        return evaluate_list((struct ast_list *)node->cmd, global);
    case SUBSHELL:
        return evaluate_subshell((struct ast_list *)node->cmd, global);
    case RULE_CASE:
        return evaluate_rule_case((struct ast_rule_case *)node->cmd, global);
    default: // RULE_UNTIL
        res = evaluate_rule_until((struct ast_rule_until *)node->cmd, global);
        return jump((struct ast *)node, node->cmd, res);
    }
}

static int evaluate_command(struct ast_command *node,
                            struct internal_state *global)
{
    int old_stdout = dup(STDOUT_FILENO);
    int res;
    switch (node->type)
    {
    case SIMPLE_COMMAND:
        res = evaluate_simple_command((struct ast_simple_command *)node->cmd,
                                      global);
        res = jump((struct ast *)node, node->cmd, res);
        break;
    case SHELL_COMMAND:
        for (size_t i = 0; node->redirs[i]; ++i)
            evaluate_redirection((struct ast_redirection *)node->redirs[i]);
        res = evaluate_shell_command((struct ast_shell_command *)node->cmd,
                                     global);
        res = jump((struct ast *)node, node->cmd, res);
        break;
    case FUNCTION:
        for (size_t i = 0; node->redirs[i]; ++i)
            evaluate_redirection((struct ast_redirection *)node->redirs[i]);
        res = evaluate_function((struct ast_function *)node->cmd, global);
        break;
    default:
        return 1;
    }
    if (dup2(old_stdout, STDOUT_FILENO) == -1)
        errx(1, "Cannot retrieve old stdout\n");
    close(old_stdout);
    return res;
}

static int exec_pipe(int in, int out, struct ast_command *node,
                     struct internal_state *global)
{
    if (in != STDIN_FILENO)
    {
        dup2(in, STDIN_FILENO);
        close(in);
    }
    if (out != STDOUT_FILENO)
    {
        dup2(out, STDOUT_FILENO);
        close(out);
    }
    return evaluate_command(node, global);
}

static int evaluate_pipeline(struct ast_pipeline *node,
                             struct internal_state *global)
{
    size_t i = 0;
    int in = STDIN_FILENO;
    int fd[2];

    int stdin_save = dup(STDIN_FILENO);
    int stdout_save = dup(STDOUT_FILENO);
    for (; node->cmds[i + 1]; ++i)
    {
        assert(pipe(fd) == 0 && "pipe failed");
        exec_pipe(in, fd[1], (struct ast_command *)node->cmds[i], global);
        jump((struct ast *)node, node->cmds[i], 0);

        close(fd[1]);

        if (in != STDIN_FILENO)
            close(in);

        in = fd[0];
        if (node->base.loop_continue)
            return 0;
    }
    if (in != STDIN_FILENO)
        dup2(in, STDIN_FILENO);

    dup2(stdout_save, STDOUT_FILENO);
    close(stdout_save);
    int res = evaluate_command((struct ast_command *)node->cmds[i], global);
    close(in);
    dup2(stdin_save, STDIN_FILENO);
    close(stdin_save);
    res = jump((struct ast *)node, node->cmds[i], res);
    if (node->neg)
        return !res;
    return res;
}

static int evaluate_and_or(struct ast_and_or *node,
                           struct internal_state *global)
{
    int res = evaluate_pipeline((struct ast_pipeline *)node->pipes[0], global);
    res = jump((struct ast *)node, node->pipes[0], res);
    if (node->base.loop_continue)
        return 0;
    for (size_t i = 1; node->pipes[i]; ++i)
    {
        if (node->is_ands[i - 1])
        {
            if (res)
                return 1;
            res = evaluate_pipeline((struct ast_pipeline *)node->pipes[i],
                                    global);
            res = jump((struct ast *)node, node->pipes[i], res);
            if (node->base.loop_continue)
                return 0;
        }
        else
        {
            if (!res)
                return 0;
            res = evaluate_pipeline((struct ast_pipeline *)node->pipes[i],
                                    global);
            res = jump((struct ast *)node, node->pipes[i], res);
            if (node->base.loop_continue)
                return 0;
        }
    }
    return res;
}

static int evaluate_list(struct ast_list *node, struct internal_state *global)
{
    size_t i = 0;
    while (node->and_ors[i + 1])
    {
        evaluate_and_or((struct ast_and_or *)node->and_ors[i], global);
        jump((struct ast *)node, node->and_ors[i], 0);
        if (node->base.loop_continue)
            return 0;
        i++;
    }
    int res = evaluate_and_or((struct ast_and_or *)node->and_ors[i], global);
    return jump((struct ast *)node, node->and_ors[i], res);
}

int evaluate_ast(struct ast *node, struct internal_state *global)
{
    if (!node)
        return 0;
    return evaluate_list((struct ast_list *)node, global);
}
