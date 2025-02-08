#ifndef DICT_H
#define DICT_H

#include "ast/free_ast.h"

struct pair
{
    char *key;
    void *value;
};

struct dict
{
    int num_pairs;
    struct pair **pairs;
};

struct dict *dict_create(void);
void dict_free(struct dict *dict);
void dict_function_free(struct dict *dict);

/// @brief Set a variable's value.
//  Both args do not need to be allocated when passed.
/// @param var_name
/// @param var_value
void dict_set(struct dict *dict, char *key, void *value);

/// @brief Set a function's value.
//  Both args are allocated when passed.
/// @param var_name
/// @param var_value
void dict_function_set(struct dict *dict, char *key, void *value);

/// @brief Removes a function from dict.
/// @param dict
/// @param key
void dict_function_remove(struct dict *dict, char *key);

/// @brief Get a variable's value
/// @param var_name
/// @return No need to free return value.
// Returns NULL if variable does not exist.
void *dict_get(struct dict *dict, char *key);

#endif /* ! DICT_H */
