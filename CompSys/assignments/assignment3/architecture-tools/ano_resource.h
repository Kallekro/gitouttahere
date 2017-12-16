//
// (C) Finn Schiermer Andersen, 2016
//
// ano_resource.h
//
// Resource timing model.
//

#ifndef __RESOURCE_H__
#define __RESOURCE_H__

struct resource_t;
typedef struct resource_t* resource_ptr;

/*
  Create a resource.
  Arguments:
  - name: name of the resource. Used when printing status for it.
  - num_resource: the number of resources available for allocation/use
  - max_timespan: the number of cycles that must be tracked for allocations.
  You should pick a timespan large enough for the longest possible duration of
  use of the resource. And then some.

  There are "inorder" resources and "unordered" resources.

  An "inorder" resource is a resource which can only be allocated in program order.
  An "unordered" resource can be allocated out of program order
*/

resource_ptr create_inorder_resource(const char* name, int num_resource, int max_timespan);
resource_ptr create_unordered_resource(const char* name, int num_resource, int max_timespan);

// print usage statistics
void print_resource(resource_ptr resource);

// be done with it!
void free_resource(resource_ptr resource);

/*
  Return the earliest cycle you can possibly acquire the resource.
  Arguments:
  - resource: well.. the resource
  - earliest: the earliest cycle of interest.

  Note: Acquire doesn't actually mark the resource as acquired. It just gives you the
  time of (potential) acquisition. To actually acquire the resource, call resource_use.
*/
unsigned long resource_acquire(resource_ptr resource, unsigned long earliest);

/*
  Acquire a resource for a specific time span.
  Arguments:
  - resource: well .. still, the resource
  - start: first cycle in which you hold the resource
  - finished: first cycle in which you no longer hold the resource
*/
void resource_use(resource_ptr resource, unsigned long start, unsigned long finished);

/*
  Use up all instances of a resource up to a point in time.
  Arguments:
  - resource: ...
  - finished: first cycle in which you no longer hold the resource
  This forcess all later allocations to happen in cycle 'finished' or later
*/
void resource_use_all(resource_ptr resource, unsigned long finished);

#endif // __RESOURCE_H__
