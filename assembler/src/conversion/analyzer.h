#ifndef _ANALYZER_H_
#define _ANALYZER_H_

#include "identifiers.h"
#include "tokens.h"
#include <stdint.h>

/* Register bit fields */
typedef struct Register {
    char *name;
    unsigned int bitfield;
} register_t;

/* Analyzer */
typedef struct Analyzer {
    TokenList *stream;
    unsigned long stream_index;
    ident_node_t *lookup_tree;
    unsigned long position;
    Token *token;
    char *__str_in_prog;
    const char *file_path;
} Analyzer;

Analyzer *analyzer_construct(TokenList *stream, const char *file_path);
void analyzer_destruct(Analyzer *analyzer);

uint16_t analyzer_next_instruction(Analyzer *analyzer);
bool analyzer_finished(Analyzer *analyzer);

#endif // _ANALYZER_H_
