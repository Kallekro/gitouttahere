//
// Nis Wegmann, 2017
//
// isa_print.h
//

#ifndef __ISA_PRINT_H__
#define __ISA_PRINT_H__

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>

// ---------------------------------------------------------------------------------------------- //

static char const * isa_registerAsString(isa_Register reg)
{
    switch (reg)
    {
        case isa_Register_rax: return "%rax";
        case isa_Register_rcx: return "%rcx";
        case isa_Register_rdx: return "%rdx";
        case isa_Register_rbx: return "%rbx";
        case isa_Register_rsi: return "%rsi";
        case isa_Register_rdi: return "%rdi";
        case isa_Register_rsp: return "%rsp";
        case isa_Register_rbp: return "%rbp";
        case isa_Register_r8 : return "%r8" ;
        case isa_Register_r9 : return "%r9" ;
        case isa_Register_r10: return "%r10";
        case isa_Register_r11: return "%r11";
        case isa_Register_r12: return "%r12";
        case isa_Register_r13: return "%r13";
        case isa_Register_r14: return "%r14";
        case isa_Register_r15: return "%r15";
    }
    
    abort();
}

// ---------------------------------------------------------------------------------------------- //

static char const * isa_opcodeAsString(isa_Opcode opcode)
{
    switch (opcode)
    {
        case isa_Opcode_hlt:        return "hlt";
        case isa_Opcode_nop:        return "nop";
        case isa_Opcode_movq_rr:    return "movq";
        case isa_Opcode_movq_ir:    return "movq";
        case isa_Opcode_movq_rm:    return "movq";
        case isa_Opcode_movq_mr:    return "movq";
        case isa_Opcode_addq_rr:    return "addq";
        case isa_Opcode_subq_rr:    return "subq";
        case isa_Opcode_andq_rr:    return "andq";
        case isa_Opcode_xorq_rr:    return "xorq";
        case isa_Opcode_cmpq_rr:    return "cmpq";
        case isa_Opcode_shrq:       return "shrq";
        case isa_Opcode_jmp:        return "jmp";
        case isa_Opcode_jle:        return "jle";
        case isa_Opcode_jl:         return "jl";
        case isa_Opcode_je:         return "je";
        case isa_Opcode_jne:        return "jne";
        case isa_Opcode_jge:        return "jge";
        case isa_Opcode_jg:         return "jg";
        case isa_Opcode_cmovle:     return "cmovle";
        case isa_Opcode_cmovl:      return "cmovl";
        case isa_Opcode_cmove:      return "cmove";
        case isa_Opcode_cmovne:     return "cmovne";
        case isa_Opcode_cmovge:     return "cmovge";
        case isa_Opcode_cmovg:      return "cmovg";
        case isa_Opcode_call:       return "call";
        case isa_Opcode_ret:        return "ret";
        case isa_Opcode_pushq:      return "pushq";
        case isa_Opcode_popq:       return "popq";
    }
    
    abort();
}

// ---------------------------------------------------------------------------------------------- //

typedef void PrintFunction(void *, char const *, ...);

static inline void isa_gPrintInstruction(void * ctx, PrintFunction * f, isa_Instruction instruction, char const * label)
{
    switch (instruction.opcode)
    {
        case isa_Opcode_hlt:
            
            f(ctx, "%s",
                isa_opcodeAsString(instruction.opcode)
            );
            
            break;
            
        case isa_Opcode_nop:
            
            f(ctx, "%s",
                isa_opcodeAsString(instruction.opcode)
            );
            
            break;
            
        case isa_Opcode_movq_rr:
            
            f(ctx, "%s %s, %s",
                isa_opcodeAsString(instruction.opcode),
                isa_registerAsString(instruction.rr.src),
                isa_registerAsString(instruction.rr.dst)
            );
            
            break;
            
        case isa_Opcode_movq_ir:
            
            f(ctx, "%s $%" PRIX64 ", %s",
                isa_opcodeAsString(instruction.opcode),
                instruction.ir.src,
                isa_registerAsString(instruction.ir.dst)
            );
            
            break;
            
        case isa_Opcode_movq_rm:
            
            if (label != NULL)
            {
                f(ctx, "%s %s, %s(%s)",
                    isa_opcodeAsString(instruction.opcode),
                    isa_registerAsString(instruction.rm.src),
                    label,
                    isa_registerAsString(instruction.rm.idx)
                );
            }
            if (instruction.rm.dst != 0)
            {
                f(ctx, "%s %s, $%" PRIX64 "(%s)",
                    isa_opcodeAsString(instruction.opcode),
                    isa_registerAsString(instruction.rm.src),
                    instruction.rm.dst,
                    isa_registerAsString(instruction.rm.idx)
                );
            }
            else
            {
                f(ctx, "%s %s, (%s)",
                    isa_opcodeAsString(instruction.opcode),
                    isa_registerAsString(instruction.rm.src),
                    isa_registerAsString(instruction.rm.idx)
                );                
            }
            
            break;
            
        case isa_Opcode_movq_mr:
            
            if (label != NULL)
            {
                f(ctx, "%s %s(%s), %s",
                    isa_opcodeAsString(instruction.opcode),
                    label,
                    isa_registerAsString(instruction.mr.idx),
                    isa_registerAsString(instruction.mr.dst)
                );            
            }
            else if (instruction.mr.src != 0)
            {
                f(ctx, "%s $%" PRIX64 "(%s), %s",
                    isa_opcodeAsString(instruction.opcode),
                    instruction.mr.src,
                    isa_registerAsString(instruction.mr.idx),
                    isa_registerAsString(instruction.mr.dst)
                );
            }
            else
            {
                f(ctx, "%s (%s), %s",
                    isa_opcodeAsString(instruction.opcode),
                    isa_registerAsString(instruction.mr.idx),
                    isa_registerAsString(instruction.mr.dst)
                );                
            }
            
            break;
            
        case isa_Opcode_addq_rr:
        case isa_Opcode_subq_rr:
        case isa_Opcode_andq_rr:
        case isa_Opcode_xorq_rr:
        case isa_Opcode_cmpq_rr:
        case isa_Opcode_shrq:
            
            f(ctx, "%s %s, %s",
                isa_opcodeAsString(instruction.opcode),
                isa_registerAsString(instruction.rr.src),
                isa_registerAsString(instruction.rr.dst)
            );
            
            break;
            
        case isa_Opcode_jmp:
        case isa_Opcode_jle:
        case isa_Opcode_jl:
        case isa_Opcode_je:
        case isa_Opcode_jne:
        case isa_Opcode_jge:
        case isa_Opcode_jg:
            
            if (label != NULL)
            {
                f(ctx, "%s %s",
                    isa_opcodeAsString(instruction.opcode),
                    label
                );
            }
            else
            {
                f(ctx, "%s $%016" PRIX64,
                    isa_opcodeAsString(instruction.opcode),
                    instruction.i
                );
            }
            
            break;
            
        case isa_Opcode_cmovle:
        case isa_Opcode_cmovl:
        case isa_Opcode_cmove:
        case isa_Opcode_cmovne:
        case isa_Opcode_cmovge:
        case isa_Opcode_cmovg:
            
            f(ctx, "%s %s, %s",
                isa_opcodeAsString(instruction.opcode),
                isa_registerAsString(instruction.rr.src),
                isa_registerAsString(instruction.rr.dst)
            );
            
            break;
            
        case isa_Opcode_call:
            
            if (label != NULL)
            {
                f(ctx, "%s %s",
                    isa_opcodeAsString(instruction.opcode),
                    label
                );
            }
            else
            {
                f(ctx, "%s $%016" PRIX64,
                    isa_opcodeAsString(instruction.opcode),
                    instruction.i
                );
            }
            
            break;
            
        case isa_Opcode_ret:
            
            f(ctx, "%s",
                isa_opcodeAsString(instruction.opcode)
            );
            
            break;
            
        case isa_Opcode_pushq:
            
            f(ctx, "%s %s",
                isa_opcodeAsString(instruction.opcode),
                isa_registerAsString(instruction.r)
            );
            
            break;
            
        case isa_Opcode_popq:
            
            f(ctx, "%s %s",
                isa_opcodeAsString(instruction.opcode),
                isa_registerAsString(instruction.r)
            );
            
            break;
    }
}

static inline void isa_fPrintInstruction(FILE * file, isa_Instruction instr, char const * label)
{
    isa_gPrintInstruction(file, (PrintFunction *) fprintf, instr, label);
}

static inline void isa_sPrintInstruction(char * buffer, char const * label, isa_Instruction instr)
{
    isa_gPrintInstruction(buffer, (PrintFunction *) sprintf, instr, label);
}

// ---------------------------------------------------------------------------------------------- //

#endif // __ISA_PRINT_H__
