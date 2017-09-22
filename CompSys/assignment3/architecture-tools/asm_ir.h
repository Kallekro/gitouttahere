//
// Nis Wegmann, 2017
//
// asm_ir.h
//

#ifndef __ASM_IR_H__
#define __ASM_IR_H__

#include "asm_prefix.h"

#include "isa.h"

// ---------------------------------------------------------------------------------------------- //

// - Forward Declarations -

typedef struct Allocator Allocator;

typedef struct asm_Serializer asm_Serializer;

// ---------------------------------------------------------------------------------------------- //

typedef void asm_IRErrorCallback
    (
        char const *                            // Error message.
    );

// ---------------------------------------------------------------------------------------------- //

//
// Opaque IR type.
//
typedef struct asm_IR asm_IR;

// ---------------------------------------------------------------------------------------------- //

//
// Creates a new IR.
// 
// Returns 'NULL' if the allocator fails.
//
asm_IR * asm_IR_create
    (
        Allocator *,                            // Allocator. (nonnull)
        asm_IRErrorCallback *                   // Error callback. (nonnull)
    );

// ---------------------------------------------------------------------------------------------- //

//
// .
//
void asm_IR_addInstruction
    (
        asm_IR *,                               // IR. (nonnull)
        isa_Instruction,                        // Instruction to add.
        char const *                            // Optional label. (nullable)
    );

// ---------------------------------------------------------------------------------------------- //

//
// .
//
void asm_IR_addLabel
    (
        asm_IR *,                               // IR. (nonnull)
        char const *                            // Label. (nonnull)
    );

// ---------------------------------------------------------------------------------------------- //

//
// .
//
void asm_IR_addQuad
    (
        asm_IR *,                               // IR. (nonnull)
        isa_Quad                                // Quad.
    );

// ---------------------------------------------------------------------------------------------- //

//
// Advances the current encoding position for a IR.
// 
// Returns 'asm_IRError_invalidPosition' if the new position is less than the current position.
//
void asm_IR_setPosition
    (
        asm_IR *,                               // IR. (nonnull)
        isa_Long                                // Position.
    );

// ---------------------------------------------------------------------------------------------- //

//
// .
//
void asm_IR_setAlignment
    (
        asm_IR *,                               // IR. (nonnull)
        isa_Long                                // Alignment.
    );

// ---------------------------------------------------------------------------------------------- //

//
// .
//
void asm_IR_addRandom
    (
        asm_IR *,                               // IR. (nonnull)
        uint64_t,                               // Seed.
        size_t,                                 // Count.
        size_t                                  // Repeat.
    );

// ---------------------------------------------------------------------------------------------- //

//
// .
//
void asm_IR_replaceReferenceAddresses
    (
        asm_IR *                                // IR. (nonnull)
    );

// ---------------------------------------------------------------------------------------------- //

//
// Serializes a IR to an output file.
//
// Returns 'asm_IRError_unknownLabel' if an unknown label (i.e. a label that is not present in the
// symbol table) is encountered.
//
void asm_IR_serialize
    (
        asm_IR const *,                         // IR. (nonnull)
        asm_Serializer const *,                 // Serializer. (nonnull)
        FILE *                                  // Output file. (nonnull)
    );

// ---------------------------------------------------------------------------------------------- //

#endif // __ASM_IR_H__
