CC = gcc
ifeq ($(OS), Windows_NT)
    EXE := .exe
else # Assume Linux
    EXE := .out
endif

OUT = orgassembler$(EXE)
OBJ_FILES = obj/*.o
CFLAGS = -O3

# Testing
TEST_OUT = orgasmt$(EXE)
TEST_PROGRAMS = ./tests/test_programs

assembler:
	$(CC) $(CFLAGS) -c src/conversion/tokens.c -o obj/tokens.o
	$(CC) $(CFLAGS) -c src/conversion/identifiers.c -o obj/identifiers.o
	$(CC) $(CFLAGS) -c src/conversion/instructions.c -o obj/instructions.o
	$(CC) $(CFLAGS) -c src/conversion/lexer.c -o obj/lexer.o
	$(CC) $(CFLAGS) -c src/conversion/analyzer.c -o obj/analyzer.o
	$(CC) $(CFLAGS) -c src/main.c -o obj/main.o
	$(CC) $(CFLAGS) $(OBJ_FILES) -o $(OUT)

tester:
	$(CC) $(CFLAGS) tests/test.c -o $(TEST_OUT)

all: assembler tester

check: assembler tester;
	@echo "RUNNING TESTS"
	$(TEST_OUT) $(abspath $(TEST_PROGRAMS)) $(abspath $(OUT))

clean:
	rm $(OBJ_FILES)
	rm $(OUT)
	rm $(TEST_OUT)
