#ifndef TOKEN_H
#define TOKEN_H

enum token_type
{
    TOKEN_NONE, // default token when peek or pop not called yet
    TOKEN_WORD,
    TOKEN_SEMICOLON,
    TOKEN_NEWLINE,
    TOKEN_IONUMBER,
    TOKEN_REDIRECTION,
    TOKEN_PIPELINE,
    TOKEN_BANG,
    TOKEN_AND,
    TOKEN_OR,
    TOKEN_ASSIGNMENT_WORD,
    TOKEN_IF,
    TOKEN_FI,
    TOKEN_THEN,
    TOKEN_ELIF,
    TOKEN_ELSE,
    TOKEN_WHILE,
    TOKEN_DO,
    TOKEN_DONE,
    TOKEN_UNTIL,
    TOKEN_FOR,
    TOKEN_IN,
    TOKEN_LEFT_BRACE,
    TOKEN_RIGHT_BRACE,
    TOKEN_LEFT_PAR,
    TOKEN_RIGHT_PAR,
    TOKEN_DOUBLE_COLON,
    TOKEN_CASE,
    TOKEN_EOF,
    TOKEN_ERROR // returned in case of found invalid token
};

struct token
{
    enum token_type type; // The type of the token
    char *value; // If token is a word then the string associated with it
};

#endif /* ! TOKEN_H */
