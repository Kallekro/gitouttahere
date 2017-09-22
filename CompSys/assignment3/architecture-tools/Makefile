CC = gcc
CFLAGS = -Wall -Wextra -O1 -std=c11 -pedantic -g
LEX = flex
YACC = bison

all: sim asm trace

parser: asm_lexer.l asm_parser.y
	$(LEX) -o lex.yy.c asm_lexer.l
	$(YACC) -d asm_parser.y -o y.tab.c

lex.yy.o: lex.yy.c
	$(CC) -w -O3 -c lex.yy.c

y.tab.o: y.tab.c
	$(CC) -w -O3 -c y.tab.c

asm: y.tab.o lex.yy.o isa_* asm_*
	$(CC) $(CFLAGS) lex.yy.o y.tab.o asm*.c -o asm

sim: sim_* isa_*
	$(CC) $(CFLAGS) sim*.c -o sim

trace: trace_* isa_*
	$(CC) $(CFLAGS) trace*.c -o trace

clean:
	rm -f *.o asm sim
