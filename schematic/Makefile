CC = gcc
CFLAGS = -O3 -Wall
OUT = mcasm
OBJ_DIR = ./obj
OBJ_FILES = $(OBJ_DIR)/*.o

all:
	if [ ! -d $(OBJ_DIR) ]; then mkdir $(OBJ_DIR); fi
	$(CC) $(CFLAGS) -c src/hashmap.c -o $(OBJ_DIR)/hashmap.o
	$(CC) $(CFLAGS) -c src/lexer.c -o $(OBJ_DIR)/lexer.o
	$(CC) $(CFLAGS) -c mcasm.c -o $(OBJ_DIR)/mcasm.o
	$(CC) $(CFLAGS) $(OBJ_FILES) -o $(OUT)

clean:
	rm -r $(OBJ_DIR)
	rm $(OUT)
