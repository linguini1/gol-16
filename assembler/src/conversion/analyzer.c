#include "analyzer.h"
#include "tokens.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Analyzer */
Analyzer *analyzer_construct(TokenList *stream) {
    Analyzer *analyzer = malloc(sizeof(Analyzer));
    analyzer->stream = stream;
    analyzer->stream_index = 0;
    analyzer->lookup_tree = lookup_tree_construct(stream);
    analyzer->position = 0;
    analyzer->__str_in_prog = NULL;
    _analyzer_read_token(analyzer); // Initialize with start token
    return analyzer;
}

void analyzer_destruct(Analyzer *analyzer) {
    token_list_destruct(analyzer->stream); // Will free analyzer->token too
    free(analyzer);
}

void analyzer_print_error(Analyzer *analyzer, char *err_msg) {
    printf("Error: \"%s\" at pos %lu and token '%s'\n", err_msg, analyzer->position, analyzer->token->literal);
}

static void _analyzer_read_token(Analyzer *analyzer) {
    analyzer->token = analyzer->stream->tokens[analyzer->stream_index];
    if (analyzer->token->type == TokenOperator) {
        analyzer->position++;
    }
    analyzer->stream_index++;
}

uint16_t analyzer_next_instruction(Analyzer *analyzer, char **err_msg) {

    // Check if a string literal is currently being translated
    if (analyzer->__str_in_prog != NULL) {
        return _str_literal(analyzer);
    }
    _analyzer_read_token(analyzer);

    // Skip initial identifiers
    if (analyzer->token->type == TokenIdentifier) {
        _analyzer_read_token(analyzer);
    }

    // There must be an operator at the start of each instruction
    if (analyzer->token->type != TokenOperator) {
        *err_msg = "Expected operator, got different token.";
        return 0;
    }

    return _analyzer_convert_statement(analyzer, err_msg);
}

static uint16_t _analyzer_convert_statement(Analyzer *analyzer, char **err_msg) {

    const operator_t *operator= _get_op_by_name(analyzer->token->literal);

    if (!strcmp(operator->name, "DCD")) {
        return _analyzer_convert_dcd(analyzer, err_msg);
    }

    switch (operator->form) {
    case Form1:
        return _analyzer_convert_form1(analyzer, operator->raw, err_msg);
    default:
        *err_msg = "Unrecognized operator.";
        return 0;
    }
}

static uint16_t _analyzer_convert_form1(Analyzer *analyzer, const unsigned short int opcodes[], char **err_msg) {

    uint16_t inst = 0;

    // Next argument must be a register
    _analyzer_read_token(analyzer);
    if (analyzer->token->type != TokenRegister) {
        *err_msg = "Expected register.";
        return 0;
    }

    inst = inst | _convert_register(analyzer->token->literal);
    inst = inst << 2;

    // Next token must be a comma
    _analyzer_read_token(analyzer);
    if (analyzer->token->type != TokenComma) {
        *err_msg = "Expected comma.";
        return 0;
    }

    // Next argument must be a register, too
    _analyzer_read_token(analyzer);
    if (analyzer->token->type != TokenRegister) {
        *err_msg = "Expected register.";
        return 0;
    }

    inst = inst | _convert_register(analyzer->token->literal);
    inst = inst << 8;

    // Next token must be a comma
    _analyzer_read_token(analyzer);
    if (analyzer->token->type != TokenComma) {
        *err_msg = "Expected comma.";
        return 0;
    }

    // Next argument can be a register or an immediate
    _analyzer_read_token(analyzer);
    switch (analyzer->token->type) {
    case TokenRegister:
        inst = inst | (opcodes[0] << 12);
        inst = inst | (_convert_register(analyzer->token->literal) << 6);
        break;
    case TokenHex:
        inst = inst | (opcodes[1] << 12);
        inst = inst | strtol(analyzer->token->literal, NULL, 16);
        break;
    case TokenBin:
        inst = inst | (opcodes[1] << 12);
        inst = inst | strtol(analyzer->token->literal, NULL, 2);
        break;
    case TokenDec:
        inst = inst | (opcodes[1] << 12);
        inst = inst | atoi(analyzer->token->literal);
        break;
    default:
        *err_msg = "Expected numerical immediate or register.";
        return 0;
    }

    return inst;
}

static uint16_t _analyzer_convert_dcd(Analyzer *analyzer, char **err_msg) {
    _analyzer_read_token(analyzer);
    switch (analyzer->token->type) {
    case TokenBin:
        return strtol(analyzer->token->literal, NULL, 2);
    case TokenHex:
        return strtol(analyzer->token->literal, NULL, 16);
    case TokenDec:
        return atoi(analyzer->token->literal);
    case TokenChar:
        return analyzer->token->literal[0];
    case TokenStr:
        analyzer->__str_in_prog = analyzer->token->literal;
        return _str_literal(analyzer);
    default:
        *err_msg = "Expected literal to follow DCD.";
        return 0;
    }
}

/* Operator identification */
static form_t _op_form(char *operator) {
    for (int i = 0; i < NUM_OPERATORS; i++) {
        if (!strcmp(OPERATORS[i].name, operator)) {
            return OPERATORS[i].form;
        }
    }
    return FormDNE;
}
static const operator_t *_get_op_by_name(char *operator) {
    for (int i = 0; i < NUM_OPERATORS; i++) {
        if (!strcmp(OPERATORS[i].name, operator)) {
            return &OPERATORS[i];
        }
    }
    return NULL;
}

/* Token conversion */
// TODO handle escape characters in strings
static uint16_t _str_literal(Analyzer *analyzer) {
    unsigned short count = 0;
    uint16_t bits = 0;
    while (*(analyzer->__str_in_prog) != '\0' && count < 2) {
        bits = bits << 8;
        bits = bits | *(analyzer->__str_in_prog);
        analyzer->__str_in_prog++;
        count++;
    }
    if (*(analyzer->__str_in_prog) == '\0') {
        analyzer->__str_in_prog = NULL;
    }
    return bits;
}

static uint8_t _convert_register(char *reg) {
    if (!strcmp(reg, "R0")) {
        return 0;
    } else if (!strcmp(reg, "R1")) {
        return 1;
    } else if (!strcmp(reg, "R2")) {
        return 2;
    }
    return 3; // R3
}
