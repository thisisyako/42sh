#define _POSIX_C_SOURCE 200809L

#include "expansion.h"

#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

struct expand_arguments
{
    char *input;
    size_t *i;
    char **output;
    size_t *j;
    size_t *output_size;
    int with_braces;
    int in_single_quotes;
    int in_double_quotes;
};

static int is_special_var_char(char c)
{
    switch (c)
    {
    case '@':
    case '?':
    case '*':
    case '$':
    case '#':
        return 1;
    default:
        return 0;
    }
}

static int is_valid_char(char c)
{
    return isalnum(c) || c == '_' || is_special_var_char(c) || c == '{'
        || c == '(';
}

static void add_to_output(struct expand_arguments args, char *var_value)
{
    size_t *j = args.j;
    size_t *output_size = args.output_size;
    char **output = args.output;

    size_t value_len = strlen(var_value);
    if (*j + value_len >= *output_size)
    {
        *output_size = *output_size + value_len + 1024;
        *output = realloc(*output, *output_size);
        assert(*output);
    }
    strcpy(*output + *j, var_value);
    *j += value_len;
}

// pour l'instant on handle pas et tant pis pour le moment
static char *handle_dollar_star(struct internal_state *global)
{
    size_t total_len = 0;
    for (size_t i = 1; global->positional_params[i]; ++i)
        total_len += strlen(global->positional_params[i]);
    char *buffer = calloc(total_len + 1 + global->param_number, sizeof(char));
    strcat(buffer, global->positional_params[1]);
    for (size_t i = 2; global->positional_params[i]; ++i)
    {
        strcat(buffer, " ");
        strcat(buffer, global->positional_params[i]);
    }
    return buffer;
}

static void handle_special_var(struct expand_arguments args,
                               struct internal_state *global)
{
    char *input = args.input;
    size_t *i = args.i;
    int is_allocated = 0;

    const char *var_value = NULL;
    char buffer[256];
    switch (input[*i])
    {
    case '@':
        return;
    case '*':
        if (args.in_double_quotes)
        {
            var_value = handle_dollar_star(global);
            is_allocated = 1;
        }
        break;
    case '#':
        sprintf(buffer, "%d", global->param_number);
        var_value = buffer;
        break;
    case '?':
        sprintf(buffer, "%d", global->return_code);
        var_value = buffer;
        break;
    default: // $
        sprintf(buffer, "%d", getpid());
        var_value = buffer;
        break;
    }

    if (var_value)
    {
        add_to_output(args, (char *)var_value);
        if (is_allocated)
            free((char *)var_value);
    }
    (*i)++;
    if (args.with_braces)
        (*i)++;
}

static void handle_positional_params(struct expand_arguments args,
                                     struct internal_state *global,
                                     char *var_name)
{
    char *input = args.input;
    size_t *i = args.i;
    int with_braces = args.with_braces;

    const char *var_value = NULL;
    char buffer[256] = { '\0' };
    if (var_name == NULL)
    {
        if (input[*i] - '0' <= global->param_number
            && global->positional_params != NULL)
        {
            sprintf(buffer, "%s", global->positional_params[input[*i] - '0']);
        }
        else
            buffer[0] = '\0';
        var_value = buffer;
    }
    else
    {
        int converted = atoi(var_name);
        if (converted < global->param_number)
            sprintf(buffer, "%s", global->positional_params[converted]);
        var_value = buffer;
    }

    add_to_output(args, (char *)var_value);
    (*i)++;
    if (with_braces)
        (*i)++;
}

static const char *expansion_handle_var_value(char *var_name, char *randd,
                                              struct internal_state *global)
{
    if (strcmp(var_name, "RANDOM") == 0)
    {
        sprintf(randd, "%d", (int)((float)rand() / (float)RAND_MAX * 32767));
        return randd;
    }
    else if (strcmp(var_name, "UID") == 0)
    {
        sprintf(randd, "%d", getuid());
        return randd;
    }
    else
    {
        return get_variable(global, var_name);
    }
}

static int evaluate_new_process(char *sub_value, struct internal_state *global)
{
    FILE *file = fmemopen(sub_value, strlen(sub_value), "r");
    struct lexer *lexer = lexer_new(file, 0);
    int return_code = 0;
    while (lexer->current_tok.type != TOKEN_EOF)
    {
        return_code = evaluate_input(lexer, global);
    }

    lexer_free(lexer);
    internal_state_free(global);
    return return_code;
}

char *append_to_string(char *original, const char *new_data,
                       size_t new_data_size)
{
    size_t original_len = original ? strlen(original) : 0;
    size_t new_size = original_len + new_data_size + 1;

    char *new_string = realloc(original, new_size);
    if (!new_string)
    {
        perror("realloc");
        free(original);
        exit(EXIT_FAILURE);
    }

    memcpy(new_string + original_len, new_data, new_data_size);
    new_string[original_len + new_data_size] = '\0';

    return new_string;
}

static char *handle_read_pipe(int pipefd)
{
    char buffer[1024];
    char *result = NULL;
    ssize_t bytes_read;

    while ((bytes_read = read(pipefd, buffer, sizeof(buffer))) > 0)
        result = append_to_string(result, buffer, bytes_read);

    if (bytes_read == -1)
    {
        free(result);
        return NULL;
    }

    return result;
}

static void trim_string(char *str, struct expand_arguments args)
{
    size_t i = strlen(str) - 1;
    while (str[i] == '\0' || str[i] == '\n' || str[i] == ' ' || str[i] == '\t')
    {
        --i;
    }
    str[i + 1] = '\0';
    i = 0;
    if (!args.in_double_quotes)
    {
        while (str[i] != '\0')
        {
            if (str[i] == '\n' || str[i] == '\t')
                str[i] = ' ';
            ++i;
        }
    }
}

static void handle_new_process(struct expand_arguments args, char *sub_value,
                               struct internal_state *global)
{
    int pipefd[2];
    assert(pipe(pipefd) != -1 && "Failed pipe in command substitution");
    int stdout_save = dup(STDOUT_FILENO);
    assert(dup2(pipefd[1], STDOUT_FILENO) != -1 && "Failed dup2");
    close(pipefd[1]);

    int pid = fork();
    if (pid == 0) // Child process, subshell process
    {
        global = get_internal_state();
        close(pipefd[0]);
        int res = evaluate_new_process(sub_value, global);
        assert(dup2(stdout_save, STDOUT_FILENO) != -1 && "Failed dup2");
        close(stdout_save);
        free(sub_value);
        exit(res);
    }
    else
    {
        int status;
        waitpid(pid, &status, 0);
        assert(dup2(stdout_save, STDOUT_FILENO) != -1 && "Failed dup2");
        close(stdout_save);
        char *result = handle_read_pipe(pipefd[0]);
        if (result)
            trim_string(result, args);
        close(pipefd[0]);
        if (result)
        {
            add_to_output(args, result);
        }
        free(result);
        free(sub_value);
        return;
    }
}

static void handle_command_substitution(struct expand_arguments args,
                                        struct internal_state *global,
                                        char type)
{
    char match = type == '(' ? ')' : '`';
    int escaped = 0;
    int in_par = 0;
    size_t len = 128;
    size_t i = 0;
    char *sub_value = calloc(len, sizeof(char));
    char c;
    (*args.i)++;

    while ((c = args.input[*args.i])
           && (c != match || escaped == 1 || in_par == 1))
    {
        escaped = 0;
        if (c == '\\')
            escaped = 1;
        if (c == '(' || c == ')')
            in_par = !in_par;
        sub_value[i++] = c;
        (*args.i)++;
        if (i + 1 >= len)
        {
            sub_value = realloc(sub_value, (len * 2) * sizeof(char));
            len *= 2;
        }
    }
    (*args.i)++;
    sub_value[i] = '\0';
    handle_new_process(args, sub_value, global);
}

static void handle_var(struct expand_arguments args,
                       struct internal_state *global)
{
    char *input = args.input;
    size_t *i = args.i;
    int with_braces = args.with_braces;

    char var_name[256] = { 0 };
    size_t var_len = 0;
    (*i)++;

    if (with_braces)
        (*i)++;

    if (is_special_var_char(input[*i]))
    {
        handle_special_var(args, global);
        return;
    }
    else if (isdigit(input[*i]) && !isdigit(input[(*i) + 1]))
    {
        handle_positional_params(args, global, NULL);
        return;
    }
    if (input[*i] == '(')
    {
        handle_command_substitution(args, global, '(');
        return;
    }

    if (with_braces)
    {
        while (input[*i] && input[*i] != '}' && var_len < 255)
            var_name[var_len++] = input[(*i)++];
        (*i)++;
        int only_digit = 1;
        for (size_t k = 0; var_name[k]; ++k)
        {
            if (!isdigit(var_name[k]))
                only_digit = 0;
        }

        if (only_digit)
        {
            handle_positional_params(args, global, var_name);
            return;
        }
    }
    else
    {
        while (input[*i] && (isalnum(input[*i]) || input[*i] == '_')
               && var_len < 255)
            var_name[var_len++] = input[(*i)++];
    }
    const char *var_value;
    char randd[6];
    var_value = expansion_handle_var_value(var_name, randd, global);
    if (var_value)
    {
        add_to_output(args, (char *)var_value);
    }
}

char *expand(char *input, struct internal_state *global)
{
    size_t output_size = strlen(input ? input : "") + 1;
    char *output = malloc(output_size);
    size_t j = 0;
    int in_single_quotes = 0;
    int in_double_quotes = 0;

    for (size_t i = 0; input && input[i];)
    {
        if (!in_single_quotes && !in_double_quotes && input[i] == '\\'
            && input[i + 1])
        {
            output[j++] = input[i + 1];
            i += 2;
        }
        else if (input[i] == '"' || input[i] == '\'')
        {
            if (input[i++] == '"')
                in_double_quotes = !in_double_quotes;
            else
                in_single_quotes = !in_single_quotes;
        }
        else if (!in_single_quotes
                 && ((input[i] == '$' && is_valid_char(input[i + 1]))
                     || input[i] == '`'))
        {
            struct expand_arguments args = { .with_braces = input[i + 1] == '{',
                                             .input = input,
                                             .output_size = &output_size,
                                             .output = &output,
                                             .i = &i,
                                             .j = &j };
            args.in_single_quotes = in_single_quotes;
            args.in_double_quotes = in_double_quotes;
            if (input[i] == '`')
                handle_command_substitution(args, global, '`');
            else
                handle_var(args, global);
        }
        else
            output[j++] = input[i++];
        if (j >= output_size - 1)
        {
            output_size += 1024;
            output = realloc(output, output_size);
        }
    }
    free(input);
    output[j] = '\0';
    return output;
}

static void handle_dollar_at(char ***args, size_t *args_pos,
                             struct internal_state *global)
{
    char *original_arg = strdup((*args)[*args_pos]);
    free((*args)[*args_pos]);
    char *prefix = strtok(original_arg, "$@");
    char *suffix = strdup(original_arg + (prefix ? strlen(prefix) : 0) + 2);
    if (prefix)
    {
        prefix = expand(prefix, global);
    }
    if (suffix)
        suffix = expand(suffix, global);

    size_t len = strlen(global->positional_params[1]);
    prefix = realloc(prefix, (len + 2 + strlen(prefix)));
    strcat(prefix, global->positional_params[1]);
    (*args)[(*args_pos)++] = prefix;

    size_t number_args = 0;
    for (size_t i = 0; (*args)[i]; ++i)
        ++number_args;

    size_t positional_count = global->param_number - 1;
    size_t new_args_count = number_args + positional_count;
    *args = realloc(*args, (new_args_count + 1) * sizeof(char *));
    assert(*args);

    for (size_t i = number_args; i >= *args_pos; --i)
    {
        if ((*args)[i])
        {
            (*args)[i + positional_count] = strdup((*args)[i]);
            free((*args)[i]);
        }
        else
            (*args)[i + positional_count] = (*args)[i];
    }

    for (int i = 2; i < global->param_number; ++i)
    {
        (*args)[(*args_pos)++] = strdup(global->positional_params[i]);
    }

    char *last_param = strdup(global->positional_params[global->param_number]);
    len = strlen(suffix);
    last_param = realloc(last_param, strlen(last_param) + len + 1);
    strcat(last_param, suffix);
    free(suffix);
    last_param = expand(last_param, global);
    (*args)[(*args_pos)] = last_param;
}

static char **field_split_pos_params(struct internal_state *global, char c)
{
    char **res = NULL;
    size_t arg_count = 0;

    size_t i = 1;
    size_t offset = 0;
    char *str;
    while ((str = global->positional_params[i]) != NULL)
    {
        for (size_t j = 0; str[j]; ++j)
        {
            if (str[j] == c)
            {
                str[j] = '\0';
                res = realloc(res, (arg_count + 2) * sizeof(char *));
                res[arg_count++] = strdup(str);
                offset = j + 1;
                str[j] = c;
            }
        }
        res = realloc(res, (arg_count + 2) * sizeof(char *));
        res[arg_count++] = strdup(str + offset);
        offset = 0;
        ++i;
    }

    res[arg_count] = NULL;
    return res;
}

static void free_pos_params(char **pos_params)
{
    for (size_t i = 0; pos_params[i]; ++i)
    {
        free(pos_params[i]);
    }
    free(pos_params);
}

static void handle_prefix(char **prefix, size_t len, size_t pref_len, char *str)
{
    *prefix = realloc(*prefix, len);
    (*prefix)[pref_len] = '\0';
    strcat(*prefix, str);
}

static void handle_both_dollars(char ***args, size_t *args_pos,
                                struct internal_state *global,
                                const char *pattern)
{
    char *original_arg = strdup((*args)[*args_pos]);
    free((*args)[*args_pos]);
    char *prefix = strtok(original_arg, pattern);
    if (!prefix || (prefix && prefix[0] != original_arg[0]))
    {
        original_arg[0] = '\0';
        prefix = original_arg;
    }
    char *suffix = strdup(original_arg + strlen(prefix) + 2);
    prefix = expand(prefix, global);
    suffix = expand(suffix, global);

    char **ifs_positional_params = field_split_pos_params(global, ' ');
    size_t pref_len = strlen(prefix);

    size_t len = strlen(ifs_positional_params[0]) + 2 + pref_len;
    handle_prefix(&prefix, len, pref_len, ifs_positional_params[0]);
    (*args)[(*args_pos)++] = prefix;

    size_t number_args = 0;
    for (size_t i = 0; (*args)[i]; ++i)
        ++number_args;

    size_t positional_count = 0;
    for (size_t i = 0; ifs_positional_params[i]; ++i)
        ++positional_count;

    size_t new_args_count = number_args + --positional_count;
    *args = realloc(*args, (new_args_count + 1) * sizeof(char *));
    assert(*args);

    for (size_t i = number_args; i >= *args_pos; --i)
    {
        if ((*args)[i])
        {
            (*args)[i + positional_count] = strdup((*args)[i]);
            free((*args)[i]);
        }
        else
            (*args)[i + positional_count] = (*args)[i];
    }

    for (size_t i = 1; i < positional_count; ++i)
    {
        (*args)[(*args_pos)++] = strdup(ifs_positional_params[i]);
    }

    char *last_param = strdup(ifs_positional_params[positional_count]);
    len = strlen(suffix);
    last_param = realloc(last_param, strlen(last_param) + len + 1);
    strcat(last_param, suffix);
    last_param = expand(last_param, global);
    (*args)[(*args_pos)] = last_param;

    free(suffix);
    free_pos_params(ifs_positional_params);
}

static int is_quoted(char *str, const char pattern[3], char quote)
{
    int quoted = 0;

    size_t i = 0;
    while (str[i] != pattern[0] && str[i + 1] && str[i + 1] != pattern[1])
    {
        if (str[i] == quote)
            quoted = !quoted;
        ++i;
    }
    return quoted;
}

char **expand_args(char **args, struct internal_state *global)
{
    for (size_t i = 0; args[i]; ++i)
    {
        if (strstr(args[i], "$@") != NULL && strstr(args[i], "\\$@") == NULL)
        {
            if (is_quoted(args[i], "$@", '"'))
            {
                handle_dollar_at(&args, &i, global);
                continue;
            }
            else if (!is_quoted(args[i], "$@", '\''))
            {
                handle_both_dollars(&args, &i, global, "$@");
                continue;
            }
        }
        else if (strstr(args[i], "$*") != NULL
                 && strstr(args[i], "\\$*") == NULL)
        {
            if (is_quoted(args[i], "$*", '"'))
            {
                args[i] = expand(args[i], global);
                continue;
            }
            else if (!is_quoted(args[i], "$*", '\''))
            {
                handle_both_dollars(&args, &i, global, "$*");
                continue;
            }
        }
        args[i] = expand(args[i], global);
    }
    return args;
}
