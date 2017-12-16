//
// sim_trace.h
//

#ifndef __SIM_TRACE_H__
#define __SIM_TRACE_H__

#include "isa.h"

// ---------------------------------------------------------------------------------------------- //

//
// .
//
void sim_traceRegisterUpdate
    (
        FILE *,                                 // Trace-file.
        size_t,                                 // Counter.
        isa_Register,                           // Register.
        isa_Quad                                // Value.
    );

//
// .
//
void sim_traceMemoryUpdate
    (
        FILE *,                                 // Trace-file.
        size_t,                                 // Counter.
        isa_Quad,                               // Memory address.
        isa_Quad                                // Value.
    );

//
// .
//
void sim_traceInstructionPointerUpdate
    (
        FILE *,                                 // Trace-file.
        size_t,                                 // Counter.
        isa_Quad                                // Value.
    );

// ---------------------------------------------------------------------------------------------- //

#endif // __SIM_TRACE_H__
