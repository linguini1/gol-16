#include "components.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

static uint16_t pc = 0;

int main(int argc, char **argv) {

    // Get program to run
    if (argc != 3) {
        fprintf(stderr, "You must provide microcode for the processor and an input program file.\n");
        return EXIT_FAILURE;
    }

    // Open microcode
    FILE *microcode = fopen(argv[1], "rb");
    if (microcode == NULL) {
        fprintf(stderr, "Could not open microcode file '%s'\n", argv[1]);
        return EXIT_FAILURE;
    }

    // Open program
    FILE *program = fopen(argv[2], "rb");
    if (program == NULL) {
        fprintf(stderr, "Could not open program file '%s'.\n", argv[2]);
        return EXIT_FAILURE;
    }

    // Display program
    while (!feof(program) && !ferror(program)) {
        uint16_t word = fetch_word(program, pc);
        printf("%04x\n", word);
        pc++;
    }

    uint8_t addr = 0;
    while (!feof(microcode) && !ferror(microcode)) {
        uint64_t signals = fetch_signals(microcode, addr);
        printf("%016llx\n", signals);
        addr++;
    }

    // Close stream when done
    fclose(microcode);
    fclose(program);

    return EXIT_SUCCESS;
}
