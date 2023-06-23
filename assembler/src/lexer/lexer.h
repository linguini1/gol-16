#ifndef _LEXER_H_
#define _LEXER_H_

#include <stdbool.h>
#include <stdio.h>

#define NUM_OPERATORS 20
#define NUM_CONDITIONS 14
static const char *OPERATORS[] = {
    "DCD", "EQU", "PUSH", "POP", "SUB", "ADD", "MUL", "DIV", "AND", "OR",
    "NOT", "LSR", "LSL",  "ROR", "ROL", "MOV", "CMP", "LDR", "LEA", "STR",
};
static const char *CONDITION_CODES[] = {
    "EQ", "NE", "HS", "HI", "LO", "LS", "MI", "PL", "VS", "VC", "GE", "LT", "GT", "LE", "AL",
};
static const char *FILE_SUFFIX = ".orgasm";

typedef enum token_type {
    TokenIdentifier,
    TokenOperator,
    TokenRegister,
    TokenSpecialRegister,
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
    TokenStart,
    TokenEOF,
    TokenIllegal,
} token_t;

/* File type verification */
bool is_orgasm_file(char *filename);

/* Token */
typedef struct Token {
    char *literal;
    token_t type;
} Token;

Token *token_construct(char *literal, token_t type);
void token_destruct(Token *token);

/* Lexer */
typedef struct Lexer {
    FILE *stream;
    char character;
} Lexer;

Lexer *lexer_construct(FILE *fptr);
void lexer_destruct(Lexer *lexer);

/* Character classification */
static char* _struprcpy(char *ident);
static bool is_letter(char c);
static bool is_num(char c);
static bool is_whitespace(char c);
static bool is_bin(char c);
static bool is_hex(char c);
static bool is_operator(char *ident);
static bool is_register(char *ident);
static bool is_special_register(char *ident);

Token *lexer_next_token(Lexer *lexer);

/* Lexer helper internals */
static char *_lexer_slice(Lexer *lexer, long start);
static char _lexer_peek(Lexer *lexer);
static void _lexer_read_char(Lexer *lexer);

/* Lexer skipping */
static void _lexer_skip_whitespace(Lexer *lexer);
static void _lexer_skip_comment(Lexer *lexer);

/* Lexer reading tokens */
static char *_lexer_read_identifier(Lexer *lexer);

static char *_lexer_read_numeric_literal(Lexer *lexer, token_t *type);
static char *_lexer_read_decimal_literal(Lexer *lexer);
static char *_lexer_read_hex_literal(Lexer *lexer);
static char *_lexer_read_bin_literal(Lexer *lexer);

static char *_lexer_read_string_literal(Lexer *lexer);
static char *_lexer_read_char_literal(Lexer *lexer);

#endif // _LEXER_H_
