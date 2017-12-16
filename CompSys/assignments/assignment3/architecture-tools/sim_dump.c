//
// Nis Wegmann, 2017
//
// sim_dump.c
//

#include "sim_dump.h"

#include <assert.h>
#include <inttypes.h>

// ---------------------------------------------------------------------------------------------- //

extern void sim_dumpState
    (
        isa_State const * state,
        unsigned char const * backup
    )
{
    assert(state != NULL);
    
    // Dump the registers.
    
    printf("Registers:\n");
    printf("\n");
    
  #define FORMAT "  %s : %16" PRIX64 "\n"
    printf(FORMAT, "rax", state->registers[isa_Register_rax]);
    printf(FORMAT, "rcx", state->registers[isa_Register_rcx]);
    printf(FORMAT, "rdx", state->registers[isa_Register_rdx]);
    printf(FORMAT, "rbx", state->registers[isa_Register_rbx]);
    printf(FORMAT, "rsi", state->registers[isa_Register_rsi]);
    printf(FORMAT, "rdi", state->registers[isa_Register_rdi]);
    printf(FORMAT, "rsp", state->registers[isa_Register_rsp]);
    printf(FORMAT, "rbp", state->registers[isa_Register_rbp]);
    printf(FORMAT, "r8 ", state->registers[isa_Register_r8 ]);
    printf(FORMAT, "r9 ", state->registers[isa_Register_r9 ]);
    printf(FORMAT, "r10", state->registers[isa_Register_r10]);
    printf(FORMAT, "r11", state->registers[isa_Register_r11]);
    printf(FORMAT, "r12", state->registers[isa_Register_r12]);
    printf(FORMAT, "r13", state->registers[isa_Register_r13]);
    printf(FORMAT, "r14", state->registers[isa_Register_r14]);
    printf(FORMAT, "r15", state->registers[isa_Register_r15]);
  #undef FORMAT
    
    // Dump the instruction pointer.
    
    printf("\n");
    printf("Instruction Pointer:\n");
    printf("\n");
    
  #define FORMAT "  %s : 0x%05" PRIX64 "\n"
    printf(FORMAT, "ip", state->ip);
  #undef FORMAT
    
    // Dump the conditional codes.
    
    //printf("\n");
    //printf("Conditional Codes:\n");
    //printf("\n");
    
  #define FORMAT "  %s : %u\n"
    //printf(FORMAT, "cf", sim_unpackCC(state->cc, sim_ConditionCode_cf));
    //printf(FORMAT, "zf", sim_unpackCC(state->cc, sim_ConditionCode_zf));
    //printf(FORMAT, "sf", sim_unpackCC(state->cc, sim_ConditionCode_sf));
    //printf(FORMAT, "of", sim_unpackCC(state->cc, sim_ConditionCode_of));
  #undef FORMAT
    
    // Dump all changes to the memory block.
    
    printf("\n");
    printf("Changes to Memory:\n");
    printf("\n");
    
    for (size_t position = 0; position < state->size; position += sizeof(isa_Quad))
    {
        // Iterate through the 'quads' in the two memory blocks.
        
        isa_Quad const old = isa_readQuad(backup + position);
        isa_Quad const new = isa_readQuad(state->memory + position);
        
        // If the quads differ then dump them.
        
        if (old != new)
        {
            printf("  [%.5lX] %16" PRIX64 " -> %16" PRIX64 "\n", position, old, new);
        }
    }
    
    printf("\n");
}

// ---------------------------------------------------------------------------------------------- //
