//
// Nis Wegmann, 2017
//
// isa_condition.h
//

#ifndef __ISA_CONDITION_H__
#define __ISA_CONDITION_H__

#include "isa_flags.h"

#include <stdbool.h>
#include <stdlib.h>

// ---------------------------------------------------------------------------------------------- //

//
// Condition type.
//
typedef enum isa_Condition
{
    isa_Condition_e,                            // Equal.
    isa_Condition_ne,                           // Not equal.
    isa_Condition_s,                            // Negative.
    isa_Condition_ns,                           // Nonnegative.
    isa_Condition_l,                            // Less. (signed <)
    isa_Condition_le,                           // Less or equal. (signed <=)
    isa_Condition_g,                            // Greater. (signed >)
    isa_Condition_ge,                           // Greater or equal. (signed >=)
    isa_Condition_a,                            // Above. (unsigned >)
    isa_Condition_ae,                           // Above or equal. (unsigned >=)
    isa_Condition_b,                            // Below. (unsigned <)
    isa_Condition_be                            // Below or equal. (unsigned <=)
}
isa_Condition;

// ---------------------------------------------------------------------------------------------- //

//
// Checks if a condition holds given a set of flags.
//
static inline bool isa_checkCondition(isa_Condition condition, isa_Flags flags)
{
    bool const cf = flags.cf; // Carry flag.
    bool const zf = flags.zf; // Zero flag.
    bool const sf = flags.sf; // Sign flag.
    bool const of = flags.of; // Overflow flag.
    
    switch (condition)
    {
        case isa_Condition_e:   return zf;
        case isa_Condition_ne:  return zf ^ 1;
        case isa_Condition_s:   return sf;
        case isa_Condition_ns:  return sf ^ 1;
        case isa_Condition_g:   return (sf ^ of ^ 1) & (zf ^ 1);        
        case isa_Condition_ge:  return sf ^ of ^ 1;
        case isa_Condition_l:   return sf ^ of;
        case isa_Condition_le:  return (sf ^ of) | zf;
        case isa_Condition_a:   return (cf ^ 1) & (zf ^ 1);
        case isa_Condition_ae:  return cf ^ 1;
        case isa_Condition_b:   return cf;
        case isa_Condition_be:  return cf | zf;
    }
    
    abort();
}

// ---------------------------------------------------------------------------------------------- //

#endif // __ISA_CONDITION_H__
