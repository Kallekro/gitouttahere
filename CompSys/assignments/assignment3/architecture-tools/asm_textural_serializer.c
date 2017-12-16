//
// Nis Wegmann, 2017
//
// asm_textual_serializer.c
//

#include "asm_serializer.h"

#include "isa.h"
#include "asm_std.h"

// ---------------------------------------------------------------------------------------------- //

static int fprintBytes(FILE * file, unsigned char * bytes, size_t size, size_t width)
{
    assert(width >= size * 3);
    
    size_t space = width - size * 3;
    
    for (size_t k = 0; k < space; k++)
    {
        fputc(' ', file);
    }
    
    for (size_t k = 0; k < size; k++)
    {
        fprintf(file, "%02X ", bytes[k]);
    }
    
    return 0;
}

// ---------------------------------------------------------------------------------------------- //

static int instruction(FILE * file, isa_Instruction instruction, char const * label, size_t address)
{
    unsigned char bytes [64];
    
    size_t size = isa_encode(bytes, instruction);
    
    fprintf(file, "%05lX", address);
    fprintBytes(file, bytes, size, 31);
    fprintf(file, "| \t\t");
    isa_fPrintInstruction(file, instruction, label);
    fprintf(file, "\n");
    
    return 0;
}

static int label(FILE * file, char const * label, size_t address)
{
    fprintf(file, "%05lX", address);
    fprintBytes(file, NULL, 0, 31);
    fprintf(file, "| %s:\n", label);
    
    return 0;
}

static int quad(FILE * file, isa_Quad quad, size_t address)
{
    unsigned char bytes [8];
    
    isa_writeQuad(bytes, quad);
    
    fprintf(file, "%05lX", address);
    fprintBytes(file, bytes, sizeof(bytes), 31);
    fprintf(file, "| \t\t.quad   %016" PRIX64 "\n", quad);
    
    return 0;
}

static int zeroes(FILE * file, size_t count, size_t address)
{
    (void) file; // Suppress warning.
    (void) count; // Suppress warning.
    (void) address; // Suppress warning.
    
    return 0;
}

// ---------------------------------------------------------------------------------------------- //

asm_Serializer const asm_textualSerializer =
{
    .instruction = instruction,
    .label = label,
    .quad = quad,
    .zeroes = zeroes
};

// ---------------------------------------------------------------------------------------------- //
