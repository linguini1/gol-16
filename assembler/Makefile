CC = gcc
OUT = orgassembler
CFLAGS = -o $(OUT)
OBJ_FILES = obj/*.o

default:
	$(CC) -c src/conversion/tokens.c -o obj/tokens.o
	$(CC) -c src/conversion/identifiers.c -o obj/identifiers.o
	$(CC) -c src/conversion/instructions.c -o obj/instructions.o
	$(CC) -c src/conversion/lexer.c -o obj/lexer.o
	$(CC) -c src/conversion/analyzer.c -o obj/analyzer.o
	$(CC) -c src/main.c -o obj/main.o
	$(CC) $(CFLAGS) $(OBJ_FILES)

clean:
	rm obj/*.o
	rm $(OUT)
