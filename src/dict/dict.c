#include "dict.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

struct dict *dict_create(void)
{
    struct dict *dict = calloc(1, sizeof(struct dict));
    assert(dict != NULL && "calloc returned NULL");

    dict->pairs = NULL;

    return dict;
}

void dict_free(struct dict *dict)
{
    assert(dict != NULL && "dict is NULL");
    for (int i = 0; i < dict->num_pairs; i++)
    {
        free(dict->pairs[i]->key);
        free(dict->pairs[i]->value);
        free(dict->pairs[i]);
    }
    free(dict->pairs);
    free(dict);
}

void dict_function_free(struct dict *dict)
{
    assert(dict != NULL && "dict is NULL");
    for (int i = 0; i < dict->num_pairs; i++)
    {
        free(dict->pairs[i]->key);
        free_shell_command(dict->pairs[i]->value);
        free(dict->pairs[i]);
    }
    free(dict->pairs);
    free(dict);
}

void dict_set(struct dict *dict, char *key, void *value)
{
    assert(dict != NULL && "dict is NULL");
    assert(key != NULL && "key is NULL");
    assert(value != NULL && "value is NULL");

    // Allocate and copy value
    char *allocated_value = malloc(strlen(value) + 1);
    assert(allocated_value != NULL && "malloc returned NULL");
    strcpy(allocated_value, value);

    // Check if key exists and update value
    for (int i = 0; i < dict->num_pairs; i++)
    {
        if (strcmp(dict->pairs[i]->key, key) == 0)
        {
            free(dict->pairs[i]->value);
            dict->pairs[i]->value = allocated_value;
            return;
        }
    }

    // Allocate and copy key
    char *allocated_key = malloc(strlen(key) + 1);
    assert(allocated_key != NULL && "malloc returned NULL");
    strcpy(allocated_key, key);

    // Extend the dict
    dict->num_pairs++;
    struct pair **new_pairs =
        realloc(dict->pairs, dict->num_pairs * sizeof(struct pair *));
    assert(new_pairs != NULL && "realloc returned NULL");
    dict->pairs = new_pairs;

    // Create new pair
    struct pair *new_pair = malloc(sizeof(struct pair));
    assert(new_pair != NULL && "malloc returned NULL");
    new_pair->key = allocated_key;
    new_pair->value = allocated_value;

    dict->pairs[dict->num_pairs - 1] = new_pair;
}

void dict_function_set(struct dict *dict, char *key, void *value)
{
    assert(dict != NULL && "dict is NULL");
    assert(key != NULL && "key is NULL");
    assert(value != NULL && "value is NULL");

    // Check if key exists and update value
    for (int i = 0; i < dict->num_pairs; i++)
    {
        if (strcmp(dict->pairs[i]->key, key) == 0)
        {
            free_shell_command(dict->pairs[i]->value);
            free(key);
            dict->pairs[i]->value = value;
            return;
        }
    }

    ++dict->num_pairs;
    struct pair **new_pairs =
        realloc(dict->pairs, dict->num_pairs * sizeof(struct pair *));
    assert(new_pairs != NULL && "realloc returned NULL");
    dict->pairs = new_pairs;

    struct pair *new_pair = malloc(sizeof(struct pair));
    assert(new_pair != NULL && "malloc returned NULL");
    new_pair->key = key;
    new_pair->value = value;

    dict->pairs[dict->num_pairs - 1] = new_pair;
}

void dict_function_remove(struct dict *dict, char *key)
{
    assert(dict != NULL && "dict is NULL");
    assert(key != NULL && "key is NULL");

    for (int i = 0; i < dict->num_pairs; i++)
    {
        if (strcmp(dict->pairs[i]->key, key) == 0)
        {
            free(dict->pairs[i]->key);
            free_shell_command(dict->pairs[i]->value);
            free(dict->pairs[i]);

            for (int j = i; j < dict->num_pairs - 1; j++)
            {
                dict->pairs[j] = dict->pairs[j + 1];
            }

            dict->num_pairs--;
            struct pair **new_pairs =
                realloc(dict->pairs, dict->num_pairs * sizeof(struct pair *));
            assert(new_pairs != NULL || dict->num_pairs == 0);
            dict->pairs = new_pairs;

            return;
        }
    }
}

void *dict_get(struct dict *dict, char *key)
{
    assert(dict != NULL && "dict is NULL");
    assert(key != NULL && "key is NULL");

    for (int i = 0; i < dict->num_pairs; i++)
    {
        if (strcmp(dict->pairs[i]->key, key) == 0)
            return dict->pairs[i]->value;
    }
    return getenv(key);
}
