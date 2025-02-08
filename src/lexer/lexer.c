#include "lexer.h"

#include <assert.h>
#include <ctype.h>
#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "token.h"

struct lexer *lexer_new(FILE *input, int is_input)
{
    struct lexer *lexer = malloc(sizeof(struct lexer));
    if (lexer == NULL)
        return NULL;

    lexer->input = input;
    lexer->current_tok.type = TOKEN_NONE;
    lexer->is_input = is_input;
    return lexer;
}

void lexer_free(struct lexer *lexer)
{
    free(lexer);
    lexer = NULL;
}

static int is_delim(char c, int is_escaped)
{
    if (is_escaped)
        return 0;
    switch (c)
    {
    case ' ':
    case '\n':
    case '\t':
    case ';':
    case '\0':
    case '>':
    case '<':
    case '&':
    case '|':
    case '(':
    case ')':
    case EOF:
        return 1;
    default:
        return 0;
    }
}

static int is_valid_char(char c)
{
    return isalnum(c) || c == '_';
}

static int lexer_reserved_keyword(struct lexer *lexer, char *str)
{
    if (strcmp(str, "if") == 0)
    {
        lexer->current_tok.type = TOKEN_IF;
    }
    else if (strcmp(str, "then") == 0)
    {
        lexer->current_tok.type = TOKEN_THEN;
    }
    else if (strcmp(str, "else") == 0)
    {
        lexer->current_tok.type = TOKEN_ELSE;
    }
    else if (strcmp(str, "elif") == 0)
    {
        lexer->current_tok.type = TOKEN_ELIF;
    }
    else if (strcmp(str, "fi") == 0)
    {
        lexer->current_tok.type = TOKEN_FI;
    }
    else if (strcmp(str, "while") == 0)
    {
        lexer->current_tok.type = TOKEN_WHILE;
    }
    else if (strcmp(str, "until") == 0)
    {
        lexer->current_tok.type = TOKEN_UNTIL;
    }
    else if (strcmp(str, "do") == 0)
    {
        lexer->current_tok.type = TOKEN_DO;
    }
    else if (strcmp(str, "done") == 0)
    {
        lexer->current_tok.type = TOKEN_DONE;
    }
    else if (strcmp(str, "for") == 0)
    {
        lexer->current_tok.type = TOKEN_FOR;
    }
    else if (strcmp(str, "case") == 0)
    {
        lexer->current_tok.type = TOKEN_CASE;
    }
    else
    {
        return 1;
    }

    free(str);
    return 0;
}

static int lexer_io_number_cases(struct lexer *lexer, char *value)
{
    size_t i = 0;
    while (value[i] != '\0')
    {
        if (value[i] <= '0' || value[i] >= '9')
            return 1;
        ++i;
    }

    lexer->current_tok.type = TOKEN_IONUMBER;
    lexer->current_tok.value = value;
    return 0;
}

int is_valid_name(char *name)
{
    if (name[0] >= '0' && name[0] <= '9')
        return 0;

    size_t i = 0;
    while (name[i] != '\0')
    {
        if (name[i] != '_' && (name[i] < '0' || name[i] > '9')
            && (name[i] < 'a' || name[i] > 'z')
            && (name[i] < 'A' || name[i] > 'Z'))
            return 0;
        ++i;
    }

    return 1;
}

static void lexer_assignment_word(struct lexer *lexer, char *value)
{
    lexer->current_tok.type = TOKEN_ASSIGNMENT_WORD;
    size_t i = 0;
    while (value[i] != '=')
        ++i;
    value[i] = '\0';
    if (!is_valid_name(value))
    {
        lexer->current_tok.type = TOKEN_WORD;
    }

    value[i] = '=';
    lexer->current_tok.value = value;

    return;
}

static void add_char_to_value(struct lexer *lexer, char **value, size_t *count)
{
    char *tmp = realloc(*value, *count + 2);
    assert(tmp != NULL && "Couldn't allocate memory");
    *value = tmp;
    (*value)[(*count)++] = lexer->current_char;
    lexer->current_char = fgetc(lexer->input);
}

static void handle_braces(struct lexer *lexer, char **value, size_t *count,
                          char match)
{
    int in_match = 1;
    char match_open = match == ')' ? '(' : '{';
    while (lexer->current_char != match || in_match == 1)
    {
        if (lexer->current_char == match_open || lexer->current_char == match)
            in_match = !in_match;
        add_char_to_value(lexer, value, count);
    }
    if (match == ')')
        add_char_to_value(lexer, value, count);
}

static void handle_quotes(struct lexer *lexer, char **value, size_t *count,
                          char quotes)
{
    (*value)[(*count)++] = lexer->current_char;
    lexer->current_char = fgetc(lexer->input);
    int escaped = 0;
    while (lexer->current_char != EOF
           && (lexer->current_char != quotes || escaped == 1))
    {
        escaped = 0;
        if (quotes != '\'' && lexer->current_char == '\\')
            escaped = 1;
        add_char_to_value(lexer, value, count);
    }
    if (lexer->current_char == EOF)
        errx(2, "unexpected EOF while looking for a closing '%c'", quotes);
    add_char_to_value(lexer, value, count);
}

static void handle_variables(struct lexer *lexer, char **value, size_t *count)
{
    // handle the writing to value of $blablba variables
    // with braces and () also like ${} $()
    (*value)[(*count)++] = lexer->current_char;
    lexer->current_char = fgetc(lexer->input);
    if (lexer->current_char == '{' || lexer->current_char == '(')
    {
        char match = lexer->current_char == '{' ? '}' : ')';
        handle_braces(lexer, value, count, match);
        return;
    }
    else if (isdigit(lexer->current_char))
    {
        add_char_to_value(lexer, value, count);
        return;
    }

    while (is_valid_char(lexer->current_char))
    {
        add_char_to_value(lexer, value, count);
    }
}

static void word_tok_post_while(struct lexer *lexer, char *value, int has_equal,
                                int could_be)
{
    if (has_equal && could_be)
    {
        lexer_assignment_word(lexer, value);
        return;
    }
    // Handle if then elif else tokens only if in the input such as we could
    // have a reserved word
    else if (could_be && lexer_reserved_keyword(lexer, value) == 0)
    {
        return;
    }
    // Handle IO numbers if any for redirections
    else if ((lexer->current_char == '<' || lexer->current_char == '>')
             && lexer_io_number_cases(lexer, value) == 0)
    {
        return;
    }
}

static void lexer_word_tok(struct lexer *lexer, int could_be)
{
    lexer->current_tok.type = TOKEN_WORD;
    char *value = calloc(2, sizeof(char));
    assert(value != NULL && "Couldn't allocate memory");

    size_t count = 0;
    int has_equal = 0;
    int is_escaped = 0;
    while (!is_delim(lexer->current_char, is_escaped))
    {
        if (lexer->current_char == '\\')
        {
            add_char_to_value(lexer, &value, &count);
            if (lexer->current_char == '\n' && !is_escaped)
                lexer->current_char = fgetc(lexer->input);
            is_escaped = !is_escaped;
            continue;
        }
        else if (lexer->current_char == '$')
        {
            handle_variables(lexer, &value, &count);
            is_escaped = 0;
            continue;
        }
        else if (lexer->current_char == '=' && !is_escaped)
            has_equal = 1;
        else if (lexer->current_char == '"' && !is_escaped)
        {
            handle_quotes(lexer, &value, &count, '"');
            is_escaped = 0;
            continue;
        }
        else if (lexer->current_char == '\'' && !is_escaped)
        {
            handle_quotes(lexer, &value, &count, '\'');
            is_escaped = 0;
            continue;
        }
        else if (lexer->current_char == '`' && !is_escaped)
        {
            handle_quotes(lexer, &value, &count, '`');
            is_escaped = 0;
            continue;
        }
        add_char_to_value(lexer, &value, &count);
        is_escaped = 0;
    }
    ungetc(lexer->current_char, lexer->input);

    value[count] = '\0';

    word_tok_post_while(lexer, value, has_equal, could_be);

    lexer->current_tok.value = value;
}

static void lexer_comment(struct lexer *lexer)
{
    while (lexer->current_char != EOF && lexer->current_char != '\n')
    {
        lexer->current_char = fgetc(lexer->input);
    }
    ungetc(lexer->current_char, lexer->input);
}

static int lexer_valid_redirect_char(char first, char second)
{
    if (first == '<')
    {
        switch (second)
        {
        case '>':
        case '&':
            return 1;
        default:
            return 0;
        }
    }
    else
    {
        switch (second)
        {
        case '>':
        case '&':
        case '|':
            return 1;
        default:
            return 0;
        }
    }
}

static void lexer_redirection_operator(struct lexer *lexer)
{
    char *value = calloc(3, sizeof(char));
    assert(value != NULL && "Failed calloc");

    value[0] = lexer->current_char;
    lexer->current_char = fgetc(lexer->input);
    if (lexer_valid_redirect_char(value[0], lexer->current_char) == 1)
    {
        value[1] = lexer->current_char;
        value[2] = '\0';
    }
    else
    {
        value[1] = '\0';
    }

    lexer->current_tok.type = TOKEN_REDIRECTION;
    lexer->current_tok.value = value;
}

static int could_be_reserved(struct token tok)
{
    switch (tok.type)
    {
    case TOKEN_NONE:
    case TOKEN_NEWLINE:
    case TOKEN_SEMICOLON:
    case TOKEN_IF:
    case TOKEN_FI:
    case TOKEN_ELIF:
    case TOKEN_ELSE:
    case TOKEN_DO:
    case TOKEN_DONE:
    case TOKEN_THEN:
    case TOKEN_UNTIL:
    case TOKEN_WHILE:
    case TOKEN_LEFT_BRACE:
    case TOKEN_RIGHT_BRACE:
    case TOKEN_BANG:
    case TOKEN_LEFT_PAR:
    case TOKEN_RIGHT_PAR:
    case TOKEN_CASE:
        return 1;
    default:
        return 0;
    }
}

static int lexer_next_token_handler1(struct lexer *lexer)
{
    if (lexer->current_char == EOF)
    {
        lexer->current_tok.type = TOKEN_EOF;
    }
    else if (lexer->current_char == '{')
    {
        char c = fgetc(lexer->input);
        if (c != ' ')
        {
            ungetc(c, lexer->input);
        }
        else
            lexer->current_tok.type = TOKEN_LEFT_BRACE;
    }
    else if (lexer->current_char == '}')
    {
        lexer->current_tok.type = TOKEN_RIGHT_BRACE;
    }
    else if (lexer->current_char == '(')
    {
        lexer->current_tok.type = TOKEN_LEFT_PAR;
    }
    else if (lexer->current_char == ')')
    {
        lexer->current_tok.type = TOKEN_RIGHT_PAR;
    }
    else if (lexer->current_char == ';')
    {
        lexer->current_char = fgetc(lexer->input);
        if (lexer->current_char == ';')
        {
            lexer->current_tok.type = TOKEN_DOUBLE_COLON;
        }
        else
        {
            ungetc(lexer->current_char, lexer->input);
            lexer->current_tok.type = TOKEN_SEMICOLON;
        }
    }
    else if (lexer->current_char == '\n')
    {
        lexer->current_tok.type = TOKEN_NEWLINE;
    }
    return !(lexer->current_tok.type == TOKEN_ERROR);
}

struct token lexer_next_token(struct lexer *lexer)
{
    int is_first_word = could_be_reserved(lexer->current_tok);
    lexer->current_tok.type = TOKEN_ERROR;
    lexer->current_char = fgetc(lexer->input);
    while (lexer->current_char == ' ' || lexer->current_char == '\t')
    {
        lexer->current_char = fgetc(lexer->input);
    }

    if (lexer_next_token_handler1(lexer))
    {
        return lexer->current_tok;
    }
    else if (lexer->current_char == '#')
    {
        lexer_comment(lexer);
        return lexer_next_token(lexer);
    }
    else if (lexer->current_char == '<' || lexer->current_char == '>')
    {
        lexer_redirection_operator(lexer);
    }
    else if (lexer->current_char == '|')
    {
        lexer->current_tok.type = TOKEN_PIPELINE;
        lexer->current_char = fgetc(lexer->input);
        if (lexer->current_char == '|')
        {
            lexer->current_tok.type = TOKEN_OR;
        }
        else
        {
            ungetc(lexer->current_char, lexer->input);
        }
    }
    else if (lexer->current_char == '!' && is_first_word)
    {
        lexer->current_tok.type = TOKEN_BANG;
    }
    else if (lexer->current_char == '&')
    {
        lexer->current_char = fgetc(lexer->input);
        if (lexer->current_char == '&')
            lexer->current_tok.type = TOKEN_AND;
        else
        {
            ungetc(lexer->current_char, lexer->input);
            lexer->current_tok.type = TOKEN_ERROR;
        }
    }
    else
    {
        lexer_word_tok(lexer, is_first_word);
    }

    if (lexer->current_tok.type == TOKEN_ERROR)
    {
        errx(2, "Lexical error near '%c'", lexer->current_char);
    }
    return lexer->current_tok;
}

struct token lexer_peek(struct lexer *lexer)
{
    if (lexer->current_tok.type == TOKEN_NONE)
    {
        return lexer_next_token(lexer);
    }
    else
    {
        return lexer->current_tok;
    }
}

struct token lexer_pop(struct lexer *lexer)
{
    if (lexer->current_tok.type == TOKEN_NONE)
    {
        return lexer_peek(lexer);
    }
    else
    {
        return lexer_next_token(lexer);
    }
}
