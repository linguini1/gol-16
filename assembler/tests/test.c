/* Define test cases for the gol-16 assembler */
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char *TEST_CASES[] = {"char", "string"};
#define array_len(a) sizeof(a) / sizeof(*a)

typedef struct TestResult {
    const bool success;
    const unsigned long fail_pos;
    const unsigned int fail_byte;
    const char *test_name;
} testres_t;

void test_result_display(testres_t res) {
    char *success = res.success ? "PASS" : "FAIL";
    printf("%s :: %s\n", res.test_name, success);
    if (!res.success) printf("\tFailed at byte %lu: '%x'", res.fail_pos, res.fail_byte);
}

char *join_path(const char *fname, const char *dir) {
    unsigned len = sizeof(char) * (strlen(fname) + strlen(dir) + 2); // +2 for null terminator and dir separator
    char *path = malloc(len);

    strcpy_s(path, len, dir);
    strcat_s(path, len, "/");
    strcat_s(path, len, fname);

    return path;
}

void full_path(char **path, const char *test_name, const char *dir, const char *suffix, unsigned len) {
    char *fname = malloc(len);
    strcpy_s(fname, len, test_name);
    strcat_s(fname, len, suffix);
    *path = join_path(fname, dir);
    free(fname);
}

void test_files(const char *test_name, const char *dir, char **src_path, char **asm_path, char **hnd_path) {
    unsigned name_len = strlen(test_name);
    unsigned srclen = sizeof(char) * name_len + 8; // + .orgasm \0
    unsigned asmlen = sizeof(char) * name_len + 3; // + .o \0
    unsigned hndlen = sizeof(char) * name_len + 5; // + _h.o \0

    full_path(src_path, test_name, dir, ".orgasm", srclen);
    full_path(asm_path, test_name, dir, ".o", asmlen);
    full_path(hnd_path, test_name, dir, "_h.o", hndlen);
}

char *assemble_cmd(const char *src_path, char const *asm_path, char const *exe_path) {

    // +3 for 2 spaces between arguments and a null terminator
    unsigned long len = sizeof(char) * (strlen(exe_path) + strlen(asm_path) + strlen(src_path) + 3);
    char *cmd = malloc(len);

    strcpy_s(cmd, len, exe_path);
    strcat_s(cmd, len, " ");
    strcat_s(cmd, len, src_path);
    strcat_s(cmd, len, " ");
    strcat_s(cmd, len, asm_path);
    return cmd;
}

testres_t run_test(const char *test_name, const char *test_dir, const char *exe_path) {

    char *hnd_path;
    char *src_path;
    char *asm_path;
    test_files(test_name, test_dir, &src_path, &asm_path, &hnd_path);

    if (system(assemble_cmd(src_path, asm_path, exe_path)) == 1) {
        return (testres_t){false, 0, 0, test_name};
    }

    FILE *hptr = fopen(hnd_path, "rb");
    FILE *aptr = fopen(asm_path, "rb");

    uint8_t a, s;
    while (!feof(hptr) && !feof(aptr)) {
        fread(&s, 1, 1, hptr);
        fread(&a, 1, 1, aptr);
        if (a != s) {
            return (testres_t){false, fseek(hptr, 0, SEEK_CUR), s, test_name};
        }
    }
    fclose(hptr);
    fclose(aptr);
    return (testres_t){true, 0, 0, test_name};
}

int main(int argc, char *argv[]) {

    // Takes two arguments
    if (argc != 3) {
        puts("USAGE: orgasmt [TEST_CASE_DIR] [ASSEMBLER_PATH]");
        return EXIT_FAILURE;
    }

    char *test_case_dir = argv[1];
    char *assembler_path = argv[2];

    unsigned pass_count = 0;
    for (unsigned i = 0; i < array_len(TEST_CASES); i++) {
        testres_t result = run_test(TEST_CASES[i], test_case_dir, assembler_path);
        pass_count += result.success;
        test_result_display(result);
    }
    printf("\nTESTS PASSED: %d/%d\n", pass_count, array_len(TEST_CASES));

    if (pass_count == array_len(TEST_CASES)) return EXIT_FAILURE;

    return EXIT_SUCCESS;
}
