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
#define _OPERATORS_                                                                                                    \
    {"SUB", {0x02, 0x12}, Form1}, {"ADD", {0x01, 0x11}, Form1}, {"MUL", {0x03, 0x13}, Form1},                          \
        {"DIV", {0x04, 0x14}, Form1}, {"AND", {0x05, 0x15}, Form1}, {"OR", {0x06, 0x16}, Form1},                       \
        {"NOT", {0x07, 0x17}, Form2}, {"CMP", {0x0A, 0x1A}, Form2}, {"MOV", {0x09, 0x19}, Form2},                      \
        {"LDR", {0x0C, 0x0E, 0x1E}, Form3}, {"STR", {0x1C, 0x0D, 0x1D}, Form3}, {"LSR", {0x08}, Form4},                \
        {"LSL", {0x08}, Form4}, {"ROR", {0x08}, Form4}, {"ROL", {0x08}, Form4}, {"LEA", {0x1B}, Form5},                \
        {"PUSH", {0x00}, FormStack}, {"POP", {0x10}, FormStack}, {"DCD", {}, FormEquiv}, {                             \
        "EQU", {}, FormEquiv                                                                                           \
    }
extern const operator_t OPERATORS[];
extern const char *CONDITION_CODES[];
extern const unsigned int NUM_OPERATORS;
extern const unsigned int NUM_CONDITIONS;

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

TokenList *token_list_construct(unsigned long length);
void token_list_destruct(TokenList *list);

void token_list_append(TokenList *list, Token *token);
Token *token_list_get(TokenList *list, int index);

/* Operator classification */
bool is_conditional(char *ident);
unsigned int _condition_code(char *cc);
#endif // _TOKENS_H_
