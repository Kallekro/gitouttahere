//
// Nis Wegmann, 2017
//
// isa_instruction.h
// 
// 
//

#ifndef __ISA_INSTRUCTION_H__
#define __ISA_INSTRUCTION_H__

#include "isa_endian.h"
#include "isa_opcode.h"
#include "isa_register.h"
#include "isa_word.h"

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>

// ---------------------------------------------------------------------------------------------- //

//
// Instruction type.
//
typedef struct isa_Instruction
{
    // Opcode.
    isa_Opcode opcode;
    
    // Operands.
    union
    {
        // Not used.
        struct
        {
            isa_Byte _;
        };
        
        // Register.
        struct
        {
            isa_Register r;
        };
        
        // Immediate.
        struct
        {
            isa_Long i;
        };
        
        // Register-register.
        struct
        {
            isa_Register src;
            isa_Register dst;
        }
        rr;
        
        // Immediate-register.
        struct
        {
            isa_Long src;
            isa_Register dst;
        }
        ir;
        
        // Register-memory.
        struct
        {
            isa_Long dst;
            isa_Register src;
            isa_Register idx;
        }
        rm;
        
        // Memory-register.
        struct
        {
            isa_Long src;
            isa_Register idx;
            isa_Register dst;
        }
        mr;
    };
}
isa_Instruction;

// ---------------------------------------------------------------------------------------------- //

// Constructors.

#define ISA_CONSTRUCTOR(_op_, _field_, ...)\
    ((isa_Instruction) { .opcode = isa_Opcode_##_op_ , ._field_ = __VA_ARGS__ })

#define ISA_HLT()                        ISA_CONSTRUCTOR(hlt,     _,  (0))
#define ISA_NOP()                        ISA_CONSTRUCTOR(nop,     _,  (0))
#define ISA_MOVQ_RR(_src_, _dst_)        ISA_CONSTRUCTOR(movq_rr, rr, { (_src_), (_dst_) })
#define ISA_MOVQ_IR(_src_, _dst_)        ISA_CONSTRUCTOR(movq_ir, ir, { (_src_), (_dst_) })
#define ISA_MOVQ_RM(_src_, _idx_, _dst_) ISA_CONSTRUCTOR(movq_rm, rm, { (_dst_), (_src_), (_idx_) })
#define ISA_MOVQ_MR(_src_, _idx_, _dst_) ISA_CONSTRUCTOR(movq_mr, mr, { (_src_), (_idx_), (_dst_) })
#define ISA_ADDQ_RR(_src_, _dst_)        ISA_CONSTRUCTOR(addq_rr, rr, { (_src_), (_dst_) })
#define ISA_SUBQ_RR(_src_, _dst_)        ISA_CONSTRUCTOR(subq_rr, rr, { (_src_), (_dst_) })
#define ISA_ANDQ_RR(_src_, _dst_)        ISA_CONSTRUCTOR(andq_rr, rr, { (_src_), (_dst_) })
#define ISA_XORQ_RR(_src_, _dst_)        ISA_CONSTRUCTOR(xorq_rr, rr, { (_src_), (_dst_) })
#define ISA_CMPQ_RR(_src_, _dst_)        ISA_CONSTRUCTOR(cmpq_rr, rr, { (_src_), (_dst_) })
#define ISA_SHRQ_RR(_src_, _dst_)        ISA_CONSTRUCTOR(shrq,    rr, { (_src_), (_dst_) })
#define ISA_JMP(_dst_)                   ISA_CONSTRUCTOR(jmp,     i,  (_dst_))
#define ISA_JLE(_dst_)                   ISA_CONSTRUCTOR(jle,     i,  (_dst_))
#define ISA_JL(_dst_)                    ISA_CONSTRUCTOR(jl,      i,  (_dst_))
#define ISA_JE(_dst_)                    ISA_CONSTRUCTOR(je,      i,  (_dst_))
#define ISA_JNE(_dst_)                   ISA_CONSTRUCTOR(jne,     i,  (_dst_))
#define ISA_JGE(_dst_)                   ISA_CONSTRUCTOR(jge,     i,  (_dst_))
#define ISA_JG(_dst_)                    ISA_CONSTRUCTOR(jg,      i,  (_dst_))
#define ISA_CMOVLE(_src_, _dst_)         ISA_CONSTRUCTOR(cmovle,  rr, { (_src_), (_dst_) })
#define ISA_CMOVL(_src_, _dst_)          ISA_CONSTRUCTOR(cmovl,   rr, { (_src_), (_dst_) })
#define ISA_CMOVE(_src_, _dst_)          ISA_CONSTRUCTOR(cmove,   rr, { (_src_), (_dst_) })
#define ISA_CMOVNE(_src_, _dst_)         ISA_CONSTRUCTOR(cmovne,  rr, { (_src_), (_dst_) })
#define ISA_CMOVGE(_src_, _dst_)         ISA_CONSTRUCTOR(cmovge,  rr, { (_src_), (_dst_) })
#define ISA_CMOVG(_src_, _dst_)          ISA_CONSTRUCTOR(cmovg,   rr, { (_src_), (_dst_) })
#define ISA_CALL(_dst_)                  ISA_CONSTRUCTOR(call,    i,  (_dst_))
#define ISA_RET()                        ISA_CONSTRUCTOR(ret,     _,  (0))
#define ISA_PUSHQ(_src_)                 ISA_CONSTRUCTOR(pushq,   r,  (_src_))
#define ISA_POPQ(_dst_)                  ISA_CONSTRUCTOR(popq,    r,  (_dst_))

// ---------------------------------------------------------------------------------------------- //

#endif // __ISA_INSTRUCTION_H__
