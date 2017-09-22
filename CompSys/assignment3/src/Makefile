CC = gcc
CFLAGS = -Wall -O0 -std=c11 -ggdb

all: sim

../src.zip: alu.c alu.h arithmetic.c arithmetic.h main.c Makefile memories.c memories.h support.c support.h trace_read.h wires.c wires.h
	cd .. && zip src.zip src/alu.c src/alu.h src/arithmetic.c src/arithmetic.h src/main.c src/Makefile src/memories.c src/memories.h src/support.c src/support.h src/trace_read.h src/wires.c src/wires.h


sim: *.c
	$(CC) $(CFLAGS) *.c -o sim

clean:
	rm -f *.o sim
