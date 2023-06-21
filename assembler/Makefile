CC = gcc
OUT = orgassembler
CFLAGS = -o $(OUT)
OBJ_FILES = obj/*.o

default:
	$(CC) -c src/lexer/lexer.c -o obj/lexer.o
	$(CC) -c src/main.c -o obj/main.o
	$(CC) $(CFLAGS) $(OBJ_FILES)

clean:
	rm obj/*.o
	rm $(OUT)
