#include "transducers.h"
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdbool.h>

struct stream {
  bool connected; // is connected to transducer or sink?
  int pid;        // pid of worker process
  FILE* fp;       // pointer to file stream
};

void transducers_free_stream(stream *s) {
  /* Free stream object */

  // close the temporary file, which also removes it from file system
  fclose(s->fp);
  // free stream memory
  free(s);
}

int transducers_link_source(stream **out,
                            transducers_source s, const void *arg) {
  /* Creates new stream and links source to it */
   
  // allocate memory for new stream
  stream* newStream = malloc(sizeof(stream));
  // create temporary file
  FILE* tfile = tmpfile();
  if (tfile == NULL) {
    printf("Unable to create temporary file");
    return 1; 
  }
  
  // fork process
  int pid = fork();
  if (pid < 0) {
    printf("Unable to create child process");
  }

  if (pid == 0) {
    // child does work and exits
    s (arg, tfile);
    exit(0);
  }

  // parent assigns values to struct and returns
  newStream->fp = tfile;        // set file pointer
  newStream->pid = pid;         // set pid of child worker process
  newStream->connected = false; // set connected to false (ready to connect)
  *out = newStream;             // set out pointer to the new stream
  return 0;
}

int transducers_link_sink(transducers_sink s, void *arg,
                          stream *in) {
  /* Link a stream to a sink */

  if (in->connected) {
    return 1;                   // stream already connected to another transducer or sink
  }
  in->connected = true;         // set connected to true (can't connect with anything else)

  int status;                   // integer for holding wait status
  waitpid(in->pid, &status, 0); // wait for the worker process to finish with stream 
  rewind(in->fp);               // go to start of file
  s(arg, in->fp);               // do blocking work
  return 0;
}

int transducers_link_1(stream **out,
                       transducers_1 t, const void *arg,
                       stream* in) {
  if (in->connected || *out->connected) {
    return 1; // stream already connected to another transducer or sink
  }
  out=out; /* unused */
  t=t; /* unused */
  arg=arg; /* unused */
  in=in; /* unused */
  return 1;
}

int transducers_link_2(stream **out,
                       transducers_2 t, const void *arg,
                       stream* in1, stream* in2) {
  if (in1->connected || in2->connected || *out->connected) {
    return 1; // stream already connected to another transducer or sink
  }
  out=out; /* unused */
  t=t; /* unused */
  arg=arg; /* unused */
  in1=in1; /* unused */
  in2=in2; /* unused */
  return 1;
}

int transducers_dup(stream **out1, stream **out2,
                    stream *in) {
  out1=out1; /* unused */
  out2=out2; /* unused */
  in=in; /* unused */
  return 1;
}
