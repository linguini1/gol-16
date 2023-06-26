#ifndef _ANALYZER_H_
#define _ANALYZER_H_
#include "tokens.h"
#include <stdbool.h>

/* Analyzer */
typedef struct Analyzer {
    TokenList *stream;
    unsigned long position;
    Token *token;
} Analyzer;

Analyzer *analyzer_construct(TokenList *stream);
void analyzer_destruct(Analyzer *analyzer);

Token *_analyzer_read_token(Analyzer *analyzer);

/* Identifiers */
typedef struct identifier {
    char *name;
    unsigned long location;
} ident_t;

ident_t *identifier_construct(char *name, unsigned long location);
void identifier_destruct(ident_t *ident);

/* Identifier lookup */
typedef struct identifier_node {
    ident_t *ident;
    struct identifier_node *left;
    struct identifier_node *right;
} ident_node_t;

ident_node_t *lookup_tree_construct(ident_t *token);
void lookup_tree_destruct(ident_node_t *root);

bool lookup_tree_insert(ident_node_t *root, char *ident, unsigned long ident_pos);
ident_t *lookup_tree_get(ident_node_t *root, char *ident);

#endif // _ANALYZER_H_
