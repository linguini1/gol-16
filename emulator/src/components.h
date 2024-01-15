#ifndef _COMPONENTS_H_
#define _COMPONENTS_H_

#include <stdint.h>
#include <stdio.h>

typedef enum {
    REG_R0 = 0,
    REG_R1 = 1,
    REG_R2 = 2,
    REG_R3 = 3,
    REG_SP,
    REG_PC,
    REG_LR,
} Register;

typedef uint16_t instruction_t;
typedef uint16_t word_t;

instruction_t fetch_word(FILE *program, word_t addr);

#endif // _COMPONENTS_H_
