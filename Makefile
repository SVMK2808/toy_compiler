CC     = gcc
CFLAGS = -Wall -Wextra -Iinclude

SRC = src/main.c src/lexer.c
OUT = compiler

all:
	$(CC) $(CFLAGS) $(SRC) -o $(OUT)

clean:
	rm -f $(OUT)