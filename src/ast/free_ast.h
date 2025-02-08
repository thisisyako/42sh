#ifndef FREE_AST_H
#define FREE_AST_H

#include "ast/ast_structs.h"

void free_shell_command(struct ast_shell_command *node);
void free_ast(struct ast *node);

#endif /* ! FREE_AST_H */
