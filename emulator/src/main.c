#include <stdio.h>
#include <stdlib.h>

static char *filename = NULL;

int main(int argc, char **argv) {

    // Get program to run
    if (argc != 2) {
        fprintf(stderr, "You must provide an input program file.\n");
        return EXIT_FAILURE;
    }

    filename = argv[1];
    FILE *program = fopen(filename, "r");
    if (program == NULL) {
        fprintf(stderr, "Could not open file '%s'.\n", filename);
        return EXIT_FAILURE;
    }

    fclose(program);

    return EXIT_SUCCESS;
}
