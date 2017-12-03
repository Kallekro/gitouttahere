// Setting _DEFAULT_SOURCE is necessary to activate visibility of
// certain header file contents on GNU/Linux systems.
#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fts.h>

// err.h contains various nonstandard BSD extensions, but they are
// very handy.
#include <err.h>

#include <pthread.h>

#include "job_queue.h"

// counter for bytes and files
unsigned long long file_counter = 0;
unsigned long long byte_counter = 0;
double latency_glob;
unsigned long linecount_glob;
long jobcount_glob;

// statically allocate and initialize mutex for printing
pthread_mutex_t stdout_mutex = PTHREAD_MUTEX_INITIALIZER;

// global constant needle argument
char const *needle;

int fauxgrep_file(char const *path) {
  FILE *f = fopen(path, "r");

  if (f == NULL) {
    warn("failed to open %s", path);
    return -1;
  }

  char *line = NULL;
  size_t linelen = 0;
  int lineno = 1;

  double latency = 0.0;
  unsigned long linecount = 0;

  struct timeval start_time;
  struct timeval end_time;
  while (getline(&line, &linelen, f) != -1) {
    if (strstr(line, needle) != NULL) {
      gettimeofday(&start_time, NULL);
      pthread_mutex_lock(&stdout_mutex);
      printf("%s:%d: %s", path, lineno, line);
      pthread_mutex_unlock(&stdout_mutex);
      gettimeofday(&end_time, NULL);
      latency += ((end_time.tv_sec * 1000000 + end_time.tv_usec) - (start_time.tv_sec * 1000000 + start_time.tv_usec))/1000000.0;
      linecount++;
    }
    lineno++;
  }
  file_counter++;
  byte_counter += ftell(f);

  pthread_mutex_lock(&stdout_mutex);
  latency_glob += latency;
  linecount_glob += linecount;
  jobcount_glob++;
  pthread_mutex_unlock(&stdout_mutex);

  free(line);
  fclose(f);

  return 0;
}

void* worker(void *arg) {
  struct job_queue *jq = arg;

  while (1) {
    char* path;
    if (job_queue_pop(jq, (void**)&path) == 0) {
      fauxgrep_file(path);
      free(path);
    } else {
      break;
    }
  }
  return NULL;
}

int main(int argc, char * const *argv) {
  if (argc < 2) {
    err(1, "usage: [-n INT] STRING paths...");
    exit(1);
  }

  int num_threads = 1;
  needle = argv[1];
  char * const *paths = &argv[2];


  if (argc > 3 && strcmp(argv[1], "-n") == 0) {
    // Since atoi() simply returns zero on syntax errors, we cannot
    // distinguish between the user entering a zero, or some
    // non-numeric garbage.  In fact, we cannot even tell whether the
    // given option is suffixed by garbage, i.e. '123foo' returns
    // '123'.  A more robust solution would use strtol(), but its
    // interface is more complicated, so here we are.
    num_threads = atoi(argv[2]);

    if (num_threads < 1) {
      err(1, "invalid thread count: %s\n", argv[2]);
    }

    needle = argv[3];
    paths = &argv[4];

  } else {
    needle = argv[1];
    paths = &argv[2];
  }

  //assert(0); // Initialise the job queue and some worker threads here.
  
  struct timeval start_time;
  gettimeofday(&start_time, NULL);

  struct job_queue jq;
  job_queue_init(&jq, 64);

  pthread_t *threads = calloc(num_threads, sizeof(pthread_t));
  for (int i=0; i < num_threads; i++) {
    if (pthread_create(&threads[i], NULL, &worker, &jq) != 0) {
      err(1, "pthread_create() failed\n");
    }
  }

  // FTS_LOGICAL = follow symbolic links
  // FTS_NOCHDIR = do not change the working directory of the process
  //
  // (These are not particularly important distinctions for our simple
  // uses.)
  int fts_options = FTS_LOGICAL | FTS_NOCHDIR;

  FTS *ftsp;
  if ((ftsp = fts_open(paths, fts_options, NULL)) == NULL) {
    err(1, "fts_open() failed\n");
    return -1;
  }

  FTSENT *p;
  while ((p = fts_read(ftsp)) != NULL) {
    switch (p->fts_info) {
    case FTS_D:
      break;
    case FTS_F:
      job_queue_push(&jq, (void*)strdup(p->fts_path));
      break;
    default:
      break;
    }
  }

  fts_close(ftsp);

  job_queue_destroy(&jq);

  for (int i = 0; i < num_threads; i++) {
    if (pthread_join(threads[i], NULL) != 0) {
      err(1, "pthread_join() failed\n");
    }
  }

  free(threads);

  struct timeval end_time;
  double total_latency;
  gettimeofday(&end_time, NULL);

  double total_time = ((end_time.tv_sec * 1000000 + end_time.tv_usec) - (start_time.tv_sec * 1000000 + start_time.tv_usec)) / 1000000.0;

  printf("\nTotal time: %f\n", total_time);
  printf("Files per second: %f\n", (file_counter / total_time));
  printf("Bytes per second: %f\n", (byte_counter / total_time)); 
  if (linecount_glob > 0) {

    if (num_threads < jobcount_glob) {
      total_latency = (latency_glob / num_threads);
    }
    else {
      total_latency = (latency_glob / jobcount_glob);
    }
    printf("Print mean latency per line: %f\n", total_latency/linecount_glob);
    printf("Total time spent printing: %f\n", total_latency);
    printf("Spent %f%% of the time printing.", (total_latency / total_time) * 100 );
  }

  return 0;
}
