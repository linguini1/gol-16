#include "analyzer.h"
#include <stdlib.h>

/* Analyzer */
Analyzer *analyzer_construct(TokenList *stream) {
    Analyzer *analyzer = malloc(sizeof(Analyzer));
    analyzer->stream = stream;
    analyzer->lookup_tree = lookup_tree_construct(stream);
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

int16_t analyzer_next_instruction(Analyzer *analyzer){
    return 0;
}
