//
// Nis Wegmann, 2017
//
// asm_allocator.c
//

#include "asm_allocator.h"

#include <assert.h>
#include <stdalign.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdnoreturn.h>

// ---------------------------------------------------------------------------------------------- //

#ifdef ALLOCATOR_DEBUG
#  include <stdio.h>
#endif

// ---------------------------------------------------------------------------------------------- //

//
// .
//
typedef struct Page
{
    struct Page * next;                         // The next allocated page.
    ALLOCATOR_SIZE_T used;                      // Amount of bytes allocated in the page.
    alignas(16) unsigned char memory [];        // Memory block.
}
Page;

// ---------------------------------------------------------------------------------------------- //

//
// .
//
struct Allocator
{
    Page * head;                                // Head page.
    Page * tail;                                // Tail page.
    ALLOCATOR_SIZE_T pageSize;                  // Capacity of each page.
    Allocator_ErrorCallback * errorCallback;    // Error callback.
    void * errorCallbackContext;                // Error callback context.
};

// ---------------------------------------------------------------------------------------------- //

//
// .
//
static noreturn inline void fail(Allocator_ErrorCallback * errorCallback, void * context, AllocatorErrorCode code)
{
    if (errorCallback != NULL)
    {
        errorCallback(context, code);
    }
    
    abort();
}

// ---------------------------------------------------------------------------------------------- //

//
// Creates a new page.
//
static inline Page * Page_create (Allocator * allocator)
{
    // Allocate the page.
    
    Page * page = malloc(sizeof(Page) + allocator->pageSize);
    
    // Check that the page was allocated.
    
    if (page == NULL)
    {
        fail(allocator->errorCallback, allocator->errorCallbackContext, AllocatorErrorCode_outOfMemory);
    }
    
  #ifdef ALLOCATOR_DEBUG
    fprintf(stdout, "Allocator: Created a page.\n");
  #endif
    
    // Initialize the page.
    
    page->next = NULL;
    page->used = 0;
    
    // Return the page.
    
    return page;
}

// ---------------------------------------------------------------------------------------------- //

//
//
//
static inline ALLOCATOR_SIZE_T padding(unsigned char * ptr, ALLOCATOR_SIZE_T alignment)
{
    return (ALLOCATOR_SIZE_T) ((alignment - (((uintptr_t) ptr) & (alignment - 1))) & (alignment - 1));
}

// ---------------------------------------------------------------------------------------------- //

char const * AllocatorErrorCode_asString(AllocatorErrorCode code)
{
    switch (code)
    {
        case AllocatorErrorCode_outOfMemory:        return "Out of memory.";
        case AllocatorErrorCode_sizeIsTooBig:       return "Size is too big.";
        case AllocatorErrorCode_overflow:           return "Overflow.";
        case AllocatorErrorCode_invalidAlignment:   return "Invalid alignment.";
        default:                                    return "Unknown error code";
    }
}

// ---------------------------------------------------------------------------------------------- //

extern Allocator * Allocator_create (ALLOCATOR_SIZE_T pageSize, Allocator_ErrorCallback * errorCallback, void * errorCallbackContext)
{
    // Allocate the allocator.
    
    Allocator * allocator = malloc(sizeof(Allocator));
    
    // Check that the allocator was allocated.
    
    if (allocator == NULL)
    {
        fail(errorCallback, errorCallbackContext, AllocatorErrorCode_outOfMemory);
    }
    
  #ifdef ALLOCATOR_DEBUG
    fprintf(stdout, "Allocator: Created an allocator.\n");
  #endif
    
    // Initialize the allocator.
    
    allocator->pageSize = pageSize;
    allocator->head = NULL;
    allocator->tail = NULL;
    allocator->errorCallback = errorCallback;
    allocator->errorCallbackContext = errorCallbackContext;
    
    // Return the allocator.
    
    return allocator;
}

// ---------------------------------------------------------------------------------------------- //

extern void * Allocator_alloc (Allocator * allocator, ALLOCATOR_SIZE_T blockSize, ALLOCATOR_SIZE_T alignment)
{
    assert(allocator != NULL);
    
    // Verify that alignment is a power of two.
    
    if ((alignment == 0) || ((alignment & (alignment - 1)) != 0))
    {
        fail(allocator->errorCallback, allocator->errorCallbackContext, AllocatorErrorCode_invalidAlignment);
    }
    
    // Verify that 'blockSize + alignment - 1' does not overflow.
    
    if (blockSize > ((ALLOCATOR_SIZE_MAX - alignment) + 1))
    {
        fail(allocator->errorCallback, allocator->errorCallbackContext, AllocatorErrorCode_overflow);
    }
    
    // Make sure that the requested amount of bytes fits within a page.
    
    if ((blockSize + (alignment - 1)) > allocator->pageSize)
    {
        // If not, call the allocation failure callback (if set).
        
        fail(allocator->errorCallback, allocator->errorCallbackContext, AllocatorErrorCode_sizeIsTooBig);
    }
    
    // If 'tail' is not set then create a new page.
    
    if (allocator->tail == NULL)
    {
        allocator->tail = Page_create(allocator);
        
        // If 'head' is not set head to the new page.
        
        if (allocator->head == NULL)
        {
            allocator->head = allocator->tail;
        }
    }
    
    // Compute alignment offset.
    
    ALLOCATOR_SIZE_T const offset = padding(allocator->tail->memory + allocator->tail->used, alignment);
    
    // If the page is full then create a new page.
    
    if ((blockSize + offset) > (allocator->pageSize - allocator->tail->used))
    {
        if (allocator->tail->next == NULL)
        {
            allocator->tail->next = Page_create(allocator);
        }
        else
        {
            allocator->tail->next->used = 0;
        }
        
        allocator->tail = allocator->tail->next;
    }
    
    // Allocate the requested amount of bytes.
    
    void * block = allocator->tail->memory + allocator->tail->used + offset;
    
    allocator->tail->used += blockSize + offset;
    
    // Return a pointer to the memory block.
    
    return block;
}

// ---------------------------------------------------------------------------------------------- //

extern void Allocator_reset (Allocator * allocator)
{
    assert(allocator != NULL);
    
    allocator->tail = allocator->head;
    
    allocator->head->used = 0;
}

// ---------------------------------------------------------------------------------------------- //

//
// Destroys an allocator.
//
extern void Allocator_destroy (Allocator * allocator)
{
    assert(allocator != NULL);
    
    // Release the pages.
    
    Page * page = allocator->head;
    
    while (page != NULL)
    {
        Page * next = page->next;
        
        free(page);
        
      #ifdef ALLOCATOR_DEBUG
        fprintf(stdout, "Allocator: Destroyed a page.\n");
      #endif
        
        page = next;
    }
    
    // Release the allocator.
    
    free(allocator);
    
  #ifdef ALLOCATOR_DEBUG
    fprintf(stdout, "Allocator: Destroyed an allocator.\n");
  #endif
}

// ---------------------------------------------------------------------------------------------- //
