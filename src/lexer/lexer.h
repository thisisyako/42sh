#ifndef LEXER_H
#define LEXER_H

#include <stdio.h>

#include "lexer/token.h"

struct lexer
{
    FILE *input;
    char current_char;
    struct token current_tok;
    struct internal_state *global;
    int is_input;
};

struct lexer *lexer_new(FILE *input, int is_input);
void lexer_free(struct lexer *lexer);

/// @brief Peeks the current token, does not affect the internal state of the
/// lexer.
/// @param lexer
/// @return
struct token lexer_peek(struct lexer *lexer);

/// @brief Pops the current token, makes the lexer advance to next token.
/// @param lexer
/// @return
struct token lexer_pop(struct lexer *lexer);

/// @brief Tells if a string is in the valid name format according to
/// the 3.230 section of SCL
/// @param name
/// @return returns 1 if name is a valid name.
int is_valid_name(char *name);

#endif /* ! LEXER_H */
