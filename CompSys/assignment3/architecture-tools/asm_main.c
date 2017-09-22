//
// Nis Wegmann, 2017
//
// asm_main.c
//

#include "asm_allocator.h"
#include "asm_error.h"
#include "asm_ir.h"
#include "asm_serializer.h"
#include "asm_std.h"
#include "asm_symtab.h"

// ---------------------------------------------------------------------------------------------- //

extern int yyparse(void);
extern char const * yyfilename;
extern FILE * yyin;
extern int yylineno;

asm_IR * yyIR;

// ---------------------------------------------------------------------------------------------- //

static void allocatorErrorCallback(void * context, int code)
{
    (void) context; (void) code;
    
    fprintf(stderr, "Out of memory!\n");
    
    exit(EXIT_FAILURE);
}

static void irErrorCallback(char const * message)
{
    asm_error(yyfilename, yylineno, 0, "%s\n", message);
    
    exit(EXIT_FAILURE);
}

// ---------------------------------------------------------------------------------------------- //

extern int main(int argc, const char * argv[])
{
    // Resources.
    
    Allocator * allocator = NULL;
    
    FILE * inputFile = NULL;
    
    FILE * outputFile = NULL;
    
    // Validate input arguments.
    
    if (argc < 2 || argc > 3)
    {
        printf("Usage: asm input-file [output-file]\n");
        
        return EXIT_SUCCESS;
    }
    
    // Create the allocator.
    
    allocator = Allocator_create(64 * 1024, allocatorErrorCallback, NULL);
    
    if (allocator == NULL)
    {
        printf("Out of memory!\n");
        
        goto cleanup;
    }
    
    // Open input-file.
    
    char const * inputFilename = argv[1];
    
    inputFile = fopen(inputFilename, "r");
    
    if (inputFile == NULL)
    {
        printf("Couldn't open file '%s'\n", inputFilename);
        
        goto cleanup;
    }
    
    yyfilename = inputFilename;
    
    yyin = inputFile;
    
    // Open output-file.
    
    if (argc == 3)
    {
        char const * outputFilename = argv[2];
        
        outputFile = fopen(outputFilename, "w");
        
        if (outputFile == NULL)
        {
            printf("Couldn't open file '%s'\n", outputFilename);
            
            goto cleanup;
        }
    }
    
    // Set up the IR.
    
    yyIR = asm_IR_create(allocator, irErrorCallback);
    
    // Parse and encode.
    
    do
    {
        if (yyparse() != 0)
        {
            goto cleanup;
        }
    }
    while(!feof(yyin));
    
    asm_IR_replaceReferenceAddresses(yyIR);
    
    if (outputFile != NULL)
    {
        asm_IR_serialize(yyIR, &(asm_rawSerializer), outputFile);
    }
    else
    {
        asm_IR_serialize(yyIR, &(asm_textualSerializer), stdout);
    }
    
    // Clean-up resources.
    
cleanup:
    
    if (outputFile != NULL)
    {
        fclose(outputFile);
    }
    
    if (inputFile != NULL)
    {
        fclose(inputFile);
    }
    
    if (allocator != NULL)
    {
        Allocator_destroy(allocator);
    }
    
    // Return.
    
    return EXIT_SUCCESS;
}

// ---------------------------------------------------------------------------------------------- //
