#ifndef _LEXER_H_
#define _LEXER_H_

#include "tokens.h"
#include <stdbool.h>
#include <stdio.h>

static const char *FILE_SUFFIX = ".orgasm";

/* Lexer */
typedef struct Lexer {
    FILE *stream;
    char character;
    unsigned long line;
    unsigned long col;
    const char *file_path;
} Lexer;

Lexer *lexer_construct(const char *file_path);
void lexer_destruct(Lexer *lexer);

bool lexer_eof(Lexer *lexer);
Token *lexer_next_token(Lexer *lexer);

#endif // _LEXER_H_
