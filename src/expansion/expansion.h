#ifndef EXPANSION_H
#define EXPANSION_H

#include "internal_state/internal_state.h"
#include "io_backend/io_backend.h"
#include "lexer/lexer.h"

/// @brief Expands the input.
/// @param input Is Freed.
/// @param global is used when special variables occur (i.e '$?').
/// @return Returns an allocated char *.
char *expand(char *input, struct internal_state *global);

/// @brief Expands the args array of string by taking in account $@ and $*.
/// @param args Is Freed.
/// @param global is used for positional parameters.
/// @return Returns an allocated char **.
char **expand_args(char **args, struct internal_state *global);

#endif /* ! EXPANSION_H */
