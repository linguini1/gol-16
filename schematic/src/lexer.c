/* Implements a lexer for micro code files */
#include "lexer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char SUFFIX[] = ".orgmc"; // Micro-code file suffix

/* Tokens */
/* Constructs a token. */
token_t *token_construct(char *name, enum TokenType type) {
    token_t *token = malloc(sizeof(token_t));
    token->name = name;
    token->type = type;
    return token;
}

/* Destructs a token */
void token_destruct(token_t *token) {
    free(token->name);
    free(token);
}

/* Detects if the file pointed to by the path is a gol-16 micro-code file. */
static bool is_microcode_file(const char *file_path) {
    const char *end = &file_path[strlen(file_path)];
    while (*--end != '.')
        ;
    return !strcmp(end, SUFFIX);
}

static bool is_whitespace(char c) { return c == ' ' || c == '\t' || c == '\n' || c == '\r'; }
static bool is_alpha(char c) { return 0x41 <= c && c <= 0x7A; }
static bool is_alphanum(char c) { return (0x30 <= c && c <= 0x39) || is_alpha(c); }

/* Reads the next character */
static void lexer_read_char(Lexer *lexer) { lexer->character = fgetc(lexer->stream); }

/* Detects if the lexer has hit the end of the file */
bool lexer_finished(Lexer *lexer) { return lexer->character == EOF; }

/* Grabs a slice of the file input. */
static char *lexer_slice(Lexer *lexer, long start) {
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

static void lexer_skip_whitespace(Lexer *lexer) {
    while (is_whitespace(lexer->character))
        lexer_read_char(lexer);
}

static void lexer_skip_comment(Lexer *lexer) {
    if (lexer->character != ';') return;
    while (lexer->character != '\n' && !lexer_finished(lexer))
        lexer_read_char(lexer);
}

static token_t *lexer_read_ident(Lexer *lexer, bool ns) {

    if (ns) lexer_read_char(lexer); // Skip preceding #

    long start = ftell(lexer->stream);
    while (is_alphanum(lexer->character))
        lexer_read_char(lexer);
    char *name = lexer_slice(lexer, start);

    // Skip intermediate white space which is allowed
    while (lexer->character == ' ' || lexer->character == '\t' || lexer->character == '\r')
        lexer_read_char(lexer);

    if (ns) return token_construct(name, TokenNS);

    if (lexer->character == ':') {
        lexer_read_char(lexer); // Skip :
        return token_construct(name, TokenState);
    }
    if (lexer->character == ',' || lexer->character == '\n') {
        lexer_read_char(lexer); // Skip , or \n
        return token_construct(name, TokenSignal);
    }
    return token_construct(NULL, TokenIllegal);
}

token_t *lexer_next_token(Lexer *lexer) {

    // Skip comments and white space
    while (is_whitespace(lexer->character) || lexer->character == ';') {
        lexer_skip_whitespace(lexer);
        lexer_skip_comment(lexer);
    }

    if (lexer->character == '#') {
        return lexer_read_ident(lexer, true);
    }

    if (!is_alpha(lexer->character)) return token_construct(NULL, TokenIllegal);

    token_t *token = lexer_read_ident(lexer, false);
    return token;
}

/* Creates a lexer, handling file errors. */
Lexer *lexer_construct(const char *filepath) {
    if (!is_microcode_file(filepath)) {
        printf("Fatal: %s is not a gol-16 microcode (%s) file.\n", filepath, SUFFIX);
        exit(EXIT_FAILURE);
    }

    FILE *fptr = fopen(filepath, "rb");
    if (fptr == NULL) {
        printf("Fatal: %s could not be read from.", filepath);
        exit(EXIT_FAILURE);
    }
    Lexer *lexer = malloc(sizeof(Lexer));
    lexer->stream = fptr;
    lexer_read_char(lexer); // Read first char
    return lexer;
}

/* Frees a lexer. */
void lexer_destruct(Lexer *lexer) {
    fclose(lexer->stream);
    free(lexer);
}

/* Resets the lexer to the file start. */
void lexer_reset(Lexer *lexer) {
    rewind(lexer->stream);
    lexer_read_char(lexer);
}
