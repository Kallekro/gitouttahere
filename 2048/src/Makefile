CC=gcc
CFLAGS=-std=c11 -Wall -Wextra 

all: core graphics

core: core.c 
	$(CC) $(CFLAGS) -o core core.c -lncurses

graphics: graphics.c 
	$(CC) $(CFLAGS) -o graphics graphics.c -lncurses

clean:
	rm -f *.o
	rm -f core
	rm -f graphics


