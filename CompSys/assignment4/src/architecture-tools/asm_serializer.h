//
// Nis Wegmann, 2017
//
// asm_serializer.h
//

#ifndef __ASM_SERIALIZER_H__
#define __ASM_SERIALIZER_H__

#include "asm_prefix.h"
#include "isa.h"

#include <errno.h>

// ---------------------------------------------------------------------------------------------- //

//
// .
//
typedef struct asm_Serializer
{
    //
    // Serializes an instruction.
    //
    int (* instruction)
        (
            FILE *,                             // Output file.
            isa_Instruction,                    // Instruction.
            char const *,                       // Label.
            size_t                              // Address.
        );
    
    //
    // Serializes a label.
    //
    int (* label)
        (
            FILE *,                             // Output file.
            char const *,                       // Instruction.
            size_t                              // Label.
        );
    
    //
    // Serializes a quad.
    //
    int (* quad)
        (
            FILE *,                             // Output file.
            isa_Quad,                           // Quad.
            size_t                              // Address.
        );
    
    //
    // Serializes a sequence of zeroes.
    //
    int (* zeroes)
        (
            FILE *,                             // Output file.
            size_t,                             // Count.
            size_t                              // Address.
        );
}
asm_Serializer;

// ---------------------------------------------------------------------------------------------- //

//
// .
//
extern asm_Serializer const asm_textualSerializer;

//
// .
//
extern asm_Serializer const asm_rawSerializer;

// ---------------------------------------------------------------------------------------------- //

#endif // __ASM_SERIALIZER_H__
