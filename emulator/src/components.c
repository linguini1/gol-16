#include "components.h"
#include <stdio.h>

/**
 * Reads the word at the given address from the program (like main memory). WARNING: This function assumes that the
 * address is valid.
 * @param program The file stream for the program.
 * @param addr The address of the word to read.
 * @return The word stored at the given address.
 */
word_t fetch_word(FILE *program, word_t addr) {

    long pos = ftell(program);
    if (pos > addr * 2) {
        fseek(program, pos - addr * 2, SEEK_CUR);
    } else {
        fseek(program, addr * 2 - pos, SEEK_CUR);
    }

    word_t word;
    fread(&word, sizeof(word), 1, program);
    return (uint16_t)((word & 0x00FF) << 8) | (uint16_t)(word >> 8);
}
