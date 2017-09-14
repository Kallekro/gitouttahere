CC=gcc
CFLAGS=-std=c11 -Wall -Werror -Wextra -pedantic -g3

file: file.c Makefile
	$(CC) $(CFLAGS) -o file file.c

gdb_test: file test.gdb Makefile
	gdb -q -x test.gdb $<

../src.zip: file.c Makefile test.sh test.gdb
	cd .. && zip src.zip src/file.c src/Makefile src/test.sh src/test.gdb

clean:
	rm -f *.o
	rm -f file

.PHONY: clean gdb_test
