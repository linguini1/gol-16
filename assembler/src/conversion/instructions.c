#include "instructions.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char *OBJ_FILE_SUFFIX = ".o";
const char *DEFAULT_OUT_FILE = "a.o";

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

uint16_t instruction_list_get(InstructionList *list, int index) {
    // Support for negative indexing
    if (index < 0) {
        index = list->length + index;
    }

    if (!(index < list->length) || index < 0) {
        return -1;
    }
    return list->instructions[index];
}

int write_all_instructions(InstructionList *list, const char *file_path) {

    if (!_is_obj_file(file_path)) {
        return 0;
    }

    FILE *fptr = fopen(file_path, "wb");

    if (fptr == NULL) {
        return 0;
    }

    fwrite(list->instructions, 2, list->length, fptr);
    fclose(fptr);
    return 1;
}

/* File type verification */
static int _is_obj_file(const char *file_path) {
    size_t len = strlen(file_path);
    const char *cur = &file_path[len]; // End of string

    // Continue backwards until suffix start or string start
    while (*cur != '.' && cur != file_path) {
        cur--;
    }
    return !strcmp(cur, OBJ_FILE_SUFFIX);
}
