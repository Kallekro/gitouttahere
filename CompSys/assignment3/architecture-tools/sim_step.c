//
// Nis Wegmann, 2017
//
// sim_step.c
//

#include "sim_step.h"

#include "sim_trace.h"

#include <stdlib.h>
#include <stdbool.h>

// ---------------------------------------------------------------------------------------------- //

#ifdef USE_ANNOTATION

static void pp_reg_res(char * buffer, isa_Register reg, isa_Quad value)
{
    sprintf(buffer,"%s = %"PRIX64, isa_registerAsString(reg), value);
}

static void pp_mem_res(char * buffer, isa_Quad value)
{
    sprintf(buffer, "MEM = %"PRIX64, value);
}

static void pp_condition(char * buffer, isa_Flags flags)
{
    bool const zf = flags.zf;
    bool const sf = flags.sf;
    bool const of = flags.of;
    
    char const cz = zf ? 'Z' : '-';
    char const cs = sf ? 'S' : '-';
    char const co = of ? 'O' : '-';
    
    sprintf(buffer, "flags = %c%c%c", cz, cs, co);
}

#endif

// ---------------------------------------------------------------------------------------------- //

static inline isa_Status sim_halt
    (
        isa_State * state,
        FILE * trace,
        Annotation * annotation
    )
{
    (void) state;
    (void) trace;
    
  #ifdef USE_ANNOTATION
    if (annotation != NULL)
    {
        note_done(annotation);
    }
  #else
    (void) annotation;
  #endif
    
    return isa_Status_hlt;
}

static inline isa_Status sim_noop
    (
        isa_State * state,
        FILE * trace,
        Annotation * annotation
    )
{
    (void) state;
    
  #ifdef USE_ANNOTATION
    if (annotation != NULL)
    {
        model_nop(annotation);
        note_done(annotation);
    }
  #else
    (void) annotation;
  #endif
    
    if (trace != NULL)
    {
        sim_traceInstructionPointerUpdate(trace, state->cycle, state->ip);
    }
    
    return isa_Status_aok;
}

static inline isa_Status sim_movq_rr
    (
        isa_State * state,
        FILE * trace,
        isa_Register src,
        isa_Register dst,
        Annotation * annotation
    )
{
    isa_Quad const value = state->registers[src];
    
    state->registers[dst] = value;
    
  #ifdef USE_ANNOTATION
    if (annotation != NULL)
    {
        char buffer [64];
        
        model_rrmov(annotation, src, dst, false);
        pp_reg_res(buffer, dst, value);
        note_done_reg(annotation, buffer);
    }
  #else
    (void) annotation;
  #endif
    
    if (trace != NULL)
    {
        sim_traceRegisterUpdate(trace, state->cycle, dst, value);
        sim_traceInstructionPointerUpdate(trace, state->cycle, state->ip);
    }
    
    return isa_Status_aok;
}

static inline isa_Status sim_movq_ir
    (
        isa_State * state,
        FILE * trace,
        isa_Long src,
        isa_Register dst,
        Annotation * annotation
    )
{
    isa_Quad const value = isa_signExtendLongToQuad(src);
    
    state->registers[dst] = value;
    
  #ifdef USE_ANNOTATION
    if (annotation != NULL)
    {
        char buffer [64];
        
        model_irmov(annotation, src);
        pp_reg_res(buffer, dst, value);
        note_done_reg(annotation, buffer);
    }
  #else
    (void) annotation;
  #endif
   
    if (trace != NULL)
    {
        sim_traceRegisterUpdate(trace, state->cycle, dst, value);
        sim_traceInstructionPointerUpdate(trace, state->cycle, state->ip);
    }
    
    return isa_Status_aok;
}

static inline isa_Status sim_movq_rm
    (
        isa_State * state,
        FILE * trace,
        isa_Register src,
        isa_Register idx,
        isa_Long dst,
        Annotation * annotation
    )
{
    isa_Quad const value = state->registers[src];
    
    isa_Quad const address = isa_signExtendLongToQuad(dst) + state->registers[idx];
    
    if (address >= state->size)
    {
        fprintf(stderr, "bad access at: %016" PRIX64, address);
        
        return isa_Status_adr;
    }
    
    if ((address & (8-1)) != 0)
    {
        fprintf(stderr, "unaligned access at: %016" PRIX64, address);
        
        return isa_Status_uaa;
    }
    
    isa_writeQuad(state->memory + address, value);
    
  #ifdef USE_ANNOTATION
    if (annotation != NULL)
    {
        char buffer [64];
        
        model_rmmov(annotation, idx, src, (word_t) address);
        pp_mem_res(buffer, state->registers[src]);
        note_done_reg(annotation, buffer);
    }
  #else
    (void) annotation;
  #endif
    
    if (trace != NULL)
    {
        sim_traceMemoryUpdate(trace, state->cycle, address, value);
        sim_traceInstructionPointerUpdate(trace, state->cycle, state->ip);
    }
    
    return isa_Status_aok;
}

static inline isa_Status sim_movq_mr
    (
        isa_State * state,
        FILE * trace,
        isa_Long src,
        isa_Register idx,
        isa_Register dst,
        Annotation * annotation
    )
{
    isa_Quad const address = isa_signExtendLongToQuad(src) + state->registers[idx];
    
    if (address >= state->size)
    {
        fprintf(stderr, "bad access at: %016" PRIX64, address);
        
        return isa_Status_adr;
    }
    
    if ((address & (8-1)) != 0)
    {
        fprintf(stderr, "unaligned access at: %016" PRIX64, address);
        
        return isa_Status_uaa;
    }
    
    isa_Quad const value = isa_readQuad(state->memory + address);
    
    state->registers[dst] = value;
    
  #ifdef USE_ANNOTATION
    if (annotation != NULL)
    {
        char buffer [64];
        
        model_mrmov(annotation, idx, dst, (word_t) address);
        pp_reg_res(buffer, dst, value);
        note_done_reg(annotation, buffer);
    }
  #else
    (void) annotation;
  #endif
    
    if (trace != NULL)
    {
        sim_traceRegisterUpdate(trace, state->cycle, dst, value);
        sim_traceInstructionPointerUpdate(trace, state->cycle, state->ip);
    }
    
    return isa_Status_aok;
}

static inline isa_Status sim_alu
    (
        isa_State * state,
        FILE * trace,
        isa_ALUOp op,
        isa_Register src,
        isa_Register dst,
        Annotation * annotation
    )
{
    isa_Quad const oprnd1 = state->registers[src]; // First operand.
    isa_Quad const oprnd2 = state->registers[dst]; // Second operand.
    isa_Quad const result = isa_computeALUOp(op, oprnd2, oprnd1); // Result.
    
    if (op != isa_ALUOp_cmpq)
    {
        state->registers[dst] = result;
    }
    
    state->flags = isa_computeFlags(state->flags, op, oprnd1, oprnd2, result);
    
  #ifdef USE_ANNOTATION
    if (annotation != NULL)
    {
        char buffer1 [64];
        char buffer2 [64];
        
        model_alu(annotation, src, dst);
        pp_condition(buffer1, state->flags);
        
        if (op == isa_ALUOp_cmpq)
        {
            note_done_cc(annotation, buffer1);
        }
        else
        {
            pp_reg_res(buffer2, dst, result);
            note_done_reg_cc(annotation, buffer2, buffer1);
        }
    }
  #else
    (void) annotation;
  #endif
    
    if (trace != NULL)
    {
        sim_traceRegisterUpdate(trace, state->cycle, dst, result);
        sim_traceInstructionPointerUpdate(trace, state->cycle, state->ip);
    }
    
    return isa_Status_aok;
}

static inline isa_Status sim_jump
    (
        isa_State * state,
        FILE * trace,
        isa_Long i,
        Annotation * annotation
    )
{
    isa_Quad const address = isa_signExtendLongToQuad(i);
    
    state->ip = address;
    
  #ifdef USE_ANNOTATION
    if (annotation != NULL)
    {
        model_jmp(annotation, (word_t) address, false /* isConditional */, true /* taken */);
        note_done(annotation);
    }
  #else
    (void) annotation;
  #endif
    
    if (trace != NULL)
    {
        sim_traceInstructionPointerUpdate(trace, state->cycle, state->ip);
    }
    
    return isa_Status_aok;
}

static inline isa_Status sim_condJump
    (
        isa_State * state,
        FILE * trace,
        isa_Condition condition,
        isa_Long i,
        Annotation * annotation
    )
{
    isa_Quad const address = isa_signExtendLongToQuad(i);
    
    bool const conditionHolds = isa_checkCondition(condition, state->flags); 
    
    if (conditionHolds)
    {
        state->ip = address;
    }
    
  #ifdef USE_ANNOTATION
    if (annotation != NULL)
    {
        model_jmp(annotation, (word_t) address, true /* isConditional */, conditionHolds);
        note_done(annotation);
    }
  #else
    (void) annotation;
  #endif
    
    if (trace != NULL)
    {
        sim_traceInstructionPointerUpdate(trace, state->cycle, state->ip);
    }
    
    return isa_Status_aok;
}

static inline isa_Status sim_condMove
    (
        isa_State * state,
        FILE * trace,
        isa_Condition condition,
        isa_Register src,
        isa_Register dst,
        Annotation * annotation
    )
{
    bool const conditionHolds = isa_checkCondition(condition, state->flags);
    
    isa_Quad const value = state->registers[src];
    
    if (conditionHolds)
    {
        state->registers[dst] = value;
    }
    
  #ifdef USE_ANNOTATION
    if (annotation != NULL)
    {
        char buffer [64];
        
        model_rrmov(annotation, src, dst, true);
        
        if (conditionHolds)
        {
            pp_reg_res(buffer, dst, value);
            note_done_reg(annotation, buffer);
        }
        else
        {
            note_done(annotation);
        }
    }
  #else
    (void) annotation;
  #endif
    
    if (trace != NULL)
    {
        if (conditionHolds)
        {
            sim_traceRegisterUpdate(trace, state->cycle, dst, value);
        }
        
        sim_traceInstructionPointerUpdate(trace, state->cycle, state->ip);
    }
    
    return isa_Status_aok;
}

static inline isa_Status sim_call
    (
        isa_State * state,
        FILE * trace,
        isa_Long i,
        Annotation * annotation
    )
{
    isa_Quad const address1 = isa_signExtendLongToQuad(i);
    
    state->registers[isa_Register_rsp] -= 8;
    
    isa_Quad const address2 = state->registers[isa_Register_rsp];
    
    if (address2 >= state->size)
    {
        fprintf(stderr, "bad access at: %016" PRIX64, address2);
        
        return isa_Status_adr;
    }
    
    isa_Quad const value = state->ip;
    
    isa_writeQuad(state->memory + address2, value);
    
    state->ip = address1;
    
  #ifdef USE_ANNOTATION
    if (annotation != NULL)
    {
        char buffer1 [64];
        char buffer2 [64];
        
        model_call(annotation, (word_t) state->ip, (word_t) address1, (word_t) address2);
        
        pp_reg_res(buffer1, isa_Register_rsp, state->registers[isa_Register_rsp]);
        pp_mem_res(buffer2, value);
        note_done_reg_mem(annotation, buffer1, buffer2);
    }
  #else
    (void) annotation;
  #endif
    
    if (trace != NULL)
    {
        sim_traceRegisterUpdate(trace, state->cycle, isa_Register_rsp, state->registers[isa_Register_rsp]);
        sim_traceMemoryUpdate(trace, state->cycle, address2, value);
        sim_traceInstructionPointerUpdate(trace, state->cycle, state->ip);
    }
    
    return isa_Status_aok;
}

static inline isa_Status sim_ret
    (
        isa_State * state,
        FILE * trace,
        Annotation * annotation
    )
{
    isa_Quad const address1 = state->registers[isa_Register_rsp];
    
    state->registers[isa_Register_rsp] += 8;
    
    if (address1 >= state->size)
    {
        fprintf(stderr, "bad access at: %016" PRIX64, address1);
        
        return isa_Status_adr;
    }
    
    isa_Quad const address2 = isa_readQuad(state->memory + address1);
    
    state->ip = address2;

  #ifdef USE_ANNOTATION
    if (annotation != NULL)
    {
        char buffer1 [64];
        
        model_ret(annotation, (word_t) address2, (word_t) address1);
        pp_reg_res(buffer1, isa_Register_rsp, state->registers[isa_Register_rsp]);
        note_done_reg(annotation, buffer1);
    }
  #else
    (void) annotation;
  #endif
    
    if (trace != NULL)
    {
        sim_traceRegisterUpdate(trace, state->cycle, isa_Register_rsp, state->registers[isa_Register_rsp]);
        sim_traceInstructionPointerUpdate(trace, state->cycle, state->ip);
    }
    
    return isa_Status_aok;
}

static inline isa_Status sim_push
    (
        isa_State * state,
        FILE * trace,
        isa_Register reg,
        Annotation * annotation
    )
{
    state->registers[isa_Register_rsp] -= 8;
    
    isa_Quad const address = state->registers[isa_Register_rsp];
    
    if (address >= state->size)
    {
        fprintf(stderr, "bad access at: %016" PRIX64, address);
        
        return isa_Status_adr;
    }
    
    isa_Quad const value = state->registers[reg];
    
    isa_writeQuad(state->memory + address, value);
    
  #ifdef USE_ANNOTATION
    if (annotation != NULL)
    {
        char buffer1 [64];
        char buffer2 [64];
        
        model_push(annotation, reg, (word_t) address);
        pp_reg_res(buffer1, isa_Register_rsp, address);
        pp_mem_res(buffer2, value);
        note_done_reg_reg(annotation, buffer1, buffer1);
    }
  #else
    (void) annotation;
  #endif
    
    if (trace != NULL)
    {
        sim_traceRegisterUpdate(trace, state->cycle, isa_Register_rsp, address);
        sim_traceMemoryUpdate(trace, state->cycle, address, value);
        sim_traceInstructionPointerUpdate(trace, state->cycle, state->ip);
    }
    
    return isa_Status_aok;
}

static inline isa_Status sim_pop
    (
        isa_State * state,
        FILE * trace,
        isa_Register reg,
        Annotation * annotation
    )
{
    isa_Quad const address = state->registers[isa_Register_rsp];
    
    state->registers[isa_Register_rsp] += 8;
    
    if (address >= state->size)
    {
        fprintf(stderr, "bad access at: %016" PRIX64, address);
        
        return isa_Status_adr;
    }
    
    isa_Quad const value = isa_readQuad(state->memory + address);
    
    state->registers[reg] = value;
    
  #ifdef USE_ANNOTATION
    if (annotation)
    {
        char buffer1 [64];
        char buffer2 [64];
        
        model_pop(annotation, reg, (word_t) address);
        pp_reg_res(buffer1, isa_Register_rsp, state->registers[isa_Register_rsp]);
        pp_reg_res(buffer2, reg, value);
        note_done_reg_reg(annotation, buffer1, buffer2);
    }
  #else
    (void) annotation;
  #endif
    
    if (trace != NULL)
    {
        sim_traceRegisterUpdate(trace, state->cycle, isa_Register_rsp, state->registers[isa_Register_rsp]);
        sim_traceRegisterUpdate(trace, state->cycle, reg, value);
        sim_traceInstructionPointerUpdate(trace, state->cycle, state->ip);
    }
    
    return isa_Status_aok;
}

// ---------------------------------------------------------------------------------------------- //

extern isa_Status sim_step
    (
        isa_State * restrict state,
        Annotation * anno,
        FILE * trace
    )
{
    // Increment cycle.
    
    state->cycle += 1;
    
    // Check that `state->ip` is not out of bounds.
    
    if (state->ip > state->size - 6)
    {
        fprintf(stderr, "bad access at: %016" PRIX64, state->ip);
        
        return isa_Status_adr;
    }
    
    // Decode.
    
    isa_Instruction instr = ISA_NOP();
    
    size_t size = isa_decode(state->memory + state->ip, &(instr));
    
    // Validate.
    
    if (size == 0)
    {
        fprintf(stderr, "bad instruction at: %016" PRIX64, state->ip);
        
        return isa_Status_ins;
    }
    
    // Advance the instruction pointer.
    
    state->ip += size;
    
    // Annotate.
    
  #ifdef USE_ANNOTATION
    if (anno != NULL)
    {
        char buffer [64];
        
        isa_sPrintInstruction(buffer, NULL, instr);
        
        note_insn(anno, buffer, (word_t) state->ip);
    }
  #endif
    
    // Execute.
    
    switch (instr.opcode)
    {
    
  #define CASE(__op__) case isa_Opcode_##__op__
    
    CASE(hlt):     return sim_halt(state, trace, anno);
    CASE(nop):     return sim_noop(state, trace, anno);
    CASE(movq_rr): return sim_movq_rr(state, trace, instr.rr.src, instr.rr.dst, anno);
    CASE(movq_ir): return sim_movq_ir(state, trace, instr.ir.src, instr.ir.dst, anno);
    CASE(movq_rm): return sim_movq_rm(state, trace, instr.rm.src, instr.rm.idx, instr.rm.dst, anno);
    CASE(movq_mr): return sim_movq_mr(state, trace, instr.mr.src, instr.mr.idx, instr.mr.dst, anno);
    CASE(addq_rr): return sim_alu(state, trace, isa_ALUOp_addq, instr.rr.src, instr.rr.dst, anno);
    CASE(subq_rr): return sim_alu(state, trace, isa_ALUOp_subq, instr.rr.src, instr.rr.dst, anno);
    CASE(andq_rr): return sim_alu(state, trace, isa_ALUOp_andq, instr.rr.src, instr.rr.dst, anno);
    CASE(xorq_rr): return sim_alu(state, trace, isa_ALUOp_xorq, instr.rr.src, instr.rr.dst, anno);
    CASE(cmpq_rr): return sim_alu(state, trace, isa_ALUOp_cmpq, instr.rr.src, instr.rr.dst, anno);
    CASE(shrq):    return sim_alu(state, trace, isa_ALUOp_shrq, instr.rr.src, instr.rr.dst, anno);
    CASE(jmp):     return sim_jump(state, trace, instr.i, anno);
    CASE(jle):     return sim_condJump(state, trace, isa_Condition_le, instr.i, anno);
    CASE(jl):      return sim_condJump(state, trace, isa_Condition_l, instr.i, anno);
    CASE(je):      return sim_condJump(state, trace, isa_Condition_e, instr.i, anno);
    CASE(jne):     return sim_condJump(state, trace, isa_Condition_ne, instr.i, anno);
    CASE(jge):     return sim_condJump(state, trace, isa_Condition_ge, instr.i, anno);
    CASE(jg):      return sim_condJump(state, trace, isa_Condition_g, instr.i, anno);
    CASE(cmovle):  return sim_condMove(state, trace, isa_Condition_le, instr.rr.src, instr.rr.dst, anno);
    CASE(cmovl):   return sim_condMove(state, trace, isa_Condition_l, instr.rr.src, instr.rr.dst, anno);
    CASE(cmove):   return sim_condMove(state, trace, isa_Condition_e, instr.rr.src, instr.rr.dst, anno);
    CASE(cmovne):  return sim_condMove(state, trace, isa_Condition_ne, instr.rr.src, instr.rr.dst, anno);
    CASE(cmovge):  return sim_condMove(state, trace, isa_Condition_ge, instr.rr.src, instr.rr.dst, anno);
    CASE(cmovg):   return sim_condMove(state, trace, isa_Condition_g, instr.rr.src, instr.rr.dst, anno);
    CASE(call):    return sim_call(state, trace, instr.i, anno);
    CASE(ret):     return sim_ret(state, trace, anno);
    CASE(pushq):   return sim_push(state, trace, instr.r, anno);
    CASE(popq):    return sim_pop(state, trace, instr.r, anno);
    
  #undef CASE
    
    default: abort();
    
    }
}

// ---------------------------------------------------------------------------------------------- //
