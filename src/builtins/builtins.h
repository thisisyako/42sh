#ifndef BUILTINS_H
#define BUILTINS_H

#include <stdio.h>
#include <stdlib.h>

#include "ast/ast_structs.h"
#include "internal_state/internal_state.h"

int builtins_exit(struct ast_builtin *command);
/// @brief
/// @param command
/// @param global
/// @return
int builtins_unset(struct ast_builtin *command, struct internal_state *global);
int cd(struct ast_builtin *command, struct internal_state *internal_state);
int builtins_export(struct ast_builtin *command);
// Builtin functions for the shell
/// @brief
// From the subject, prints on stdout:
// -n inhibits printing a newline.
// -e enable the interpretation of \n, \t and \\ escapes.
// -E disable the interpretation of \n, \t and \\ escapes.
// Flushes stdout at the end.
/// @param command
/// @return
int echo(struct ast_builtin *command);
int builtins_dot(struct ast_builtin *command);

#endif /* ! BUILTINS_H */
