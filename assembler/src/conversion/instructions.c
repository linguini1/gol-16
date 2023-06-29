#include "instructions.h"
#include <stdlib.h>

InstructionList *instruction_list_construct(unsigned long length) {
    InstructionList *list = malloc(sizeof(InstructionList));
    list->length = 0;
    list->__capacity = length;
    list->instructions = malloc(sizeof(uint16_t) * list->__capacity);
    return list;
}

void instruction_list_destruct(InstructionList *list) {
    free(list->instructions);
    free(list);
}

void instruction_list_append(InstructionList *list, uint16_t instruction) {
    if (list->length == list->__capacity) {
        list->__capacity *= 2;
        uint16_t *new_arr = malloc(sizeof(uint16_t) * list->__capacity);

        // Copy over tokens
        for (int i = 0; i < list->length; i++) {
            new_arr[i] = list->instructions[i];
        }

        // Replace old list
        free(list->instructions);
        list->instructions = new_arr;
    }

    list->instructions[list->length] = instruction;
    list->length++;
}
