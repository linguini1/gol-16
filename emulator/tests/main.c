#include "../src/components.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>

static void test_alu_add(void) {
    // TODO revisit when carry and overflow are added
    uint8_t flags;

    assert(alu(ALU_ADD, 1, 3, &flags) == 4);
    assert(flags == 0);

    assert(alu(ALU_ADD, 0xFFFF, 1, &flags) == 0);
    assert(flags & FLAG_ZERO);
    assert(flags & FLAG_OVERFLOW);

    assert(alu(ALU_ADD, 0xFFFE, 1, &flags) == 0xFFFF);
    assert(flags & FLAG_NEGATIVE);
}

static void test_alu_sub(void) {
    // TODO revisit when carry and overflow are added
    uint8_t flags;

    assert(alu(ALU_SUB, 5, 1, &flags) == 4);
    assert(flags == 0);

    assert(alu(ALU_SUB, 1, 1, &flags) == 0);
    assert(flags & FLAG_ZERO);

    assert(alu(ALU_SUB, 1, 2, &flags) == 0xFFFF);
    assert(flags & FLAG_NEGATIVE);
}

static void test_alu_and(void) {
    uint8_t flags;

    assert(alu(ALU_AND, 0x0F, 0xF0, &flags) == 0);
    assert(flags & FLAG_ZERO);

    assert(alu(ALU_AND, 0x01, 0x03, &flags) == 0x1);
    assert(flags == 0);

    assert(alu(ALU_AND, 0xFFFF, 0xFFFF, &flags) == 0xFFFF);
    assert(flags & FLAG_NEGATIVE);
}

static void test_alu_or(void) {
    uint8_t flags;

    assert(alu(ALU_OR, 0x00FF, 0xFF00, &flags) == 0xFFFF);
    assert(flags & FLAG_NEGATIVE);

    assert(alu(ALU_OR, 0x0, 0x0, &flags) == 0x0);
    assert(flags & FLAG_ZERO);

    assert(alu(ALU_OR, 0x0, 0x07, &flags) == 0x7);
    assert(flags == 0);
}

static void test_alu_not(void) {
    uint8_t flags;

    assert(alu(ALU_NOT, 0x0F, 0x0, &flags) == 0xFFFF);
    assert(flags & FLAG_NEGATIVE);

    assert(alu(ALU_NOT, 0x0, 0xFFFF, &flags) == 0x0);
    assert(flags & FLAG_ZERO);

    assert(alu(ALU_NOT, 0x0, 0x07, &flags) == (word_t)~0x7);
    assert(flags & FLAG_NEGATIVE);
}

static void test_alu_rb(void) {
    uint8_t flags;

    assert(alu(ALU_RB, 4, 5, &flags) == 5);
    assert(flags == 0);

    assert(alu(ALU_RB, 4, 0, &flags) == 0);
    assert(flags & FLAG_ZERO);

    assert(alu(ALU_RB, 4, 0xFFFF, &flags) == 0xFFFF);
    assert(flags & FLAG_NEGATIVE);
}

static void test_alu_mul(void) {
    // TODO revisit when carry and overflow are added
    uint8_t flags;

    assert(alu(ALU_MUL, 5, 5, &flags) == 25);
    assert(flags == 0);

    assert(alu(ALU_MUL, 0, 10, &flags) == 0);
    assert(flags & FLAG_ZERO);

    assert(alu(ALU_MUL, 13107, 5, &flags) == 0xFFFF);
    assert(flags & FLAG_NEGATIVE);

    assert(alu(ALU_MUL, 13107, 6, &flags) == 13106);
    assert(flags == 0);
}

static void test_alu_div(void) {
    // TODO revisit when carry and overflow are added
    uint8_t flags;

    assert(alu(ALU_DIV, 5, 5, &flags) == 1);
    assert(flags == 0);

    assert(alu(ALU_DIV, 0, 10, &flags) == 0);
    assert(flags & FLAG_ZERO);

    assert(alu(ALU_DIV, 1, 0, &flags) == 0);
    assert(flags & FLAG_ZERO);

    assert(alu(ALU_DIV, 0xFFFF, 13107, &flags) == 5);
    assert(flags == 0);
}

static void test_alu_lsl(void) {
    uint8_t flags;

    assert(alu(ALU_LSL, 2, 1, &flags) == 4);
    assert(flags == 0);

    assert(alu(ALU_LSL, 0x8000, 1, &flags) == 0);
    assert(flags & FLAG_ZERO);

    assert(alu(ALU_LSL, 0x4000, 1, &flags) == 0x8000);
    assert(flags & FLAG_NEGATIVE);
}

static void test_alu_lsr(void) {
    uint8_t flags;

    assert(alu(ALU_LSR, 2, 1, &flags) == 1);
    assert(flags == 0);

    assert(alu(ALU_LSR, 0x1, 1, &flags) == 0);
    assert(flags & FLAG_ZERO);

    assert(alu(ALU_LSR, 0x8000, 2, &flags) == 0x2000);
    assert(flags == 0);
}

static void test_alu_noop(void) {
    uint8_t flags;

    assert(alu(ALU_NOOP, 7, 3, &flags) == 0);
    assert(flags == 0);

    assert(alu(ALU_NOOP, 4, 0, &flags) == 0);
    assert(flags == 0);

    assert(alu(ALU_NOOP, 18, 10002, &flags) == 0);
    assert(flags == 0);
}

static void test_alu_rol(void) {
    uint8_t flags;

    assert(alu(ALU_ROL, 0x8000, 2, &flags) == 0x0002);
    assert(flags == 0);

    assert(alu(ALU_ROL, 0x4000, 1, &flags) == 0x8000);
    assert(flags & FLAG_NEGATIVE);

    assert(alu(ALU_ROL, 0x4000, 2, &flags) == 0x0001);
    assert(flags == 0);

    assert(alu(ALU_ROL, 0x4010, 1, &flags) == 0x8020);
    assert(flags & FLAG_NEGATIVE);
}

static void test_alu_ror(void) {
    uint8_t flags;

    assert(alu(ALU_ROR, 0x8000, 2, &flags) == 0x2000);
    assert(flags == 0);

    assert(alu(ALU_ROR, 0x0001, 1, &flags) == 0x8000);
    assert(flags & FLAG_NEGATIVE);

    assert(alu(ALU_ROR, 0x4010, 1, &flags) == 0x2008);
    assert(flags == 0);
}

int main(void) {

    puts("Running tests...");

    /* ALU TESTS */
    test_alu_add();
    test_alu_sub();
    test_alu_and();
    test_alu_or();
    test_alu_not();
    test_alu_rb();
    test_alu_mul();
    test_alu_div();
    test_alu_lsl();
    test_alu_lsr();
    test_alu_rol();
    test_alu_ror();
    test_alu_noop();

    return 0;
}
