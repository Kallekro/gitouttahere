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
    return -1;
  }
  return pid;
}

int Pipe(int* fd) {
  if (pipe(fd) < 0) {
    return -1;
  }
  return 0;
}

void* Malloc(size_t size) {
  void* ptr = malloc(size);
  if (ptr == NULL) {
    return NULL;
  }
  return ptr;
}

int Waitpid(pid_t pid) {
  int status;
  waitpid(pid, &status, 0);

  if (!WIFEXITED(status)) {
    printf("child %d terminated abnormally with exit status=%d\n", pid, WEXITSTATUS(status));
    unix_error("waitpid error");
    return -1;
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
    return -1;
  }
  return 0;
}
/* End of helper functions */

int Close(int fd) {
  int ec = close(fd);
  if (ec < 0) {
    return -1; // pipe didnt close properly
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
  Close(s->read_fd);
  free(s->pids);
  free(s);
}

int transducers_link_source(stream **out,
                            transducers_source s, const void *arg) {

  // two file descriptors / pipe-ends
  int fd[2]; // read-end , write-end

  // create pipes
  if (Pipe(fd) == -1) {
     return -1;
  }


  int pid = Fork();
  if (pid == -1) {
    return -1; // on fork error
  }

  if (pid == 0) {

    if(Close(fd[0]) == -1) { // close read-end (not needed for child)
      return -1; // pipe close error
    }
    FILE* file_stream = Fdopen(fd[1], "w");

    if (file_stream == NULL) {
      exit(1); // File was not opened correctly
    }
    s(arg, file_stream);

    if (Fclose(file_stream) == -1) { 
      exit(2); // file didn't close normally
    };

    exit(0);
  }
  if(Close(fd[1]) == -1)  { // close write-end (not needed for parent)
    return -1; // on error
  }

  stream* new_stream = Malloc(sizeof(stream));
  if (new_stream == NULL) {
    return -1; // on malloc error
  }
  new_stream->read_fd = fd[0];
  new_stream->pids = Malloc(sizeof(int)); 

  if (new_stream->pids == NULL) {
    return -1; // malloc error
  }
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
      return -1;
    }
  }

  FILE* file_stream = Fdopen(in->read_fd, "r");
  s(arg, file_stream);
  if(Fclose(file_stream) == -1) {
    return -1; // fclose error
  }
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

  if (Pipe(fd) == -1) {
    return -1; // on pipe error
  }

  int pid = Fork();

  if (pid == -1) {
    return -1;
  }

  if (pid == 0) {
    if(Close(fd[0]) == -1) {
      exit(1); // on close error
    }

    FILE* file_read_stream = Fdopen(in->read_fd, "r");
    FILE* file_write_stream = Fdopen(fd[1], "w");

    if (file_read_stream == NULL || file_write_stream == NULL) {
      exit(2); // on fdopen error
    }

    t(arg, file_write_stream, file_read_stream);

    if (Fclose(file_read_stream) == -1 || Fclose(file_write_stream) == -1) {
      exit(3); // on fclose error
    }
    exit(0);
  }
  if (Close(fd[1]) == -1) {
    return -1;
  }

  stream* new_stream = Malloc(sizeof(stream)); 

  if (new_stream == NULL) {
    return -1; // on malloc error;
  }

  new_stream->read_fd = fd[0];
  new_stream->pids = Malloc(sizeof(int) * in->pid_len + sizeof(int)); 

  if (new_stream->pids == NULL) {
    return -1; // on malloc error
  }
  if( memcpy(new_stream->pids, in->pids, sizeof(int) * in->pid_len) == NULL ) {
    return -1; // memcopy error
  }
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

  if (Pipe(fd) == -1) {
     return -1;
  }

  int pid = Fork();

  if(pid == -1) {
    return -1; // on fork error()
  }

  if (pid == 0) {
    if (Close(fd[0]) == -1) {
      exit(1); // on close error
    }

    FILE* file_read_stream1 = Fdopen(in1->read_fd, "r");
    FILE* file_read_stream2 = Fdopen(in2->read_fd, "r");
    FILE* file_write_stream = Fdopen(fd[1], "w");

    if (file_read_stream1 == NULL || file_read_stream2 == NULL
        || file_write_stream == NULL) {
      exit(2); //on fdopen error
    }

    t(arg, file_write_stream, file_read_stream1, file_read_stream2);

    if (Fclose(file_read_stream1) == -1 || Fclose(file_read_stream2) == -1
        || Fclose(file_write_stream) == -1) {
      exit(3); // on fclose error
    }
    exit(0);
  }
  Close(fd[1]);
  stream* new_stream = Malloc(sizeof(stream)); 
  if (new_stream == NULL) {
    return -1; // on malloc error
  }

  new_stream->read_fd = fd[0];
  new_stream->pids = Malloc(sizeof(int) * (in1->pid_len + in2->pid_len) + sizeof(int)); 

  if (new_stream->pids == NULL) {
    return -1; // malloc error
  }

  if(memcpy(new_stream->pids, in1->pids, sizeof(int) * in1->pid_len) == NULL) {
    return -1;
  }
  if(memcpy((new_stream->pids + in1->pid_len * sizeof(int)), in2->pids, sizeof(int) * in2->pid_len) == NULL) {
    return -1;
  }

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

  stream* new_stream1 = Malloc(sizeof(stream));
  stream* new_stream2 = Malloc(sizeof(stream));

  if (new_stream1 == NULL || new_stream2 == NULL) {
    return -1; // on malloc error
  }
  new_stream1->read_fd = in->read_fd;
  new_stream2->read_fd = in->read_fd;

  new_stream1->pids = Malloc(in->pid_len * sizeof(int));
  new_stream2->pids = Malloc(in->pid_len * sizeof(int));

  if (new_stream1 == NULL || new_stream2 == NULL) {
    return -1; // on malloc error
  }
  if(memcpy(new_stream1->pids, in->pids, in->pid_len * sizeof(int)) == NULL) {
    return -1;
  }
  if(memcpy(new_stream2->pids, in->pids, in->pid_len * sizeof(int)) == NULL) {
    return -1;
  }

  new_stream1->pid_len = in->pid_len;
  new_stream2->pid_len = in->pid_len;

  new_stream1->connected = 0;
  new_stream2->connected = 0;


  *out1 = new_stream1;
  *out2 = new_stream2;
  return 0;
}
