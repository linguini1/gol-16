#include "analyzer.h"
#include "identifiers.h"
#include "tokens.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Register bit fields */
static const unsigned int NUM_REGISTERS = 8;
static const register_t BITFIELDS[] = {{"R0", 0x80}, {"R1", 0x40}, {"R2", 0x20}, {"R3", 0x10},
                                       {"PC", 0x08}, {"SP", 0x04}, {"LR", 0x02}, {"FR", 0x01}};

/* Analyzer */
Analyzer *analyzer_construct(TokenList *stream) {
    Analyzer *analyzer = malloc(sizeof(Analyzer));
    analyzer->err_msg = malloc(sizeof(char) * 80);
    analyzer->err_msg = NULL;
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
    free(analyzer->err_msg);
    free(analyzer);
}

void analyzer_print_error(Analyzer *analyzer, const char *file_name) {
    Token *t = analyzer->token;
    printf("%s:%lu:%lu error: %s\n\t Token: '%s'\n", file_name, t->line, t->col, analyzer->err_msg, t->literal);
}

bool analyzer_finished(Analyzer *analyzer) {
    return analyzer->token->type == TokenEOF || analyzer->stream->tokens[analyzer->stream_index]->type == TokenEOF;
}

bool analyzer_err(Analyzer *analyzer) { return analyzer->err_msg != NULL; }

static void _analyzer_read_token(Analyzer *analyzer) {
    analyzer->token = analyzer->stream->tokens[analyzer->stream_index];
    if (analyzer->token->type == TokenOperator) {
        analyzer->position++;
    }
    analyzer->stream_index++;
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
    if (analyzer->token->type != TokenOperator) {
        analyzer->err_msg = "Expected operator, got different token.";
        return 0;
    }

    return _analyzer_convert_statement(analyzer);
}

static uint16_t _analyzer_convert_statement(Analyzer *analyzer) {

    const operator_t *operator= _get_op_by_name(analyzer->token->literal);

    if (operator== NULL && is_conditional(analyzer->token->literal)) {
        return _analyzer_convert_conditional(analyzer);
    }

    if (!strcmp(operator->name, "DCD")) {
        return _analyzer_convert_dcd(analyzer);
    }

    // TODO implement Form4, Form5, EQU
    switch (operator->form) {
    case Form1:
        return _analyzer_convert_form1(analyzer, operator->raw);
    case Form2:
        return _analyzer_convert_form2(analyzer, operator->raw);
    case Form3:
        return _analyzer_convert_form3(analyzer, operator->raw);
    case FormStack:
        return _analyzer_convert_stack(analyzer, operator->raw);
    default:
        analyzer->err_msg = "Unrecognized operator.";
        return 0;
    }
}

static uint16_t _analyzer_convert_dcd(Analyzer *analyzer) {
    _analyzer_read_token(analyzer);
    switch (analyzer->token->type) {
    case TokenBin:
        return strtol(analyzer->token->literal, NULL, 2);
    case TokenHex:
        return strtol(analyzer->token->literal, NULL, 16);
    case TokenDec:
        return atoi(analyzer->token->literal);
    case TokenChar:
        return _char_literal(analyzer->token->literal);
    case TokenStr:
        analyzer->__str_in_prog = analyzer->token->literal;
        return _str_literal(analyzer);
    default:
        analyzer->err_msg = "Expected literal to follow DCD.";
        return 0;
    }
}

static uint16_t _analyzer_convert_conditional(Analyzer *analyzer) {
    uint16_t inst = 0;
    int length = strlen(analyzer->token->literal);

    if (length == 3) {
        inst = inst | 0x0F;
        inst = (inst << 4) | _condition_code(analyzer->token->literal + 1);
    } else {
        inst = inst | 0x1F;
        inst = (inst << 4) | _condition_code(analyzer->token->literal + 2);
    }
    inst = inst << 7;

    // Next token must be an immediate
    _analyzer_read_token(analyzer);
    switch (analyzer->token->type) {
    case TokenHex:
        inst = inst | (strtol(analyzer->token->literal, NULL, 16) & 0x7F);
        break;
    case TokenBin:
        inst = inst | (strtol(analyzer->token->literal, NULL, 2) & 0x7F);
        break;
    case TokenDec:
        inst = inst | (atoi(analyzer->token->literal) & 0x7F);
        break;
    case TokenIdentifier: {
        ident_t *ident = lookup_tree_get(analyzer->lookup_tree, analyzer->token->literal);
        if (ident == NULL) {
            analyzer->err_msg = "Undefined identifier.";
        }
        inst = inst | ((ident->location - analyzer->position) & 0x7F);
        break;
    }
    default:
        analyzer->err_msg = "Expected numerical immediate.";
        return 0;
    }

    return inst;
}

static uint16_t _analyzer_convert_form1(Analyzer *analyzer, const unsigned short int opcodes[]) {
    uint16_t inst = 0;

    // Next argument must be a register
    _analyzer_read_token(analyzer);
    if (analyzer->token->type != TokenRegister) {
        analyzer->err_msg = "Expected register.";
        return 0;
    }

    inst = inst | _convert_register(analyzer->token->literal);
    inst = inst << 2;

    // Next token must be a comma
    _analyzer_read_token(analyzer);
    if (analyzer->token->type != TokenComma) {
        analyzer->err_msg = "Expected comma.";
        return 0;
    }

    // Next argument must be a register, too
    _analyzer_read_token(analyzer);
    if (analyzer->token->type != TokenRegister) {
        analyzer->err_msg = "Expected register.";
        return 0;
    }

    inst = inst | _convert_register(analyzer->token->literal);
    inst = inst << 7;

    // Next token must be a comma
    _analyzer_read_token(analyzer);
    if (analyzer->token->type != TokenComma) {
        analyzer->err_msg = "Expected comma.";
        return 0;
    }

    // Next argument can be a register or an immediate
    _analyzer_read_token(analyzer);
    switch (analyzer->token->type) {
    case TokenRegister:
        inst = inst | (opcodes[0] << 11);
        inst = inst | (_convert_register(analyzer->token->literal) << 5);
        break;
    case TokenHex:
        inst = inst | (opcodes[1] << 11);
        inst = inst | (strtol(analyzer->token->literal, NULL, 16) & 0x7F);
        break;
    case TokenBin:
        inst = inst | (opcodes[1] << 11);
        inst = inst | (strtol(analyzer->token->literal, NULL, 2) & 0x7F);
        break;
    case TokenDec:
        inst = inst | (opcodes[1] << 11);
        inst = inst | (atoi(analyzer->token->literal) & 0x7F);
        break;
    case TokenChar:
        inst = inst | (opcodes[1] << 11);
        inst = inst | (_char_literal(analyzer->token->literal) & 0x7F);
        break;
    default:
        analyzer->err_msg = "Expected numerical immediate or register.";
        return 0;
    }

    return inst;
}

static uint16_t _analyzer_convert_form2(Analyzer *analyzer, const unsigned short int opcodes[]) {

    uint16_t inst = 0;

    // Next argument must be a register
    _analyzer_read_token(analyzer);
    if (analyzer->token->type != TokenRegister) {
        analyzer->err_msg = "Expected register.";
        return 0;
    }

    inst = inst | _convert_register(analyzer->token->literal);
    inst = inst << 9;

    // Next token must be a comma
    _analyzer_read_token(analyzer);
    if (analyzer->token->type != TokenComma) {
        analyzer->err_msg = "Expected comma.";
        return 0;
    }

    // Next argument can be a register or an immediate
    _analyzer_read_token(analyzer);
    switch (analyzer->token->type) {
    case TokenRegister:
        inst = inst | (opcodes[0] << 11);
        inst = inst | (_convert_register(analyzer->token->literal) << 7);
        break;
    case TokenHex:
        inst = inst | (opcodes[1] << 11);
        inst = inst | (strtol(analyzer->token->literal, NULL, 16) & 0x1FF);
        break;
    case TokenBin:
        inst = inst | (opcodes[1] << 11);
        inst = inst | (strtol(analyzer->token->literal, NULL, 2) & 0x1FF);
        break;
    case TokenDec:
        inst = inst | (opcodes[1] << 11);
        inst = inst | (_char_literal(analyzer->token->literal) & 0x1FF);
        break;
    case TokenChar:
        inst = inst | (opcodes[1] << 11);
        inst = inst | (_char_literal(analyzer->token->literal) & 0x7F);
        break;
    case TokenIdentifier: {
        inst = inst | (opcodes[1] << 11);
        ident_t *ident = lookup_tree_get(analyzer->lookup_tree, analyzer->token->literal);
        if (ident == NULL) {
            analyzer->err_msg = "Undefined identifier.";
            return 0;
        }
        inst = inst | (ident->location & 0x1FF);
        break;
    }
    default:
        analyzer->err_msg = "Expected numerical immediate or register.";
        return 0;
    }

    return inst;
}

static uint16_t _analyzer_convert_form3(Analyzer *analyzer, const unsigned short int opcodes[]) {

    uint16_t inst = 0;

    // Next argument must be a register
    _analyzer_read_token(analyzer);
    if (analyzer->token->type != TokenRegister) {
        analyzer->err_msg = "Expected register.";
        return 0;
    }

    inst = inst | _convert_register(analyzer->token->literal);
    inst = inst << 2;

    // Next token must be a comma
    _analyzer_read_token(analyzer);
    if (analyzer->token->type != TokenComma) {
        analyzer->err_msg = "Expected comma.";
        return 0;
    }

    // Next token must be open [
    _analyzer_read_token(analyzer);
    if (analyzer->token->type != TokenLBrack) {
        analyzer->err_msg = "Expected open bracket ([).";
        return 0;
    }

    // Next token can either be a register, identifier or immediate
    _analyzer_read_token(analyzer);
    uint16_t immediate;
    bool imm = true;
    switch (analyzer->token->type) {
    case TokenHex:
        immediate = (strtol(analyzer->token->literal, NULL, 16) & 0x1FF);
        break;
    case TokenBin:
        immediate = (strtol(analyzer->token->literal, NULL, 2) & 0x1FF);
        break;
    case TokenDec:
        immediate = (atoi(analyzer->token->literal) & 0x1FF);
        break;
    case TokenIdentifier: {
        ident_t *ident = lookup_tree_get(analyzer->lookup_tree, analyzer->token->literal);
        if (ident == NULL) {
            analyzer->err_msg = "Undefined identifier.";
            return 0;
        }
        immediate = ident->location & 0x1FF;
        break;
    }
    case TokenRegister:
        imm = false;
        inst = inst | _convert_register(analyzer->token->literal);
        inst = inst << 2;
        break;
    default:
        analyzer->err_msg = "Expected register, identifier or numerical immediate.";
        return 0;
    }

    // Argument was an immediate
    if (imm) {
        inst = inst << 9;                  // Move over register bits
        inst = inst | (opcodes[0] << 11);  // Add the opcode
        inst = inst | (immediate & 0x1FF); // Add the immediate to the end

        // Check that instruction closes with ]
        _analyzer_read_token(analyzer);
        if (analyzer->token->type != TokenRBrack) {
            analyzer->err_msg = "Expected closing bracket.";
            return 0;
        }

        return inst;
    }

    // By now, instruction is either [register, register] or [register, imm7]

    // Next token must be a comma
    _analyzer_read_token(analyzer);
    if (analyzer->token->type != TokenComma) {
        analyzer->err_msg = "Expected comma.";
        return 0;
    }

    // Next token is either a register or an immediate/identifier
    _analyzer_read_token(analyzer);
    imm = true;
    switch (analyzer->token->type) {
    case TokenHex:
        inst = inst << 9;
        immediate = (strtol(analyzer->token->literal, NULL, 16) & 0x7F);
        break;
    case TokenBin:
        inst = inst << 9;
        immediate = (strtol(analyzer->token->literal, NULL, 2) & 0x7F);
        break;
    case TokenDec:
        inst = inst << 9;
        immediate = (atoi(analyzer->token->literal) & 0x7F);
        break;
    case TokenIdentifier: {
        inst = inst << 9;
        ident_t *ident = lookup_tree_get(analyzer->lookup_tree, analyzer->token->literal);
        if (ident == NULL) {
            analyzer->err_msg = "Undefined identifier.";
            return 0;
        }
        immediate = ident->location & 0x7F;
        break;
    }
    case TokenRegister:
        imm = false;
        inst = inst | _convert_register(analyzer->token->literal);
        inst = inst << 5;
        break;
    default:
        analyzer->err_msg = "Expected register, identifier or numerical immediate.";
        return 0;
    }

    // Check that instruction closes with ]
    _analyzer_read_token(analyzer);
    if (analyzer->token->type != TokenRBrack) {
        analyzer->err_msg = "Expected closing bracket.";
        return 0;
    }

    if (imm) {
        return inst | (opcodes[2] << 11);
    }

    return inst | (opcodes[1] << 11);
}

static uint16_t _analyzer_convert_stack(Analyzer *analyzer, const unsigned short int opcodes[]) {

    uint16_t inst = 0;
    inst = (inst | opcodes[0]) << 11;

    // Next token must be a {
    _analyzer_read_token(analyzer);
    if (analyzer->token->type != TokenLCurl) {
        analyzer->err_msg = "Expected '{'.";
        return 0;
    }

    // Tokens can be special registers or registers
    _analyzer_read_token(analyzer);
    while (analyzer->token->type == TokenRegister || analyzer->token->type == TokenSpecialRegister) {
        inst |= _get_bitfield(analyzer->token->literal);
        _analyzer_read_token(analyzer);

        // Closing curly brace signifies end of argument
        if (analyzer->token->type == TokenRCurl) {
            break;
        }

        // Must be followed by a comma otherwise
        if (analyzer->token->type != TokenComma) {
            analyzer->err_msg = "Expected a comma.";
            return 0;
        }
        _analyzer_read_token(analyzer);
    }

    // Next token must be a }
    if (analyzer->token->type != TokenRCurl) {
        analyzer->err_msg = "Expected ']'.";
        return 0;
    }
    _analyzer_read_token(analyzer); // Skip last curly brace

    return inst;
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

    // After processing two characters, if the next character is a null terminator, mark as done so
    // as to not return an extra 0x00 next call.
    if (*(analyzer->__str_in_prog) == '\0') {
        analyzer->__str_in_prog = NULL;
    }

    return bits;
}

static uint8_t _char_literal(char *literal) {
    if (strlen(literal) == 1) {
        return literal[0];
    }
    return _escape_character(literal[1]);
}

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
