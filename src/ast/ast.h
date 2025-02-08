#ifndef AST_H
#define AST_H

#include "ast/ast_structs.h"
#include "ast_structs.h"
#include "builtins/builtins.h"
#include "internal_state/internal_state.h"
#include "parser/parser.h"

int evaluate_ast(struct ast *node, struct internal_state *global);

#endif /* ! AST_H */
