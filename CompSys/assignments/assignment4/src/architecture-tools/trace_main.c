//
// trace_main.c
//

#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>

#include "isa.h"
#include "trace_read.h"

// ---------------------------------------------------------------------------------------------- //

extern int main(int argc, const char * argv[])
{
    (void) argc;
    (void) argv;
    
    if (argc != 2)
    {
        printf("Usage: ./trace trace-file\n");
    }
    
    FILE * file = fopen(argv[1], "r");
    
    if (file == NULL)
    {
        fprintf(stderr, "Couldn't open trace file");
        
        exit(EXIT_FAILURE);
    }
    
    fprintf(stdout, "%12s %8s  %16s  %16s\n", "Instr. #:", "Type:", "Destination:", "Value:");
    fprintf(stdout, "---------------------------------------------------------\n");
    
    for (;;)
    {
        Trace_Entry entries [1];
        
        if (Trace_read(file, entries, 1) != 1)
        {
            break;
        }
        
        switch (entries[0].type)
        {
            case Trace_Type_reg:
                
                fprintf(stdout, "%12" PRIu64 " %8s  %16s  %016" PRIX64 "\n",
                    entries[0].counter,
                    "reg",
                    isa_registerAsString((isa_Register) entries[0].destination),
                    entries[0].value
                );
                
                continue;
                
            case Trace_Type_mem:
                
                fprintf(stdout, "%12" PRIu64 " %8s  %016" PRIX64 "  %016" PRIX64 "\n",
                    entries[0].counter,
                    "mem",
                    entries[0].destination,
                    entries[0].value
                );
                
                continue;

            case Trace_Type_pc:
                
                fprintf(stdout, "%12" PRIu64 " %8s  %16s  %016" PRIX64 "\n",
                    entries[0].counter,
                    "pc",
                    "",
                    entries[0].value
                );
                
                continue;
        }
    }
    
    fclose(file);
    
    return EXIT_SUCCESS;
}

// ---------------------------------------------------------------------------------------------- //
