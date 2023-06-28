#ifndef _TOKENS_H_
#define _TOKENS_H_

/* Condition code and operator lists */
#define FORM1 "SUB", "ADD", "MUL", "DIV", "AND", "OR"
#define FORM2 "NOT", "CMP", "MOV"
#define FORM3 "LDR", "STR"
#define FORM4 "LSR", "LSL", "ROR", "ROL"
#define FORM5 "LEA"
#define STACK "PUSH", "POP"
#define EQUIVALENCY "DCD", "EQU"
extern const char *OPERATORS[];
extern const char *FORM1_OPS[];
extern const char *FORM2_OPS[];
extern const char *FORM3_OPS[];
extern const char *FORM4_OPS[];
extern const char *FORM5_OPS[];
extern const char *STACK_OPS[];
extern const char *EQUIVALENCY_OPS[];
extern const char *CONDITION_CODES[];
extern const unsigned short int NUM_OPERATORS;
extern const unsigned short int NUM_FORM1;
extern const unsigned short int NUM_FORM2;
extern const unsigned short int NUM_FORM3;
extern const unsigned short int NUM_FORM4;
extern const unsigned short int NUM_FORM5;
extern const unsigned short int NUM_STACK;
extern const unsigned short int NUM_EQUIVALENCY;
extern const unsigned short int NUM_CONDITIONS;

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
