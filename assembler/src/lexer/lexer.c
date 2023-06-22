/* Library that contains the tools needed to tokenize an assembly file. */
#include "lexer.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Tokens */
Token *token_construct(char *literal, token_t type) {
    Token *token = malloc(sizeof(Token));
    token->literal = literal;
    token->type = type;
    return token;
}

void token_destruct(Token *token) {
    if (token->literal != NULL) {
        free(token->literal);
    }
    free(token);
}

/* Lexer */
Lexer *lexer_construct(FILE *fptr) {
    Lexer *lexer = malloc(sizeof(Lexer));
    lexer->stream = fptr;
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
        return token_construct(NULL, TokenComma);
    case '[':
        _lexer_read_char(lexer);
        return token_construct(NULL, TokenLBrack);
    case ']':
        _lexer_read_char(lexer);
        return token_construct(NULL, TokenRBrack);
    case '{':
        _lexer_read_char(lexer);
        return token_construct(NULL, TokenLBrack);
    case '}':
        _lexer_read_char(lexer);
        return token_construct(NULL, TokenRBrack);
    case -1:
        return token_construct(NULL, TokenEOF);
    case '"':
        return token_construct(_lexer_read_string_literal(lexer), TokenStr);
    case '\'':
        return token_construct(_lexer_read_char_literal(lexer), TokenChar);
    case '#': {
        token_t num_type = TokenIllegal; // Illegal by default until set
        char *literal = _lexer_read_numeric_literal(lexer, &num_type);
        return token_construct(literal, num_type);
    }
    }

    if (is_letter(lexer->character) || lexer->character == '_') {
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
    size_t length = ftell(lexer->stream) - (start - 1);

    // Store identifier
    char *slice = malloc(length + 1);

    fseek(lexer->stream, -length, SEEK_CUR); // Send file stream to start position of slice
    fseek(lexer->stream, 0, SEEK_CUR);       // Reset stream (saw on StackOverflow lol)

    fgets(slice, length, lexer->stream);
    fseek(lexer->stream, 1, SEEK_CUR); // Correct by one
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

    // Early return if not comment
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
    char *ident = _lexer_slice(lexer, start);

    return ident;
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
    char *num = _lexer_slice(lexer, start);
    return num;
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
    char *num = _lexer_slice(lexer, start);
    return num;
}

static char *_lexer_read_decimal_literal(Lexer *lexer) {
    long start = ftell(lexer->stream);
    while (is_num(lexer->character)) {
        _lexer_read_char(lexer);
    }
    char *num = _lexer_slice(lexer, start);
    return num;
}

static char *_lexer_read_numeric_literal(Lexer *lexer, token_t *type) {

    long start_pos = ftell(lexer->stream);

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
