#include "wires.h"
/*
  A simple ALU matching the subset of x86 that we'll need
 */
// Encoding of ALU operations
#define ADD 0
#define SUB 1
#define AND 2
#define XOR 3
#define CMP 4

// Encoding of conditions
#define ALWAYS 0
#define LE 1
#define LT 2
#define EQ 3
#define NE 4
#define GE 5
#define GT 6

typedef struct {
    bool of;
    bool zf;
    bool sf;
} conditions;

bool eval_condition(conditions cc, val op);

typedef struct {
    val result;
    conditions cc;
} alu_execute_result;

alu_execute_result alu_execute(val op, val op_a, val op_b);

