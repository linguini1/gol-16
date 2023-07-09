#ifndef _TOKENS_H_
#define _TOKENS_H_
#include <stdbool.h>

typedef enum OperatorForm { Form1, Form2, Form3, Form4, Form5, FormStack, FormEquiv, FormDNE } form_t;

typedef struct Operator {
    char *name;
    unsigned short int raw[3];
    form_t form;
} operator_t;

/* Condition code and operator lists */
extern const operator_t OPERATORS[];
extern const unsigned NUM_OPERATORS;
extern const char *CONDITION_CODES[];
extern const unsigned NUM_CONDITION_CODES;

/* Token types */
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

/* Token */
typedef struct Token {
    char *literal;
    token_t type;
    unsigned long line;
    unsigned long col;
} Token;

Token *token_construct(char *literal, token_t type, unsigned long line, unsigned long col);
void token_destruct(Token *token);

/* Token list */
typedef struct TokenList {
    Token **tokens;
    unsigned long length;
    unsigned long __capacity;
} TokenList;

TokenList *token_list_construct(unsigned long length);
void token_list_destruct(TokenList *list);

void token_list_append(TokenList *list, Token *token);
Token *token_list_get(TokenList *list, int index);

/* Operator classification */
bool is_conditional(char *ident);
unsigned int _condition_code(char *cc);
#endif // _TOKENS_H_
