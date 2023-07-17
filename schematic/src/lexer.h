/* Define the interface for a lexer which tokenizes micro code files */
#ifndef _LEXER_H_
#define _LEXER_H_
#include <stdio.h>
#include <stdbool.h>

/* Tokens */
enum TokenType {
    TokenState,
    TokenNS,
    TokenSignal,
    TokenOpcode,
    TokenEOF,
    TokenIllegal,
};

typedef struct Token {
    enum TokenType type;
    char *name;
} token_t;

token_t *token_construct(char *name, enum TokenType type);
void token_destruct(token_t *token);

/* Lexer */
typedef struct Lexer {
    FILE *stream;
    char character;
} Lexer;

Lexer *lexer_construct(const char *filepath);
void lexer_destruct(Lexer *lexer);

token_t *lexer_next_token(Lexer *lexer);
bool lexer_finished(Lexer *lexer);
void lexer_reset(Lexer *lexer);
#endif // _LEXER_H_
