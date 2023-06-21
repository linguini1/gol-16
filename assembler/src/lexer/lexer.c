/* Library that contains the tools needed to tokenize an assembly file. */
#include "lexer.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

/* Tokens */
Token *token_construct(char const *literal, token_t type) {
    Token *token = malloc(sizeof(Token));
    token->literal = literal;
    token->type = type;
    return token;
}

void token_destruct(Token *token) {
    free(&(token->literal));
    free(token);
}

/* Lexer */
Lexer *lexer_construct(FILE *fptr) {
    Lexer *lexer = malloc(sizeof(Lexer));
    lexer->stream = fptr;
    lexer->character = _lexer_peek(lexer);
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
    case '#': {
        token_t num_type = TokenIllegal; // Illegal by default until set
        return token_construct(_lexer_read_numeric_literal(lexer, &num_type), num_type);
    }
    case '"':
        return token_construct(_lexer_read_string_literal(lexer), TokenStr);
    case '\'':
        return token_construct(_lexer_read_char_literal(lexer), TokenChar);
    }

    if (is_letter(lexer->character)) {
        return token_construct(_lexer_read_identifier(lexer), TokenIdentifier);
    }

    return token_construct(NULL, TokenIllegal);
}

/* Helper internals */
static char _lexer_peek(Lexer *lexer) {
    char c = fgetc(lexer->stream);
    ungetc(c, lexer->stream);
    return c;
}

static void _lexer_read_char(Lexer *lexer) { lexer->character = fgetc(lexer->stream); }

static char *_lexer_slice(Lexer *lexer, long start) {
    size_t length = ftell(lexer->stream) - (start - 1); // Start 1 char before current char

    // Store identifier
    char *slice = malloc(length + 1);

    fseek(lexer->stream, -length, SEEK_CUR); // Send file stream to start position of slice
    fseek(lexer->stream, 0, SEEK_CUR); // Reset stream (saw on StackOverflow lol)

    fgets(slice, length, lexer->stream);

    fseek(lexer->stream, 0, start + length); // Send file stream to end position of slice
    fseek(lexer->stream, 0, SEEK_CUR);

    // Add null terminator
    slice[length] = '\0';

    return slice;
}

static void _lexer_skip_whitespace(Lexer *lexer) {
    while (is_whitespace(lexer->character)) {
        _lexer_read_char(lexer);
    }
}

static void _lexer_skip_comment(Lexer *lexer) {

    // Early return if not comment
    if (lexer->character != ';') {
        return;
    }

    printf("COMMENT: "); // TODO remove debug print
    while (lexer->character != '\n') {
        _lexer_read_char(lexer);
        printf("%c", lexer->character);
    }
}

static char *_lexer_read_identifier(Lexer *lexer) {

    long start_pos = ftell(lexer->stream);
    while (is_letter(lexer->character) || lexer->character == '_' || is_num(lexer->character)) {
        _lexer_read_char(lexer);
    }
    char *ident = _lexer_slice(lexer, start_pos);
    printf("Identifier: %s\n", ident);

    return ident;
}

static char *_lexer_read_bin_literal(Lexer *lexer) {
    long start = ftell(lexer->stream);
    while (is_bin(lexer->character)) {
        _lexer_read_char(lexer);
    }
    return _lexer_slice(lexer, start);
}

static char *_lexer_read_hex_literal(Lexer *lexer) {
    long start = ftell(lexer->stream);
    while (is_hex(lexer->character)) {
        _lexer_read_char(lexer);
    }
    return _lexer_slice(lexer, start);
}

static char *_lexer_read_decimal_literal(Lexer *lexer) {
    long start = ftell(lexer->stream) - 1; // First digit is skipped in read_numeric
    while (is_num(lexer->character)) {
        _lexer_read_char(lexer);
    }
    return _lexer_slice(lexer, start);
}

static char *_lexer_read_numeric_literal(Lexer *lexer, token_t *type) {

    long start_pos = ftell(lexer->stream);

    // Initial read to determine literal type
    _lexer_read_char(lexer);

    if (lexer->character == '0') {
        // Read next character to determine hex or bin character
        _lexer_read_char(lexer);

        if (lexer->character == 'b') {
            *type = TokenBin;
            return _lexer_read_bin_literal(lexer);
        } else if (lexer->character == 'x') {
            *type = TokenHex;
            return _lexer_read_hex_literal(lexer);
        } else {
            *type = TokenIllegal;
            return NULL; // Undefined literal
        }
    }
    // Detected a decimal number
    *type = TokenDec;
    return _lexer_read_decimal_literal(lexer);

    char *signifier = NULL; // Detects if there is an x or b for hex/bin literals
    while (is_num(lexer->character)) {
        _lexer_read_char(lexer);
    }

    return _lexer_slice(lexer, start_pos);
}

static char *_lexer_read_string_literal(Lexer *lexer) {
    long start_pos = ftell(lexer->stream);
    while (lexer->character != '"') {
        _lexer_read_char(lexer);
    }
    return _lexer_slice(lexer, start_pos);
}

static char *_lexer_read_char_literal(Lexer *lexer) {
    long start_pos = ftell(lexer->stream);
    while (lexer->character != '\'') {
        _lexer_read_char(lexer);
    }
    return _lexer_slice(lexer, start_pos);
}

/* Character classification */
bool is_letter(char c) { return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z'); }
bool is_num(char c) { return '0' <= c && c <= '9'; }
bool is_bin(char c) { return c == '0' || c == '1'; }
bool is_hex(char c) { return is_num(c) || ('A' <= c && c <= 'F') || ('a' <= c && c <= 'f'); }
bool is_whitespace(char c) { return c == ' ' || c == '\n' || c == '\t' || c == '\r'; }
