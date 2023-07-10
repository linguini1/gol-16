CC = gcc
ifeq ($(OS), Windows_NT)
    EXE := .exe
else # Assume Linux
    EXE := .out
endif

OUT = orgassembler$(EXE)
OBJ_DIR = ./obj
OBJ_FILES = $(OBJ_DIR)/*.o
CFLAGS = -O3 -Wall

# Testing
TEST_OUT = orgasmt$(EXE)
TEST_PROGRAMS = ./tests/test_programs

assembler:

	if [ ! -d $(OBJ_DIR) ]; then mkdir $(OBJ_DIR); fi 

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
	$(abspath $(TEST_OUT)) $(abspath $(TEST_PROGRAMS)) $(abspath $(OUT))

clean:
	rm -r $(OBJ_DIR)
	rm $(OUT)
	rm $(TEST_OUT)
