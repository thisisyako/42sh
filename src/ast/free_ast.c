#include "ast/ast.h"

static void free_builtin(struct ast_builtin *node)
{
    if (!node)
        return;
    free(node->command_name);
    size_t i = 0;
    i = 0;
    while (node->args[i])
    {
        free(node->args[i]);
        i++;
    }
    free(node->args);
    free(node);
}

static void free_redirection(struct ast_redirection *node)
{
    if (!node)
        return;
    free(node->type);
    free(node->word);
    free(node);
}

static void free_prefix(struct ast_prefix *node)
{
    if (!node)
        return;
    free(node->assignment);
    free_redirection((struct ast_redirection *)node->redir);
    free(node);
}

static void free_simple_command(struct ast_simple_command *node)
{
    free_builtin((struct ast_builtin *)node->builtin);
    for (size_t i = 0; node->prefs[i]; ++i)
        free_prefix((struct ast_prefix *)node->prefs[i]);
    for (size_t i = 0; node->redirs[i]; ++i)
        free_redirection((struct ast_redirection *)node->redirs[i]);
    free(node->prefs);
    free(node->redirs);
    free(node);
}

static void free_rule_if(struct ast_rule_if *node);
static void free_list(struct ast_list *node);

static void free_else_clause(struct ast_else_clause *node)
{
    if (node->otherwise)
        free_list((struct ast_list *)node->otherwise);
    else
        free_rule_if((struct ast_rule_if *)node->elif);
    free(node);
}

static void free_rule_if(struct ast_rule_if *node)
{
    free_list((struct ast_list *)node->cond);
    free_list((struct ast_list *)node->then);
    if (((struct ast_rule_if *)node)->otherwise)
        free_else_clause((struct ast_else_clause *)node->otherwise);
    free(node);
}

static void free_rule_while(struct ast_rule_while *node)
{
    free_list((struct ast_list *)node->cond);
    free_list((struct ast_list *)node->cmd);
    free(node);
}

static void free_rule_until(struct ast_rule_until *node)
{
    free_list((struct ast_list *)node->cond);
    free_list((struct ast_list *)node->cmd);
    free(node);
}

static void free_rule_case(struct ast_rule_case *node)
{
    for (size_t i = 0; node->items[i]; ++i)
    {
        struct ast_case_item *item = (struct ast_case_item *)node->items[i];
        if (item->list)
            free_list((struct ast_list *)item->list);
        for (size_t j = 0; item->item[j]; ++j)
            free(item->item[j]);
        free(item->item);
        free(item);
    }
    free(node->enumerator);
    free(node->items);
    free(node);
}

static void free_rule_for(struct ast_rule_for *node)
{
    free(node->enumerator);
    for (size_t i = 0; node->list[i]; ++i)
        free(node->list[i]);
    free(node->list);
    free_list((struct ast_list *)node->cmd);
    free(node);
}

void free_shell_command(struct ast_shell_command *node)
{
    switch (node->type)
    {
    case RULE_IF:
        free_rule_if((struct ast_rule_if *)node->cmd);
        break;
    case RULE_WHILE:
        free_rule_while((struct ast_rule_while *)node->cmd);
        break;
    case RULE_FOR:
        free_rule_for((struct ast_rule_for *)node->cmd);
        break;
    case RULE_CASE:
        free_rule_case((struct ast_rule_case *)node->cmd);
        break;
    case LIST:
    case SUBSHELL:
        free_list((struct ast_list *)node->cmd);
        break;
    default:
        free_rule_until((struct ast_rule_until *)node->cmd);
        break;
    }
    free(node);
}

static void free_command(struct ast_command *node)
{
    switch (node->type)
    {
    case SIMPLE_COMMAND:
        free_simple_command((struct ast_simple_command *)node->cmd);
        break;
    case SHELL_COMMAND:
        free_shell_command((struct ast_shell_command *)node->cmd);
        for (size_t i = 0; node->redirs[i]; ++i)
            free_redirection((struct ast_redirection *)node->redirs[i]);
        break;
    default:
        break;
    }
    free(node->redirs);
    free(node);
}

static void free_pipeline(struct ast_pipeline *node)
{
    for (size_t i = 0; node->cmds[i]; ++i)
        free_command((struct ast_command *)node->cmds[i]);
    free(node->cmds);
    free(node);
}

static void free_and_or(struct ast_and_or *node)
{
    free_pipeline((struct ast_pipeline *)node->pipes[0]);
    for (size_t i = 1; node->pipes[i]; ++i)
        free_pipeline((struct ast_pipeline *)node->pipes[i]);
    free(node->pipes);
    free(node->is_ands);
    free(node);
}

static void free_list(struct ast_list *node)
{
    for (size_t i = 0; node->and_ors[i]; ++i)
        free_and_or((struct ast_and_or *)node->and_ors[i]);
    free(node->and_ors);
    free(node);
}

void free_ast(struct ast *node)
{
    if (node)
        free_list((struct ast_list *)node);
}
