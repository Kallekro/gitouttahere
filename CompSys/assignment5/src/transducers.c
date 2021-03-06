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
// error handled helper functions

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
    unix_error("malloc error");
    return NULL;
  }
  return ptr;
}

int Waitpid(pid_t pid) {
  int status;
  waitpid(pid, &status, 0);

  if (!WIFEXITED(status)) {
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

int Close(int fd) {
  int ec = close(fd);
  if (ec < 0) {
    return -1; // pipe didnt close properly
  }
  return 0;
}

/* End of helper functions */

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

  if (pid == 0) { // child process

    if(Close(fd[0]) == -1) { // close read-end (not needed for child)
      return -1; // pipe close error
    }

    // opening read-end of the file in write mode
    FILE* file_stream = Fdopen(fd[1], "w");

    if (file_stream == NULL) {
      exit(1); // File was not opened correctly
    }
    // executing transducer source-function
    s(arg, file_stream);

    if (Fclose(file_stream) == -1) { 
      exit(2); // file didn't close normally
    };

    exit(0);
  }
  if(Close(fd[1]) == -1)  { // close write-end (not needed for parent)
    return -1; // on error
  }

  stream* new_stream = Malloc(sizeof(stream)); // space for new stream.
  if (new_stream == NULL) {
    return -1; // on malloc error
  }
  new_stream->read_fd = fd[0];
  new_stream->pids = Malloc(sizeof(int));  // space for new streams child-pid-array

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

  FILE* file_stream = Fdopen(in->read_fd, "r"); // opening the read-end of the pipe in read-mode
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

  if (Pipe(fd) == -1) { // creating pipe
    return -1; // on pipe error
  }

  int pid = Fork();

  if (pid == -1) {
    return -1;
  }

  if (pid == 0) { // if child process.
    if(Close(fd[0]) == -1) {
      exit(1); // on close error
    }


    // two fd's for reading and writing to and from the pipe
    FILE* file_read_stream = Fdopen(in->read_fd, "r");
    FILE* file_write_stream = Fdopen(fd[1], "w");

    if (file_read_stream == NULL || file_write_stream == NULL) {
      exit(2); // on fdopen error
    }

    // execute linking transducer for reading and writing from pipe(files)
    t(arg, file_write_stream, file_read_stream);

    if (Fclose(file_read_stream) == -1 || Fclose(file_write_stream) == -1) {
      exit(3); // on fclose error
    }
    exit(0);
  }
  if (Close(fd[1]) == -1) {
    return -1;
  }
  // space for new output stream
  stream* new_stream = Malloc(sizeof(stream)); 

  if (new_stream == NULL) {
    return -1; // on malloc error;
  }

  new_stream->read_fd = fd[0]; // having the new pipes read-end as fd
  new_stream->pids = Malloc(sizeof(int) * in->pid_len + sizeof(int)); // space for child pids 

  if (new_stream->pids == NULL) {
    return -1; // on malloc error
  }
  // copying child-pid-array from input-stream to output-stream
  if( memcpy(new_stream->pids, in->pids, sizeof(int) * in->pid_len) == NULL ) {
    unix_error("memcopy error");
    return -1; // memcopy error
  }
  new_stream->pids[in->pid_len] = pid;
  new_stream->pid_len = in->pid_len+1;
  new_stream->connected = 0;
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

  if (Pipe(fd) == -1) { // creating a pipe
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

    // openening two files in read-mode and one in write-mode for passing to the link2-transducer
    FILE* file_read_stream1 = Fdopen(in1->read_fd, "r");
    FILE* file_read_stream2 = Fdopen(in2->read_fd, "r");
    FILE* file_write_stream = Fdopen(fd[1], "w");

    if (file_read_stream1 == NULL || file_read_stream2 == NULL
        || file_write_stream == NULL) {
      exit(2); //on fdopen error
    }
    // executing the double linking transducer with the initialised filepointers.
    t(arg, file_write_stream, file_read_stream1, file_read_stream2);

    if (Fclose(file_read_stream1) == -1 || Fclose(file_read_stream2) == -1
        || Fclose(file_write_stream) == -1) {
      exit(3); // on fclose error
    }
    exit(0);
  }
  Close(fd[1]);
  // space for the new output-stream
  stream* new_stream = Malloc(sizeof(stream)); 
  if (new_stream == NULL) {
    return -1; // on malloc error
  }

  new_stream->read_fd = fd[0];
  new_stream->pids = Malloc(sizeof(int) * (in1->pid_len + in2->pid_len) + sizeof(int)); 

  if (new_stream->pids == NULL) {
    return -1; // malloc error
  }

  // copying the child-pid-array from the two input-streams to the new output-stream-array
  if(memcpy(new_stream->pids, in1->pids, sizeof(int) * in1->pid_len) == NULL) {
    unix_error("memcopy error");
    return -1;
  }
  if(memcpy((new_stream->pids + in1->pid_len), in2->pids, sizeof(int) * in2->pid_len) == NULL) {
    unix_error("memcopy error");
    return -1;
  }

  new_stream->pids[in1->pid_len + in2->pid_len] = pid; // adding this transducers child-pid to the stream.
  new_stream->pid_len = in1->pid_len + in2->pid_len + 1;
  new_stream->connected = 0;
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

  int new_fd1[2];
  int new_fd2[2];

  pipe(new_fd1);
  pipe(new_fd2);

  int pid = Fork();
  if (pid == 0) {

    Close(new_fd1[0]); Close(new_fd2[0]);

    // opening read-end of the pipe and two write-ends of two pipes.
    FILE* read_stream = Fdopen(in->read_fd, "r");
    FILE* write_stream1 = Fdopen(new_fd1[1], "w");
    FILE* write_stream2 = Fdopen(new_fd2[1], "w");


    // read the input pipe and write to the output pipes.
    unsigned char c;
    while (fread(&c, sizeof(unsigned char), 1, read_stream) == 1) {
      fwrite(&c, sizeof(unsigned char), 1, write_stream1);
      fwrite(&c, sizeof(unsigned char), 1, write_stream2);
    }

    Fclose(read_stream);
    Fclose(write_stream1);
    Fclose(write_stream2);

    exit(0);
  }
  Close(new_fd1[1]); Close(new_fd2[1]);

  new_stream1->read_fd = new_fd2[0];
  new_stream2->read_fd = new_fd1[0];

  new_stream1->pids = Malloc(in->pid_len * sizeof(int));
  new_stream2->pids = Malloc(in->pid_len * sizeof(int));

  if (new_stream1 == NULL || new_stream2 == NULL) {
    return -1; // on malloc error
  }

  // copy child-pid-array to the new output-stream
  if(memcpy(new_stream1->pids, in->pids, in->pid_len * sizeof(int)) == NULL) {
    unix_error("memcopy error");
    return -1;
  }
  if(memcpy(new_stream2->pids, in->pids, in->pid_len * sizeof(int)) == NULL) {
    unix_error("memcopy error");
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
