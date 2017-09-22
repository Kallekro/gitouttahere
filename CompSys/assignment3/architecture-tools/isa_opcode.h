//
// Nis Wegmann, 2017
// 
// isa_opcode.h
// 
// Subset of X86-64 opcodes.
//

#ifndef __ISA_OPCODE_H__
#define __ISA_OPCODE_H__

// ---------------------------------------------------------------------------------------------- //

//
// Opcode type. Opcodes are encoded by a single byte.
//
typedef enum isa_Opcode
{
    isa_Opcode_hlt      = 0x00,                 // Halt.
    isa_Opcode_nop      = 0x10,                 // No-op.
    isa_Opcode_movq_rr  = 0x20,                 // Move (register-register).
    isa_Opcode_movq_ir  = 0x30,                 // Move (register-immediate).
    isa_Opcode_movq_rm  = 0x40,                 // Move (register-memory).
    isa_Opcode_movq_mr  = 0x50,                 // Move (memory-register).
    isa_Opcode_addq_rr  = 0x60,                 // Add (register-register).
    isa_Opcode_subq_rr  = 0x61,                 // Subtract (register-register).
    isa_Opcode_andq_rr  = 0x62,                 // Logical AND (register-register).
    isa_Opcode_xorq_rr  = 0x63,                 // Logical exclusive OR (register-register).
    isa_Opcode_cmpq_rr  = 0x64,                 // Compare (register-register).
    isa_Opcode_shrq     = 0x65,                 // Shift right (register-register).
    isa_Opcode_jmp      = 0x70,                 // Jump (immediate).
    isa_Opcode_jle      = 0x71,                 // Conditional jump (less than or equal).
    isa_Opcode_jl       = 0x72,                 // Conditional jump (less than).
    isa_Opcode_je       = 0x73,                 // Conditional jump (equal).
    isa_Opcode_jne      = 0x74,                 // Conditional jump (not equal).
    isa_Opcode_jge      = 0x75,                 // Conditional jump (greather than or equal).
    isa_Opcode_jg       = 0x76,                 // Conditional jump (greather than).
    isa_Opcode_cmovle   = 0x21,                 // Conditional move (less than or equal).
    isa_Opcode_cmovl    = 0x22,                 // Conditional move (less than).
    isa_Opcode_cmove    = 0x23,                 // Conditional move (equal).
    isa_Opcode_cmovne   = 0x24,                 // Conditional move (not equal).
    isa_Opcode_cmovge   = 0x25,                 // Conditional move (greather than or equal).
    isa_Opcode_cmovg    = 0x26,                 // Conditional move (greather than).
    isa_Opcode_call     = 0x80,                 // Call (immediate).
    isa_Opcode_ret      = 0x90,                 // Return.
    isa_Opcode_pushq    = 0xA0,                 // Push (register).
    isa_Opcode_popq     = 0xB0                  // Pop (register).
}
isa_Opcode;

// ---------------------------------------------------------------------------------------------- //

#endif // __ISA_OPCODE_H__
