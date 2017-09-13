CC=gcc-6
CFLAGS=-std=c11 -Wall -Werror -Wextra -pedantic -g
PROGRAMS=badd badd_64 bitflip

.PHONY: clean all

all:
	make $(PROGRAMS)

%.o: %.c
	$(CC) $(CFLAGS) --m64 -o $@ $< 


clean:
	rm -f $(PROGRAMS)

