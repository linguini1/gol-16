#include "../src/components.h"
#include <assert.h>
#include <stdio.h>

static void test_alu_add(void) {
    uint8_t flags;

    assert(alu(ALU_ADD, 1, 3, &flags) == 4);
    assert(flags == 0);

    assert(alu(ALU_ADD, 0xFFFF, 1, &flags) == 0);
    assert(flags & FLAG_ZERO);
    assert(flags & FLAG_OVERFLOW);

    assert(alu(ALU_ADD, 0xFFFE, 1, &flags) == 0xFFFF);
    assert(flags & FLAG_NEGATIVE);
}

static void test_alu_and(void) {
    uint8_t flags;

    assert(alu(ALU_AND, 0x0F, 0xF0, &flags) == 0);
    assert(flags & FLAG_ZERO);

    assert(alu(ALU_AND, 0x01, 0x02, &flags) == 0x1);
    assert(flags == 0);

    assert(alu(ALU_AND, 0xFF, 0xFF, &flags) == 0x1);
    assert(flags & FLAG_NEGATIVE);
}

static void test_alu_or(void) {
    uint8_t flags;

    assert(alu(ALU_OR, 0x0F, 0xF0, &flags) == 0xFF);
    assert(flags & FLAG_NEGATIVE);

    assert(alu(ALU_OR, 0x0, 0x0, &flags) == 0x0);
    assert(flags & FLAG_ZERO);

    assert(alu(ALU_AND, 0x0, 0x07, &flags) == 0x7);
    assert(flags == 0);
}

int main(void) {

    printf("Running tests...");

    /* ALU TESTS */
    test_alu_add();
    test_alu_and();
    test_alu_or();

    return 0;
}
