CC=gcc
CFLAGS=-std=c11 -Wall -Wextra -g


all: main

main: main.c core.c graphics.c 
	$(CC) $(CFLAGS) graphics.c core.c main.c -o main -lncurses

ai_weights: ai_weights.c
	$(CC) $(CFLAGS) ai_weights.c -o ai_weights

clean:
	rm -f *.o
	rm -f main
	rm -f ai_weights

