#include "analyzer.h"
#include "tokens.h"
#include <stdlib.h>
#include <string.h>

/* Analyzer */
Analyzer *analyzer_construct(TokenList *stream) {
    Analyzer *analyzer = malloc(sizeof(Analyzer));
    analyzer->stream = stream;
    analyzer->position = 0;
    analyzer->token = _analyzer_read_token(analyzer);
    return analyzer;
}

void analyzer_destruct(Analyzer *analyzer) {
    token_list_destruct(analyzer->stream); // Will free analyzer->token too
    free(analyzer);
}

Token *_analyzer_read_token(Analyzer *analyzer) {
    Token *next = analyzer->stream->tokens[analyzer->position];
    analyzer->position++;
    return next;
}

/* Identifiers */
ident_t *identifier_construct(char *name, unsigned long location) {
    ident_t *ident = malloc(sizeof(ident_t));
    ident->name = name;
    ident->location = location;
    return ident;
}

void identifier_destruct(ident_t *ident) {
    // Freeing name value will be done by token list
    free(ident);
}

/* Identifier lookup */
ident_node_t *lookup_tree_construct(ident_t *ident) {
    ident_node_t *tree = malloc(sizeof(ident_node_t));
    tree->ident = ident;
    tree->left = NULL;
    tree->right = NULL;
    return tree;
}

void lookup_tree_destruct(ident_node_t *root) {

    if (root == NULL) {
        return;
    }

    // Don't free tokens, they are the responsibility of the token list stream
    lookup_tree_destruct(root->left);
    lookup_tree_destruct(root->right);
    root = NULL;
}

/* Returns true if successful, false if ident was already in the tree. */
bool lookup_tree_insert(ident_node_t *root, char *ident, unsigned long ident_pos) {
    if (root == NULL) {
        ident_t *identifier = identifier_construct(ident, ident_pos);
        root = lookup_tree_construct(identifier); // TODO not sure if this overwrites address value correctly
        return true;
    } else {
        int comp = strcmp(root->ident->name, ident);
        if (comp > 0) {
            return lookup_tree_insert(root->right, ident, ident_pos);
        } else if (comp < 0) {
            return lookup_tree_insert(root->left, ident, ident_pos);
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
        return lookup_tree_get(root->right, ident);
    } else {
        return root->ident;
    }
}
