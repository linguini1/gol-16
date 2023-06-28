#ifndef _TOKENS_H_
#define _TOKENS_H_

typedef struct Operator {
    char *name;
    unsigned short int raw[3];
} operator_t;

/* Condition code and operator lists */
#define FORM1                                                                                                          \
    {"SUB", {0x02, 0x12}}, {"ADD", {0x01, 0x11}}, {"MUL", {0x03, 0x13}}, {"DIV", {0x04, 0x14}}, {"AND", {0x05, 0x15}}, \
    {                                                                                                                  \
        "OR", { 0x06, 0x16 }                                                                                           \
    }
#define FORM2                                                                                                          \
    {"NOT", {0x07, 0x17}}, {"CMP", {0x0A, 0x1A}}, {                                                                    \
        "MOV", { 0x09, 0x19 }                                                                                          \
    }
#define FORM3                                                                                                          \
    {"LDR", {0x0C, 0x0E, 0x1E}}, {                                                                                     \
        "STR", { 0x1C, 0x0D, 0x1D }                                                                                    \
    }
#define FORM4                                                                                                          \
    {"LSR", {0x08}}, {"LSL", {0x08}}, {"ROR", {0x08}}, {                                                               \
        "ROL", { 0x08 }                                                                                                \
    }
#define FORM5                                                                                                          \
    {                                                                                                                  \
        "LEA", { 0x1B }                                                                                                \
    }
#define STACK                                                                                                          \
    {"PUSH", {0x00}}, {                                                                                                \
        "POP", { 0x10 }                                                                                                \
    }
#define EQUIVALENCY {"DCD", {}}, {"EQU", {}}
extern const operator_t OPERATORS[];
extern const operator_t FORM1_OPS[];
extern const operator_t FORM2_OPS[];
extern const operator_t FORM3_OPS[];
extern const operator_t FORM4_OPS[];
extern const operator_t FORM5_OPS[];
extern const operator_t STACK_OPS[];
extern const operator_t EQUIVALENCY_OPS[];
extern const char *CONDITION_CODES[];
extern const unsigned int NUM_OPERATORS;
extern const unsigned int NUM_FORM1;
extern const unsigned int NUM_FORM2;
extern const unsigned int NUM_FORM3;
extern const unsigned int NUM_FORM4;
extern const unsigned int NUM_FORM5;
extern const unsigned int NUM_STACK;
extern const unsigned int NUM_EQUIVALENCY;
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

TokenList *token_list_construct(unsigned int length);
void token_list_destruct(TokenList *list);

void token_list_append(TokenList *list, Token *token);
Token *token_list_get(TokenList *list, int index);
#endif // _TOKENS_H_
