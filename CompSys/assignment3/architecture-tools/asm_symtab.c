//
// Nis Wegmann, 2017
//
// asm_symtab.c
//

#include "asm_symtab.h"

#include "asm_allocator.h"
#include "asm_std.h"

#include <inttypes.h>

// ---------------------------------------------------------------------------------------------- //

typedef struct asm_Symbol
{
    char const * label;                         // Label. (nonnull)
    isa_Long address;                           // Address.
    struct asm_Symbol * prev;                   // Previous symbol. (nullable)
}
asm_Symbol;

// ---------------------------------------------------------------------------------------------- //

struct asm_Symtab
{
    Allocator * allocator;                      // Allocator. (nonnull)
    asm_Symbol * head;                          // Head of symbol list. (nullable)
};

// ---------------------------------------------------------------------------------------------- //

extern asm_Symtab * asm_Symtab_create(Allocator * allocator)
{
    assert(allocator != NULL);
    
    asm_Symtab * symtab = Allocator_alloc(allocator, sizeof(asm_Symtab), 1);
    
    symtab->allocator = allocator;
    symtab->head = NULL;
    
    return symtab;
}

// ---------------------------------------------------------------------------------------------- //

extern void asm_Symtab_setLabelAddress(asm_Symtab * symtab, char const * label, isa_Long address)
{    
    assert(label != NULL);
    assert(symtab != NULL);
    
    // Copy the label.
    
    char * labelCopy = Allocator_alloc(symtab->allocator, strlen(label) + 1, 1);
    
    strcpy(labelCopy, label);
    
    // Create a new symbol.
    
    asm_Symbol * symbol = Allocator_alloc(symtab->allocator, sizeof(asm_Symbol), 1);
    
    symbol->label = labelCopy;
    symbol->address = address;
    symbol->prev = symtab->head;
    
    // Attach the symbol to the head of the symbol table.
    
    symtab->head = symbol;
}

// ---------------------------------------------------------------------------------------------- //

extern isa_Long asm_Symtab_getLabelAddress(asm_Symtab const * symtab, char const * label)
{
    assert(label != NULL);
    assert(symtab != NULL);
    
    // Iterate through the symbols.
    
    for (asm_Symbol const * symbol = symtab->head; symbol != NULL; symbol = symbol->prev)
    {
        // If the enquired label matches the label associated with the current symbol, return it's
        // address.
        
        if (strcmp(label, symbol->label) == 0)
        {
            return symbol->address;
        }
    }
    
    // The label was not found.
    
    return asm_notAnAddress;
}

// ---------------------------------------------------------------------------------------------- //

extern void asm_Symtab_print(asm_Symtab const * symtab)
{
    assert(symtab != NULL);
    
    // Iterate through the symbols.
    
    for (asm_Symbol const * symbol = symtab->head; symbol != NULL; symbol = symbol->prev)
    {
        // Print each symbol.
        
        printf("%s : %" PRIX32 "\n", symbol->label, symbol->address);
    }
}

// ---------------------------------------------------------------------------------------------- //
