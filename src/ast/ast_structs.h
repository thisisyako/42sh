#ifndef AST_STRUCTS_H
#define AST_STRUCTS_H

enum node_type
{
    LIST,
    AND_OR,
    PIPELINE,
    COMMAND,
    SHELL_COMMAND,
    RULE_FOR,
    RULE_WHILE,
    RULE_UNTIL,
    RULE_CASE,
    CASE_ITEM,
    RULE_IF,
    ELSE_CLAUSE,
    SIMPLE_COMMAND,
    PREFIX,
    REDIRECTION,
    BUILTIN,
    FUNCTION,
    SUBSHELL
};

struct ast
{
    enum node_type type;
    int loop_continue;
};

enum command_type
{
    SYS_COMMAND,
    TRUE,
    FALSE,
    ECHO,
    DOT,
    BREAK,
    EXPORT,
    CONTINUE,
    EXIT,
    CD,
    UNSET
};

struct ast_builtin
{
    struct ast base;
    enum command_type type;
    char *command_name;
    char **args;
};

struct ast_redirection
{
    struct ast base;
    int ionumber;
    char *type;
    char *word;
};

struct ast_prefix
{
    struct ast base;
    char *assignment;
    struct ast *redir;
};

struct ast_simple_command
{
    struct ast base;
    struct ast **prefs;
    struct ast *builtin;
    struct ast **redirs;
};

struct ast_else_clause
{
    struct ast base;
    struct ast *otherwise;
    struct ast *elif;
};

struct ast_rule_if
{
    struct ast base;
    struct ast *cond;
    struct ast *then;
    struct ast *otherwise;
};

struct ast_case_item
{
    struct ast base;
    char **item;
    struct ast *list;
};

struct ast_rule_case
{
    struct ast base;
    char *enumerator;
    struct ast **items;
};

struct ast_rule_until
{
    struct ast base;
    struct ast *cond;
    struct ast *cmd;
};

struct ast_rule_while
{
    struct ast base;
    struct ast *cond;
    struct ast *cmd;
};

struct ast_rule_for
{
    struct ast base;
    char *enumerator;
    char **list;
    struct ast *cmd;
};

struct ast_function
{
    struct ast base;
    char *name;
    struct ast *body;
};

struct ast_shell_command
{
    struct ast base;
    enum node_type type;
    struct ast *cmd;
};

struct ast_command
{
    struct ast base;
    enum node_type type;
    struct ast *cmd;
    struct ast **redirs;
};

struct ast_pipeline
{
    struct ast base;
    int neg;
    struct ast **cmds;
};

struct ast_and_or
{
    struct ast base;
    int *is_ands;
    struct ast **pipes;
};

struct ast_list
{
    struct ast base;
    struct ast **and_ors;
};

#endif /* ! AST_STRUCTS_H */
