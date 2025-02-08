#include <criterion/criterion.h>
#include <stdlib.h>
#include <string.h>

#include "ast/ast.h"
#include "expansion/expansion.h"
#include "internal_state/internal_state.h"
#include "io_backend/io_backend.h"

Test(expansion, simple_variable)
{
    struct internal_state *global = internal_state_new();
    set_variable(global, "VAR", "value");
    char *input = strdup("Simple $VAR expansion");
    char *result = expand(input, global);
    cr_assert_str_eq(result, "Simple value expansion",
                     "Failed simple variable expansion. Expected: 'Simple "
                     "value expansion', Got: '%s'",
                     result);
    free(result);
    internal_state_free(global);
}

Test(expansion, undefined_variable)
{
    struct internal_state *global = internal_state_new();
    char *input = strdup("Undefined $UNDEFINED variable");
    char *result = expand(input, global);
    cr_assert_str_eq(result, "Undefined  variable",
                     "Failed undefined variable test. Expected: 'Undefined  "
                     "variable', Got: '%s'",
                     result);
    free(result);
    internal_state_free(global);
}

Test(expansion, empty_variable)
{
    struct internal_state *global = internal_state_new();
    set_variable(global, "EMPTY", "");
    char *input = strdup("Empty $EMPTY variable");
    char *result = expand(input, global);
    cr_assert_str_eq(
        result, "Empty  variable",
        "Failed empty variable test. Expected: 'Empty  variable', Got: '%s'",
        result);
    free(result);
    internal_state_free(global);
}

Test(expansion, adjacent_variables)
{
    struct internal_state *global = internal_state_new();
    set_variable(global, "VAR1", "first");
    set_variable(global, "VAR2", "second");
    char *input = strdup("$VAR1$VAR2");
    char *result = expand(input, global);
    cr_assert_str_eq(result, "firstsecond",
                     "Failed adjacent variables expansion. Expected: "
                     "'firstsecond', Got: '%s'",
                     result);
    free(result);
    internal_state_free(global);
}

Test(expansion, multiple_occurrences)
{
    struct internal_state *global = internal_state_new();
    set_variable(global, "REPEAT", "again");
    char *input = strdup("Expand $REPEAT and $REPEAT");
    char *result = expand(input, global);
    cr_assert_str_eq(result, "Expand again and again",
                     "Failed multiple occurrences of variable. Expected: "
                     "'Expand again and again', Got: '%s'",
                     result);
    free(result);
    internal_state_free(global);
}

Test(expansion, variable_with_numbers)
{
    struct internal_state *global = internal_state_new();
    set_variable(global, "VAR123", "numeric");
    char *input = strdup("$VAR123 is valid");
    char *result = expand(input, global);
    cr_assert_str_eq(
        result, "numeric is valid",
        "Failed variable with numbers. Expected: 'numeric is valid', Got: '%s'",
        result);
    free(result);
    internal_state_free(global);
}

Test(expansion, variable_with_underscores)
{
    struct internal_state *global = internal_state_new();
    set_variable(global, "VAR_UNDERSCORE", "underscore");
    char *input = strdup("Testing $VAR_UNDERSCORE variable");
    char *result = expand(input, global);
    cr_assert_str_eq(result, "Testing underscore variable",
                     "Failed variable with underscores. Expected: 'Testing "
                     "underscore variable', Got: '%s'",
                     result);
    free(result);
    internal_state_free(global);
}

Test(expansion, no_variable_in_text)
{
    struct internal_state *global = internal_state_new();
    char *input = strdup("Text with no variables");
    char *result = expand(input, global);
    cr_assert_str_eq(result, "Text with no variables",
                     "Failed no variable in text. Expected: 'Text with no "
                     "variables', Got: '%s'",
                     result);
    free(result);
    internal_state_free(global);
}

Test(expansion, dollar_alone)
{
    struct internal_state *global = internal_state_new();
    char *input = strdup("A single $ with no variable");
    char *result = expand(input, global);
    cr_assert_str_eq(result, "A single $ with no variable",
                     "Failed single dollar sign. Expected: 'A single $ with no "
                     "variable', Got: '%s'",
                     result);
    free(result);
    internal_state_free(global);
}

Test(expansion, mixed_variables_and_text)
{
    struct internal_state *global = internal_state_new();
    set_variable(global, "VAR1", "Hello");
    set_variable(global, "VAR2", "World");
    char *input = strdup("$VAR1, $VAR2! This is $VAR1$VAR2.");
    char *result = expand(input, global);
    cr_assert_str_eq(result, "Hello, World! This is HelloWorld.",
                     "Failed mixed variables and text. Expected: 'Hello, "
                     "World! This is HelloWorld.', Got: '%s'",
                     result);
    free(result);
    internal_state_free(global);
}

Test(expansion, variable_at_end)
{
    struct internal_state *global = internal_state_new();
    set_variable(global, "ENDVAR", "end");
    char *input = strdup("Variable at the $ENDVAR");
    char *result = expand(input, global);
    cr_assert_str_eq(result, "Variable at the end",
                     "Failed variable at the end of input. Expected: 'Variable "
                     "at the end', Got: '%s'",
                     result);
    free(result);
    internal_state_free(global);
}

Test(expansion, variable_in_quotes)
{
    struct internal_state *global = internal_state_new();
    set_variable(global, "QUOTED", "quoted");
    char *input = strdup("\"This is $QUOTED\"");
    char *result = expand(input, global);
    cr_assert_str_eq(
        result, "This is quoted",
        "Failed variable in quotes. Expected: 'This is quoted', Got: '%s'",
        result);
    free(result);
    internal_state_free(global);
}

Test(expansion, complex_text)
{
    struct internal_state *global = internal_state_new();
    set_variable(global, "USER", "john");
    set_variable(global, "HOST", "localhost");
    char *input = strdup("User: $USER, Host: $HOST, Path: /home/$USER");
    char *result = expand(input, global);
    cr_assert_str_eq(result, "User: john, Host: localhost, Path: /home/john",
                     "Failed complex text expansion. Expected: 'User: john, "
                     "Host: localhost, Path: /home/john', Got: '%s'",
                     result);
    free(result);
    internal_state_free(global);
}
