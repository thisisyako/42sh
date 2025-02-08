#define _POSIX_C_SOURCE 200809L
#include <criterion/criterion.h>

#include "lexer/lexer.h"

FILE *get_string(const char *string)
{
    size_t len = strlen(string);
    return fmemopen((void *)string, len, "r");
}

Test(lexer, lexer_new)
{
    FILE *file = get_string("echo Hello_World");

    struct lexer *new_lexer = lexer_new(file, 0);
    cr_assert(new_lexer != NULL);

    fclose(file);
}

Test(lexer, lexer_free)
{
    struct lexer *new_lexer = NULL;
    lexer_free(new_lexer);
}

// Step 1
Test(lexer, step1_lexer_simple)
{
    // if
    FILE *file = get_string("if if  if if    if");
    struct lexer *l = lexer_new(file, 0);
    struct token t;
    for (size_t i = 0; i < 5; i++)
    {
        t = lexer_pop(l);
        cr_assert(t.type == TOKEN_IF);
    }
    t = lexer_pop(l);
    cr_assert(t.type == TOKEN_EOF, "%d\n", t.type);
    lexer_free(l);
    fclose(file);

    // else
    file = get_string("else else    else else  else");
    l = lexer_new(file, 0);
    for (size_t i = 0; i < 5; i++)
    {
        t = lexer_pop(l);
        cr_assert(t.type == TOKEN_ELSE);
    }
    t = lexer_pop(l);
    cr_assert(t.type == TOKEN_EOF);
    lexer_free(l);
    fclose(file);

    // elif
    file = get_string("elif elif   elif   elif elif");
    l = lexer_new(file, 0);
    for (size_t i = 0; i < 5; i++)
    {
        t = lexer_pop(l);
        cr_assert(t.type == TOKEN_ELIF);
    }
    t = lexer_pop(l);
    cr_assert(t.type == TOKEN_EOF);
    lexer_free(l);
    fclose(file);

    // else
    file = get_string("else else    else else else");
    l = lexer_new(file, 0);
    for (size_t i = 0; i < 5; i++)
    {
        t = lexer_pop(l);
        cr_assert(t.type == TOKEN_ELSE);
    }
    t = lexer_pop(l);
    cr_assert(t.type == TOKEN_EOF);
    lexer_free(l);
    fclose(file);

    // fi
    file = get_string("fi fi  fi fi   fi");
    l = lexer_new(file, 0);
    for (size_t i = 0; i < 5; i++)
    {
        t = lexer_pop(l);
        cr_assert(t.type == TOKEN_FI);
    }
    t = lexer_pop(l);
    cr_assert(t.type == TOKEN_EOF);
    lexer_free(l);
    fclose(file);

    // ;
    file = get_string("; ; ; ; ;");
    l = lexer_new(file, 0);
    for (size_t i = 0; i < 5; i++)
    {
        t = lexer_pop(l);
        cr_assert(t.type == TOKEN_SEMICOLON);
    }
    t = lexer_pop(l);
    cr_assert(t.type == TOKEN_EOF);
    lexer_free(l);
    fclose(file);

    // \n
    file = get_string("\n \n \n \n\n");
    l = lexer_new(file, 0);
    for (size_t i = 0; i < 5; i++)
    {
        t = lexer_pop(l);
        cr_assert(t.type == TOKEN_NEWLINE);
    }
    t = lexer_pop(l);
    cr_assert(t.type == TOKEN_EOF);
    lexer_free(l);
    fclose(file);

    // '
    file = get_string("'Hello_World'");
    l = lexer_new(file, 0);
    t = lexer_pop(l);
    cr_assert(t.type == TOKEN_WORD);
    cr_assert(strcmp(t.value, "'Hello_World'") == 0);
    t = lexer_pop(l);
    cr_assert(t.type == TOKEN_EOF);
    lexer_free(l);
    fclose(file);

    // words
    file = get_string("echo ls mv");
    l = lexer_new(file, 0);
    for (size_t i = 0; i < 3; i++)
    {
        t = lexer_pop(l);
        cr_assert(t.type == TOKEN_WORD);
    }
    t = lexer_pop(l);
    cr_assert(t.type == TOKEN_EOF);
    lexer_free(l);
    fclose(file);

    file = get_string("#comment");
    l = lexer_new(file, 0);
    t = lexer_pop(l);
    cr_assert(t.type == TOKEN_EOF);
    lexer_free(l);
    fclose(file);
}

Test(lexer, step1_lexer_hard)
{
    FILE *file = get_string("if then elif else fi ; \n 'MY_WORD' echo world");
    struct lexer *l = lexer_new(file, 0);
    struct token t;

    // Expected tokens in order
    cr_assert((t = lexer_pop(l)).type == TOKEN_IF);
    cr_assert((t = lexer_pop(l)).type == TOKEN_THEN);
    cr_assert((t = lexer_pop(l)).type == TOKEN_ELIF);
    cr_assert((t = lexer_pop(l)).type == TOKEN_ELSE);
    cr_assert((t = lexer_pop(l)).type == TOKEN_FI);
    cr_assert((t = lexer_pop(l)).type == TOKEN_SEMICOLON);
    cr_assert((t = lexer_pop(l)).type == TOKEN_NEWLINE);
    cr_assert((t = lexer_pop(l)).type == TOKEN_WORD
              && strcmp(t.value, "'MY_WORD'") == 0);
    cr_assert((t = lexer_pop(l)).type == TOKEN_WORD
              && strcmp(t.value, "echo") == 0);
    cr_assert((t = lexer_pop(l)).type == TOKEN_WORD
              && strcmp(t.value, "world") == 0);

    lexer_free(l);
    fclose(file);

    // Mixed complex case with varying tokens
    file = get_string("if elif; else\nfi 'complex_case_1'; echo 'Hello'; then "
                      "elif else fi; if 'nested_case' \n ;; ; ");
    l = lexer_new(file, 0);
    cr_assert((t = lexer_pop(l)).type == TOKEN_IF);
    cr_assert((t = lexer_pop(l)).type == TOKEN_ELIF);
    cr_assert((t = lexer_pop(l)).type == TOKEN_SEMICOLON);
    cr_assert((t = lexer_pop(l)).type == TOKEN_ELSE);
    cr_assert((t = lexer_pop(l)).type == TOKEN_NEWLINE);
    cr_assert((t = lexer_pop(l)).type == TOKEN_FI);
    cr_assert((t = lexer_pop(l)).type == TOKEN_WORD
              && strcmp(t.value, "'complex_case_1'") == 0);
    cr_assert((t = lexer_pop(l)).type == TOKEN_SEMICOLON);
    cr_assert((t = lexer_pop(l)).type == TOKEN_WORD
              && strcmp(t.value, "echo") == 0);
    cr_assert((t = lexer_pop(l)).type == TOKEN_WORD
              && strcmp(t.value, "'Hello'") == 0);
    cr_assert((t = lexer_pop(l)).type == TOKEN_SEMICOLON);
    cr_assert((t = lexer_pop(l)).type == TOKEN_THEN);
    cr_assert((t = lexer_pop(l)).type == TOKEN_ELIF);
    cr_assert((t = lexer_pop(l)).type == TOKEN_ELSE);
    cr_assert((t = lexer_pop(l)).type == TOKEN_FI);
    cr_assert((t = lexer_pop(l)).type == TOKEN_SEMICOLON);
    cr_assert((t = lexer_pop(l)).type == TOKEN_IF);
    cr_assert((t = lexer_pop(l)).type == TOKEN_WORD
              && strcmp(t.value, "'nested_case'") == 0);
    cr_assert((t = lexer_pop(l)).type == TOKEN_NEWLINE);
    cr_assert((t = lexer_pop(l)).type == TOKEN_DOUBLE_COLON);
    cr_assert((t = lexer_pop(l)).type == TOKEN_SEMICOLON);
    lexer_free(l);
    fclose(file);

    // Case with nested conditions and mixed separators
    file = get_string("if elif then; else fi fi; if else elif 'end_case';\n;");
    l = lexer_new(file, 0);
    cr_assert((t = lexer_pop(l)).type == TOKEN_IF);
    cr_assert((t = lexer_pop(l)).type == TOKEN_ELIF);
    cr_assert((t = lexer_pop(l)).type == TOKEN_THEN);
    cr_assert((t = lexer_pop(l)).type == TOKEN_SEMICOLON);
    cr_assert((t = lexer_pop(l)).type == TOKEN_ELSE);
    cr_assert((t = lexer_pop(l)).type == TOKEN_FI);
    cr_assert((t = lexer_pop(l)).type == TOKEN_FI);
    cr_assert((t = lexer_pop(l)).type == TOKEN_SEMICOLON);
    cr_assert((t = lexer_pop(l)).type == TOKEN_IF);
    cr_assert((t = lexer_pop(l)).type == TOKEN_ELSE);
    cr_assert((t = lexer_pop(l)).type == TOKEN_ELIF);
    cr_assert((t = lexer_pop(l)).type == TOKEN_WORD
              && strcmp(t.value, "'end_case'") == 0);
    cr_assert((t = lexer_pop(l)).type == TOKEN_SEMICOLON);
    cr_assert((t = lexer_pop(l)).type == TOKEN_NEWLINE);
    cr_assert((t = lexer_pop(l)).type == TOKEN_SEMICOLON);
    lexer_free(l);
    fclose(file);

    // Case with some words in quotes
    file = get_string("if elif then; else fi fi; if else elif 'if else';\n;");
    l = lexer_new(file, 0);
    cr_assert((t = lexer_pop(l)).type == TOKEN_IF);
    cr_assert((t = lexer_pop(l)).type == TOKEN_ELIF);
    cr_assert((t = lexer_pop(l)).type == TOKEN_THEN);
    cr_assert((t = lexer_pop(l)).type == TOKEN_SEMICOLON);
    cr_assert((t = lexer_pop(l)).type == TOKEN_ELSE);
    cr_assert((t = lexer_pop(l)).type == TOKEN_FI);
    cr_assert((t = lexer_pop(l)).type == TOKEN_FI);
    cr_assert((t = lexer_pop(l)).type == TOKEN_SEMICOLON);
    cr_assert((t = lexer_pop(l)).type == TOKEN_IF);
    cr_assert((t = lexer_pop(l)).type == TOKEN_ELSE);
    cr_assert((t = lexer_pop(l)).type == TOKEN_ELIF);
    cr_assert((t = lexer_pop(l)).type == TOKEN_WORD
              && strcmp(t.value, "'if else'") == 0);
    cr_assert((t = lexer_pop(l)).type == TOKEN_SEMICOLON);
    cr_assert((t = lexer_pop(l)).type == TOKEN_NEWLINE);
    cr_assert((t = lexer_pop(l)).type == TOKEN_SEMICOLON);
    lexer_free(l);
    fclose(file);

    // Single-line comment with newline
    file = get_string("# This is a single-line comment\nif else");
    l = lexer_new(file, 0);
    cr_assert((t = lexer_pop(l)).type == TOKEN_NEWLINE);
    cr_assert((t = lexer_pop(l)).type == TOKEN_IF);
    cr_assert((t = lexer_pop(l)).type == TOKEN_ELSE);
    cr_assert((t = lexer_pop(l)).type == TOKEN_EOF);
    lexer_free(l);
    fclose(file);

    // Inline comment after a token
    file = get_string("if # This is a comment\nelse fi");
    l = lexer_new(file, 0);
    cr_assert((t = lexer_pop(l)).type == TOKEN_IF);
    cr_assert((t = lexer_pop(l)).type == TOKEN_NEWLINE);
    cr_assert((t = lexer_pop(l)).type == TOKEN_ELSE);
    cr_assert((t = lexer_pop(l)).type == TOKEN_FI);
    cr_assert((t = lexer_pop(l)).type == TOKEN_EOF);
    lexer_free(l);
    fclose(file);

    // Multiple comments in sequence
    file = get_string("# Comment 1\n# Comment 2\nif elif else # Comment 3");
    l = lexer_new(file, 0);
    cr_assert((t = lexer_pop(l)).type == TOKEN_NEWLINE);
    cr_assert((t = lexer_pop(l)).type == TOKEN_NEWLINE);
    cr_assert((t = lexer_pop(l)).type == TOKEN_IF);
    cr_assert((t = lexer_pop(l)).type == TOKEN_ELIF);
    cr_assert((t = lexer_pop(l)).type == TOKEN_ELSE);
    cr_assert((t = lexer_pop(l)).type == TOKEN_EOF);
    lexer_free(l);
    fclose(file);
}

// Step 2
Test(lexer, step2_lexer_easy)
{
    // New keywords: while, do, done, until, for
    FILE *file = get_string("while do done until for");
    struct lexer *l = lexer_new(file, 0);
    struct token t;

    cr_assert((t = lexer_pop(l)).type == TOKEN_WHILE);
    cr_assert((t = lexer_pop(l)).type == TOKEN_DO);
    cr_assert((t = lexer_pop(l)).type == TOKEN_DONE);
    cr_assert((t = lexer_pop(l)).type == TOKEN_UNTIL);
    cr_assert((t = lexer_pop(l)).type == TOKEN_FOR);
    cr_assert((t = lexer_pop(l)).type == TOKEN_EOF);

    lexer_free(l);
    fclose(file);

    // Operators: &&, ||, |
    file = get_string("&& || |");
    l = lexer_new(file, 0);

    cr_assert((t = lexer_pop(l)).type == TOKEN_AND);
    cr_assert((t = lexer_pop(l)).type == TOKEN_OR);
    cr_assert((t = lexer_pop(l)).type == TOKEN_PIPELINE);
    cr_assert((t = lexer_pop(l)).type == TOKEN_EOF);

    lexer_free(l);
    fclose(file);

    // Bang operator: !
    file = get_string("!");
    l = lexer_new(file, 0);

    cr_assert((t = lexer_pop(l)).type == TOKEN_BANG);
    cr_assert((t = lexer_pop(l)).type == TOKEN_EOF);

    lexer_free(l);
    fclose(file);
}

// Step 3
Test(lexer, step3_lexer_easy)
{
    FILE *file = get_string("{ echo hello world; } ");
    struct lexer *l = lexer_new(file, 0);
    struct token t;

    cr_assert((t = lexer_pop(l)).type == TOKEN_LEFT_BRACE);
    cr_assert((t = lexer_pop(l)).type == TOKEN_WORD
              && strcmp(t.value, "echo") == 0);
    cr_assert((t = lexer_pop(l)).type == TOKEN_WORD
              && strcmp(t.value, "hello") == 0);
    cr_assert((t = lexer_pop(l)).type == TOKEN_WORD
              && strcmp(t.value, "world") == 0);
    cr_assert((t = lexer_pop(l)).type == TOKEN_SEMICOLON);
    cr_assert((t = lexer_pop(l)).type == TOKEN_RIGHT_BRACE);
    cr_assert((t = lexer_pop(l)).type == TOKEN_EOF);

    lexer_free(l);
    fclose(file);

    file = get_string("(echo hello world;) ");
    l = lexer_new(file, 0);

    cr_assert((t = lexer_pop(l)).type == TOKEN_LEFT_PAR);
    cr_assert((t = lexer_pop(l)).type == TOKEN_WORD
              && strcmp(t.value, "echo") == 0);
    cr_assert((t = lexer_pop(l)).type == TOKEN_WORD
              && strcmp(t.value, "hello") == 0);
    cr_assert((t = lexer_pop(l)).type == TOKEN_WORD
              && strcmp(t.value, "world") == 0);
    cr_assert((t = lexer_pop(l)).type == TOKEN_SEMICOLON);
    cr_assert((t = lexer_pop(l)).type == TOKEN_RIGHT_PAR);
    cr_assert((t = lexer_pop(l)).type == TOKEN_EOF);

    lexer_free(l);
    fclose(file);
}
