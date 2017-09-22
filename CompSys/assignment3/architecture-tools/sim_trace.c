//
// sim_trace.c
//

#include "sim_trace.h"

// ---------------------------------------------------------------------------------------------- //

#if 0

void sim_traceRegisterUpdate(FILE * file, size_t counter, isa_Register reg, isa_Quad value)
{
    fprintf(file, "%6zu reg %16s %016" PRIX64 "\n", counter, isa_registerAsString(reg), value);
}

void sim_traceMemoryUpdate(FILE * file, size_t counter, isa_Quad address, isa_Quad value)
{
    fprintf(file, "%6zu mem %016" PRIX64 " %016" PRIX64 "\n", counter, address, value);    
}

void sim_traceInstructionPointerUpdate(FILE * file, size_t counter, isa_Quad value)
{
    fprintf(file, "%6zu rip                  %016" PRIX64 "\n", counter, value);
}

#else

void sim_traceRegisterUpdate(FILE * file, size_t counter, isa_Register reg, isa_Quad value)
{
    fprintf(file, "%016" PRIX64 " 0000000000000000 %016" PRIX64 " %016" PRIX64 "\n", (uint64_t) counter, (uint64_t) reg, value);
}

void sim_traceMemoryUpdate(FILE * file, size_t counter, isa_Quad address, isa_Quad value)
{
    fprintf(file, "%016" PRIX64 " 0000000000000001 %016" PRIX64 " %016" PRIX64 "\n", (uint64_t) counter, address, value);    
}

void sim_traceInstructionPointerUpdate(FILE * file, size_t counter, isa_Quad value)
{
    fprintf(file, "%016" PRIX64 " 0000000000000002 %016" PRIX64 " %016" PRIX64 "\n", (uint64_t) counter, (uint64_t) 0, value);
}

#endif

// ---------------------------------------------------------------------------------------------- //
