//
// Nis Wegmann, 2017
//
// asm_error.h
//

#ifndef __ASM_ERROR_H__
#define __ASM_ERROR_H__

#include "asm_prefix.h"

// ---------------------------------------------------------------------------------------------- //

//
// .
//
void asm_error
    (
        char const *,                           // Filename.
        int,                                    // Line number.
        int,                                    // Column number.
        char const *,                           // Formatted message.
        ...                                     // Extra items.
    );

// ---------------------------------------------------------------------------------------------- //

#endif // __ASM_ERROR_H__
