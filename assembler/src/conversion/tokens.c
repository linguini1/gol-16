#include "tokens.h"
#include <stdlib.h>

/* Condition code and operator lists */
const operator_t OPERATORS[] = {_OPERATORS_};
const char *CONDITION_CODES[] = {
    "EQ", "NE", "HS", "HI", "LO", "LS", "MI", "PL", "VS", "VC", "GE", "LT", "GT", "LE", "AL",
};
const unsigned int NUM_CONDITIONS = sizeof(CONDITION_CODES) / sizeof(char *);
const unsigned int NUM_OPERATORS = sizeof(OPERATORS) / sizeof(operator_t);

/* Tokens */
Token *token_construct(char *literal, token_t type) {
    Token *token = malloc(sizeof(Token));
    token->literal = literal;
    token->type = type;
    return token;
}

void token_destruct(Token *token) {
    if (token->literal != NULL) {
        free(token->literal);
    }
    free(token);
}

/* Token list */
TokenList *token_list_construct(unsigned int length) {
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
