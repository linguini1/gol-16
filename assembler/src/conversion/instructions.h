#ifndef _INSTRUCTIONS_H_
#define _INSTRUCTIONS_H_

#include <stdint.h>

/* Instruction list */
typedef struct InstructionList {
    uint16_t *instructions;
    unsigned long length;
    unsigned long __capacity;
} InstructionList;

InstructionList *instruction_list_construct(unsigned long length);
void instruction_list_destruct(InstructionList *list);

void instruction_list_append(InstructionList *list, uint16_t instruction);

#endif // _INSTRUCTIONS_H_
