#ifndef _INSTRUCTIONS_H_
#define _INSTRUCTIONS_H_

#include <stdint.h>

extern const char *OBJ_FILE_SUFFIX;
extern const char *DEFAULT_OUT_FILE;

/* Instruction list */
typedef struct InstructionList {
    uint16_t *instructions;
    unsigned long length;
    unsigned long __capacity;
} InstructionList;

InstructionList *instruction_list_construct(unsigned long length);
void instruction_list_destruct(InstructionList *list);

void instruction_list_append(InstructionList *list, uint16_t instruction);
uint16_t instruction_list_get(InstructionList *list, int index);

int write_all_instructions(InstructionList *list, const char *file_path);

/* File type verification */
static int _is_obj_file(const char *file_path);

#endif // _INSTRUCTIONS_H_
