#include "identifiers.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Identifiers */
ident_t *identifier_construct(char *name, unsigned long location) {
    ident_t *ident = malloc(sizeof(ident_t));
    ident->name = name;
    ident->location = location;
    return ident;
}

void identifier_destruct(ident_t *ident) {
    free(ident); // Freeing name value will be done by token list
}

/* Identifier lookup */
static ident_node_t *_lookup_tree_construct(ident_t *ident) {
    ident_node_t *tree = malloc(sizeof(ident_node_t));
    tree->ident = ident;
    tree->left = NULL;
    tree->right = NULL;
    return tree;
}

void lookup_tree_destruct(ident_node_t *root) {
    // Don't free tokens, they are the responsibility of the token list stream
    if (root == NULL) {
        return;
    }
    lookup_tree_destruct(root->left);
    lookup_tree_destruct(root->right);
    root = NULL;
}

ident_node_t *lookup_tree_construct(TokenList *list) {
    ident_node_t *root = NULL;
    unsigned long current_pos = 0;
    for (int i = 0; i < list->length; i++) {
        Token *t = list->tokens[i];

        switch (t->type) {
        case TokenOperator:
            current_pos++;
            break;
        case TokenStr:
            current_pos += _str_literal_len(t->literal) - 1; // Subtract DCD operator offset, all strings follow DCD
            break;
        case TokenIdentifier: {
            // Only record identifier if it precedes an operator (then it is a label, not argument)
            // TODO this will break when MOV R0, identifier precedes any instruction. Should identifiers be passed as
            // arguments this way? Or should it be required to first LEA and then move the register value?
            if (i + 1 < list->length && list->tokens[i + 1]->type == TokenOperator) {
                ident_t *identifier = identifier_construct(t->literal, current_pos);
                _lookup_tree_insert(&root, identifier);
            }
        }
        default:
            continue;
        }
    }
    return root;
}

/* Returns true if successful, false if identifier was already in the tree. */
static bool _lookup_tree_insert(ident_node_t **root, ident_t *ident) {

    if (root == NULL) {
        return false;
    }

    if (*root == NULL) {
        *root = _lookup_tree_construct(ident);
        return true;
    } else {
        int comp = strcmp((*root)->ident->name, ident->name);
        if (comp > 0) {
            return _lookup_tree_insert(&((*root)->right), ident);
        } else if (comp < 0) {
            return _lookup_tree_insert(&((*root)->left), ident);
        } else {
            return false; // identifier already in tree
        }
    }
}

ident_t *lookup_tree_get(ident_node_t *root, char *ident) {
    if (root == NULL) {
        return NULL;
    }

    int comp = strcmp(root->ident->name, ident);
    if (comp > 0) {
        return lookup_tree_get(root->right, ident);
    } else if (comp < 0) {
        return lookup_tree_get(root->left, ident);
    } else {
        return root->ident;
    }
}

// TODO remove this
void in_order_print(ident_node_t *root) {
    if (root == NULL) {
        return;
    }

    printf("%s\n", root->ident->name);
    in_order_print(root->left);
    in_order_print(root->right);
}

/* Progressing position */
static unsigned int _str_literal_len(char *literal) {
    unsigned int length = 0;
    while (*literal != '\0') {
        literal += *literal == '\\'; // Skip one char if escape found
        literal++;
        length++;
    }
    length += (length % 2 == 0) + 1; // Round up to nearest instruction length (2 bytes)
    return length / 2;
}
