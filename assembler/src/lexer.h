#include <stdbool.h>
#include <stdio.h>
#ifndef _LEXER_H_

typedef enum TokenT {
    TokenIdentifier,
    TokenRegister,
    TokenOperator,
    TokenHex,
    TokenBin,
    TokenDec,
    TokenChar,
    TokenStr,
    TokenLBrack,
    TokenRBrack,
    TokenLCurl,
    TokenRCurl,
    TokenComma,
    TokenSemicolon,
    TokenPound,
    TokenNewline,
    TokenEOF,
    TokenIllegal,
} TokenT;

typedef struct Token {
    const char *literal;
    TokenT type;
} Token;

typedef struct Lexer {
    FILE *stream;
    long input_length;
    char character;
} Lexer;

bool is_letter(char c);
bool is_num(char c);
bool is_whitespace(char c);

Token *token_construct(char const *literal, TokenT type);
void token_destruct(Token *token);

Lexer *lexer_construct(FILE *fptr);
void lexer_destruct(Lexer *lexer);

static void _lexer_read_char(Lexer *lexer);
static void _lexer_skip_whitespace(Lexer *lexer);
static char* _lexer_read_identifier(Lexer *lexer, size_t *length);

#endif // _LEXER_H_
