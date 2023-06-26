#ifndef _ANALYZER_H_
#define _ANALYZER_H_

#include "identifiers.h"
#include "tokens.h"
#include <stdint.h>

/* Analyzer */
typedef struct Analyzer {
    TokenList *stream;
    ident_node_t *lookup_tree;
    unsigned long position;
    Token *token;
} Analyzer;

Analyzer *analyzer_construct(TokenList *stream);
void analyzer_destruct(Analyzer *analyzer);

Token *_analyzer_read_token(Analyzer *analyzer);
int16_t analyzer_next_instruction(Analyzer *analyzer);

#endif // _ANALYZER_H_
