/* Library that contains the tools needed to tokenize an assembly file. */
#include "lexer.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* File type verification */
static bool _is_gasm_file(const char *filename) {

    size_t length = strlen(filename);
    const char *cur = &filename[length]; // End of string

    // Continue backwards until suffix start or string start reached
    while (*cur != '.' && cur != &filename[0]) {
        cur--;
    }
    return !strcmp(cur, FILE_SUFFIX);
}

static char *_struprcpy(char *ident) {
    size_t length = strlen(ident);
    char *upr_ident = malloc(length + 1);
    strcpy(upr_ident, ident);
    string_to_uppercase(upr_ident);
    return upr_ident;
}

/* Character classification */
static bool is_letter(char c) { return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z'); }
static bool is_num(char c) { return '0' <= c && c <= '9'; }
static bool is_bin(char c) { return c == '0' || c == '1'; }
static bool is_hex(char c) { return is_num(c) || ('A' <= c && c <= 'F') || ('a' <= c && c <= 'f'); }
static bool is_whitespace(char c) { return c == ' ' || c == '\n' || c == '\t' || c == '\r'; }

static bool is_register(char *ident) {

    if (ident == NULL) {
        return false;
    }

    char *upr_ident = _struprcpy(ident);

    bool reg =
        !strcmp(upr_ident, "R0") || !strcmp(upr_ident, "R1") || !strcmp(upr_ident, "R2") || !strcmp(upr_ident, "R3");
    free(upr_ident);
    return reg;
}

static bool is_special_register(char *ident) {
    if (ident == NULL) {
        return false;
    }

    char *upr_ident = _struprcpy(ident);

    bool spec =
        !strcmp(upr_ident, "SP") || !strcmp(upr_ident, "PC") || !strcmp(upr_ident, "LR") || !strcmp(upr_ident, "FR");

    free(upr_ident);
    return spec;
}

static bool is_operator(char *ident) {

    if (ident == NULL) {
        return false;
    }

    // Create uppercase copy of identifier
    size_t length = strlen(ident);
    char *upr_ident = malloc(length + 1);
    strcpy(upr_ident, ident);
    string_to_uppercase(upr_ident);

    if (length == 0) {
        free(upr_ident);
        return false;
    }

    // Check unconditional operators
    for (size_t i = 0; i < NUM_OPERATORS; i++) {
        if (!strcmp(upr_ident, OPERATORS[i].name)) {
            free(upr_ident);
            return true;
        }
    }

    free(upr_ident);
    return is_conditional(ident);
}

bool lexer_eof(Lexer *lexer) { return lexer->character == EOF; }

static void lexer_fatal_error(Lexer *lexer, const char *err_msg) {
    const char *format_string = "%s:%lu:%lu: error: %s\n\tcharacter: '%c'\n";
    if (lexer->character > ' ' || lexer->character < '~') {
        format_string = "%s:%lu:%lu: error: %s\n\tcharacter: 0x%x (ascii)\n";
    }
    printf(format_string, lexer->file_path, lexer->line, lexer->col, err_msg, lexer->character);
    fclose(lexer->stream);
    exit(EXIT_FAILURE);
}

/* Helper internals */
static char _lexer_peek(Lexer *lexer) {
    char c = fgetc(lexer->stream);
    ungetc(c, lexer->stream);
    return c;
}

static void _lexer_read_char(Lexer *lexer) {
    lexer->character = fgetc(lexer->stream);
    lexer->col++;

    // Keeps track of lines and columns
    if (lexer->character == '\n') {
        lexer->line++;
        lexer->col = 1;
    }
}

static char *_lexer_slice(Lexer *lexer, long start) {
    size_t length = ftell(lexer->stream) - (start - 1);

    // Store identifier
    char *slice = malloc(length + 1);

    fseek(lexer->stream, -length, SEEK_CUR); // Send file stream to start position of slice
    fseek(lexer->stream, 0, SEEK_CUR);       // Reset stream (saw on StackOverflow lol)

    fgets(slice, length, lexer->stream);
    fseek(lexer->stream, 1, SEEK_CUR);
    fseek(lexer->stream, 0, SEEK_CUR);

    slice[length] = '\0'; // Add null terminator

    return slice;
}

static void _lexer_skip_whitespace(Lexer *lexer) {
    while (is_whitespace(lexer->character)) {
        _lexer_read_char(lexer);
    }
}

static void _lexer_skip_comment(Lexer *lexer) {

    if (lexer->character != ';') {
        return;
    }

    while (lexer->character != '\n') {
        _lexer_read_char(lexer);
    }
}

static char *_lexer_read_identifier(Lexer *lexer) {

    long start = ftell(lexer->stream);
    while (is_letter(lexer->character) || lexer->character == '_' || is_num(lexer->character)) {
        _lexer_read_char(lexer);
    }
    return _lexer_slice(lexer, start);
}

static char *_lexer_read_bin_literal(Lexer *lexer) {

    if (lexer->character != 'b') {
        return NULL;
    }
    _lexer_read_char(lexer);

    long start = ftell(lexer->stream);
    while (is_bin(lexer->character)) {
        _lexer_read_char(lexer);
    }
    return _lexer_slice(lexer, start);
}

static char *_lexer_read_hex_literal(Lexer *lexer) {

    if (lexer->character != 'x') {
        return NULL;
    }
    _lexer_read_char(lexer);

    long start = ftell(lexer->stream);
    while (is_hex(lexer->character)) {
        _lexer_read_char(lexer);
    }
    return _lexer_slice(lexer, start);
}

static char *_lexer_read_decimal_literal(Lexer *lexer) {
    long start = ftell(lexer->stream);
    while (is_num(lexer->character)) {
        _lexer_read_char(lexer);
    }
    return _lexer_slice(lexer, start);
}

static char *_lexer_read_numeric_literal(Lexer *lexer, token_t *type) {
    // Initial read to determine literal type
    _lexer_read_char(lexer);

    if (lexer->character == '0' && _lexer_peek(lexer) == 'b') {
        _lexer_read_char(lexer); // Skip over 'b'
        *type = TokenBin;
        return _lexer_read_bin_literal(lexer);
    } else if (lexer->character == '0' && _lexer_peek(lexer) == 'x') {
        _lexer_read_char(lexer); // Skip over 'x'
        *type = TokenHex;
        return _lexer_read_hex_literal(lexer);
    }

    *type = TokenDec;
    return _lexer_read_decimal_literal(lexer);
}

static char *_lexer_read_string_literal(Lexer *lexer) {
    _lexer_read_char(lexer); // Skip first quote
    long start_pos = ftell(lexer->stream);
    bool escape = false; // Allow \" as a valid character
    while ((lexer->character != '"' || escape) && lexer->character != '\n' && lexer->character != EOF) {
        escape = lexer->character == '\\' && !escape;
        _lexer_read_char(lexer);
    }

    if (lexer->character == '\n' || lexer->character == EOF) lexer_fatal_error(lexer, "Unterminated string literal");

    char *value = _lexer_slice(lexer, start_pos);
    _lexer_read_char(lexer); // Skip last quote
    return value;
}

static char *_lexer_read_char_literal(Lexer *lexer) {
    _lexer_read_char(lexer); // Read internal character
    long start_pos = ftell(lexer->stream);

    if (lexer->character == '\\') _lexer_read_char(lexer); // Escape detected, read another char

    _lexer_read_char(lexer); // Get final quote
    if (lexer->character != '\'') lexer_fatal_error(lexer, "Multi-character character literal.");

    char *value = _lexer_slice(lexer, start_pos);
    _lexer_read_char(lexer); // Skip last quote
    return value;
}

/* Lexer */
Lexer *lexer_construct(const char *file_path) {

    // Verify compatible file
    if (!_is_gasm_file(file_path)) {
        return NULL;
    }

    // Verify file could be opened
    FILE *fptr = fopen(file_path, "rb");
    if (fptr == NULL) {
        return NULL;
    }

    Lexer *lexer = malloc(sizeof(Lexer));
    lexer->file_path = file_path;
    lexer->stream = fptr;
    lexer->line = 1;
    lexer->col = 1;
    _lexer_read_char(lexer);
    return lexer;
}

void lexer_destruct(Lexer *lexer) {
    fclose(lexer->stream);
    free(lexer);
}

Token *lexer_next_token(Lexer *lexer) {

    // Skip white space and comments until we reach something
    while (is_whitespace(lexer->character) || lexer->character == ';') {
        _lexer_skip_whitespace(lexer);
        _lexer_skip_comment(lexer);
    }

    switch (lexer->character) {
    case ',':
        _lexer_read_char(lexer);
        return token_construct(",", TokenComma, lexer->line, lexer->col);
    case '[':
        _lexer_read_char(lexer);
        return token_construct("[", TokenLBrack, lexer->line, lexer->col);
    case ']':
        _lexer_read_char(lexer);
        return token_construct("]", TokenRBrack, lexer->line, lexer->col);
    case '{':
        _lexer_read_char(lexer);
        return token_construct("{", TokenLCurl, lexer->line, lexer->col);
    case '}':
        _lexer_read_char(lexer);
        return token_construct("}", TokenRCurl, lexer->line, lexer->col);
    case -1:
        return token_construct(NULL, TokenEOF, lexer->line, lexer->col);
    case '"':
        return token_construct(_lexer_read_string_literal(lexer), TokenStr, lexer->line, lexer->col);
    case '\'':
        return token_construct(_lexer_read_char_literal(lexer), TokenChar, lexer->line, lexer->col);
    case '#': {
        token_t num_type = TokenIllegal; // Illegal by default until set
        char *literal = _lexer_read_numeric_literal(lexer, &num_type);
        return token_construct(literal, num_type, lexer->line, lexer->col);
    }
    }

    if (is_letter(lexer->character) || lexer->character == '_') {
        char *identifier = _lexer_read_identifier(lexer);
        token_t ident_type = TokenIdentifier;

        if (is_operator(identifier)) {
            ident_type = TokenOperator;
            string_to_uppercase(identifier);
        } else if (is_register(identifier)) {
            ident_type = TokenRegister;
            string_to_uppercase(identifier);
        } else if (is_special_register(identifier)) {
            ident_type = TokenSpecialRegister;
            string_to_uppercase(identifier);
        }
        return token_construct(identifier, ident_type, lexer->line, lexer->col);
    }

    lexer_fatal_error(lexer, "Illegal token.");
    return NULL;
}
