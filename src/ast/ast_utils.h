#ifndef AST_UTILS_H
#define AST_UTILS_H

#include "internal_state/internal_state.h"

void update_environment(const char *name, const char *value);
void assign_variable(struct internal_state *global, char *str);

#endif /* ! AST_UTILS_H */
