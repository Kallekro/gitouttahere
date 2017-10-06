CC=gcc
CFLAGS=-std=c11 -Wall -Wextra -g


all: main

main: main.c core.c graphics.c support.c ai_weights.c 
	$(CC) $(CFLAGS) ai_weights.c graphics.c support.c core.c  main.c -o main -lncurses -lm

clean:
	rm -f *.o
	rm -f main

