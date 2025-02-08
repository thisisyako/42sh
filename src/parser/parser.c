#define _POSIX_C_SOURCE 200809L
#include "parser.h"

#include <string.h>

#include "expansion/expansion.h"

static struct ast *parse_list(struct lexer *lexer);
static struct ast *parse_and_or(struct lexer *lexer);
static struct ast *parse_pipeline(struct lexer *lexer);
static struct ast *parse_command(struct lexer *lexer);
static struct ast *parse_shell_command(struct lexer *lexer);
static struct ast *parse_rule_case(struct lexer *lexer);
static struct ast_rule_case *parse_case_clause(struct lexer *lexer,
                                               struct ast_rule_case *res);
static struct ast_case_item *parse_case_item(struct lexer *lexer);
static struct ast *parse_funcdec(struct lexer *lexer);
static struct ast *parse_rule_if(struct lexer *lexer, int elif);
static struct ast *parse_else_clause(struct lexer *lexer);
static struct ast *parse_rule_while(struct lexer *lexer);
static struct ast *parse_rule_until(struct lexer *lexer);
static struct ast *parse_rule_for(struct lexer *lexer);
static struct ast *parse_compound_list(struct lexer *lexer);
static struct ast *parse_simple_command(struct lexer *lexer);
static struct ast *parse_prefix(struct lexer *lexer);
static struct ast *parse_redirection(struct lexer *lexer);
static struct ast *parse_builtin(struct lexer *lexer,
                                 struct ast_simple_command *father);
static struct token parse_element(struct lexer *lexer);

static char *tok_type_map1(struct token tok)
{
    switch (tok.type)
    {
    case TOKEN_FOR:
        return "for";
    case TOKEN_IN:
        return "in";
    case TOKEN_LEFT_BRACE:
        return "{";
    case TOKEN_LEFT_PAR:
        return "(";
    case TOKEN_RIGHT_BRACE:
        return "}";
    case TOKEN_RIGHT_PAR:
        return ")";
    default:
        return tok.value;
    }
}

static char *tok_type_map(struct token tok)
{
    switch (tok.type)
    {
    case TOKEN_NONE:
        return "none tok";
    case TOKEN_SEMICOLON:
        return ";";
    case TOKEN_NEWLINE:
        return "newline";
    case TOKEN_PIPELINE:
        return "|";
    case TOKEN_BANG:
        return "!";
    case TOKEN_AND:
        return "&&";
    case TOKEN_OR:
        return "||";
    case TOKEN_IF:
        return "if";
    case TOKEN_ELSE:
        return "else";
    case TOKEN_THEN:
        return "then";
    case TOKEN_FI:
        return "fi";
    case TOKEN_ELIF:
        return "elif";
    case TOKEN_WHILE:
        return "while";
    case TOKEN_DO:
        return "do";
    case TOKEN_DONE:
        return "done";
    case TOKEN_UNTIL:
        return "until";
    case TOKEN_ERROR:
        return "error";
    case TOKEN_EOF:
        return "EOF";
    default:
        return tok_type_map1(tok);
    }
}

static void wrong_look_ahead(struct token tok, const char *expected)
{
    errx(2, "Expected '%s' tokens but got '%s'\n", expected, tok_type_map(tok));
}

struct ast *parse_input(struct lexer *lexer)
{
    struct ast *res = NULL;
    switch (lexer_peek(lexer).type)
    {
    case TOKEN_LEFT_BRACE:
    case TOKEN_LEFT_PAR:
    case TOKEN_IF:
    case TOKEN_WHILE:
    case TOKEN_UNTIL:
    case TOKEN_CASE:
    case TOKEN_FOR:
    case TOKEN_ASSIGNMENT_WORD:
    case TOKEN_BANG:
    case TOKEN_WORD:
        res = parse_list(lexer);
        struct token peek = lexer_peek(lexer);
        if (peek.type == TOKEN_NEWLINE)
        {
            if (lexer->is_input == 1)
                lexer->current_tok.type =
                    TOKEN_NONE; // Pour "reset" le lexer sans relire stdin sinon
                                // bug
            else
                lexer_pop(lexer);
        }
        else if (peek.type != TOKEN_EOF)
            wrong_look_ahead(lexer->current_tok, "EOF - NEWLINE");
        break;
    case TOKEN_NEWLINE:
        if (lexer->is_input == 1)
            lexer->current_tok.type =
                TOKEN_NONE; // Pour "reset" le lexer sans relire stdin sinon bug
        else
            lexer_pop(lexer);
        break;
    case TOKEN_EOF:
        break;
    default:
        wrong_look_ahead(lexer->current_tok, "WORD - NEWLINE - EOF");
    }
    return res;
}

static struct ast *parse_list(struct lexer *lexer)
{
    struct ast_list *res = calloc(1, sizeof(struct ast_list));
    res->base.type = LIST;
    res->and_ors = calloc(2, sizeof(struct ast *));
    res->and_ors[0] = parse_and_or(lexer);
    size_t i = 1;
    struct token peek = lexer_peek(lexer);
    while (peek.type == TOKEN_SEMICOLON)
    {
        lexer_pop(lexer);
        peek = lexer_peek(lexer);
        if (peek.type == TOKEN_EOF || peek.type == TOKEN_NEWLINE)
            break;
        res->and_ors[i++] = parse_and_or(lexer);
        res->and_ors =
            realloc(res->and_ors, sizeof(struct ast_and_or) * (i + 1));
        peek = lexer_peek(lexer);
    }
    res->and_ors[i] = NULL;
    return (struct ast *)res;
}

static struct ast *parse_and_or(struct lexer *lexer)
{
    struct ast_and_or *res = calloc(1, sizeof(struct ast_and_or));
    res->base.type = AND_OR;

    size_t len_pipes = 1;
    res->pipes = calloc(2, sizeof(struct ast_pipeline));
    res->pipes[0] = parse_pipeline(lexer);

    size_t len_and_ors = 0;
    res->is_ands = calloc(1, sizeof(int));

    struct token peek = lexer_peek(lexer);
    while (peek.type == TOKEN_AND || peek.type == TOKEN_OR)
    {
        enum token_type type = peek.type;
        lexer_pop(lexer);
        peek = lexer_peek(lexer);
        while (peek.type == TOKEN_NEWLINE)
        {
            if (lexer->input == stdin)
                peek.type = TOKEN_NONE;
            else
            {
                lexer_pop(lexer);
                peek = lexer_peek(lexer);
            }
        }
        len_pipes++;
        res->pipes =
            realloc(res->pipes, sizeof(struct ast_pipeline) * (len_pipes + 1));
        res->pipes[len_pipes - 1] = parse_pipeline(lexer);
        res->pipes[len_pipes] = NULL;

        ++len_and_ors;
        res->is_ands = realloc(res->is_ands, sizeof(int) * (len_and_ors));
        res->is_ands[len_and_ors - 1] = type == TOKEN_AND;
    }
    return (struct ast *)res;
}

static struct ast *parse_pipeline(struct lexer *lexer)
{
    struct ast_pipeline *res = calloc(1, sizeof(struct ast_pipeline));
    res->base.type = PIPELINE;
    struct token peek = lexer_peek(lexer);
    if (peek.type == TOKEN_BANG)
    {
        res->neg = 1;
        lexer_pop(lexer);
        peek = lexer_peek(lexer);
    }
    if (peek.type != TOKEN_ASSIGNMENT_WORD && peek.type != TOKEN_IONUMBER
        && peek.type != TOKEN_REDIRECTION && peek.type != TOKEN_WORD
        && peek.type != TOKEN_IF && peek.type != TOKEN_WHILE
        && peek.type != TOKEN_UNTIL && peek.type != TOKEN_FOR
        && peek.type != TOKEN_LEFT_PAR && peek.type != TOKEN_LEFT_BRACE
        && peek.type != TOKEN_CASE && peek.type != TOKEN_DOUBLE_COLON)
        wrong_look_ahead(peek, "ASSIGNMENT_WORD - IONUMBER - REDIRECTION - IF");
    size_t len_cmds = 1;
    res->cmds = calloc(2, sizeof(struct ast_command));
    res->cmds[0] = parse_command(lexer);
    peek = lexer_peek(lexer);
    while (peek.type == TOKEN_PIPELINE)
    {
        lexer_pop(lexer);
        len_cmds++;
        res->cmds =
            realloc(res->cmds, sizeof(struct ast_command) * (len_cmds + 1));
        res->cmds[len_cmds - 1] = parse_command(lexer);
        res->cmds[len_cmds] = NULL;
        peek = lexer_peek(lexer);
        while (peek.type == TOKEN_NEWLINE)
        {
            if (lexer->input == stdin) // Pour avoir une sorte de EOF
                peek.type =
                    TOKEN_NONE; // en mode interactif sinon ca va lire le stdin
            //                        indefiniment et c'est pas le comportement
            //                        qu'on veut
            else
            {
                lexer_pop(lexer);
                peek = lexer_peek(lexer);
            }
        }
    }
    return (struct ast *)res;
}

static void while_redirection(struct ast ***redirs, struct lexer *lexer)
{
    struct token peek = lexer_peek(lexer);
    size_t len_redirs = 0;
    while (peek.type == TOKEN_IONUMBER || peek.type == TOKEN_REDIRECTION)
    {
        len_redirs++;
        **redirs = realloc(**redirs,
                           sizeof(struct ast_redirection) * (len_redirs + 1));
        (*redirs)[len_redirs - 1] = parse_redirection(lexer);
        (*redirs)[len_redirs] = NULL;
        peek = lexer_peek(lexer);
    }
}

static struct ast *parse_command(struct lexer *lexer)
{
    struct ast_command *res = calloc(1, sizeof(struct ast_command));
    res->base.type = COMMAND;
    res->redirs = calloc(1, sizeof(struct ast_redirection));
    struct token peek = lexer_peek(lexer);
    if (peek.type == TOKEN_IF || peek.type == TOKEN_WHILE
        || peek.type == TOKEN_UNTIL || peek.type == TOKEN_FOR
        || peek.type == TOKEN_LEFT_PAR || peek.type == TOKEN_LEFT_BRACE
        || peek.type == TOKEN_CASE)
    {
        res->type = SHELL_COMMAND;
        res->cmd = parse_shell_command(lexer);
        peek = lexer_peek(lexer);
        while_redirection(&res->redirs, lexer);
    }
    else if (peek.type == TOKEN_WORD || peek.type == TOKEN_ASSIGNMENT_WORD
             || peek.type == TOKEN_IONUMBER || peek.type == TOKEN_REDIRECTION
             || peek.type == TOKEN_DOUBLE_COLON)
    {
        if (peek.type == TOKEN_WORD)
        {
            char c;
            while ((c = fgetc(lexer->input)) == ' ')
            {
                continue;
            }
            ungetc(c, lexer->input);
            if (c == '(')
            {
                res->type = FUNCTION;
                res->cmd = parse_funcdec(lexer);
                while_redirection(&res->redirs, lexer);
            }
            else
            {
                res->type = SIMPLE_COMMAND;
                res->cmd = parse_simple_command(lexer);
            }
        }
        else
        {
            res->type = SIMPLE_COMMAND;
            res->cmd = parse_simple_command(lexer);
        }
    }
    else
        wrong_look_ahead(lexer->current_tok, "WORD - IF");
    return (struct ast *)res;
}

static struct ast *parse_shell_command(struct lexer *lexer)
{
    struct ast_shell_command *res = calloc(1, sizeof(struct ast_shell_command));
    res->base.type = SHELL_COMMAND;

    struct token peek = lexer_peek(lexer);
    enum token_type type =
        peek.type == TOKEN_LEFT_PAR ? TOKEN_RIGHT_PAR : TOKEN_RIGHT_BRACE;
    switch (peek.type)
    {
    case TOKEN_LEFT_BRACE:
    case TOKEN_LEFT_PAR:
        lexer_pop(lexer);
        res->cmd = parse_compound_list(lexer);
        res->type = type == TOKEN_RIGHT_PAR ? SUBSHELL : LIST;
        peek = lexer_peek(lexer);
        if (peek.type != type)
            wrong_look_ahead(lexer->current_tok,
                             type == TOKEN_RIGHT_PAR ? ")" : "}");
        lexer_pop(lexer);
        break;
    case TOKEN_IF:
        res->cmd = parse_rule_if(lexer, 0);
        res->type = RULE_IF;
        break;
    case TOKEN_WHILE:
        res->cmd = parse_rule_while(lexer);
        res->type = RULE_WHILE;
        break;
    case TOKEN_FOR:
        res->cmd = parse_rule_for(lexer);
        res->type = RULE_FOR;
        break;
    case TOKEN_CASE:
        res->cmd = parse_rule_case(lexer);
        res->type = RULE_CASE;
        break;
    default: // UNTIL
        res->cmd = parse_rule_until(lexer);
        res->type = RULE_UNTIL;
        break;
    }
    return (struct ast *)res;
}

static struct ast *parse_rule_case(struct lexer *lexer)
{
    lexer_pop(lexer);
    struct ast_rule_case *res = calloc(1, sizeof(struct ast_rule_case));
    res->base.type = RULE_CASE;

    struct token peek = lexer_peek(lexer);
    if (peek.type != TOKEN_WORD)
        wrong_look_ahead(peek, "WORD");

    res->enumerator = peek.value;
    lexer_pop(lexer);
    while ((peek = lexer_peek(lexer)).type == TOKEN_NEWLINE)
        lexer_pop(lexer);

    if (peek.type != TOKEN_WORD && strcmp(peek.value, "in"))
    {
        free(res);
        wrong_look_ahead(peek, "in");
    }
    free(peek.value);
    lexer_pop(lexer);
    while ((peek = lexer_peek(lexer)).type == TOKEN_NEWLINE)
        lexer_pop(lexer);

    if ((peek.type == TOKEN_WORD && strcmp(peek.value, "esac"))
        || peek.type == TOKEN_LEFT_PAR)
        res = parse_case_clause(lexer, res);

    lexer_pop(lexer);
    peek = lexer_peek(lexer);
    if (peek.type == TOKEN_WORD && strcmp(peek.value, "esac"))
    {
        free(res);
        wrong_look_ahead(peek, "esac");
    }
    free(peek.value);
    lexer_pop(lexer);
    return (struct ast *)res;
}

static struct ast_rule_case *parse_case_clause(struct lexer *lexer,
                                               struct ast_rule_case *node)
{
    node->items = calloc(2, sizeof(struct ast *));
    size_t i = 0;
    node->items[i++] = (struct ast *)parse_case_item(lexer);
    struct token peek = lexer_peek(lexer);

    while (peek.type == TOKEN_DOUBLE_COLON)
    {
        lexer_pop(lexer);
        while ((peek = lexer_peek(lexer)).type == TOKEN_NEWLINE)
            lexer_pop(lexer);

        if (peek.type == TOKEN_WORD && !strcmp(peek.value, "esac"))
            break;

        node->items = realloc(node->items, sizeof(struct ast *) * (i + 2));
        node->items[i++] = (struct ast *)parse_case_item(lexer);
        peek = lexer_peek(lexer);
    }
    if (peek.type != TOKEN_WORD || strcmp(peek.value, "esac"))
        wrong_look_ahead(peek, "esac");
    node->items[i] = NULL;

    return node;
}

static struct ast_case_item *parse_case_item(struct lexer *lexer)
{
    struct ast_case_item *res = calloc(1, sizeof(struct ast_case_item));
    struct token peek = lexer_peek(lexer);
    if (peek.type == TOKEN_LEFT_PAR)
        lexer_pop(lexer);
    peek = lexer_peek(lexer);
    if (peek.type != TOKEN_WORD)
    {
        free(res);
        wrong_look_ahead(peek, "item for 'case'");
    }
    res->item = calloc(2, sizeof(char *));
    size_t i = 0;
    res->item[i++] = peek.value;

    lexer_pop(lexer);
    peek = lexer_peek(lexer);
    while (peek.type == TOKEN_PIPELINE)
    {
        lexer_pop(lexer);
        peek = lexer_peek(lexer);
        if (peek.type != TOKEN_WORD)
        {
            free(res->item);
            free(res);
            wrong_look_ahead(peek, "item for 'case'");
        }
        res->item = realloc(res->item, sizeof(char *) * (i + 2));
        res->item[i++] = peek.value;
        lexer_pop(lexer);
        peek = lexer_peek(lexer);
    }
    res->item[i] = NULL;
    if (peek.type != TOKEN_RIGHT_PAR)
    {
        free(res->item);
        free(res);
        wrong_look_ahead(peek, "')' for item");
        return NULL;
    }
    lexer_pop(lexer);
    while ((peek = lexer_peek(lexer)).type == TOKEN_NEWLINE)
        lexer_pop(lexer);
    if ((peek.type == TOKEN_WORD && !strcmp(peek.value, "esac"))
        || peek.type == TOKEN_DOUBLE_COLON || peek.type == TOKEN_NEWLINE)
        return res;
    res->list = parse_compound_list(lexer);
    return res;
}

static struct ast *parse_funcdec(struct lexer *lexer)
{
    struct ast_function *res = calloc(1, sizeof(struct ast_function));
    res->base.type = FUNCTION;
    struct token peek = lexer_peek(lexer);
    if (!is_valid_name(peek.value))
        errx(1, "'%s' not a valid identifier", peek.value);
    res->name = peek.value;
    lexer_pop(lexer);
    lexer_pop(lexer);
    peek = lexer_peek(lexer);
    if (peek.type != TOKEN_RIGHT_PAR)
        wrong_look_ahead(lexer->current_tok, ")");

    lexer_pop(lexer);
    while ((peek = lexer_peek(lexer)).type == TOKEN_NEWLINE)
        lexer_pop(lexer);

    res->body = parse_shell_command(lexer);
    return (struct ast *)res;
}

static struct ast *parse_rule_if(struct lexer *lexer, int elif)
{
    struct ast_rule_if *res = calloc(1, sizeof(struct ast_rule_if));
    res->base.type = RULE_IF;
    lexer_pop(lexer);
    res->cond = parse_compound_list(lexer);
    if (lexer_peek(lexer).type != TOKEN_THEN)
        wrong_look_ahead(lexer->current_tok, "THEN");
    lexer_pop(lexer);
    res->then = parse_compound_list(lexer);
    struct token peek = lexer_peek(lexer);
    if (peek.type != TOKEN_FI && peek.type != TOKEN_ELSE
        && peek.type != TOKEN_ELIF)
        wrong_look_ahead(lexer->current_tok, "FI - ELSE - ELIF");
    if (peek.type == TOKEN_FI)
    {
        if (elif == 0)
            lexer_pop(lexer);
        return (struct ast *)res;
    }
    res->otherwise = parse_else_clause(lexer);
    peek = lexer_peek(lexer);
    if (elif == 0)
    {
        if (peek.type != TOKEN_FI)
            wrong_look_ahead(peek, "FI");
        lexer_pop(lexer);
    }
    return (struct ast *)res;
}

static struct ast *parse_else_clause(struct lexer *lexer)
{
    struct ast_else_clause *res = calloc(1, sizeof(struct ast_else_clause));
    res->base.type = ELSE_CLAUSE;
    struct token peek = lexer_peek(lexer);
    if (peek.type == TOKEN_ELSE)
    {
        lexer_pop(lexer);
        res->otherwise = parse_compound_list(lexer);
    }
    else if (peek.type == TOKEN_ELIF)
        res->elif = parse_rule_if(lexer, 1);
    else
        wrong_look_ahead(peek, "ELSE - ELIF");
    return (struct ast *)res;
}

static struct ast *parse_rule_while(struct lexer *lexer)
{
    lexer_pop(lexer);
    struct ast_rule_while *res = calloc(1, sizeof(struct ast_rule_while));
    res->base.type = RULE_WHILE;

    res->cond = parse_compound_list(lexer);

    struct token peek = lexer_peek(lexer);
    if (peek.type != TOKEN_DO)
        wrong_look_ahead(peek, "DO");
    lexer_pop(lexer);
    res->cmd = parse_compound_list(lexer);
    peek = lexer_peek(lexer);
    if (peek.type != TOKEN_DONE)
        wrong_look_ahead(peek, "DONE");
    lexer_pop(lexer);

    return (struct ast *)res;
}

static struct ast *parse_rule_until(struct lexer *lexer)
{
    lexer_pop(lexer);
    struct ast_rule_until *res = calloc(1, sizeof(struct ast_rule_until));
    res->base.type = RULE_UNTIL;

    res->cond = parse_compound_list(lexer);

    struct token peek = lexer_peek(lexer);
    if (peek.type != TOKEN_DO)
        wrong_look_ahead(peek, "DO");
    lexer_pop(lexer);
    res->cmd = parse_compound_list(lexer);
    peek = lexer_peek(lexer);
    if (peek.type != TOKEN_DONE)
        wrong_look_ahead(peek, "DONE");
    lexer_pop(lexer);

    return (struct ast *)res;
}

static struct ast *parse_rule_for(struct lexer *lexer)
{
    struct ast_rule_for *res = calloc(1, sizeof(struct ast_rule_for));
    res->base.type = RULE_FOR;
    lexer_pop(lexer);
    struct token peek = lexer_peek(lexer);
    if (peek.type != TOKEN_WORD)
        wrong_look_ahead(peek, "WORD");
    res->enumerator = peek.value;
    lexer_pop(lexer);
    if ((peek = lexer_peek(lexer)).type == TOKEN_SEMICOLON)
        lexer_pop(lexer);
    else if (peek.type == TOKEN_WORD && !strcmp(peek.value, "in"))
    {
        free(peek.value);
        lexer_pop(lexer);
        size_t list_size = 0;
        res->list = NULL;
        while ((peek = lexer_peek(lexer)).type == TOKEN_WORD)
        {
            res->list = realloc(res->list, sizeof(char *) * (list_size + 1));
            res->list[list_size++] = peek.value;
            lexer_pop(lexer);
        }
        res->list = realloc(res->list, sizeof(char *) * (list_size + 1));
        res->list[list_size] = NULL;
        peek = lexer_peek(lexer);
        if (peek.type == TOKEN_SEMICOLON || peek.type == TOKEN_NEWLINE)
            lexer_pop(lexer);
        else
            wrong_look_ahead(peek, "; - NEWLINE");
    }
    else
        wrong_look_ahead(peek, "; - IN");
    while ((peek = lexer_peek(lexer)).type == TOKEN_NEWLINE)
        lexer_pop(lexer);
    if (peek.type != TOKEN_DO)
        wrong_look_ahead(peek, "DO");
    lexer_pop(lexer);
    res->cmd = parse_compound_list(lexer);
    if ((peek = lexer_peek(lexer)).type != TOKEN_DONE)
        wrong_look_ahead(peek, "'done'");
    lexer_pop(lexer);
    return (struct ast *)res;
}

static struct ast *parse_compound_list(struct lexer *lexer)
{
    struct token peek = lexer_peek(lexer);
    while (peek.type == TOKEN_NEWLINE)
    {
        lexer_pop(lexer);
        peek = lexer_peek(lexer);
    }

    struct ast_list *res = calloc(1, sizeof(struct ast_list));
    res->base.type = LIST;
    res->and_ors = calloc(2, sizeof(struct ast *));
    res->and_ors[0] = parse_and_or(lexer);
    size_t i = 1;
    peek = lexer_peek(lexer);
    while (peek.type == TOKEN_SEMICOLON || peek.type == TOKEN_NEWLINE)
    {
        lexer_pop(lexer);
        peek = lexer_peek(lexer);
        while (peek.type == TOKEN_NEWLINE)
        {
            lexer_pop(lexer);
            peek = lexer_peek(lexer);
        }

        if (peek.type == TOKEN_FI || peek.type == TOKEN_ELIF
            || peek.type == TOKEN_ELSE || peek.type == TOKEN_THEN
            || peek.type == TOKEN_EOF || peek.type == TOKEN_DO
            || peek.type == TOKEN_DONE || peek.type == TOKEN_RIGHT_BRACE
            || peek.type == TOKEN_RIGHT_PAR)
            break;

        res->and_ors[i++] = parse_and_or(lexer);
        res->and_ors =
            realloc(res->and_ors, sizeof(struct ast_and_or) * (i + 1));
        peek = lexer_peek(lexer);
    }

    res->and_ors[i] = NULL;
    return (struct ast *)res;
}

static struct ast *parse_simple_command(struct lexer *lexer)
{
    struct ast_simple_command *res =
        calloc(1, sizeof(struct ast_simple_command));
    size_t len_prefs = 0;
    res->prefs = calloc(1, sizeof(struct ast_prefix));
    int has_prefix = 0;
    struct token peek = lexer_peek(lexer);
    while (peek.type == TOKEN_ASSIGNMENT_WORD || peek.type == TOKEN_IONUMBER
           || peek.type == TOKEN_REDIRECTION)
    {
        has_prefix = 1;
        len_prefs++;
        res->prefs =
            realloc(res->prefs, sizeof(struct ast_prefix) * (len_prefs + 1));
        res->prefs[len_prefs - 1] = parse_prefix(lexer);
        res->prefs[len_prefs] = NULL;
        peek = lexer_peek(lexer);
    }
    res->redirs = calloc(1, sizeof(struct ast_redirection));
    if (peek.type != TOKEN_WORD && !has_prefix)
    {
        wrong_look_ahead(peek, "WORD");
    }
    if (peek.type == TOKEN_WORD)
        res->builtin = parse_builtin(lexer, res);
    return (struct ast *)res;
}

static struct ast *parse_prefix(struct lexer *lexer)
{
    struct ast_prefix *res = calloc(1, sizeof(struct ast_prefix));
    struct token peek = lexer_peek(lexer);
    res->assignment = NULL;
    if (peek.type == TOKEN_ASSIGNMENT_WORD)
    {
        res->assignment = peek.value;
        lexer_pop(lexer);
    }
    else
        res->redir = parse_redirection(lexer);
    return (struct ast *)res;
}

static struct ast *parse_redirection(struct lexer *lexer)
{
    struct ast_redirection *res = calloc(1, sizeof(struct ast_redirection));
    struct token peek = lexer_peek(lexer);
    res->ionumber = -1;
    if (peek.type == TOKEN_IONUMBER)
    {
        res->ionumber = atoi(peek.value);
        if (res->ionumber == 0 && errno == EINVAL)
            errx(2, "Invalid ionumber in redirection\n");
        free(peek.value);
        lexer_pop(lexer);
        peek = lexer_peek(lexer);
    }
    if (peek.type != TOKEN_REDIRECTION)
        wrong_look_ahead(peek, "REDIRECTION");
    if (res->ionumber == -1)
        res->ionumber = strcmp(peek.value, "<") && strcmp(peek.value, "<&")
            && strcmp(peek.value, "<>");
    res->type = peek.value;
    lexer_pop(lexer);
    peek = lexer_peek(lexer);
    if (peek.type != TOKEN_WORD)
        wrong_look_ahead(peek, "WORD");
    res->word = peek.value;
    lexer_pop(lexer);
    return (struct ast *)res;
}

static enum command_type command_to_enum(char *cmd)
{
    if (!strcmp(cmd, "true"))
        return TRUE;
    if (!strcmp(cmd, "echo"))
        return ECHO;
    if (!strcmp(cmd, "false"))
        return FALSE;
    if (!strcmp(cmd, "."))
        return DOT;
    if (!strcmp(cmd, "break"))
        return BREAK;
    if (!strcmp(cmd, "continue"))
        return CONTINUE;
    if (!strcmp(cmd, "exit"))
        return EXIT;
    if (!strcmp(cmd, "export"))
        return EXPORT;
    if (!strcmp(cmd, "cd"))
        return CD;
    if (!strcmp(cmd, "unset"))
        return UNSET;

    return SYS_COMMAND;
}

static void split_command_name_args(char *source, char ***args, size_t *i)
{
    size_t j = 0;
    while (source[j] != '\0' && source[j] != ' ')
        ++j;

    if (source[j] == ' ')
    {
        size_t k = j + 1;
        size_t l = j + 1;
        size_t next_arg_start = j + 1;
        while (source[k] != '\0')
        {
            while (source[l] != '\0' && source[l] != ' ' && source[l] != '\t'
                   && source[l] != '\n')
                ++l;
            char save = source[l];
            source[l] = '\0';
            (*args)[(*i)++] = strdup(source + next_arg_start);
            *args = realloc(*args, (*i + 1) * sizeof(char *));
            next_arg_start = l + 1;
            k = l;
            source[l++] = save;
        }
        (*args)[(*i)] = NULL;
    }
    source[j] = '\0';
}

static struct ast *parse_builtin(struct lexer *lexer,
                                 struct ast_simple_command *father)
{
    struct ast_builtin *res = calloc(1, sizeof(struct ast_simple_command));
    res->base.type = SIMPLE_COMMAND;
    struct token peek = lexer_peek(lexer);
    res->command_name = expand(peek.value, lexer->global);
    // handle command substitution for example $(echo echo prout)
    size_t i = 0;
    res->args = calloc(1, sizeof(char *));
    split_command_name_args(res->command_name, &res->args, &i);

    res->type = command_to_enum(res->command_name);

    lexer_pop(lexer);
    peek = lexer_peek(lexer);

    size_t len_redirs = 0;
    while (peek.type == TOKEN_WORD || peek.type == TOKEN_IONUMBER
           || peek.type == TOKEN_REDIRECTION)
    {
        if (peek.type == TOKEN_IONUMBER || peek.type == TOKEN_REDIRECTION)
        {
            len_redirs++;
            father->redirs =
                realloc(father->redirs,
                        sizeof(struct ast_redirection) * (len_redirs + 1));
            father->redirs[len_redirs - 1] = parse_redirection(lexer);
            father->redirs[len_redirs] = NULL;
        }
        else
        {
            parse_element(lexer);
            res->args[i++] = peek.value;
            res->args = realloc(res->args, sizeof(char *) * (i + 1));
            res->args[i] = NULL;
        }
        peek = lexer_peek(lexer);
    }
    return (struct ast *)res;
}

static struct token parse_element(struct lexer *lexer)
{
    return lexer_pop(lexer);
}
