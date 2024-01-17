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

/**
 * Reads the signals at the given address (state) of the decode ROM.
 * @param decode_rom The file stream for the decode ROM/microcode.
 * @param addr The address of the signals to read.
 * @return The signals stored at the given address.
 */
signals_t fetch_signals(FILE *decode_rom, uint8_t addr) {

    fseek(decode_rom, addr * 2, SEEK_SET);

    signals_t signals;
    fread(&signals, sizeof(signals), 1, decode_rom);
    return signals; // Don't care that this is little-endian, because signals are bit values
}

static word_t rotr(word_t a, word_t n) { return (a >> n) | (a << (sizeof(word_t) * 8 - n)); }
static word_t rotl(word_t a, word_t n) { return (a << n) | (a >> (sizeof(word_t) * 8 - n)); }

/**
 * Performs ALU operations.
 * @param op The ALU operation to perform.
 * @param a The operand a for the calculation.
 * @param b The operand b for the calculation.
 * @param flags A reference to the flags register to store resulting flags. Bottom four bits are used for COZN.
 */
word_t alu(ALUOperation op, word_t a, word_t b, uint8_t *flags) {

    word_t result = 0;
    *flags = 0;

    switch (op) {
    case ALU_AND:
        result = a & b;
        break;
    case ALU_OR:
        result = a | b;
        break;
    case ALU_NOT:
        result = ~b;
        break;
    case ALU_RB:
        result = b;
        break;
    case ALU_ADD:
        result = a + b;
        if (result < a || result < b) *flags |= (0xFF & FLAG_OVERFLOW); // TODO is this right?
        break;
    case ALU_SUB:
        result = a - b;
        break;
    case ALU_MUL:
        result = a * b;
        break;
    case ALU_DIV:
        if (b == 0)
            result = 0;
        else
            result = a / b;
        break;
    case ALU_LSL:
        result = a << b;
        break;
    case ALU_LSR:
        result = a >> b;
        break;
    case ALU_ROL:
        result = rotl(a, b);
        break;
    case ALU_ROR:
        result = rotr(a, b);
        break;
    case ALU_NOOP:
        break;
    }

    // TODO implement flags for all operations
    if (result == 0 && op != ALU_NOOP) *flags |= (0xFF & FLAG_ZERO);
    if (result & 0x8000) *flags |= (0xFF & FLAG_NEGATIVE);

    return result;
}
