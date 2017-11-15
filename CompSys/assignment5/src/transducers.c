#include "transducers.h"


// system call error handling.
void unix_error(char *msg) {
  fprintf(stderr, "%s: %s\n", msg, strerror(errno));
}

/* End of helper functions */

struct stream {
  int read_fd;
  int *pids;
  int pid_len;
};

void transducers_free_stream(stream *s) {
  s=s; /* unused */
}

int transducers_link_source(stream **out,
                            transducers_source s, const void *arg) {

  int fd[2]; // read-end , write-end

  if (pipe(fd) == -1) {
     unix_error("pipe");
     return 1;
  }

  int pid = fork();

  if (pid == 0) {
    close(fd[0]); // close read-end (not needed)
    s(arg, fd[1]);
    close(fd[1]); // close write-end (send EOF to reader)
    exit(0);
  }
  close(fd[1]);
  
  stream* new_stream = malloc(sizeof(stream)); 
  new_stream.read_fd = fd[0];
  new_stream.pids = malloc(sizeof(int)); 
  new_streams.pids[0] = pid;
  new_streams.pid_len = 1;
  *out = new_stream;

  return 0;
}

int transducers_link_sink(transducers_sink s, void *arg,
                          stream *in) {
  int status;
  for (int i=0; i < in.pid_len; i++) {
    waitpid(in.pids[i], &status, 0); 
  }
  s(arg, in.read_fd);

  return 0;
}

int transducers_link_1(stream **out,
                       transducers_1 t, const void *arg,
                       stream* in) {
  out=out; /* unused */
  t=t; /* unused */
  arg=arg; /* unused */
  in=in; /* unused */
  return 1;
}

int transducers_link_2(stream **out,
                       transducers_2 t, const void *arg,
                       stream* in1, stream* in2) {
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
