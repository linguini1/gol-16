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
    char *err_msg;
} Analyzer;

Analyzer *analyzer_construct(TokenList *stream);
void analyzer_destruct(Analyzer *analyzer);

uint16_t analyzer_next_instruction(Analyzer *analyzer);
bool analyzer_finished(Analyzer *analyzer);
bool analyzer_err(Analyzer *analyzer);
void analyzer_print_error(Analyzer *analyzer, const char *file_name);

static void _analyzer_read_token(Analyzer *analyzer);
static uint16_t _analyzer_convert_statement(Analyzer *analyzer);

static uint16_t _analyzer_convert_dcd(Analyzer *analyzer);
static uint16_t _analyzer_convert_conditional(Analyzer *analyzer);
static uint16_t _analyzer_convert_form1(Analyzer *analyzer, const unsigned short int opcodes[]);
static uint16_t _analyzer_convert_form2(Analyzer *analyzer, const unsigned short int opcodes[]);
static uint16_t _analyzer_convert_form3(Analyzer *analyzer, const unsigned short int opcodes[]);
static uint16_t _analyzer_convert_stack(Analyzer *analyzer, const unsigned short int opcodes[]);

/* Operator identification */
static form_t _op_form(char *operator);
static const operator_t *_get_op_by_name(char *operator);

/* Token conversion */
static uint16_t _str_literal(Analyzer *analyzer);
static uint8_t _char_literal(char *literal);
static char _escape_character(char esc);
static uint8_t _convert_register(char *reg);
static uint8_t _get_bitfield(char *reg);

#endif // _ANALYZER_H_
