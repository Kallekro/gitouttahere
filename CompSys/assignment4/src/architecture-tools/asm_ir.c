//
// Nis Wegmann, 2017
//
// asm_ir.c
//

#include "asm_ir.h"

#include "asm_allocator.h"
#include "asm_serializer.h"
#include "asm_random.h"
#include "asm_std.h"
#include "asm_symtab.h"

// ---------------------------------------------------------------------------------------------- //

// - Internal Data types -

//
typedef enum EntryType
{
    EntryType_instruction,                      // Instruction entry.
    EntryType_label,                            // Label entry.
    EntryType_quad,                             // Quad entry.
    EntryType_zeroes,                           // Zeroes entry.
    EntryType_rand                              // Rand entry.
}
EntryType;

//
typedef struct Entry
{
    EntryType type;                             //
    
    union
    {
    
    isa_Instruction instruction;                // (nonnull)
    isa_Quad quad;                              //
    size_t zeroes;                              //
    
    struct
    {
        uint64_t seed;                          //
        size_t count;                           //
        size_t repeat;                          //
    }
    rand;
    
    };
    
    char const * label;                         // (nullable)
    isa_Long address;                           //
    struct Entry * next;                        // (nullable)
}
Entry;

// ---------------------------------------------------------------------------------------------- //

// - External Data types -

struct asm_IR
{
    Allocator * allocator;                      // Allocator. (nonnull)
    asm_Symtab * symtab;                        // Symbol table. (nonnull)
    Entry * head;                               // Entries head. (nullable)
    Entry * tail;                               // Entries tail. (nullable)
    isa_Long size;                              // Size of the IR.
    asm_IRErrorCallback * errorCallback;        // Error callback.
};

// ---------------------------------------------------------------------------------------------- //

// - Internal Functions -

static Entry * asm_Entry_createWithInstruction
    (Allocator * allocator, isa_Instruction instruction, char const * label, isa_Long address)
{
    assert(allocator != NULL);
    
    char * labelCopy = NULL;
    
    if (label != NULL)
    {
        labelCopy = Allocator_alloc(allocator, strlen(label) + 1, 1);
        strcpy(labelCopy, label);
    }
    
    Entry * entry = Allocator_alloc(allocator, sizeof(Entry), 1);
    
    entry->type = EntryType_instruction;
    entry->instruction = instruction;
    entry->label = labelCopy;
    entry->address = address;
    entry->next = NULL;
    
    return entry;
}

static Entry * asm_Entry_createWithLabel(Allocator * allocator, char const * label, isa_Long address)
{
    assert(allocator != NULL);
    assert(label != NULL);
    
    char * labelCopy = Allocator_alloc(allocator, strlen(label) + 1, 1);
    strcpy(labelCopy, label);
    
    Entry * entry = Allocator_alloc(allocator, sizeof(Entry), 1);
    
    entry->type = EntryType_label;
    entry->label = labelCopy;
    entry->address = address;
    entry->next = NULL;
    
    return entry;
}

static Entry * asm_Entry_createWithQuad(Allocator * allocator, isa_Quad quad, isa_Long address)
{
    assert(allocator != NULL);
    
    Entry * entry = Allocator_alloc(allocator, sizeof(Entry), 1);
    
    entry->type = EntryType_quad;
    entry->quad = quad;
    entry->label = NULL;
    entry->address = address;
    entry->next = NULL;
    
    return entry;
}

static Entry * asm_Entry_createWithZeroes(Allocator * allocator, size_t count, isa_Long address)
{
    assert(allocator != NULL);
    
    Entry * entry = Allocator_alloc(allocator, sizeof(Entry), 1);
    
    entry->type = EntryType_zeroes;
    entry->zeroes = count;
    entry->label = NULL;
    entry->address = address;
    entry->next = NULL;
    
    return entry;
}

static Entry * asm_Entry_createWithRand(Allocator * allocator, uint64_t seed, size_t count, size_t repeat, isa_Long address)
{
    assert(allocator != NULL);
    
    Entry * entry = Allocator_alloc(allocator, sizeof(Entry), 1);
    
    entry->type = EntryType_rand;
    entry->rand.seed = seed;
    entry->rand.count = count;
    entry->rand.repeat = repeat;
    entry->label = NULL;
    entry->address = address;
    entry->next = NULL;
    
    return entry;
}

static void asm_IR_addEntry(asm_IR * ir, Entry * entry)
{
    assert(ir != NULL);
    assert(entry != NULL);
    
    if (ir->head == NULL)
    {
        ir->head = entry;
    }
    
    if (ir->tail != NULL)
    {
        ir->tail->next = entry;
    }
    
    ir->tail = entry;
}

// ---------------------------------------------------------------------------------------------- //

// - External Functions -

extern asm_IR * asm_IR_create(Allocator * allocator, asm_IRErrorCallback * errorCallback)
{
    assert(allocator != NULL);
    assert(errorCallback != NULL);
    
    asm_IR * ir = Allocator_alloc(allocator, sizeof(asm_IR), 1);
    
    ir->allocator = allocator;
    ir->symtab = asm_Symtab_create(allocator);
    ir->head = NULL;
    ir->tail = NULL;
    ir->size = 0;
    ir->errorCallback = errorCallback;
    
    return ir;
}

extern void asm_IR_addInstruction(asm_IR * ir, isa_Instruction instruction, char const * label)
{
    assert(ir != NULL);
    
    // Create an instruction entry.
    
    Entry * entry = asm_Entry_createWithInstruction(ir->allocator, instruction, label, ir->size);
    
    // Add the entry to the IR.
    
    asm_IR_addEntry(ir, entry);
    
    //
    
    ir->size += isa_sizeofInstruction(instruction);
}

void asm_IR_addLabel(asm_IR * ir, char const * label)
{
    assert(ir != NULL);
    assert(label != NULL);
    
    // Check if the label already exists.
    
    if (asm_Symtab_getLabelAddress(ir->symtab, label) != asm_notAnAddress)
    {
        ir->errorCallback("Label defined multiple times.");
    }
    
    // Add the label to the symbol table.
    
    asm_Symtab_setLabelAddress(ir->symtab, label, ir->size);
    
    // Create a label entry.
    
    Entry * entry = asm_Entry_createWithLabel(ir->allocator, label, ir->size);
    
    // Add the entry to the IR.
    
    asm_IR_addEntry(ir, entry);
}

extern void asm_IR_setAlignment(asm_IR * ir, isa_Long alignment)
{
    assert(ir != NULL);
    
    // Check that 'alignment' is positive.
    
    if (isa_isSigned(alignment))
    {
        ir->errorCallback("Alignment must be positive.");
    }
    
    // Check that 'alignment' is a power of two.
    
    if ((alignment == 0) || ((alignment & (alignment - 1)) != 0))
    {
        ir->errorCallback("Alignment must be a power of two.");
    }
    
    // Compute padding.
    
    isa_Long padding = (alignment - (ir->size & (alignment - 1))) & (alignment - 1);
    
    // Insert zeroes.
    
    Entry * entry = asm_Entry_createWithZeroes(ir->allocator, padding, ir->size);
    
    asm_IR_addEntry(ir, entry);
    
    ir->size += padding;
}

extern void asm_IR_addQuad(asm_IR * ir, isa_Quad quad)
{
    assert(ir != NULL);
    
    Entry * entry = asm_Entry_createWithQuad(ir->allocator, quad, ir->size);
    
    asm_IR_addEntry(ir, entry);
    
    ir->size += 8;
}

extern void asm_IR_setPosition(asm_IR * ir, isa_Long position)
{
    assert(ir != NULL);
    assert(position >= ir->size);
    
    size_t count = position - ir->size;
    
    Entry * entry = asm_Entry_createWithZeroes(ir->allocator, count, ir->size);
    
    asm_IR_addEntry(ir, entry);
    
    ir->size = position;
}

extern void asm_IR_addRandom(asm_IR * ir, uint64_t seed, size_t count, size_t repeat)
{
    assert(ir != NULL);
    
    Entry * entry = asm_Entry_createWithRand(ir->allocator, seed, count, repeat, ir->size);
    
    asm_IR_addEntry(ir, entry);
    
    ir->size += 8;
}

extern void asm_IR_replaceReferenceAddresses(asm_IR * ir)
{
    assert(ir != NULL);
    
    // Iterate through the entries.
    
    for (Entry * entry = ir->head; entry != NULL; entry = entry->next)
    {
        // Replace addresses for label references.
        
        if (entry->type == EntryType_instruction && entry->label != NULL)
        {
            isa_Long const address = asm_Symtab_getLabelAddress(ir->symtab, entry->label);
            
            if (address == asm_notAnAddress)
            {
                fprintf(stderr, "undefined label: %s\n", entry->label);
                
                exit(EXIT_FAILURE);
            }
            
            switch (entry->instruction.opcode)
            {
                case isa_Opcode_movq_ir:    entry->instruction.ir.src = address; continue;
                case isa_Opcode_movq_mr:    entry->instruction.mr.src = address; continue;
                case isa_Opcode_movq_rm:    entry->instruction.rm.dst = address; continue;
                case isa_Opcode_jmp:
                case isa_Opcode_jle:
                case isa_Opcode_jl:
                case isa_Opcode_je:
                case isa_Opcode_jne:
                case isa_Opcode_jge:
                case isa_Opcode_jg:         entry->instruction.i = address; continue;
                
                case isa_Opcode_call:       entry->instruction.i = address; continue;
                
                default: abort();
            }
        }
    }
}

extern void asm_IR_serialize(asm_IR const * ir, asm_Serializer const * serializer, FILE * file)
{
    assert(ir != NULL);
    assert(file != NULL);
    
    // Iterate through the entries.
    
    for (Entry * entry = ir->head; entry != NULL; entry = entry->next)
    {
        // Serialize the entry.
        
        switch (entry->type)
        {
            case EntryType_instruction:
                
                serializer->instruction(file, entry->instruction, entry->label, entry->address);
                
                continue;
                
            case EntryType_label:
                
                serializer->label(file, entry->label, entry->address);
                
                continue;
                
            case EntryType_quad:
                
                serializer->quad(file, entry->quad, entry->address);
                
                continue;
                
            case EntryType_zeroes:
                
                serializer->zeroes(file, entry->zeroes, entry->address);
                
                continue;
                
            case EntryType_rand:
            {
                uint64_t * buffer = malloc(sizeof(uint64_t) * entry->rand.count);
                
                if (buffer == NULL)
                {
                    fprintf(stderr, "Out of memory!\n");
                    
                    exit(EXIT_FAILURE);
                }
                
                isa_Long address = entry->address;
                
                for (size_t rep = 0; rep < entry->rand.repeat; rep++)
                {
                    asm_random(entry->rand.seed, buffer, entry->rand.count);
                    
                    for (size_t index = 0; index < entry->rand.count; index++)
                    {
                        serializer->quad(file, buffer[index], address);
                        
                        address += 8;
                    }
                }
                
                free(buffer);
                
                continue;
            }
        }
    }
}

// ---------------------------------------------------------------------------------------------- //
