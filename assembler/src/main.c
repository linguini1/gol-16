/* An assembler for the original gol-16 assembly language (org-asm) */
#include "conversion/analyzer.h"
#include "conversion/instructions.h"
#include "conversion/lexer.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {

    // Grab file name from arguments
    if (argc > 3) {
        printf("Too many arguments.");
        return EXIT_FAILURE;
    } else if (argc == 1) {
        printf("Too few arguments.");
        return EXIT_FAILURE;
    }

    const char *in_file = argv[1];
    const char *out_file;
    if (argc == 3) {
        out_file = argv[2];
    } else {
        out_file = DEFAULT_OUT_FILE;
    }

    // Create lexer
    Lexer *lexer = lexer_construct(in_file);

    if (lexer == NULL) {
        printf("Could not read from %s: ensure file is of type '%s'.", in_file, FILE_SUFFIX);
    }

    // Parse some tokens
    TokenList *list = token_list_construct(1);
    token_list_append(list, token_construct("START", TokenStart, 0));

    while (!lexer_eof(lexer) && !lexer_err(lexer) && token_list_get(list, -1)->type != TokenIllegal) {
        token_list_append(list, lexer_next_token(lexer));
    }

    // Handle errors
    if (lexer_err(lexer)) {
        lexer_print_error(lexer);
        return EXIT_FAILURE;
    }
    lexer_destruct(lexer);

    // Create analyzer
    Analyzer *analyzer = analyzer_construct(list);
    InstructionList *instructions = instruction_list_construct(1);
    while (!analyzer_finished(analyzer) && !analyzer_err(analyzer)) {
        instruction_list_append(instructions, analyzer_next_instruction(analyzer));
    }

    // Analyzer error handling
    if (analyzer_err(analyzer)) {
        analyzer_print_error(analyzer);
        return EXIT_FAILURE;
    }
    analyzer_destruct(analyzer);

    // Write instructions to output
    bool success = write_all_instructions(instructions, out_file);
    if (!success) {
        printf("Could not write to file %s. Ensure that file is of type '%s'.\n", out_file, OBJ_FILE_SUFFIX);
    }
    instruction_list_destruct(instructions);

    puts("File parsed successfully.");
    return EXIT_SUCCESS;
}
