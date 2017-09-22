//
// Nis Wegmann, 2017
//
// asm_random.c
//
// Random number generator. Uses xoroshiro128+ and SplitMix64 from 'http://xoroshiro.di.unimi.it'.
//

#include "asm_random.h"

#include <stdint.h>

// ---------------------------------------------------------------------------------------------- //

static inline uint64_t SplitMix64(uint64_t x)
{
    uint64_t z;
    
    z = (x += UINT64_C(0x9E3779B97F4A7C15));
    z = (z ^ (z >> 30)) * UINT64_C(0xBF58476D1CE4E5B9);
    z = (z ^ (z >> 27)) * UINT64_C(0x94D049BB133111EB);
    
    return z ^ (z >> 31);
}

static inline uint64_t rotl(const uint64_t x, int k)
{
    return (x << k) | (x >> (64 - k));
}

// ---------------------------------------------------------------------------------------------- //

extern void asm_random(uint64_t seed, uint64_t * buffer, size_t count)
{
    // State variables.
    
    uint64_t s0 = seed, s1 = SplitMix64(seed);
    
    // Loop through indexes in 'buffer'.
    
    for (size_t index = 0; index < count; index++)
    {
        // Write random to 'buffer'.
        
        buffer[index] = s0 + s1;
        
        // Update state. (xoroshiro128)
        
        s1 = s1 ^ s0;
        s0 = rotl(s0, 55) ^ s1 ^ (s1 << 14);
        s1 = rotl(s1, 36);
    }
}

// ---------------------------------------------------------------------------------------------- //
