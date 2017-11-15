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

int Fork() {
  int pid;
  if ((pid = fork()) < 0){
    unix_error("fork error");
  }
  return pid;
}

int Waitpid(pid_t pid) {
  int status;
  waitpid(pid, &status, 0);

  if (!WIFEXITED(status)) {
    printf("child %d terminated abnormally with exit status=%d\n", pid, WEXITSTATUS(status));
    unix_error("waitpid error");
    return 1;
  }
  return 0;
}

FILE* Fdopen(int fd, char* mode) {
  FILE* fp = fdopen(fd, mode);
  if(fp == NULL) {
    unix_error("fopen error");
    return NULL;
  }
  return fp;
}

int Fclose(FILE* fd) {
  int ret = fclose(fd);
  if (ret < 0) {
    unix_error("fclose error");
    return 1;
  }
  return 0;
}
/* End of helper functions */

int Close(int fd) {
  int ec = close(fd);
  if (ec < 0) {
    return 1; // pipe didnt close properly
  }
  return 0;
}


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
     unix_error("pipe error");
     return 1;
  }

  stream* new_stream = malloc(sizeof(stream)); 
  int pid = fork();

  if (pid == 0) {
    Close(fd[0]); // close read-end (not needed for child)
    FILE* file_stream = Fdopen(fd[1], "w");

    if (file_stream == NULL) {
      exit(1); // File was not opened correctly
    }
    s(arg, file_stream);

    if (Fclose(file_stream) != 0) { 
      exit(2); // file didn't close normally
    };

    exit(0);
  }
  Close(fd[1]); // close write-end (not needed for parent)

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

  for (int i=0; i < in->pid_len; i++) {
    if(Waitpid(in->pids[i]) != 0) {
      return 1;
    }
  }
  FILE* file_stream = Fdopen(in->read_fd, "r");
  s(arg, file_stream);
  Fclose(file_stream);
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

  new_stream1->connected = 0;
  new_stream2->connected = 0;


  *out1 = new_stream1;
  *out2 = new_stream2;
  return 0;
}
