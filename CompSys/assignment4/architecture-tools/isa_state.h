//
// Nis Wegmann, 2017
//
// isa_state.h
//

#ifndef __ISA_STATE_H__
#define __ISA_STATE_H__

#include "isa_word.h"
#include "isa_flags.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

// ---------------------------------------------------------------------------------------------- //

//
// State type. Represents the state of an X86-64 machine, i.e. registers and memory.
//
typedef struct isa_State
{
    isa_Quad registers [16];                    // Register bank.
    isa_Flags flags;                            // Flags register.
    isa_Quad ip;                                // Instruction pointer.
    size_t size;                                // Size of memory block.
    size_t cycle;                               // Cycle counter.
    _Alignas(8) unsigned char memory [];        // Memory block.
}
isa_State;

// ---------------------------------------------------------------------------------------------- //

//
// Creates a new state with memory size 'memorySize'.
//
// Returns 'NULL' if malloc fails.
//
static inline isa_State * isa_State_create(size_t memorySize)
{
    isa_State * state = malloc(sizeof(isa_State) + memorySize);
    
    if (state == NULL)
    {
        return NULL;
    }
    
    memset(state, 0, sizeof(isa_State) + memorySize);
    
    state->size = memorySize;
    state->flags = isa_defaultFlags;
    
    return state;
}

//
// Destroys a state.
//
static inline void isa_State_destroy(isa_State * state)
{
    assert(state != NULL);
    
    free(state);
}

// ---------------------------------------------------------------------------------------------- //

#endif // __ISA_STATE_H__
