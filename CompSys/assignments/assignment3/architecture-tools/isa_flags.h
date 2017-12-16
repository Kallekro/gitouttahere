//
// Nis Wegmann, 2017
//
// isa_flags.h
//

#ifndef __ISA_FLAGS_H__
#define __ISA_FLAGS_H__

#include "isa_alu.h"
#include "isa_word.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

// ---------------------------------------------------------------------------------------------- //

//
// Flag type.
//
typedef union isa_Flags
{
    struct
    {
        uint16_t cf : 1;                        // Carry flag.
        uint16_t _1 : 1;                        // 
        uint16_t pf : 1;                        // Parity flag.
        uint16_t _3 : 1;                        // 
        uint16_t af : 1;                        // Adjust flag.
        uint16_t _5 : 1;                        // 
        uint16_t zf : 1;                        // Zero flag.
        uint16_t sf : 1;                        // Sign flag.
        uint16_t _8 : 1;                        // 
        uint16_t _9 : 1;                        // 
        uint16_t _a : 1;                        // 
        uint16_t of : 1;                        // Overflow flag.
        uint16_t _c : 1;                        // 
        uint16_t _d : 1;                        // 
        uint16_t _e : 1;                        // 
        uint16_t _f : 1;                        // 
    };
    
    uint16_t bits;
}
isa_Flags;

// ---------------------------------------------------------------------------------------------- //

//
// Default flags.
//
static isa_Flags const isa_defaultFlags = { .bits = 0 };

// ---------------------------------------------------------------------------------------------- //

//
// Computes new flags given an ALU operation.
//
static inline isa_Flags isa_computeFlags
    (
        isa_Flags oldFlags,
        isa_ALUOp op,
        isa_Quad operandA,
        isa_Quad operandB,
        isa_Quad result
    )
{
    isa_Flags newFlags = oldFlags;
    
    bool const aSign = isa_isSigned(operandA);
    bool const bSign = isa_isSigned(operandB);
    bool const rSign = isa_isSigned(result);
    
    newFlags.zf = result == 0;
    newFlags.sf = rSign;
    
    switch (op)
    {
        case isa_ALUOp_addq:
            
            newFlags.cf = result < operandA;
            newFlags.af = ((operandA ^ operandB ^ result) & 0x10) != 0;
            newFlags.of = (aSign == bSign) && (rSign != aSign);
            
            return newFlags;
            
        case isa_ALUOp_subq:
        case isa_ALUOp_cmpq:
            
            newFlags.cf = result < operandA;
            newFlags.af = ((operandA ^ operandB ^ result) & 0x10) != 0;
            newFlags.of = (aSign != bSign) && (rSign != aSign);
            
            return newFlags;
            
        case isa_ALUOp_andq:
        case isa_ALUOp_xorq:
        case isa_ALUOp_shrq:
            
            newFlags.cf = false;
            newFlags.af = false;
            newFlags.of = false;
            
            return newFlags;
    }
    
    abort();
}

// ---------------------------------------------------------------------------------------------- //

#endif // __ISA_FLAGS_H__
