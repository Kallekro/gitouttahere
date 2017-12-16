//
// Nis Wegmann, 2017
//
// isa_alu.h
//

#ifndef __ISA_ALU_H__
#define __ISA_ALU_H__

#include "isa_word.h"

#include <stdlib.h>

// ---------------------------------------------------------------------------------------------- //

//
// ALU operation type.
//
typedef enum isa_ALUOp
{
    isa_ALUOp_addq,                             // Addition.
    isa_ALUOp_subq,                             // Subtraction.
    isa_ALUOp_cmpq,                             // Comparison.
    isa_ALUOp_andq,                             // Logical and.
    isa_ALUOp_xorq,                             // Logical exclusive-or.
    isa_ALUOp_shrq                              // Logical exclusive-or.
}
isa_ALUOp;

// ---------------------------------------------------------------------------------------------- //

//
// Computes the result of an ALU operation.
//
static inline isa_Quad isa_computeALUOp(isa_ALUOp op, isa_Quad operandA, isa_Quad operandB)
{
    switch (op)
    {
        case isa_ALUOp_addq: return operandA + operandB;
        case isa_ALUOp_subq: return operandA - operandB;
        case isa_ALUOp_cmpq: return operandA - operandB;
        case isa_ALUOp_andq: return operandA & operandB;
        case isa_ALUOp_xorq: return operandA ^ operandB;
        case isa_ALUOp_shrq: return operandA >> operandB;
        
        default: abort();
    }
}

// ---------------------------------------------------------------------------------------------- //

#endif // __ISA_ALU_H__
