/* An assembler for the original gol-16 assembly language (org-asm) */
#include "lexer/lexer.h"
#include <stdio.h>
#include <stdlib.h>

char *filename = "./test.orgasm";

int main(int argc, char *argv[]) {

    // Open file
    FILE *fptr = fopen(filename, "rb");
    if (fptr == NULL) {
        printf("Assembly file %s could not be read.\n", filename);
        return EXIT_FAILURE;
    }

    // Create lexer
    printf("Reading from %s\n", filename);
    Lexer *lexer = lexer_construct(fptr);

    // Parse some tokens
    Token *token = token_construct(NULL, TokenStart);
    while (token->type != TokenEOF && token->type != TokenIllegal) {
        token = lexer_next_token(lexer);
    }

    // Handle illegal token error
    if (token->type == TokenIllegal) {
        printf("Parsing error: Illegal token at position %d\n", ftell(lexer->stream));
        return EXIT_FAILURE;
    }
    printf("File parsed successfully.");

    // Teardown
    lexer_destruct(lexer);
    return EXIT_SUCCESS;
}
