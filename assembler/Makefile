CC = gcc
OUT = orgassembler
CFLAGS = -O3
OBJ_FILES = obj/*.o

default:
	$(CC) $(CFLAGS) -c src/conversion/tokens.c -o obj/tokens.o
	$(CC) $(CFLAGS) -c src/conversion/identifiers.c -o obj/identifiers.o
	$(CC) $(CFLAGS) -c src/conversion/instructions.c -o obj/instructions.o
	$(CC) $(CFLAGS) -c src/conversion/lexer.c -o obj/lexer.o
	$(CC) $(CFLAGS) -c src/conversion/analyzer.c -o obj/analyzer.o
	$(CC) $(CFLAGS) -c src/main.c -o obj/main.o
	$(CC) $(CFLAGS) $(OBJ_FILES) -o $(OUT)

clean:
	rm $(OBJ_FILES)
	rm $(OUT)
