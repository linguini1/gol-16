#include "analyzer.h"
#include "identifiers.h"
#include "tokens.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Register bit fields */
static const unsigned int NUM_REGISTERS = 8;
static const asm_register BITFIELDS[] = {{"R0", 0x80}, {"R1", 0x40}, {"R2", 0x20}, {"R3", 0x10},
                                         {"PC", 0x08}, {"SP", 0x04}, {"LR", 0x02}, {"FR", 0x01}};

static void analyzer_fatal_error(Analyzer *analyzer, const char *err_msg) {
    Token *t = analyzer->token;
    printf("%s:%lu:%lu error: %s\n\t Token: '%s'\n", analyzer->file_path, t->line, t->col, err_msg, t->literal);
    exit(EXIT_FAILURE);
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
static char _escape_character(char esc) {
    switch (esc) {
    case 'n':
        return '\n';
    case '0':
        return '\0';
    case 't':
        return '\t';
    case 'a':
        return '\a';
    case 'b':
        return '\b';
    case 'e':
        return '\e';
    case 'f':
        return '\f';
    case 'r':
        return '\r';
    case 'v':
        return '\v';
    case '\\':
        return '\\';
    case '\'':
        return '\'';
    case '"':
        return '"';
    case '?':
        return '\?';
    }
    return -1;
}

static uint16_t _str_literal(Analyzer *analyzer) {
    uint16_t bits = 0;
    for (unsigned short i = 0; i < 2; i++) {

        // End early if string is over
        if (*(analyzer->__str_in_prog) == '\0') {
            analyzer->__str_in_prog = NULL;
            return bits << 8;
        }

        // Detect escape sequence and replace with correct character code
        if (*(analyzer->__str_in_prog) == '\\') {
            analyzer->__str_in_prog++;
            *(analyzer->__str_in_prog) = _escape_character(*(analyzer->__str_in_prog));
        }

        bits = bits << 8;
        bits = bits | *(analyzer->__str_in_prog);
        analyzer->__str_in_prog++;
    }

    return bits;
}

static uint8_t _char_literal(char *literal) {
    if (strlen(literal) == 1) {
        return literal[0];
    }
    return _escape_character(literal[1]);
}

static uint16_t _convert_numeric_literal(Analyzer *analyzer, uint16_t bitmask) {
    switch (analyzer->token->type) {
    case TokenHex:
        return strtol(analyzer->token->literal, NULL, 16) & bitmask;
    case TokenBin:
        return strtol(analyzer->token->literal, NULL, 2) & bitmask;
    case TokenDec:
        return atoi(analyzer->token->literal) & bitmask;
    case TokenChar:
        return _char_literal(analyzer->token->literal) & bitmask;
    default:
        analyzer_fatal_error(analyzer, "Expected numeric literal");
        return 0;
    }
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

static uint8_t _get_bitfield(char *reg) {
    for (unsigned int i = 0; i < NUM_REGISTERS; i++) {
        if (!strcmp(reg, BITFIELDS[i].name)) {
            return BITFIELDS[i].bitfield;
        }
    }
    return 0;
}

bool analyzer_finished(Analyzer *analyzer) {
    return analyzer->token->type == TokenEOF || analyzer->stream->tokens[analyzer->stream_index]->type == TokenEOF;
}

static void _analyzer_read_token(Analyzer *analyzer) {
    analyzer->token = analyzer->stream->tokens[analyzer->stream_index];
    if (analyzer->token->type == TokenOperator) {
        analyzer->position++;
    }
    analyzer->stream_index++;
}

static void _analyzer_expect_register(Analyzer *analyzer) {
    _analyzer_read_token(analyzer);
    if (analyzer->token->type != TokenRegister) analyzer_fatal_error(analyzer, "Expected register.");
}

static void _analyzer_expect_comma(Analyzer *analyzer) {
    _analyzer_read_token(analyzer);
    if (analyzer->token->type != TokenComma) analyzer_fatal_error(analyzer, "Expected comma.");
}

static uint16_t _analyzer_convert_dcd(Analyzer *analyzer) {
    _analyzer_read_token(analyzer);
    switch (analyzer->token->type) {
    case TokenBin:
    case TokenHex:
    case TokenDec:
    case TokenChar:
        return _convert_numeric_literal(analyzer, 0xFFFF);
    case TokenStr:
        analyzer->__str_in_prog = analyzer->token->literal;
        return _str_literal(analyzer);
    default:
        analyzer_fatal_error(analyzer, "Expected literal to follow DCD.");
        return 0;
    }
}

static uint16_t _analyzer_convert_conditional(Analyzer *analyzer) {
    uint16_t inst = 0;
    int length = strlen(analyzer->token->literal);

    switch (length) {
    case 1:
        inst = inst | 0x0F;
        inst = (inst << 4) | _condition_code("AL");
        break;
    case 2:
        inst = inst | 0x1F;
        inst = (inst << 4) | _condition_code("AL");
        break;
    case 3:
        inst = inst | 0x0F;
        inst = (inst << 4) | _condition_code(analyzer->token->literal + 1);
        break;
    default:
        inst = inst | 0x1F;
        inst = (inst << 4) | _condition_code(analyzer->token->literal + 2);
        break;
    }
    inst = inst << 7;

    // Next token must be an immediate
    _analyzer_read_token(analyzer);
    switch (analyzer->token->type) {
    case TokenHex:
    case TokenBin:
    case TokenDec:
    case TokenChar:
        inst |= _convert_numeric_literal(analyzer, 0x7F);
    case TokenIdentifier: {
        ident_t *ident = lookup_tree_get(analyzer->lookup_tree, analyzer->token->literal);
        if (ident == NULL) analyzer_fatal_error(analyzer, "Undefined identifier.");
        inst = inst | ((ident->location - analyzer->position + 1) & 0x7F);
        break;
    }
    default:
        analyzer_fatal_error(analyzer, "Expected numerical immediate.");
    }

    return inst;
}

static uint16_t _analyzer_convert_form1(Analyzer *analyzer, const unsigned short int opcodes[]) {
    uint16_t inst = 0;

    _analyzer_expect_register(analyzer);
    inst = inst | _convert_register(analyzer->token->literal);
    inst = inst << 2;

    _analyzer_expect_comma(analyzer);

    _analyzer_expect_register(analyzer);
    inst = inst | _convert_register(analyzer->token->literal);
    inst = inst << 7;

    _analyzer_expect_comma(analyzer);

    // Next argument can be a register or an immediate
    _analyzer_read_token(analyzer);
    switch (analyzer->token->type) {
    case TokenRegister:
        inst |= opcodes[0] << 11;
        inst |= _convert_register(analyzer->token->literal) << 5;
        break;
    case TokenHex:
    case TokenBin:
    case TokenDec:
    case TokenChar:
        inst |= (opcodes[1] << 11);
        inst |= _convert_numeric_literal(analyzer, 0x7F);
        break;
    default:
        analyzer_fatal_error(analyzer, "Expected numerical immediate or register.");
    }

    return inst;
}

static uint16_t _analyzer_convert_form2(Analyzer *analyzer, const unsigned short int opcodes[]) {

    uint16_t inst = 0;

    _analyzer_expect_register(analyzer);
    inst = inst | _convert_register(analyzer->token->literal);
    inst = inst << 9;

    _analyzer_expect_comma(analyzer);

    // Next argument can be a register or an immediate
    _analyzer_read_token(analyzer);
    switch (analyzer->token->type) {
    case TokenRegister:
        inst |= opcodes[0] << 11;
        inst |= (_convert_register(analyzer->token->literal) << 7);
        break;
    case TokenHex:
    case TokenBin:
    case TokenDec:
    case TokenChar:
        inst |= opcodes[1] << 11;
        inst |= _convert_numeric_literal(analyzer, 0x7F);
        break;
    default:
        analyzer_fatal_error(analyzer, "Expected numerical immediate or register.");
    }

    return inst;
}

static uint16_t _analyzer_convert_form3(Analyzer *analyzer, const unsigned short int opcodes[]) {

    uint16_t inst = 0;

    _analyzer_expect_register(analyzer);
    inst = inst | _convert_register(analyzer->token->literal);
    inst = inst << 2;

    _analyzer_expect_comma(analyzer);

    // Next token must be open [
    _analyzer_read_token(analyzer);
    if (analyzer->token->type != TokenLBrack) analyzer_fatal_error(analyzer, "Expected open bracket: '['.");

    // Next token can either be a register, identifier or immediate
    _analyzer_read_token(analyzer);
    uint16_t immediate;
    bool imm = true;
    switch (analyzer->token->type) {
    case TokenHex:
    case TokenBin:
    case TokenDec:
    case TokenChar:
        immediate = _convert_numeric_literal(analyzer, 0x1FF);
        break;
    case TokenIdentifier: { // (PC-relative)
        ident_t *ident = lookup_tree_get(analyzer->lookup_tree, analyzer->token->literal);
        if (ident == NULL) analyzer_fatal_error(analyzer, "Undefined identifier.");
        immediate = (ident->location - analyzer->position + 1) & 0x1FF;
        break;
    }
    case TokenRegister:
        imm = false;
        inst = inst | _convert_register(analyzer->token->literal);
        inst = inst << 2;
        break;
    default:
        analyzer_fatal_error(analyzer, "Expected register, identifier or numerical immediate.");
    }

    // Argument was an immediate
    if (imm) {
        inst = inst << 7;                 // Move over register bits to make room for immediate
        inst = inst | (opcodes[0] << 11); // Add the opcode
        inst = inst | (immediate & 0x1FF); // Add the immediate to the end

        // Check that instruction closes with ]
        _analyzer_read_token(analyzer);
        if (analyzer->token->type != TokenRBrack) analyzer_fatal_error(analyzer, "Expected closing bracket.");
        return inst;
    }

    // By now, instruction is either [register, register] or [register, imm7]
    _analyzer_expect_comma(analyzer);

    // Next token is either a register or an immediate/identifier
    _analyzer_read_token(analyzer);
    imm = true;
    switch (analyzer->token->type) {
    case TokenHex:
    case TokenBin:
    case TokenDec:
    case TokenChar:
        inst = inst << 9;
        immediate = _convert_numeric_literal(analyzer, 0x7F);
        break;
    case TokenIdentifier: {
        inst = inst << 9;
        ident_t *ident = lookup_tree_get(analyzer->lookup_tree, analyzer->token->literal);
        if (ident == NULL) analyzer_fatal_error(analyzer, "Undefined identifier.");
        immediate = ident->location & 0x7F;
        break;
    }
    case TokenRegister:
        imm = false;
        inst = inst | _convert_register(analyzer->token->literal);
        inst = inst << 5;
        break;
    default:
        analyzer_fatal_error(analyzer, "Expected register, identifier or numerical immediate.");
    }

    // Check that instruction closes with ]
    _analyzer_read_token(analyzer);
    if (analyzer->token->type != TokenRBrack) analyzer_fatal_error(analyzer, "Expected closing bracket.");

    if (imm) return inst | (opcodes[2] << 11);
    return inst | (opcodes[1] << 11);
}

static uint16_t _analyzer_convert_form4(Analyzer *analyzer, const unsigned short int opcodes[]) {
    uint16_t inst = opcodes[0] << 2; // Only one opcode

    if (analyzer->token->literal[0] == 'R') inst |= 0x1; // Type: rotate
    if (analyzer->token->literal[2] == 'R') inst |= 0x2; // Direction: right
    inst = inst << 2;

    // Expect register
    _analyzer_expect_register(analyzer);
    inst |= _convert_register(analyzer->token->literal);
    inst = inst << 2;

    _analyzer_expect_comma(analyzer);

    _analyzer_expect_register(analyzer);
    inst |= _convert_register(analyzer->token->literal);
    inst = inst << 2;

    _analyzer_expect_comma(analyzer);

    // Expect register or immediate
    _analyzer_read_token(analyzer);
    bool imm = true;
    uint8_t immediate;
    switch (analyzer->token->type) {

    case TokenHex:
    case TokenBin:
    case TokenDec:
    case TokenChar:
        inst = inst << 3;
        immediate = _convert_numeric_literal(analyzer, 0xF);
        break;
    case TokenRegister:
        imm = false;
        inst |= _convert_register(analyzer->token->literal);
        inst = inst << 3;
        break;
    default:
        analyzer_fatal_error(analyzer, "Expected register, identifier or numerical immediate.");
    }

    if (imm) inst |= immediate;
    return inst;
}

static uint16_t _analyzer_convert_form5(Analyzer *analyzer, const unsigned short int opcodes[]) {
    uint16_t inst = opcodes[0] << 2;

    _analyzer_expect_register(analyzer);
    inst |= _convert_register(analyzer->token->literal);
    inst = inst << 9;

    _analyzer_expect_comma(analyzer);

    // Expect identifier
    _analyzer_read_token(analyzer);
    if (analyzer->token->type != TokenIdentifier) analyzer_fatal_error(analyzer, "Expected identifer.");

    ident_t *ident = lookup_tree_get(analyzer->lookup_tree, analyzer->token->literal);
    if (ident == NULL) analyzer_fatal_error(analyzer, "Undefined identifier.");
    inst |= (ident->location - analyzer->position + 1) & 0x1FF;

    return inst;
}

static uint16_t _analyzer_convert_stack(Analyzer *analyzer, const unsigned short int opcodes[]) {

    uint16_t inst = 0;
    inst = (inst | opcodes[0]) << 11;

    // Next token must be a {
    _analyzer_read_token(analyzer);
    if (analyzer->token->type != TokenLCurl) analyzer_fatal_error(analyzer, "Expected '{'.");

    // Tokens can be special registers or registers
    _analyzer_read_token(analyzer);
    while (analyzer->token->type == TokenRegister || analyzer->token->type == TokenSpecialRegister) {
        inst |= _get_bitfield(analyzer->token->literal);
        _analyzer_read_token(analyzer);

        // Closing curly brace signifies end of argument
        if (analyzer->token->type == TokenRCurl) break;

        if (analyzer->token->type != TokenComma) analyzer_fatal_error(analyzer, "Expected comma");
        _analyzer_read_token(analyzer);
    }

    // Next token must be a }
    if (analyzer->token->type != TokenRCurl) analyzer_fatal_error(analyzer, "Expected ']'.");
    return inst;
}

static uint16_t _analyzer_convert_statement(Analyzer *analyzer) {

    const operator_t *operator= _get_op_by_name(analyzer->token->literal);

    if (operator== NULL && is_conditional(analyzer->token->literal)) {
        return _analyzer_convert_conditional(analyzer);
    }

    if (!strcmp(operator->name, "DCD")) {
        return _analyzer_convert_dcd(analyzer);
    }

    // TODO implement EQU
    switch (operator->form) {
    case Form1:
        return _analyzer_convert_form1(analyzer, operator->raw);
    case Form2:
        return _analyzer_convert_form2(analyzer, operator->raw);
    case Form3:
        return _analyzer_convert_form3(analyzer, operator->raw);
    case Form4:
        return _analyzer_convert_form4(analyzer, operator->raw);
    case Form5:
        return _analyzer_convert_form5(analyzer, operator->raw);
    case FormStack:
        return _analyzer_convert_stack(analyzer, operator->raw);
    default:
        analyzer_fatal_error(analyzer, "Unrecognized operator.");
        return 0;
    }
}

/* Analyzer */
Analyzer *analyzer_construct(TokenList *stream, const char *file_path) {
    Analyzer *analyzer = malloc(sizeof(Analyzer));
    analyzer->file_path = file_path;
    analyzer->stream = stream;
    analyzer->lookup_tree = lookup_tree_construct(stream);
    analyzer->stream_index = 0;
    analyzer->position = 0;
    analyzer->__str_in_prog = NULL;
    _analyzer_read_token(analyzer); // Initialize with start token
    return analyzer;
}

void analyzer_destruct(Analyzer *analyzer) {
    token_list_destruct(analyzer->stream); // Will free analyzer->token too
    lookup_tree_destruct(analyzer->lookup_tree);
    free(analyzer);
}

uint16_t analyzer_next_instruction(Analyzer *analyzer) {

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
    if (analyzer->token->type != TokenOperator)
        analyzer_fatal_error(analyzer, "Expected operator, got different token.");

    return _analyzer_convert_statement(analyzer);
}
