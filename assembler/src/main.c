/* An assembler for the original gol-16 assembly language (org-asm) */
#include "lexer/lexer.h"
#include <stdio.h>

char *filename = "./test.orgasm";

int main(int argc, char *argv[]) {

    // Open file
    FILE *fptr = fopen(filename, "r");
    if (fptr == NULL) {
        printf("Assembly file %s could not be read.\n", filename);
        return -1;
    }

    // Create lexer
    printf("Reading from %s\n", filename);
    Lexer *lexer = lexer_construct(fptr);

    // Parse some tokens
    Token *t = lexer_next_token(lexer);

    // Teardown
    lexer_destruct(lexer);
}
