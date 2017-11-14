#include "transducers.h"
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <errno.h>
#include <string.h>

struct stream {
  bool connected; // is connected to transducer or sink?
  int pid;        // pid of worker process
  FILE* fp;       // pointer to file stream
};

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

//

void transducers_free_stream(stream *s) {
  /* Free stream object */
  // close the temporary file, which also removes it from file system
  fclose(s->fp);
  // free stream memory
  free(s);
}

int createNewStream(stream **out) {
  // allocate memory for new stream
  stream* newStream = malloc(sizeof(stream));
  if (newStream) {
    unix_error("malloc error");
  }
  // create temporary file
  FILE* tfile = tmpfile();
  if (tfile == NULL) {
    printf("Unable to create temporary file");
    return 1;
  }

  // parent assigns values to struct and returns
  newStream->fp = tfile;        // set file pointer
  newStream->connected = false; // set connected to false (ready to connect)
  *out = newStream;             // set out pointer to the new stream

  return 0;
}

int transducers_link_source(stream **out,
                            transducers_source s, const void *arg) {
  /* Creates new stream and links source to it */
  if (createNewStream(out) != 0) {
    return 1;
  };

  // fork process
  int pid = Fork();
  if (pid == 0) {
    // child does work and exits
    s (arg, (*out)->fp);
    exit(0);
  }
  (*out)->pid = pid;         // set pid of child worker process

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
  if (!WIFEXITED(status)) {
    printf("child %d terminated abnormally with exit status=%d", in->pid, WEXITSTATUS(status));
    return 1;
  }
  rewind(in->fp);  // go to start of file
  s(arg, in->fp);
  return 0;
}

int transducers_link_1(stream **out,
                       transducers_1 t, const void *arg,
                       stream* in) {
  if (in->connected) {
    return 1; // stream already connected to another transducer or sink
  }

  createNewStream(out);

  int pid = Fork();
  if (pid == 0) {
    int status;
    waitpid(in->pid, &status, 0);
    rewind(in->fp);
    t(arg, (*out)->fp, in->fp);
    exit(0);
  }
  (*out)->pid = pid;

  return 0;
}

int transducers_link_2(stream **out,
                       transducers_2 t, const void *arg,
                       stream* in1, stream* in2) {
  if (in1->connected || in2->connected) {
    return 1; // stream already connected to another transducer or sink
  }

  createNewStream(out);

  int pid = Fork();
  if (pid == 0) {
    int status;
    waitpid(in1->pid, &status, 0);
    // error handle here plz
    waitpid(in2->pid, &status, 0);
    rewind(in1->fp); rewind(in2->fp);
    t(arg, (*out)->fp, in1->fp, in2->fp);
    exit(0);
  }
  (*out)->pid = pid;

  return 0;
}

int transducers_dup(stream **out1, stream **out2,
                    stream *in) {
  createNewStream(out1); createNewStream(out2);

  int pid = Fork();
  if (pid == 0) {
    int status;
    waitpid(in->pid, &status, 0);

    unsigned char c;
    while (fread(&c, sizeof(unsigned char), 1, in->fp) == 1) {
      if (fwrite(&c, sizeof(unsigned char), 1, (*out1)->fp) != 1
          || fwrite(&c, sizeof(unsigned char), 1, (*out2)->fp) != 1) {
        return 1;
      }
    }
  }
  (*out1)->pid = pid;
  (*out2)->pid = pid;

  return 0;
}
