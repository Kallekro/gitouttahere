//
// Nis Wegmann, 2017
//
// asm_random.h
//

#ifndef __ASM_RANDOM_H__
#define __ASM_RANDOM_H__

#include <stddef.h>
#include <stdint.h>

// ---------------------------------------------------------------------------------------------- //

//
// Fills a buffer with random numbers.
//
void asm_random
    (
        uint64_t,                               // Seed.
        uint64_t *,                             // Output buffer. (nonnull)
        size_t                                  // Number of elements in output buffer.
    );

// ---------------------------------------------------------------------------------------------- //

#endif // __ASM_RANDOM_H__
