/* An assembler for the original gol-16 assembly language (org-asm) */
#include "conversion/analyzer.h"
#include "conversion/instructions.h"
#include "conversion/lexer.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {

    // Grab file name from arguments
    if (argc > 2) {
        printf("Too many arguments.");
        return EXIT_FAILURE;
    } else if (argc == 1) {
        printf("Too few arguments.");
        return EXIT_FAILURE;
    }

    char *filename = argv[1];

    if (!is_orgasm_file(filename)) {
        printf("Unexpected file type.");
        return EXIT_FAILURE;
    }

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
    TokenList *list = token_list_construct(1);
    token_list_append(list, token_construct("START", TokenStart));

    while (token_list_get(list, -1)->type != TokenEOF && token_list_get(list, -1)->type != TokenIllegal) {
        token_list_append(list, lexer_next_token(lexer));
    }
    lexer_destruct(lexer);

    // Handle illegal token error
    if (token_list_get(list, -1)->type == TokenIllegal) {
        printf("Parsing error: Illegal token on line %d\n", lexer->line);
        return EXIT_FAILURE;
    }

    // Create analyzer
    Analyzer *analyzer = analyzer_construct(list);
    char *err_msg = NULL;
    InstructionList *instructions = instruction_list_construct(1);
    while (err_msg == NULL && !analyzer_finished(analyzer)) {
        instruction_list_append(instructions, analyzer_next_instruction(analyzer, &err_msg));
    }

    // Analyzer error handling
    if (err_msg != NULL) {
        analyzer_print_error(analyzer, err_msg);
        return EXIT_FAILURE;
    }
    analyzer_destruct(analyzer);

    puts("File parsed successfully.");

    return EXIT_SUCCESS;
}
