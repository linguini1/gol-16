/* Library that contains the tools needed to tokenize an assembly file. */
#include "lexer.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

/* Helper functions */
bool is_letter(char c) { return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z'); }
bool is_num(char c) { return '0' <= c && c <= '9'; }
bool is_whitespace(char c) { return c == ' ' || c == '\n' || c == '\t' || c == '\r'; }

/* Tokens */
Token *token_construct(char const *literal, TokenT type) {
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

    // Get file length
    fseek(fptr, 0, SEEK_END);
    lexer->input_length = ftell(fptr);
    fseek(fptr, 0, SEEK_SET); // Reset position to beginning again

    return lexer;
}

void lexer_destruct(Lexer *lexer) {
    fclose(lexer->stream);
    free(lexer);
}

static char _lexer_peek(Lexer *lexer) {
    char c = fgetc(lexer->stream);
    ungetc(c, lexer->stream);
    return c;
}

static void _lexer_read_char(Lexer *lexer) {
    if (ftell(lexer->stream) == lexer->input_length) {
        lexer->character = '\0';
        return;
    }
    lexer->character = fgetc(lexer->stream);
}

static void _lexer_skip_whitespace(Lexer *lexer) {
    while (is_whitespace(lexer->character)) {
        _lexer_read_char(lexer);
    }
}

static void _lexer_skip_comment(Lexer *lexer) {
    while (lexer->character != '\n') {
        _lexer_read_char(lexer);
    }
}

static char *_lexer_slice(Lexer *lexer, long start, size_t *length) {
    // Ensure that length pointer is not null before calculation
    if (length) {
        *length = ftell(lexer->stream) - start;
    }

    // Store identifier
    char *identifier = malloc(*length);
    fseek(lexer->stream, 0, start);
    fgets(identifier, *length, lexer->stream);
    fseek(lexer->stream, 0, start + *length);

    return identifier;
}

static char *_lexer_read_identifier(Lexer *lexer, size_t *length) {

    long start_pos = ftell(lexer->stream);
    while (is_letter(lexer->character) || lexer->character == '_' || is_num(lexer->character)) {
        _lexer_read_char(lexer);
    }

    return _lexer_slice(lexer, start_pos, length);
}

static char *_lexer_read_num(Lexer *lexer, size_t *length) {

    long start_pos = ftell(lexer->stream);
    while (is_num(lexer->character)) {
        _lexer_read_char(lexer);
    }

    return _lexer_slice(lexer, start_pos, length);
}
