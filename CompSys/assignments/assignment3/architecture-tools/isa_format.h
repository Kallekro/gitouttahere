//
// Nis Wegmann, 2017
// 
// isa_format.h
// 
// Contains functions for encoding and decoding instructions.
//

#ifndef __ISA_FORMAT_H__
#define __ISA_FORMAT_H__

#include "isa_instruction.h"

// ---------------------------------------------------------------------------------------------- //

//
// Returns the size in bytes of an --encoded-- instruction.
//
static inline size_t isa_sizeofInstruction(isa_Instruction instruction)
{
    switch (instruction.opcode)
    {
        case isa_Opcode_hlt:        return 1;
        case isa_Opcode_nop:        return 1;
        case isa_Opcode_movq_rr:    return 2;
        case isa_Opcode_movq_ir:    return 6;
        case isa_Opcode_movq_rm:    return 6;
        case isa_Opcode_movq_mr:    return 6;
        case isa_Opcode_addq_rr:
        case isa_Opcode_subq_rr:
        case isa_Opcode_andq_rr:
        case isa_Opcode_xorq_rr:
        case isa_Opcode_cmpq_rr:    return 2;
        case isa_Opcode_jmp:
        case isa_Opcode_jle:
        case isa_Opcode_jl:
        case isa_Opcode_je:
        case isa_Opcode_jne:
        case isa_Opcode_jge:
        case isa_Opcode_jg:         return 5;
        case isa_Opcode_cmovle:
        case isa_Opcode_cmovl:
        case isa_Opcode_cmove:
        case isa_Opcode_cmovne:
        case isa_Opcode_cmovge:
        case isa_Opcode_cmovg:      return 2;
        case isa_Opcode_call:       return 5;
        case isa_Opcode_ret:        return 1;
        case isa_Opcode_pushq:      return 2;
        case isa_Opcode_popq:       return 2;
        case isa_Opcode_shrq:       return 2;
    }
    
    abort();
}

//
// Encodes an instruction into a block of memory.
// 
// Returns the size in bytes of the encoded instruction.
//
static inline size_t isa_encode(unsigned char * memory, isa_Instruction instruction)
{
    assert(memory != NULL);
    
    isa_writeByte(memory, (isa_Byte) instruction.opcode);
    
    switch (instruction.opcode)
    {
        case isa_Opcode_hlt:
        {
            break;
        }
        case isa_Opcode_nop:
        {
            break;
        }
        case isa_Opcode_movq_rr:
        {
            isa_writeByte(memory + 1, isa_packHalfs(instruction.rr.dst, instruction.rr.src));
            
            break;
        }
        case isa_Opcode_movq_ir:
        {
            isa_writeByte(memory + 1, isa_packHalfs(instruction.ir.dst, 0));
            isa_writeLong(memory + 2, instruction.ir.src);
            
            break;
        }
        case isa_Opcode_movq_rm:
        {
            isa_writeByte(memory + 1, isa_packHalfs(instruction.rm.idx, instruction.rm.src));
            isa_writeLong(memory + 2, instruction.rm.dst);
            
            break;
        }
        case isa_Opcode_movq_mr:
        {
            isa_writeByte(memory + 1, isa_packHalfs(instruction.mr.idx, instruction.mr.dst));
            isa_writeLong(memory + 2, instruction.mr.src);
            
            break;
        }
        case isa_Opcode_addq_rr:
        case isa_Opcode_subq_rr:
        case isa_Opcode_andq_rr:
        case isa_Opcode_xorq_rr:
        case isa_Opcode_cmpq_rr:
        case isa_Opcode_shrq:
        {
            isa_writeByte(memory + 1, isa_packHalfs(instruction.rr.dst, instruction.rr.src));
            
            break;
        }
        case isa_Opcode_jmp:
        case isa_Opcode_jle:
        case isa_Opcode_jl:
        case isa_Opcode_je:
        case isa_Opcode_jne:
        case isa_Opcode_jge:
        case isa_Opcode_jg:
        {
            isa_writeLong(memory + 1, instruction.i);
            
            break;
        }
        case isa_Opcode_cmovle:
        case isa_Opcode_cmovl:
        case isa_Opcode_cmove:
        case isa_Opcode_cmovne:
        case isa_Opcode_cmovge:
        case isa_Opcode_cmovg:
        {
            isa_writeByte(memory + 1, isa_packHalfs(instruction.rr.dst, instruction.rr.src));
            
            break;
        }
        case isa_Opcode_call:
        {
            isa_writeLong(memory + 1, instruction.i);
            
            break;
        }
        case isa_Opcode_ret:
        {
            break;
        }
        case isa_Opcode_pushq:
        {
            isa_writeByte(memory + 1, isa_packHalfs(0, instruction.r));
            
            break;
        }
        case isa_Opcode_popq:
        {
            isa_writeByte(memory + 1, isa_packHalfs(0, instruction.r));
            
            break;
        }
        default:
        {
            abort(); // Invalid instruction.
        }
    }
    
    return isa_sizeofInstruction(instruction);
}

//
// Decodes an instruction from a block of memory.
// 
// Returns the size in bytes of the encoded instruction or '0' if it could not be decoded.
//
static inline size_t isa_decode(unsigned char const * memory, isa_Instruction * instruction)
{
    assert(memory != NULL);
    
    instruction->opcode = isa_readByte(memory);
    
    switch (instruction->opcode)
    {
        case isa_Opcode_hlt:
        {
            break;
        }   
        case isa_Opcode_nop:
        {
            break;
        }
        case isa_Opcode_movq_rr:
        {
            isa_Byte const byte = isa_readByte(memory + 1);
            
            instruction->rr.dst = isa_lo(byte);
            instruction->rr.src = isa_hi(byte);
            
            break;
        }
        case isa_Opcode_movq_ir:
        {
            isa_Byte const byte = isa_readByte(memory + 1);
            
            instruction->ir.dst = isa_lo(byte);
            instruction->ir.src = isa_readLong(memory + 2);
            
            break;
        }
        case isa_Opcode_movq_rm:
        {
            isa_Byte const regs = isa_readByte(memory + 1);
            
            instruction->rm.idx = isa_lo(regs);
            instruction->rm.src = isa_hi(regs);
            instruction->rm.dst = isa_readLong(memory + 2);
            
            break;
        }
        case isa_Opcode_movq_mr:
        {
            isa_Byte const regs = isa_readByte(memory + 1);
            
            instruction->mr.idx = isa_lo(regs);
            instruction->mr.dst = isa_hi(regs);
            instruction->mr.src = isa_readLong(memory + 2);
            
            break;
        }
        case isa_Opcode_addq_rr:
        case isa_Opcode_subq_rr:
        case isa_Opcode_andq_rr:
        case isa_Opcode_xorq_rr:
        case isa_Opcode_cmpq_rr:
        case isa_Opcode_shrq:
        {
            isa_Byte const byte = isa_readByte(memory + 1);
            
            instruction->rr.dst = isa_lo(byte);
            instruction->rr.src = isa_hi(byte);
            
            break;
        }
        case isa_Opcode_jmp:
        case isa_Opcode_jle:
        case isa_Opcode_jl:
        case isa_Opcode_je:
        case isa_Opcode_jne:
        case isa_Opcode_jge:
        case isa_Opcode_jg:
        {
            instruction->i = isa_readLong(memory + 1);
            
            break;
        }
        case isa_Opcode_cmovle:
        case isa_Opcode_cmovl:
        case isa_Opcode_cmove:
        case isa_Opcode_cmovne:
        case isa_Opcode_cmovge:
        case isa_Opcode_cmovg:
        {
            isa_Byte const byte = isa_readByte(memory + 1);
            
            instruction->rr.dst = isa_lo(byte);
            instruction->rr.src = isa_hi(byte);
            
            break;
        }
        case isa_Opcode_call:
        {
            instruction->i = isa_readLong(memory + 1);
            
            break;
        }
        case isa_Opcode_ret:
        {
            break;
        }
        case isa_Opcode_pushq:
        {
            isa_Byte const byte = isa_readByte(memory + 1);
            
            instruction->r = isa_hi(byte);
            
            break;
        }
        case isa_Opcode_popq:
        {
            isa_Byte const byte = isa_readByte(memory + 1);
            
            instruction->r = isa_hi(byte);
            
            break;
        }
        default:
        {
            return 0; // Invalid instruction.
        }
    }
    
    return isa_sizeofInstruction(*(instruction));
}

// ---------------------------------------------------------------------------------------------- //

#endif // __ISA_FORMAT_H__
