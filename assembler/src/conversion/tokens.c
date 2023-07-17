#include "tokens.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

/* Condition code and operator lists */
const operator_t OPERATORS[] = {{"ADD", {0x01, 0x11}, Form1},
                                {"SUB", {0x02, 0x12}, Form1},
                                {"MUL", {0x03, 0x13}, Form1},
                                {"DIV", {0x04, 0x14}, Form1},
                                {"AND", {0x05, 0x15}, Form1},
                                {"OR", {0x06, 0x16}, Form1},
                                {"NOT", {0x07, 0x17}, Form2},
                                {"CMP", {0x0A, 0x1A}, Form2},
                                {"MOV", {0x09, 0x19}, Form2},
                                {"LDR", {0x0C, 0x0E, 0x1E}, Form3},
                                {"STR", {0x1C, 0x0D, 0x1D}, Form3},
                                {"LSR", {0x08}, Form4},
                                {"LSL", {0x08}, Form4},
                                {"ROR", {0x08}, Form4},
                                {"ROL", {0x08}, Form4},
                                {"LEA", {0x1B}, Form5},
                                {"PUSH", {0x00}, FormStack},
                                {"POP", {0x10}, FormStack},
                                {"DCD", {0}, FormEquiv},
                                {"EQU", {0}, FormEquiv}};
const unsigned NUM_OPERATORS = sizeof(OPERATORS) / sizeof(operator_t);
const char *CONDITION_CODES[] = {
    "EQ", "NE", "HS", "HI", "LO", "LS", "MI", "PL", "VS", "VC", "GE", "LT", "GT", "LE", "AL",
};
const unsigned NUM_CONDITION_CODES = sizeof(CONDITION_CODES) / sizeof(char *);

/* Tokens */
Token *token_construct(char *literal, token_t type, unsigned long line, unsigned long col) {
    Token *token = malloc(sizeof(Token));
    token->literal = literal;
    token->type = type;
    token->line = line;
    token->col = col;
    return token;
}

void token_destruct(Token *token) {
    if (token->literal != NULL) {
        free(token->literal);
    }
    free(token);
}

/* Token list */
TokenList *token_list_construct(unsigned long length) {
    TokenList *list = malloc(sizeof(TokenList));
    list->length = 0;
    list->__capacity = length;

    if (length == 0) {
        list->tokens = NULL;
    } else {
        list->tokens = malloc(sizeof(Token *) * list->__capacity);
    }

    return list;
}

void token_list_destruct(TokenList *list) {
    for (int i = 0; i < list->length; i++) {
        free(list->tokens[i]);
    }
    free(list);
}

void token_list_append(TokenList *list, Token *token) {
    if (list->length == list->__capacity) {
        list->__capacity = 2 * list->__capacity;
        Token **new_arr = malloc(sizeof(Token *) * list->__capacity);

        // Copy over tokens
        for (int i = 0; i < list->length; i++) {
            new_arr[i] = list->tokens[i];
        }

        // Replace old list
        free(list->tokens);
        list->tokens = new_arr;
    }

    list->tokens[list->length] = token;
    list->length++;
}

/* This function supports negative numbers for the index parameter to support indexing from the rear (-1 is the last
 * element in the list).
 */
Token *token_list_get(TokenList *list, int index) {

    // Support for negative indexing
    if (index < 0) {
        index = list->length + index;
    }

    if (!(index < list->length) || index < 0) {
        return NULL;
    }
    return list->tokens[index];
}

/* Utility functions */
void string_to_uppercase(char *string) {
    while (*string) {
        *string = toupper(*string);
        string++;
    }
}

/* Operator classification */
bool is_conditional(char *ident) {

    if (ident == NULL) {
        return false;
    }

    // Create uppercase copy of identifier
    size_t length = strlen(ident);
    char *upr_ident = malloc(length + 1);
    strcpy(upr_ident, ident);
    string_to_uppercase(upr_ident);

    if (length == 0) {
        free(upr_ident);
        return false;
    }

    bool starts_b = upr_ident[0] == 'B';

    if (length == 1 && starts_b) {
        free(upr_ident);
        return true; // B always case
    }

    if (length == 2 && starts_b && ident[1] == 'L') {
        free(upr_ident);
        return true; // BL always case
    }

    // Contains a condition code
    char *condition;
    if (length == 3 && starts_b) {
        condition = ident + 1; // Bcc
    } else if (length == 4) {
        condition = ident + 2; // BLcc
    } else {
        free(upr_ident);
        return false;
    }

    // Check if condition code is valid
    for (size_t i = 0; i < NUM_CONDITION_CODES; i++) {
        if (!strcmp(condition, CONDITION_CODES[i])) {
            free(upr_ident);
            return true;
        }
    }

    free(upr_ident);
    return false;
}

unsigned int _condition_code(char *cc) {
    for (unsigned int i = 0; i < NUM_CONDITION_CODES; i++) {
        if (!strcmp(cc, CONDITION_CODES[i])) {
            return i;
        }
    }
    return 32; // Randomly selected number which exceeds allowable condition codes
}
