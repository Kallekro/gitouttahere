//
// (C) Finn Schiermer Andersen, 2016
//
// ano_cache.h
//
// Simple timing  model for one level caching.
//

#ifndef __CACHE_H__
#define __CACHE_H__

struct cache_t;
typedef struct cache_t* cache_ptr;

// Create a cache from given configuration
cache_ptr create_cache(const char* name,  // name of the cache, used for pretty printing
		       int block_bits,    // number of bits used to address inside a cache block
		       int index_bits,    // number of bits used to select a cache set (index)
		       int assoc,         // associativity (number of cache blocks pr set)
		       int hit_latency,   // latency of a read from the cache
		       int miss_latency); // latency of a miss from the cache

void free_cache(cache_ptr cache);
void print_cache(cache_ptr cache);

/* timing cache access:
   - Given the time when the access is initiated, return the time in which the requested
     data is actually available (cache_read). For cache_writes, just return the time of
     initiation plus the configured latency.
   - As a side effect, modify the cache LRU order
   - As a side effect, trigger loading of all the data in the referenced cache block.
   - When writing data to the cache, note the timing such that a later read from
     the cache that references written data will return the timing of the write
     plus the hit latency. (sometimes referred to as store-to-load forwarding).
   - no modelling of write data queueing.
   - no modelling of bandwidth limitations: you can do any number of accesses at the same time,
     if you want model these (or other) limitations, use resources (see resource.h)
*/
unsigned long cache_read(cache_ptr cache, unsigned long earliest, unsigned long addr);
unsigned long cache_write(cache_ptr cache, unsigned long earliest, unsigned long addr);

#endif // __CACHE_H__
