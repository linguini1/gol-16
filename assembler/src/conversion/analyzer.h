#ifndef _ANALYZER_H_
#define _ANALYZER_H_

#include "identifiers.h"
#include "tokens.h"
#include <stdint.h>

/* Analyzer */
typedef struct Analyzer {
    TokenList *stream;
    unsigned long stream_index;
    ident_node_t *lookup_tree;
    unsigned long position;
    Token *token;
    char *__str_in_prog;
} Analyzer;

Analyzer *analyzer_construct(TokenList *stream);
void analyzer_destruct(Analyzer *analyzer);

void analyzer_print_error(Analyzer *analyzer, char *err_msg);

static void _analyzer_read_token(Analyzer *analyzer);
uint16_t analyzer_next_instruction(Analyzer *analyzer, char **err_msg);
static uint16_t _analyzer_convert_statement(Analyzer *analyzer, char **err_msg);

static uint16_t _analyzer_convert_dcd(Analyzer *analyzer, char **err_msg);
static uint16_t _analyzer_convert_conditional(Analyzer *analyzer, char **err_msg);
static uint16_t _analyzer_convert_form1(Analyzer *analyzer, const unsigned short int opcodes[], char **err_msg);
static uint16_t _analyzer_convert_form2(Analyzer *analyzer, const unsigned short int opcodes[], char **err_msg);
static uint16_t _analyzer_convert_form3(Analyzer *analyzer, const unsigned short int opcodes[], char **err_msg);

/* Operator identification */
static form_t _op_form(char *operator);
static const operator_t *_get_op_by_name(char *operator);

/* Token conversion */
static uint16_t _str_literal(Analyzer *analyzer);
static uint8_t _convert_register(char *reg);

#endif // _ANALYZER_H_
