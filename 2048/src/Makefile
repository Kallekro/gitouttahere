CC=gcc
CFLAGS=-std=c11 -Wall -Wextra -g


all: main

main: main.c graphics.c 
	$(CC) $(CFLAGS) graphics.c core.c main.c -o main -lncurses

clean:
	rm -f *.o
	rm -f core
	rm -f main


