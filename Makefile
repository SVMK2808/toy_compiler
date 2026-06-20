CC     = gcc
CFLAGS = -Wall -Wextra -Iinclude

SRC = src/main.c src/lexer.c src/parser.c src/ast.c src/codegen.c src/vm.c src/symtable.c
OUT = compiler

all:
	$(CC) $(CFLAGS) $(SRC) -o $(OUT)

clean:
	rm -f $(OUT)