/*
 * STREAM TRANSDUCERS
 *
 * A stream transducer takes one or more streams as inputs, and
 * produces zero or more streams as output.  For simplicity, the only
 * transducer that produces more than one stream is built-in.  all
 * user-defined transducers produce just one stream.
 *
 * We can construct a transducer tree, which consists of a number of
 * _sources_ that produce streams, which are then modified by various
 * _transducers_, and finally connected to a single _sink_.
 *
 * Sources, sinks, and transducers are produced by providing a
 * source-, sink-, and transduction-function, respectively.  Whenever
 * we create a transducer or a source, a new stream is returned on
 * which we can read the result.  This stream can then be passed to
 * another transducer, or to a sink.
 *
 * Each stream may be used as input to only one transducer, but a
 * special built-in transducer exists that duplicates a stream.
 *
 */

#ifndef PIPELINE_H
#define PIPELINE_H

#include <stdio.h>

/* We treat streams as an opaque type named 'stream'.  All interface
   functions will return and accept pointers to 'stream'. */
typedef struct stream stream;

/* Steams must eventually be freed.  This function may only be called
   once the network sink has stopped. */
void transducers_free_stream(stream *s);

/* A source function produces a stream. */
typedef void (*transducers_source)(const void *arg, FILE *out);

/* A sink function consumes a stream and produces nothing. */
typedef void (*transducers_sink)(void *arg, FILE *in);

/* A transducer that takes one stream as input.  */
typedef void (*transducers_1)(const void *arg, FILE *out, FILE *in);

/* A transducer that takes two streams as input.  */
typedef void (*transducers_2)(const void *arg, FILE *out, FILE *in1, FILE *in2);

/* Create a source and a stream for its output. */
int transducers_link_source(stream **out,
                            transducers_source s, const void *arg);

/* Create a sink that reads input from the given stream.  This
   function blocks until the sink finishes.  The sink function is
   passed the 'arg' value.

   Important: The sink _must_ be run in the parent process, such that
   changes to the value pointed to by 'arg' value are visible
   afterwards. */
int transducers_link_sink(transducers_sink s, void *arg,
                          stream *in);

/* Create a transducer that reads input from a stream and writes
   output to a new stream.  The transduction function is passed the
   'arg' value. */
int transducers_link_1(stream **out,
                       transducers_1 t, const void *arg,
                       stream* in);

/* Create a transducer that reads input from two streams and writes
   output to a new stream.  The transduction function is passed the
   'arg' value. */
int transducers_link_2(stream **out,
                       transducers_2 t, const void *arg,
                       stream* in1, stream* in2);

/* Create a transducer that duplicates every byte read onto two other
   streams. */
int transducers_dup(stream **out1, stream** out2, stream *in);

#endif
