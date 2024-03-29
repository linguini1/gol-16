/* Tool for assembling micro code into binary from a gol-16 microcode file */
#include "hashmap.h"
#include "lexer.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define OPCODE_COUNT 32
static uint8_t DECODE_ROM[OPCODE_COUNT] = {0};

typedef struct MicroCode {
    unsigned len;
    signal_bf_t *code;
} mcode_t;

mcode_t *microcode_construct(unsigned len);
void write_microcode(mcode_t *microcode, const char *file_path);
void write_decode_rom(const char *file_path);
void record_states(Lexer *lexer, hmap_t *states);
void record_microcode(Lexer *lexer, mcode_t *microcode, hmap_t *states, hmap_t *signals);

int main(int argc, char *argv[]) {

    if (argc != 2) {
        puts("Please provide a path to the micro-code file.");
        return EXIT_FAILURE;
    }
    const char *file_path = argv[1];
    Lexer *lexer = lexer_construct(file_path);

    hmap_t *signals = valid_signals(); // Signal lookup

    // State lookup construction
    hmap_t *states = hmap_construct(15);
    record_states(lexer, states);

    lexer_reset(lexer); // Reset for pass 2

    // Create microcode
    mcode_t *mcode = microcode_construct(states->key_count);
    record_microcode(lexer, mcode, states, signals);

    // Clean up
    lexer_destruct(lexer);
    hmap_destruct(signals);
    hmap_destruct(states);

    // Write microcode file
    write_microcode(mcode, "./mcode.o");

    // Write decode ROM file
    write_decode_rom("./decode.o");

    free(mcode);
    puts("Success!");
    return EXIT_SUCCESS;
}

/* Allocate microcode buffer. */
mcode_t *microcode_construct(unsigned len) {
    mcode_t *mcode = malloc(sizeof(mcode_t));
    mcode->code = malloc(sizeof(signal_bf_t) * len);
    mcode->len = len;
    for (unsigned i = 0; i < len; i++)
        mcode->code[i] = 0;
    return mcode;
}

/* Write microcode buffer to output file. */
void write_microcode(mcode_t *microcode, const char *file_path) {
    FILE *fptr = fopen("./mcode.o", "wb");

    if (fptr == NULL) {
        printf("Could not write to file '%s'.\n", file_path);
        exit(EXIT_FAILURE);
    }

    // Write one byte at a time to preserve big-endianness
    size_t signal_size = sizeof(signal_bf_t);
    for (unsigned i = 0; i < microcode->len; i++) {
        for (size_t b = 0; b < signal_size; b++) {
            signal_bf_t mask = 0xFF;
            unsigned shift_dist = (signal_size - b - 1) * 8;
            mask = mask << shift_dist;
            uint8_t byte_chunk = (microcode->code[i] & mask) >> shift_dist;
            fwrite(&byte_chunk, 1, 1, fptr);
        }
    }
    fclose(fptr);
}

/* Writes decode ROM contents to a file. */
void write_decode_rom(const char *file_path) {
    FILE *fptr = fopen(file_path, "wb");
    if (fptr == NULL) {
        printf("Could not write to file '%s'\n", file_path);
        exit(EXIT_FAILURE);
    }

    // Find the lowest multiple of STATE_ADDRESS_BITS which forms an whole number of bytes
    unsigned multiple = 1;
    while ((multiple * STATE_ADDRESS_BITS) % 8 != 0)
        multiple++;
    // unsigned num_bytes = (multiple * STATE_ADDRESS_BITS) / 8; // Number of bytes required

    // TODO make this dynamic based on STATE_ADDRESS_BITS instead of hard-coded for value 8
    for (unsigned i = 0; i < OPCODE_COUNT; i++)
        fwrite(&DECODE_ROM[i], 1, 1, fptr);
    fclose(fptr);
}

/* Throw illegal token error. */
void illegal_token(const char *val) {
    printf("Illegal token '%s'\n", val);
    exit(EXIT_FAILURE);
}

/* Stores defined states in a hash map. */
void record_states(Lexer *lexer, hmap_t *states) {

    signal_bf_t state_count = 0;
    while (!lexer_finished(lexer)) {
        token_t *token = lexer_next_token(lexer);
        if (state_count >> STATE_ADDRESS_BITS != 0) {
            printf("Address count of %lld exceeds state address bit limit of %d.\n", state_count, STATE_ADDRESS_BITS);
            exit(EXIT_FAILURE);
        }

        if (token->type == TokenIllegal) illegal_token(token->name);

        if (token->type == TokenState) {
            hmap_add_entry(states, token->name, state_count++);
            token->name = NULL; // Prevents name string from being freed later
        }

        token_destruct(token); // Free as we go
    }
}

/* Store the signals and next states within the code. */
void record_microcode(Lexer *lexer, mcode_t *microcode, hmap_t *states, hmap_t *signals) {

    signal_bf_t cur_state = -1;

    while (!lexer_finished(lexer)) {
        token_t *token = lexer_next_token(lexer);

        switch (token->type) {
        case TokenIllegal:
            illegal_token(token->name);
        case TokenSignal: {
            signal_bf_t *bitmask = hmap_get(signals, token->name);
            if (bitmask == NULL) {
                printf("Invalid signal: '%s'\n", token->name);
                exit(EXIT_FAILURE);
            }
            microcode->code[cur_state] |= *bitmask;
            break;
        }
        case TokenState:
            cur_state = *hmap_get(states, token->name);
            break;
        case TokenNS: {
            signal_bf_t *bitmask = hmap_get(states, token->name);
            if (bitmask == NULL) {
                printf("Invalid state: '%s'\n", token->name);
                return exit(EXIT_FAILURE);
            }
            microcode->code[cur_state] |= *bitmask;
            break;
        }
        case TokenOpcode: {
            unsigned op = strtol(token->name, NULL, 16);
            DECODE_ROM[op] = cur_state;
            break;
        }
        case TokenEOF:
            return; // Token stream depleted
        }

        token_destruct(token); // Delete as used
    }
}
