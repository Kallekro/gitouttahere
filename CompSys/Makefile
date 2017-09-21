CC=gcc
CFLAGS=-std=c11 -Wall -Werror -Wextra -pedantic -g -Og
TYPE=char

all: variables

cpp-store: variables.c Makefile
	$(CC) $(CFLAGS) -DTYPE="$(TYPE)" -E store.c

store.s: variables.c Makefile
	$(CC) $(CFLAGS) -DTYPE="$(TYPE)" -S variables.c -o variables..s

store.o: variables.c Makefile
	$(CC) $(CFLAGS) -DTYPE="$(TYPE)" -c variables.c -o variables.o

show-store: store.s
	cat variables.s | sed -E '/\s*\./d'

dump-store: store.o
	objdump -M no-aliases -d store.o

clean:
	rm -f store.o
	rm -f store.s

.PHONY: all variables.s variables.o cpp-store show-store dump-store clean
