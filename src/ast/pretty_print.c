#include "ast/ast.h"

static char *enum_to_node(struct ast *node)
{
    switch (node->type)
    {
    case LIST:
        return "list";
    case AND_OR:
        return "and_or";
    case PIPELINE:
        return "pipeline";
    case COMMAND:
        return "command";
    case SHELL_COMMAND:
        return "shell_command";
    case RULE_IF:
        return "rule_if";
    case ELSE_CLAUSE:
        return "else_clause";
    case SIMPLE_COMMAND:
        return "simple_command";
    default:
        return "enum error";
    }
}

static char *enum_to_command(struct ast *node)
{
    switch (((struct ast_builtin *)node)->type)
    {
    case TRUE:
        return "true";
    case FALSE:
        return "false";
    case ECHO:
        return "echo";
    default:
        return "enum error";
    }
}

static int pretty_print_rec(struct ast *node, size_t i);

static int pretty_list(struct ast *node, size_t i)
{
    size_t old_i = i - 1;
    size_t j = 0;
    while (((struct ast_list *)node)->and_ors[j])
    {
        size_t link_i = i;
        i = pretty_print_rec(((struct ast_list *)node)->and_ors[j], i);
        printf("%zu -> %zu\n", old_i, link_i);
        j++;
    }
    return i;
}

static int pretty_command(struct ast *node, size_t i)
{
    size_t old_i = i - 1;
    if (((struct ast_command *)node)->type == SIMPLE_COMMAND)
        i = pretty_print_rec(((struct ast_command *)node)->cmd, i);
    else
        i = pretty_print_rec(((struct ast_command *)node)->cmd, i);
    printf("%zu -> %zu\n", old_i, old_i + 1);
    return i;
}

static int pretty_rule_if(struct ast *node, size_t i)
{
    size_t old_i = i - 1;
    i = pretty_print_rec(((struct ast_rule_if *)node)->cond, i);
    printf("%zu -> %zu\n", old_i, old_i + 1);
    size_t link_i = i;
    i = pretty_print_rec(((struct ast_rule_if *)node)->then, i);
    printf("%zu -> %zu\n", old_i, link_i);
    if (((struct ast_rule_if *)node)->otherwise)
    {
        link_i = i;
        i = pretty_print_rec(((struct ast_rule_if *)node)->otherwise, i);
        printf("%zu -> %zu\n", old_i, link_i);
    }
    return i;
}

static int pretty_else_clause(struct ast *node, size_t i)
{
    size_t old_i = i - 1;
    if (((struct ast_else_clause *)node)->elif)
    {
        i = pretty_print_rec(((struct ast_else_clause *)node)->elif, i);
        printf("%zu -> %zu\n", old_i, old_i + 1);
    }
    else
    {
        i = pretty_print_rec(((struct ast_else_clause *)node)->otherwise, i);
        printf("%zu -> %zu\n", old_i, old_i + 1);
    }
    return i;
}

static int pretty_builtin(struct ast *node, size_t i)
{
    size_t old_i = i - 1;
    printf("%zu [label=\"%s\"]\n", i, enum_to_command(node));
    printf("%zu -> %zu\n", old_i, i++);
    size_t j = 0;
    while (((struct ast_builtin *)node)->args[j])
    {
        printf("%zu [label=\"%s\"]\n", i,
               ((struct ast_builtin *)node)->args[j]);
        printf("%zu -> %zu\n", old_i, i++);
        j++;
    }
    return i;
}

static int pretty_print_rec(struct ast *node, size_t i)
{
    size_t old_i = i;
    printf("%zu [label=\"%s\"]\n", i++, enum_to_node(node));
    switch (node->type)
    {
    case LIST:
        return pretty_list(node, i);
    case AND_OR:
        i = pretty_print_rec(((struct ast_and_or *)node)->pipes[0], i);
        printf("%zu -> %zu\n", old_i, old_i + 1);
        return i;
    case PIPELINE:
        i = pretty_print_rec(((struct ast_pipeline *)node)->cmds[0], i);
        printf("%zu -> %zu\n", old_i, old_i + 1);
        return i;
    case COMMAND:
        return pretty_command(node, i);
    case SHELL_COMMAND:
        i = pretty_print_rec(((struct ast_shell_command *)node)->cmd, i);
        printf("%zu -> %zu\n", old_i, old_i + 1);
        return i;
    case RULE_IF:
        return pretty_rule_if(node, i);
    case ELSE_CLAUSE:
        return pretty_else_clause(node, i);
    case SIMPLE_COMMAND:
        return pretty_builtin(node, i);
    default:
        return i;
    }
}

void pretty_print(struct ast *node)
{
    printf("digraph AST {\n0 [label=\"input\"]\n");
    size_t i = 1;
    size_t j = 0;
    while (((struct ast_list *)node)->and_ors[j])
    {
        size_t link_i = i;
        i = pretty_print_rec(((struct ast_list *)node)->and_ors[j], i);
        printf("0 -> %zu\n", link_i);
        j++;
    }
    printf("}\n");
}
