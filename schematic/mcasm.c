/* Tool for assembling micro code into binary from a .orgmc file */
#include "src/hashmap.h"
#include "src/lexer.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void print_map(hmap_t *map); // TODO remove
signal_bf_t *microcode_construct(unsigned len);
void write_microcode(signal_bf_t *microcode, unsigned len, const char *file_path);
void record_states(Lexer *lexer, hmap_t *states);
void record_microcode(Lexer *lexer, signal_bf_t *microcode, hmap_t *states, hmap_t *signals);

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
    signal_bf_t *microcode = microcode_construct(states->key_count);
    record_microcode(lexer, microcode, states, signals);

    // Clean up
    lexer_destruct(lexer);
    hmap_destruct(signals);
    hmap_destruct(states);

    // Write microcode file
    write_microcode(microcode, states->key_count, "./mcode.o");

    free(microcode);
    puts("Success!");
    return EXIT_SUCCESS;
}

/* Print hash map chains TODO remove */
void print_map(hmap_t *map) {
    for (unsigned i = 0; i < map->__backing_len; i++) {
        for (hmap_entry_t *e = map->entries[i]; e != NULL; e = e->next)
            printf("(%s, %llx) -> ", e->name, e->value);
        putchar('\n');
    }
}

/* Allocate microcode buffer. */
signal_bf_t *microcode_construct(unsigned len) {
    signal_bf_t *microcode = malloc(sizeof(signal_bf_t) * len);
    for (unsigned i = 0; i < len; i++)
        microcode[i] = 0;
    return microcode;
}

/* Write microcode buffer to output file. */
void write_microcode(signal_bf_t *microcode, unsigned len, const char *file_path) {
    FILE *fptr = fopen("./mcode.o", "wb");

    if (fptr == NULL) {
        puts("Problem writing to output file.");
        exit(EXIT_FAILURE);
    }

    // Write one byte at a time to preserve big-endianness
    size_t signal_size = sizeof(signal_bf_t);
    for (unsigned i = 0; i < len; i++) {
        for (size_t b = 0; b < signal_size; b++) {
            signal_bf_t mask = 0xFF;
            unsigned shift_dist = (signal_size - b - 1) * 8;
            mask = mask << shift_dist;
            uint8_t byte_chunk = (microcode[i] & mask) >> shift_dist;
            fwrite(&byte_chunk, 1, 1, fptr);
        }
    }
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
void record_microcode(Lexer *lexer, signal_bf_t *microcode, hmap_t *states, hmap_t *signals) {

    signal_bf_t cur_state = -1;

    while (!lexer_finished(lexer)) {
        token_t *token = lexer_next_token(lexer);

        switch (token->type) {
        case TokenIllegal:
            illegal_token(token->name);
        case TokenSignal: {
            signal_bf_t *bitmask = hmap_get(signals, token->name);
            if (bitmask == NULL) {
                printf("Invalid signal %s\n", token->name);
                exit(EXIT_FAILURE);
            }
            microcode[cur_state] |= *bitmask;
            break;
        }
        case TokenState:
            cur_state = *hmap_get(states, token->name);
            break;
        case TokenEOF:
            return; // Token stream depleted
        }

        token_destruct(token); // Delete as used
    }
}
