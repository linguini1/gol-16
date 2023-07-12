/* Define test cases for the gol-16 assembler */
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifdef WIN32
#include <io.h>
#define F_OK 0
#define access _access
#endif

const char *TEST_CASES[] = {"char", "string", "comment"};
#define array_len(a) sizeof(a) / sizeof(*a)

typedef struct TestResult {
    const bool success;
    const unsigned long fail_pos;
    const uint8_t fbyte;
    const uint8_t hbyte;
    char err_msg[30];
    const char *test_name;
} testres_t;
void test_result_display(testres_t res);
testres_t run_test(const char *test_name, const char *test_dir, const char *exe_path);

char *join_path(const char *fname, const char *dir);
void full_path(char **path, const char *test_name, const char *dir, const char *suffix, unsigned len);
void test_files(const char *test_name, const char *dir, char **src_path, char **asm_path, char **hnd_path);
char *assemble_cmd(const char *src_path, char const *asm_path, char const *exe_path);

int main(int argc, char *argv[]) {

    // Takes two arguments
    if (argc != 3) {
        puts("USAGE: orgasmt [TEST_CASE_DIR] [ASSEMBLER_PATH]");
        return EXIT_FAILURE;
    }

    char *test_case_dir = argv[1];
    char *assembler_path = argv[2];

    if (access(assembler_path, X_OK | F_OK) != 0) {
        printf("orgassembler at %s does not exist or is not executable.", assembler_path);
        return EXIT_FAILURE;
    }

    unsigned pass_count = 0;
    for (unsigned i = 0; i < array_len(TEST_CASES); i++) {
        testres_t result = run_test(TEST_CASES[i], test_case_dir, assembler_path);
        pass_count += result.success;
        test_result_display(result);
    }
    printf("\nTESTS PASSED: %d/%lld\n", pass_count, array_len(TEST_CASES));

    if (pass_count != array_len(TEST_CASES)) return EXIT_FAILURE;

    return EXIT_SUCCESS;
}

/* Constructs a failing test result. */
testres_t test_result_construct_f(const unsigned long fail_pos, const uint8_t fbyte, const uint8_t hbyte,
                                  const char *err_msg, const char *test_name) {
    testres_t result = {false, fail_pos, fbyte, hbyte, "", test_name};
    strcpy(result.err_msg, err_msg);
    return result;
}

/* Constructs a successful test result. */
testres_t test_result_construct_s(const char *test_name) {
    testres_t result = {true, 0, 0, 0, "", test_name};
    strcpy(result.err_msg, "Success.");
    return result;
}

/* Displays the result of a test case being run with failure message and byte comparison. */
void test_result_display(testres_t res) {
    char *success = res.success ? "PASS" : "FAIL";
    printf("%s :: %s", res.test_name, success);
    if (!res.success)
        printf(" :: '%s' at pos %lu\n\tsrc: %x\n\thnd: %x", res.err_msg, res.fail_pos, res.fbyte, res.hbyte);
    putchar('\n');
}

/* Joins a file name and a directory on a '/' character. The returned path must be freed by the caller. */
char *join_path(const char *fname, const char *dir) {
    unsigned len = sizeof(char) * (strlen(fname) + strlen(dir) + 2); // +2 for null terminator and dir separator
    char *path = malloc(len);

    strcpy(path, dir);
    strcat(path, "/");
    strcat(path, fname);

    return path;
}

/* Stores the full path of {dir}/{test_name}{suffix} in path pointer. Path must be freed by caller. */
void full_path(char **path, const char *test_name, const char *dir, const char *suffix, unsigned len) {
    char *fname = malloc(len);
    strcpy(fname, test_name);
    strcat(fname, suffix);
    *path = join_path(fname, dir);
    free(fname);
}

/* Stores the full path to the three required test case files in their respective pointers. All three paths must be
 * freed by the caller.
 * src_path: .orgasm source file
 * asm_path: Assembler-assembled object file
 * hnd_path: Hand assembled file
 */
void test_files(const char *test_name, const char *dir, char **src_path, char **asm_path, char **hnd_path) {
    unsigned name_len = strlen(test_name);
    unsigned srclen = sizeof(char) * name_len + 8; // + .orgasm \0
    unsigned asmlen = sizeof(char) * name_len + 3; // + .o \0
    unsigned hndlen = sizeof(char) * name_len + 5; // + _h.o \0

    full_path(src_path, test_name, dir, ".orgasm", srclen);
    full_path(asm_path, test_name, dir, ".o", asmlen);
    full_path(hnd_path, test_name, dir, "_h.o", hndlen);
}

/* Returns the assembler command required to build the test object file from source. Returned command must be freed by
 * the caller.
 */
char *assemble_cmd(const char *src_path, char const *asm_path, char const *exe_path) {
    // +3 for 2 spaces between arguments and a null terminator
    unsigned long len = sizeof(char) * (strlen(exe_path) + strlen(asm_path) + strlen(src_path) + 3);
    char *cmd = malloc(len);

    strcpy(cmd, exe_path);
    strcat(cmd, " ");
    strcat(cmd, src_path);
    strcat(cmd, " ");
    strcat(cmd, asm_path);
    return cmd;
}

/* Runs a test case (assembles source file and compares its contents with the hand assembled copy) and returns the
 * results. Handles failures where required TC files DNE or system/assembler return failure.
 * exe_path: The path to the assembler executable
 */
testres_t run_test(const char *test_name, const char *test_dir, const char *exe_path) {

    char *hnd_path;
    char *src_path;
    char *asm_path;
    test_files(test_name, test_dir, &src_path, &asm_path, &hnd_path);

    // Check that necessary files exist and have read permission
    if (access(src_path, F_OK | R_OK) != 0) return test_result_construct_f(0, 0, 0, "Source file DNE.", test_name);
    if (access(hnd_path, F_OK | R_OK) != 0)
        return test_result_construct_f(0, 0, 0, "Hand assembled file DNE.", test_name);

    // Check that assembler/system did not return failure
    if (system(assemble_cmd(src_path, asm_path, exe_path)) != 0)
        return test_result_construct_f(0, 0, 0, "Assembler failed.", test_name);

    FILE *hptr = fopen(hnd_path, "rb");
    FILE *aptr = fopen(asm_path, "rb");

    uint8_t a, h;
    while (!feof(hptr) && !feof(aptr)) {
        fread(&h, 1, 1, hptr);
        fread(&a, 1, 1, aptr);
        if (a != h) return test_result_construct_f(ftell(hptr), a, h, "Byte mismatch.", test_name);
    }
    fclose(hptr);
    fclose(aptr);
    return test_result_construct_s(test_name);
}
