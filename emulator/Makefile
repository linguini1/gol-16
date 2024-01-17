CC = gcc
OUT = gemu

### SOURCE FILES ###
SRCDIR = src
SRC_FILES = $(wildcard $(SRCDIR)/*.c)
OBJ_FILES = $(patsubst %.c,%.o,$(SRC_FILES))

### TESTING ###
TESTDIR = tests
TEST_FILES = $(wildcard $(TESTDIR)/*.c)
# Don't compile the entry point for the actual executable
TEST_FILES += $(filter-out %main.c,$(SRC_FILES))
TEST_OBJ = $(patsubst %.c,%.o,$(TEST_FILES))
TEST_OUT = gemu_tester

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
CFLAGS += -O3
CFLAGS += -lm

all: $(OBJ_FILES)
	$(CC) $(CFLAGS) $^ -o $(OUT)

%.o: %.c
	$(CC) $(CFLAGS) $(WARNINGS) -o $@ -c $<

test: $(TEST_OBJ)
	$(CC) $(CFLAGS) $^ -o $(TEST_OUT)
	./$(TEST_OUT)
	@rm $(TEST_OUT)

clean:
	@rm $(OBJ_FILES)
	@rm $(OUT)
