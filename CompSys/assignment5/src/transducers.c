#include "transducers.h"
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>

// system call error handling.
void unix_error(char *msg) {
  fprintf(stderr, "%s: %s\n", msg, strerror(errno));
}

/* End of helper functions */

struct stream {
  int read_fd;
  int *pids;
  int pid_len;
  int connected;
};

void transducers_free_stream(stream *s) {
  close(s->read_fd);
  free(s->pids);
  free(s);
}

int transducers_link_source(stream **out,
                            transducers_source s, const void *arg) {

  // two file descriptors / pipe-ends
  int fd[2]; // read-end , write-end

  // create pipes
  if (pipe(fd) == -1) {
     unix_error("pipe");
     return 1;
  }

  stream* new_stream = malloc(sizeof(stream)); 
  int pid = fork();

  if (pid == 0) {
    close(fd[0]); // close read-end (not needed for child)
    FILE* file_stream = fdopen(fd[1], "w");
    s(arg, file_stream);
    fclose(file_stream); // close write-end (send EOF to reader)
    exit(0);
  }
  close(fd[1]); // close write-end (not needed for parent)
  
  new_stream->read_fd = fd[0];
  new_stream->pids = malloc(sizeof(int)); 
  new_stream->pids[0] = pid;
  new_stream->pid_len = 1;
  new_stream->connected = 0;
  *out = new_stream;

  return 0;
}

int transducers_link_sink(transducers_sink s, void *arg,
                          stream *in) {

  if (in->connected) {
    return 2;
  }
  in->connected = 1;

  int status;
  for (int i=0; i < in->pid_len; i++) {
    waitpid(in->pids[i], &status, 0); 
  }
  FILE* file_stream = fdopen(in->read_fd, "r");
  s(arg, file_stream);
  fclose(file_stream);
  return 0;
}

int transducers_link_1(stream **out,
                       transducers_1 t, const void *arg,
                       stream* in) {
  if (in->connected) {
    return 2;
  }
  in->connected = 1;

  int fd[2]; // read-end , write-end

  if (pipe(fd) == -1) {
     unix_error("pipe");
     return 1;
  }

  int pid = fork();

  if (pid == 0) {
    close(fd[0]);
    FILE* file_read_stream = fdopen(in->read_fd, "r");
    FILE* file_write_stream = fdopen(fd[1], "w");
    t(arg, file_write_stream, file_read_stream);
    fclose(file_read_stream); 
    fclose(file_write_stream);
    exit(0);
  }
  close(fd[1]);

  stream* new_stream = malloc(sizeof(stream)); 
  new_stream->read_fd = fd[0];
  new_stream->pids = malloc(sizeof(int) * in->pid_len + sizeof(int)); 
  memcpy(new_stream->pids, in->pids, sizeof(int) * in->pid_len);
  new_stream->pids[in->pid_len] = pid;
  new_stream->pid_len = in->pid_len+1;
  *out = new_stream;

  return 0;
}

int transducers_link_2(stream **out,
                       transducers_2 t, const void *arg,
                       stream* in1, stream* in2) {
  if (in1->connected || in2->connected) {
    return 2;
  }
  in1->connected = 1;
  in2->connected = 1;

  int fd[2]; // read-end , write-end

  if (pipe(fd) == -1) {
     unix_error("pipe");
     return 1;
  }

  int pid = fork();

  if (pid == 0) {
    close(fd[0]);
    FILE* file_read_stream1 = fdopen(in1->read_fd, "r");
    FILE* file_read_stream2 = fdopen(in2->read_fd, "r");
    FILE* file_write_stream = fdopen(fd[1], "w");
    t(arg, file_write_stream, file_read_stream1, file_read_stream2);
    fclose(file_read_stream1);
    fclose(file_read_stream2);
    fclose(file_write_stream);
    exit(0);
  }
  close(fd[1]);
  stream* new_stream = malloc(sizeof(stream)); 
  new_stream->read_fd = fd[0];
  new_stream->pids = malloc(sizeof(int) * (in1->pid_len + in2->pid_len) + sizeof(int)); 

  memcpy(new_stream->pids, in1->pids, sizeof(int) * in1->pid_len);
  memcpy((new_stream->pids + in1->pid_len * sizeof(int)), in2->pids, sizeof(int) * in2->pid_len);

  new_stream->pids[in1->pid_len + in2->pid_len] = pid;
  new_stream->pid_len = in1->pid_len + in2->pid_len + 1;
  *out = new_stream;
  
  return 0;
}

int transducers_dup(stream **out1, stream **out2,
                    stream *in) {

  if (in->connected) {
    return 2;
  }
  in->connected = 1;
  stream* new_stream1 = malloc(sizeof(stream));
  stream* new_stream2 = malloc(sizeof(stream));

  new_stream1->read_fd = in->read_fd;
  new_stream2->read_fd = in->read_fd;

  new_stream1->pids = malloc(in->pid_len * sizeof(int));
  new_stream2->pids = malloc(in->pid_len * sizeof(int));

  memcpy(new_stream1->pids, in->pids, in->pid_len * sizeof(int));
  memcpy(new_stream2->pids, in->pids, in->pid_len * sizeof(int));

  new_stream1->pid_len = in->pid_len;
  new_stream2->pid_len = in->pid_len;
  
  *out1 = new_stream1;
  *out2 = new_stream2;
  return 0;
}
