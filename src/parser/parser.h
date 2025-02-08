#ifndef PARSER_H
#define PARSER_H

#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ast/ast_structs.h"
#include "lexer/lexer.h"

/// @brief Creates an AST based on the lexer.
/// @param lexer
/// @return
struct ast *parse_input(struct lexer *lexer);

#endif /* ! PARSER_H */
