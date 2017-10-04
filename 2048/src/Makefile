CC=gcc
CFLAGS=-std=c11 -Wall -Werror -Wextra -pedantic -g3

file: core.c Makefile
	$(CC) $(CFLAGS) -o core core.c

clean:
	rm -f *.o
	rm -f core
