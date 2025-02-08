#ifndef IO_BACKEND_H
#define IO_BACKEND_H

#include <stdbool.h>
#include <stdio.h>

#include "internal_state/internal_state.h"
#include "lexer/lexer.h"

// Methods that handle taking 3 different types of input.
// 1.file
// 2.string
// 3.stdin

/// @brief Takes a file name and converts it to a file descriptor.
/// @param file_name
/// @param file
/// @return Bool, indicating if operation succeeded.
bool get_file(const char *file_name, FILE **file);

/// @brief Takes a string and converts it to a file descriptor.
/// @param string
/// @param file
/// @return Bool, indicating if operation succeeded.
bool get_string(const char *string, FILE **file);

/// @brief Sets the file arguent to stdin.
/// @param file
/// @return Bool, indicating if operation succeeded.
bool get_stdin(FILE **file);

/// @brief Evaluates the input stream.
/// @param lexer
/// @return The return code of evaluation.
int evaluate_input(struct lexer *lexer, struct internal_state *global);

#endif /* ! IO_BACKEND_H */
