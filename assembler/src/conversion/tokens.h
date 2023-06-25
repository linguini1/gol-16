#ifndef _TOKENS_H_
#define _TOKENS_H_

/* Condition code and operator lists */
extern const unsigned short int NUM_OPERATORS;
extern const unsigned short int NUM_CONDITIONS;
extern const char *OPERATORS[];
extern const char *CONDITION_CODES[];

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
} Token;

Token *token_construct(char *literal, token_t type);
void token_destruct(Token *token);

/* Token list */
typedef struct TokenList {
    Token **tokens;
    unsigned long length;
    unsigned long __capacity;
} TokenList;

TokenList *token_list_construct(unsigned int length);
void token_list_destruct(TokenList *list);

void token_list_append(TokenList *list, Token *token);
Token *token_list_get(TokenList *list, int index);
#endif // _TOKENS_H_
