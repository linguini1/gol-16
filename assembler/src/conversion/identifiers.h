#ifndef _IDENTIFIERS_H_
#define _IDENTIFIERS_H_

#include "tokens.h"
#include <stdbool.h>

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

static ident_node_t *_lookup_tree_construct(ident_t *ident);
ident_node_t *lookup_tree_construct(TokenList *list);
void lookup_tree_destruct(ident_node_t *root);

bool _lookup_tree_insert(ident_node_t **root, ident_t *ident);
ident_t *lookup_tree_get(ident_node_t *root, char *ident);
void in_order_print(ident_node_t *root);
#endif // _IDENTIFIERS_H_
