/*
  Arithmetic stuff and simple gates
*/

#include "wires.h"

// mask out a value if control is false
val use_if(bool control, val value);

// bitwise and, or, xor and negate for bitvectors
val and(val a, val b);
val or(val a, val b);
val xor(val a, val b);
val neg(int num_bits, val);

// reduce a bit vector to a bool by and'ing or or'ing all elements
bool reduce_and(int num_bits, val);
bool reduce_or(val);

// 64 bit addition
val add(val a, val b);

// detect specific value
bool is(uint64_t cnst, val a);

// 64-bit adder that can also take a carry-in and deliver an overflow status.
typedef struct {
    bool of;
    val result;
} generic_adder_result;

generic_adder_result generic_adder(val val_a, val val_b, bool carry_in);


