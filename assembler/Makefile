CC = gcc
OUT = gassemble

### SOURCE FILES ###
SRCDIR = src
SRC_FILES = $(wildcard $(SRCDIR)/*.c)
OBJ_FILES = $(patsubst %.c,%.o,$(SRC_FILES))

### WARNINGS ###
# (see https://gcc.gnu.org/onlinedocs/gcc-6.3.0/gcc/Warning-Options.html)
WARNINGS += -Wall -Wextra -Wshadow -Wundef -Wformat=2 -Wtrampolines -Wfloat-equal
WARNINGS += -Wbad-function-cast -Wstrict-prototypes -Wpacked
WARNINGS += -Wno-aggressive-loop-optimizations -Wmissing-prototypes -Winit-self
WARNINGS += -Wmissing-declarations -Wmissing-format-attribute -Wunreachable-code
WARNINGS += -Wshift-overflow=2 -Wduplicated-cond -Wpointer-arith -Wwrite-strings
WARNINGS += -Wnested-externs -Wcast-align -Wredundant-decls
WARNINGS += -Werror=implicit-function-declaration -Wlogical-not-parentheses
WARNINGS += -Wlogical-op -Wold-style-definition -Wcast-qual -Wdouble-promotion
WARNINGS += -Wunsuffixed-float-constants -Wmissing-include-dirs -Wnormalized
WARNINGS += -Wdisabled-optimization -Wsuggest-attribute=const

### COMPILER OPTIONS ###
CFLAGS = -O3

# Testing
TEST_OUT = gasmt
TEST_PROGRAMS = ./tests/test_programs

all: assembler tester

assembler: $(OBJ_FILES)
	$(CC) $(CFLAGS) $(OBJ_FILES) -o $(OUT)

tester:
	$(CC) $(CFLAGS) tests/test.c -o $(TEST_OUT)

%.o: %.c
	$(CC) $(CFLAGS) $(WARNINGS) -o $@ -c $<

test: assembler tester
	@echo "RUNNING TESTS"
	$(abspath $(TEST_OUT)) $(abspath $(TEST_PROGRAMS)) $(abspath $(OUT))

clean:
	@rm $(OBJ_FILES)
	@rm $(OUT)
	@rm $(TEST_OUT)
