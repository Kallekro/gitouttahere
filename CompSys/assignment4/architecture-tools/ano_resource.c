//
// (C) Finn Schiermer Andersen, 2016
//
// ano_resource.c
//
// Generic resource timing model.
//

#include <stdlib.h>
#include <stdio.h>

#include "ano_resource.h"

typedef enum { ordered, unordered } ordering_t;

struct resource_t {
    const char* name;
    ordering_t ordering;
    int max_timespan;
    int num_resource;
    unsigned long window_start;
    int* num_free;
    unsigned long collision_count;
    unsigned long total_use;
    unsigned long total_allocations;
};

static resource_ptr create_resource(const char* name, int num_resource, int max_timespan) {
    resource_ptr res = malloc(sizeof(struct resource_t));
    res->name = name;
    res->max_timespan = max_timespan;
    res->num_resource = num_resource;
    res->num_free = malloc(max_timespan * sizeof(int));
    for (int i=0; i<max_timespan; ++i)
	res->num_free[i] = num_resource;
    res->window_start = 0;
    res->collision_count = 0;
    res->total_use = 0;
    res->total_allocations = 0;
    return res;
}

resource_ptr create_inorder_resource(const char* name, int num_resource, int max_timespan) {
    resource_ptr res = create_resource(name, num_resource, max_timespan);
    res->ordering = ordered;
    return res;
}

resource_ptr create_unordered_resource(const char* name, int num_resource, int max_timespan) {
    resource_ptr res = create_resource(name, num_resource, max_timespan);
    res->ordering = unordered;
    return res;
}

void print_resource(resource_ptr resource) {
    if (resource->total_use) {
	printf("    Resource: %s: use %ld   avg %f\n",
	       resource->name, resource->total_use,
	       resource->total_allocations*1.0/resource->total_use);
    } else {
	printf("    Resource: %s: unused\n", resource->name);
    }
}

void free_resource(resource_ptr resource) {
    free(resource->num_free);
    free(resource);
}

// move resource usage window one cycle
static void move_window(resource_ptr resource) {
    int t = resource->window_start % resource->max_timespan;
    resource->num_free[t] = resource->num_resource;
    resource->window_start++;
}

unsigned long resource_acquire(resource_ptr resource, unsigned long earliest) {
    // allocations cannot take place before start of window
    if (earliest < resource->window_start) earliest = resource->window_start;

    // inorder allocations cannot move backwards in time
    if (resource->ordering == ordered) {
	while (resource->window_start < earliest)
	    move_window(resource);
    }

    // try to make the earliest possible allocation, potentially moving the window if needed
    for (;;) {
	while (earliest >= resource->window_start + resource->max_timespan)
	    move_window(resource);
	int index = earliest % resource->max_timespan;
	if (resource->num_free[index]) {
	    return earliest;
	}
	++earliest;
    }
}

void resource_use(resource_ptr resource, unsigned long start, unsigned long finished) {
    // ensure that finished is inside the window
    while (finished >= resource->window_start + resource->max_timespan)
	move_window(resource);

    // if this is an ordered resource, prevent later requests from being granted earlier
    if (resource->ordering == ordered) {
	while (resource->window_start < start)
	    move_window(resource);
    }

    // we assume that the allocation fits within the window, if not, larger window is needed
    if (start < resource->window_start) {
	printf("Error: too large resource allocation (%ld to %ld) from %s\n",
	       start, finished, resource->name);
	exit(-1);
    }

    for (unsigned long t = start; t < finished; ++t) {
	int index = t % resource->max_timespan;
	if (resource->num_free[index])
	    --resource->num_free[index];
	else
	    resource->collision_count++;
    }

    resource->total_allocations += finished - start;
    resource->total_use++;
}


void resource_use_all(resource_ptr resource, unsigned long finished) {
    // move window until finished is the earliest possible allocation
    while (resource->window_start < finished)
	move_window(resource);
}
