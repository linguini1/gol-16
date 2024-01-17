#ifndef _COMPONENTS_H_
#define _COMPONENTS_H_

#include <stdint.h>
#include <stdio.h>

/** Mask for the carry flag in the flag register. */
#define FLAG_CARRY 0x1
/** Mask for the overflow flag in the flag register. */
#define FLAG_OVERFLOW 0x2
/** Mask for the zero flag in the flag register. */
#define FLAG_ZERO 0x4
/** Mask for the negative flag in the flag register. */
#define FLAG_NEGATIVE 0x8

/** Enumerates all of the register in the gol-16 processor. */
typedef enum {
    REG_R0 = 0x0, /**< Register 0 */
    REG_R1 = 0x1, /**< Register 1 */
    REG_R2 = 0x2, /**< Register 2 */
    REG_R3 = 0x3, /**< Register 3 */
    REG_PC = 0x4, /**< Program counter */
    REG_SP = 0x5, /**< Stack pointer */
    REG_LR = 0x6, /**< Link register */
} Register;

/** Enumerates all of the ALU operations in the gol-16 processor. */
typedef enum {
    ALU_NOOP = 0x0, /**< No operation */
    ALU_ADD = 0x1,  /**< Addition */
    ALU_SUB = 0x2,  /**< Subtraction */
    ALU_MUL = 0x3,  /**< Multiplication */
    ALU_DIV = 0x4,  /**< Division */
    ALU_AND = 0x5,  /**< Logical AND */
    ALU_OR = 0x6,   /**< Logical OR */
    ALU_NOT = 0x7,  /**< Logical NOT (inversion) */
    ALU_LSL = 0x8,  /**< Logical shift left */
    ALU_RB = 0x9,   /**< Pass input B */
    ALU_LSR = 0xA,  /**< Logical shift right */
    ALU_ROL = 0xB,  /**< Logical rotate left */
    ALU_ROR = 0xC,  /**< Logical rotate right */
} ALUOperation;

/** Lists all of the possible condition codes for the gol-16 processor. */
typedef enum {
    COND_EQ = 0x0, /**< Equal */
    COND_NE = 0x1, /**< Not equal */
    COND_HS = 0x2, /**< Higher or same (Unsigned) */
    COND_HI = 0x3, /**< Higher (Unsigned) */
    COND_LO = 0x4, /**< Lower than (Unsigned) */
    COND_LS = 0x5, /**< Lower or same (Unsigned) */
    COND_MI = 0x6, /**< Negative */
    COND_PL = 0x7, /**< Positive or zero */
    COND_VS = 0x8, /**< Overflow */
    COND_VC = 0x9, /**< No overflow */
    COND_GE = 0xA, /**< Greater than or equal to  */
    COND_LT = 0xB, /**< Lower than (Signed) */
    COND_GT = 0xC, /**< Greater than (Signed) */
    COND_LE = 0xD, /**< Less than or equal to (Signed) */
    COND_AL = 0xE, /**< Always */
} ConditionCode;

typedef enum {
    OP_PUSH = 0x00, /**< PUSH {r, ...} | Push passed registers onto the stack and decrement stack pointer. */
    OP_ADD = 0x01,  /**< ADD rd, rx, ry | Adds rx to ry and stores the result into rd. */
    OP_SUB = 0x02,  /**< SUB rd, rx, ry | Subtracts rx from ry and stores the result into rd. */
    OP_MUL = 0x03,  /**< MUL rd, rx, ry | Multiplies rx by ry and stores the result into rd. */
    OP_DIV = 0x04,  /**< DIV rd, rx, ry | Divides rx by ry and stores the result into rd. */
    OP_AND = 0x05,  /**< AND rd, rx, ry | Bitwise AND rx with ry and stores the result into rd. */
    OP_OR = 0x06,   /**< OR rd, rx, ry | Bitwise OR rx with ry and stores the result into rd. */
    OP_NOT = 0x07,  /**< NOT rd, r | Stores bitwise NOT(r) into rd (generates flags). */
    /** LSd/ROd rd, r, imm4 | Logical shift/rotate left/right r by imm4 bits and stores the result into rd. */
    OP_LSd_ROd_IMM = 0x08,
    OP_MOV = 0x09,       /**< MOV rd, r | Copies the value in r to rd. */
    OP_CMP = 0x0A,       /**< CMP rx, ry | Subtract ry from rx to generate flags. */
    OP_RESERVED = 0x0B,  /**< Reserved for future use. */
    OP_LDR_PCREL = 0x0C, /**< LDR rd, [imm9] | Load address imm9 + PC into rd. */
    /** STR rs, [rx, ry] | Add contents of rx and ry and store contents of rs in the resulting address in memory. */
    OP_STR = 0x0D,
    /** LDR rd, [rx, ry] | Add contents of rx and ry and load contents of the resulting address in memory into rd. */
    OP_LDR = 0x0E,
    OP_Bcc = 0x0F,     /**< Bcc imm7 | Branch to address PC + sign-ext(imm7) if cc is true based on flags. */
    OP_POP = 0x10,     /**< POP {r, ...} | Pop passed registers from the stack and increment stack pointer. */
    OP_ADD_IMM = 0x11, /**< ADD rd, r, imm7 | Add r and zero-ext(imm7) and store the result into rd. */
    OP_SUB_IMM = 0x12, /**< SUB rd, r, imm7 | Subtract zero-ext(imm7) from r and store the result into rd. */
    OP_MUL_IMM = 0x13, /**< MUL rd, r, imm7 | Multiply r by zero-ext(imm7) and store the result into rd. */
    OP_DIV_IMM = 0x14, /**< DIV rd, r, imm7 | Divide r by zero-ext(imm7) and store the result into rd. */
    OP_AND_IMM = 0x15, /**< AND rd, r, imm7 | Bitwise AND r with zero-ext(imm7) and store the result into rd. */
    OP_OR_IMM = 0x16,  /**< OR rd, r, imm7 | Bitwise OR r with zero-ext(imm7) and store the result into rd. */
    OP_NOT_IMM = 0x17, /**< NOT r, imm9 | Stores bitwise NOT zero-ext(imm7) into rd. */
    /** LSd/ROd rd, rx, ry | Logical shift/rotate left/right rx by contents of ry # of bits and store result in rd. */
    OP_LSd_ROd = 0x18,
    OP_MOV_IMM = 0x19, /**< MOV rd, imm9 | Copy the value of zero-ext(imm9) into rd. */
    OP_CMP_IMM = 0x1A, /**< CMP r, imm9 | Subtract zero-ext(imm9) from r to generate flags. */
    OP_LEA = 0x1B,     /**< LEA rd, imm9 | Add PC and sign-ext(imm9) and store the result in rd. */
    /** STR rs, [imm9] | Add PC and sign-ext(imm9) and store contents of rs into resulting address in memory. */
    OP_STR_PCREL = 0x1C,
    /** STR rs, [r, imm7] | Add contents of r to sign-ext(imm7), store contents of rs in resulting memory address. */
    OP_STR_OFFR = 0x1D,
    /** LDR rd, [r, imm7] | Add contents of r to sign-ext(imm7), load contents of resulting memory address in rd. */
    OP_LDR_OFFR = 0x1E,
    /** BLcc imm7 | Branch to address PC + sign-ext(imm7) if cc is true based on flags, and store PC in LR. */
    OP_BLcc = 0x1F,
} Opcodes;

/** Defines a single memory word. */
typedef uint16_t word_t;
/** Defines a set of internal signals. */
typedef uint64_t signals_t;

word_t fetch_word(FILE *program, word_t addr);
signals_t fetch_signals(FILE *decode_rom, uint8_t addr);
word_t alu(ALUOperation op, word_t a, word_t b, uint8_t *flags);

#endif // _COMPONENTS_H_
