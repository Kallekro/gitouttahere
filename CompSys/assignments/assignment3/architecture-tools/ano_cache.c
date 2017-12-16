//
// (C) Finn Schiermer Andersen, 2016
//
// ano_cache.c
//
// Simple timing  model for one level caching.
//

#include <stdlib.h>
#include <stdio.h>

#include "ano_cache.h"

/*
  cache layout: (1<<index_bits) of assoc of cache lines, 
  each line with a tag and (1<<(block_bits-3)) of time stamps.
*/

#define FAR_FUTURE ((unsigned long) -1)

struct line_t {
    unsigned long tag;
    unsigned long* timing;
};

struct cache_t {
    const char* name;
    int block_bits;
    int index_bits;
    int assoc;
    int hit_latency;
    int miss_latency;
    struct line_t** timing; // see above for layout
    unsigned long reads;
    unsigned long writes;
    unsigned long hits;
    unsigned long total_latency;
};

static void cache_config_error(const char* message) {
    printf("\n *** ERROR: %s *** \n", message);
    exit(-1);
}

cache_ptr create_cache(const char* name, int block_bits, int index_bits, int assoc,
		       int hit_latency, int miss_latency) {
    if (block_bits < 3)
	cache_config_error("Block size too small");
    if (block_bits > 8)
	cache_config_error("Block size too large");
    if (index_bits < 0)
	cache_config_error("Index size too small");
    if (index_bits > 16)
	cache_config_error("Index size too large");
    if (assoc < 1)
	cache_config_error("Associativity too small");
    if (assoc > 16)
	cache_config_error("Associativity too high");

    cache_ptr res = malloc(sizeof(struct cache_t));
    res->name = name;
    res->block_bits = block_bits;
    res->index_bits = index_bits;
    res->assoc = assoc;
    res->hit_latency = hit_latency;
    res->miss_latency = miss_latency;
    res->timing = malloc((1<<index_bits) * sizeof(struct line_t*));
    for (int index = 0; index < (1<<index_bits); ++index) {
	res->timing[index] = malloc(assoc * sizeof(struct line_t));
	for (int a = 0; a < assoc; ++a) {
	    res->timing[index][a].timing = malloc((1<<(block_bits-3)) * sizeof(unsigned long));
	    res->timing[index][a].tag = 0;
	    for (int quad = 0; quad < (1<<(block_bits-3)); ++quad) {
		res->timing[index][a].timing[quad] = FAR_FUTURE;
	    }
	}
    }
    res->reads = 0;
    res->writes = 0;
    res->hits = 0;
    res->total_latency = 0;
    return res;
}

void free_cache(cache_ptr cache) {
    for (int index = 0; index < (1<<cache->index_bits); ++index) {
	for (int a = 0; a < cache->assoc; ++a) {
	    free(cache->timing[index][a].timing);
	}
	free(cache->timing[index]);
    }
    free(cache->timing);
    free(cache);
}

void print_cache(cache_ptr cache) {
    printf("    Cache: %s\n", cache->name);
    printf("        Reads: %ld,    Writes %ld\n", cache->reads, cache->writes);
    unsigned long accesses = cache->reads + cache->writes;
    if (accesses) {
	printf("        Hits: %ld,    Hitrate: %f\n", cache->hits, cache->hits*1.0/accesses);
    }
    if (cache->reads) {
	printf("        Average latency: %f\n", cache->total_latency*1.0/cache->reads);
    }
}

// timing cache access:
static unsigned long low_bits(unsigned long val, int num_bits) {
    return val & ((1UL<<num_bits)-1);
}

static unsigned long time_read(cache_ptr cache, unsigned long earliest, unsigned long t) {
    if (t < earliest) t = earliest;
    t += cache->hit_latency;
    cache->total_latency += (t - earliest);
    return t;
}

static unsigned long* cache_op(cache_ptr cache, unsigned long earliest, unsigned long addr) {
    unsigned long quad = low_bits(addr, cache->block_bits) >> 3;
    unsigned long index = low_bits(addr >> cache->block_bits, cache->index_bits);
    unsigned long tag = addr >> (cache->index_bits + cache->block_bits);
    //    printf("::%ld  index:%ld quad:%ld tag:%ld\n", addr, index, quad, tag);
    struct line_t* line = cache->timing[index];
    // search for matching line
    for (int a=0; a<cache->assoc; ++a) {
	if (line[a].tag == tag) {
	    if (line[a].timing[quad] == FAR_FUTURE)
		break; // miss, handle below
	    // hit!
	    cache->hits++;
	    return &(line[a].timing[quad]);
	}
    }
    // no block found, add new in front and move all older ones down in MRU order:
    struct line_t oldest = cache->timing[index][cache->assoc-1];
    for (int a = cache->assoc-1; a > 0; --a) {
	cache->timing[index][a] = cache->timing[index][a-1];
    }
    cache->timing[index][0] = oldest;
    // initialize new block
    cache->timing[index][0].tag = tag;
    unsigned long data_ready = earliest + cache->miss_latency;
    for (int q = 0; q < (1<<(cache->block_bits-3)); ++q) {
	cache->timing[index][0].timing[q] = data_ready;
	++data_ready;
    }
    return &(cache->timing[index][0].timing[quad]);
}

unsigned long cache_read(cache_ptr cache, unsigned long earliest, unsigned long addr) {
    unsigned long* timing_info = cache_op(cache, earliest, addr);
    cache->reads++;
    return time_read(cache, earliest, *timing_info);
}

unsigned long cache_write(cache_ptr cache, unsigned long earliest, unsigned long addr) {
    unsigned long* timing_info = cache_op(cache, earliest, addr);
    // this corresponds to perfect store->load forwarding within the cache:
    *timing_info = earliest;
    cache->writes++;
    return earliest + cache->hit_latency;
}

