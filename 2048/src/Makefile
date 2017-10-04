CC=gcc
CFLAGS=-std=c11 -Wall -Wextra 

all: core main

core: core.c 
	$(CC) $(CFLAGS) -o core core.c -lncurses

main: main.c graphics.c 
	$(CC) $(CFLAGS) graphics.c main.c -o main -lncurses

clean:
	rm -f *.o
	rm -f core
	rm -f main


