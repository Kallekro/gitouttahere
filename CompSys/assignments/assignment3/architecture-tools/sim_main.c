//
// Nis Wegmann, 2017
//
// sim_main.c
//

#include "isa.h"
#include "sim_dump.h"
#include "sim_step.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ---------------------------------------------------------------------------------------------- //

// Memory size.
static size_t const kMemorySize = 0x10000; // 1 MB

extern int main(int argc, const char * argv[])
{
    // Resources.
    
    FILE * inputFile = NULL;
    FILE * traceFile = NULL;
    isa_State * state = NULL;
    unsigned char * backup = NULL;
    Annotation * annotation = NULL;
    
    // Validate input arguments.
    
  #ifdef USE_ANNOTATION
    if (argc < 2)
    {  
        printf("Usage: sim input-file [options*]\n");
        
        annotation_usage();
        
        return EXIT_SUCCESS;
    }
  #else
    if (argc < 2 || argc > 3)
    {
        printf("Usage: sim input-file [trace-file]\n");
        
        return EXIT_SUCCESS;
    }
  #endif
    
    // Create the annotator.
    
  #ifdef USE_ANNOTATION
    annotation = create_annotation(argc - 1, argv + 1);
  #else
    annotation = NULL;
  #endif
    
    // Open input-file.
    
    char const * inputFilename = argv[1];
    
    inputFile = fopen(inputFilename, "r");
    
    if (inputFile == NULL)
    {
        fprintf(stderr, "Couldn't open input-file '%s'\n", inputFilename);
        
        goto cleanup;
    }
    
    // Open trace-file.
    
    char const * traceFilename = argv[2];
    
    if (argc == 3)
    {
        traceFile = fopen(traceFilename, "w+");
        
        if (traceFile == NULL)
        {
            fprintf(stderr, "Couldn't open trace file '%s'\n", inputFilename);
            
            goto cleanup;
        }
    }
    
    // Allocate and read memory.
    
    state = isa_State_create(kMemorySize); 
    
    backup = malloc(kMemorySize);
    
    if (state == NULL || backup == NULL)
    {
        fprintf(stderr, "Memory allocation failure.\n");
        
        goto cleanup;
    }
    
    if (fread(state->memory, 1, kMemorySize, inputFile) == 0)
    {
        fprintf(stderr, "Could not read input-file.\n");
    }
    
    if (getc(inputFile) != EOF)
    {
        fprintf(stderr, "Memory block is not large enough to contain program.\n");
        
        goto cleanup;
    }
    
    memcpy(backup, state->memory, kMemorySize);
    
    // Run simulation.
    
    isa_Status status = isa_Status_aok;
    
    for (;;)
    {
        status = sim_step(state, annotation, traceFile);
        
        switch (status)
        {
            case isa_Status_aok:
                
                continue;
                
            case isa_Status_hlt:
                
                goto dump;
                
            case isa_Status_adr:
            case isa_Status_ins:
            case isa_Status_uaa:
                
                fprintf(stderr, ", ip = 0x%016" PRIX64 "\n", state->ip);
                
                goto cleanup;
        }
    }
    
dump:
    
    // Dump simulator state and changes to the memory.
    
    sim_dumpState(state, backup);
    
  #ifdef USE_ANNOTATION
    if (annotation != NULL)
    {
        print_annotation(annotation);
    }
  #endif
    
    // Free resources.
    
cleanup:
    
    if (backup != NULL)
    {
        free(backup);
    }
    
    if (state != NULL)
    {
        isa_State_destroy(state);
    }
    
    if (traceFile != NULL)
    {
        fclose(traceFile);
    }
    
    if (inputFile != NULL)
    {
        fclose(inputFile);
    }
    
  #ifdef USE_ANNOTATION
    if (annotation != NULL)
    {
        free_annotation(annotation);
    }
  #endif
}

// ---------------------------------------------------------------------------------------------- //
