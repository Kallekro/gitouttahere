//
// Nis Wegmann, 2017
//
// asm_allocator.h
//

#ifndef __ALLOCATOR_H__
#define __ALLOCATOR_H__

// ---------------------------------------------------------------------------------------------- //

#include <stddef.h>
#include <stdint.h>

#define ALLOCATOR_SIZE_T size_t
#define ALLOCATOR_SIZE_MAX SIZE_MAX

//#define ALLOCATOR_DEBUG

// ---------------------------------------------------------------------------------------------- //

//
// Allocator error code.
//
typedef enum AllocatorErrorCode
{
    AllocatorErrorCode_outOfMemory       = -1,  // .
    AllocatorErrorCode_invalidAlignment  = -2,  // .
    AllocatorErrorCode_sizeIsTooBig      = -3,  // .
    AllocatorErrorCode_overflow          = -4,  // .
}
AllocatorErrorCode;

// ---------------------------------------------------------------------------------------------- //

//
// .
//
char const * AllocatorErrorCode_asString
    (
        AllocatorErrorCode                      // Error code.
    );

// ---------------------------------------------------------------------------------------------- //

//
// Allocator error callback.
//
typedef void Allocator_ErrorCallback
    (
        void *,                                 // Context. (nullable)
        AllocatorErrorCode                      // Error code.
    );

// ---------------------------------------------------------------------------------------------- //

//
// Opaque allocator type.
//
typedef struct Allocator Allocator;

// ---------------------------------------------------------------------------------------------- //

//
// Creates a new allocator.
//
Allocator * Allocator_create
    (
        ALLOCATOR_SIZE_T,                       // Capacity per page.
        Allocator_ErrorCallback,                // Error callback. (nullable)
        void *                                  // Error callback context. (nullable)
    );

// ---------------------------------------------------------------------------------------------- //

//
// Allocates a memory block.
//
void * Allocator_alloc
    (
        Allocator *,                            // Allocator to allocate with. (nonnull)
        ALLOCATOR_SIZE_T,                       // Size of block to allocate.
        ALLOCATOR_SIZE_T                        // Alignment of block to allocate.
    );

// ---------------------------------------------------------------------------------------------- //

//
// Resets an allocator.
//
void Allocator_reset
    (
        Allocator *                             // Allocator to reset. (nonnull)
    );

// ---------------------------------------------------------------------------------------------- //

//
// Destroys an allocator.
//
void Allocator_destroy
    (
        Allocator *                             // Allocator to destroy. (nonnull)
    );

// ---------------------------------------------------------------------------------------------- //

#endif // __ALLOCATOR_H__
