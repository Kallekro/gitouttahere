//
// Nis Wegmann, 2017
//
// asm_raw_serializer.c
//

#include "asm_serializer.h"

#include "isa.h"
#include "asm_std.h"

// ---------------------------------------------------------------------------------------------- //

static int instruction(FILE * file, isa_Instruction instruction, char const * label, size_t address)
{
    (void) label; // Suppress warning.
    (void) address; // Suppress warning.
    
    unsigned char bytes [64];
    
    size_t size = isa_encode(bytes, instruction);
    
    fwrite(bytes, 1, size, file);
    
    return 0;
}

static int label(FILE * file, char const * label, size_t address)
{
    (void) file; // Suppress warning.
    (void) label; // Suppress warning.
    (void) address; // Suppress warning.
    
    return 0;
}

static int quad(FILE * file, isa_Quad quad, size_t address)
{
    (void) address; // Suppress warning.
    
    unsigned char bytes [8];
    
    isa_writeQuad(bytes, quad);
    
    fwrite(bytes, 1, 8, file);
    
    return 0;
}

static int zeroes(FILE * file, size_t count, size_t address)
{
    (void) address; // Suppress warning.
    
    for (size_t n = 0; n < count; n++)
    {
        fputc(0, file);
    }
    
    return 0;
}

// ---------------------------------------------------------------------------------------------- //

asm_Serializer const asm_rawSerializer =
{
    .instruction = instruction,
    .label = label,
    .quad = quad,
    .zeroes = zeroes
};

// ---------------------------------------------------------------------------------------------- //
