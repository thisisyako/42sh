#ifndef INTERNAL_STATE_H
#define INTERNAL_STATE_H

#include "ast/ast_structs.h"
#include "dict/dict.h"

struct internal_state
{
    char **positional_params;
    int param_number;
    int return_code;
    struct dict *functions;
    struct dict *variables;
};

struct internal_state *internal_state_new(void);
struct internal_state *get_internal_state(void);
void internal_state_free(struct internal_state *global);
void set_return_code(int return_code, struct internal_state *global);
void set_params(struct internal_state *global, int argc, char *argv[],
                int mode);
void set_variable(struct internal_state *global, char *key, char *value);
void set_function(struct internal_state *global, char *name,
                  struct ast_shell_command *function);
void remove_function(struct internal_state *global, char *name);
char *get_variable(struct internal_state *global, char *key);
struct ast_shell_command *get_function(struct internal_state *global,
                                       char *name);

#endif /* ! INTERNAL_STATE_H */
