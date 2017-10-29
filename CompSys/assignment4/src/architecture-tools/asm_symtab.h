//
// Nis Wegmann, 2017
//
// asm_symtab.h
//

#ifndef __ASM_SYMTAB_H__
#define __ASM_SYMTAB_H__

#include "asm_prefix.h"
#include "isa.h"

typedef struct Allocator Allocator;

// ---------------------------------------------------------------------------------------------- //

//
// Opaque symbol table type.
//
typedef struct asm_Symtab asm_Symtab;

// ---------------------------------------------------------------------------------------------- //

//
// Creates a new symbol table.
//
asm_Symtab * asm_Symtab_create
    (
        Allocator *                             // Allocator.
    );

// ---------------------------------------------------------------------------------------------- //

//
// Sets the address of a label.
//
void asm_Symtab_setLabelAddress
    (
        asm_Symtab *,                           // The symbol table. (nonnull)
        char const *,                           // The label. (nonnull)
        isa_Long                                // The address.
    );

// ---------------------------------------------------------------------------------------------- //

//
// .
//
static const isa_Long asm_notAnAddress = UINT32_MAX;

//
// Returns the address of a label, or 'asm_notAnAddress' if the label is not found.
//
isa_Long asm_Symtab_getLabelAddress
    (
        asm_Symtab const *,                     // The symbol table. (nonnull)
        char const *                            // The label. (nonnull)
    );

// ---------------------------------------------------------------------------------------------- //

//
// Prints the contents of a symbol table to 'stdout'. (For debugging.)
//
void asm_Symtab_print
    (
        asm_Symtab const *                      // The symbol table. (nonnull)
    );

// ---------------------------------------------------------------------------------------------- //

#endif // __ASM_SYMTAB_H__
