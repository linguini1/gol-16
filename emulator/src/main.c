#include "components.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

static char *filename = NULL;
static uint16_t pc = 0;

int main(int argc, char **argv) {

    // Get program to run
    if (argc != 2) {
        fprintf(stderr, "You must provide an input program file.\n");
        return EXIT_FAILURE;
    }

    // Open program
    filename = argv[1];
    FILE *program = fopen(filename, "rb");
    if (program == NULL) {
        fprintf(stderr, "Could not open file '%s'.\n", filename);
        return EXIT_FAILURE;
    }

    // Display program
    while (!feof(program) && !ferror(program)) {
        uint16_t word = fetch_word(program, pc);
        printf("original: %04x\n", word);
        pc++;
    }

    // Close stream when done
    fclose(program);

    return EXIT_SUCCESS;
}
