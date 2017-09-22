//
// trace_read.h
//

#ifndef __TRACE_READ_H__
#define __TRACE_READ_H__

//
// Trace entry type.
//
typedef enum Trace_Type
{
    Trace_Type_reg = 0,                     // A register-type entry.
    Trace_Type_mem = 1,                     // A memory-type entry.
    Trace_Type_pc  = 2                      // An instruction-pointer-type entry.
}
Trace_Type;

// ---------------------------------------------------------------------------------------------- //

//
// An entry from a trace file.
//
typedef struct Trace_Entry
{
    uint64_t counter;                       // Instruction #.
    uint64_t type;                          // Type of the entry.
    uint64_t destination;                   // Register or address in memory.
    uint64_t value;                         // Value written into register or memory.
}
Trace_Entry;

// ---------------------------------------------------------------------------------------------- //

//
// Reads the next 'count' entries from a trace-file. Returns the number of entries that where
// successfully read.
//
static inline size_t Trace_read(FILE * file, Trace_Entry entries [], size_t count)
{
    for (size_t index = 0; index < count; index++)
    {
        if (fscanf(file, "%" PRIX64 " %" PRIX64 " %" PRIX64 " %" PRIX64 "\n",
               &(entries[index].counter),
               &(entries[index].type),
               &(entries[index].destination),
               &(entries[index].value)
        ) != 4)
        {
            return index;
        }
    }
    
    return count;
}

#endif // __TRACE_READ_H__
