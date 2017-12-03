// Setting _DEFAULT_SOURCE is necessary to activate visibility of
// certain header file contents on GNU/Linux systems.
#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdint.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fts.h>

#include "job_queue.h"

// counter for bytes and files
unsigned long long file_counter = 0;
unsigned long long byte_counter = 0;


double latency_glob;
unsigned long print_count_glob;
long jobcount_glob;

pthread_mutex_t stdout_mutex = PTHREAD_MUTEX_INITIALIZER;

// err.h contains various nonstandard BSD extensions, but they are
// very handy.
#include <err.h>

#include "histogram.h"

int global_histogram[8] = { 0 };
pthread_mutex_t hist_mutex = PTHREAD_MUTEX_INITIALIZER;

int fhistogram(char const *path) {
  FILE *f = fopen(path, "r");

  int local_histogram[8] = { 0 };

  if (f == NULL) {
    fflush(stdout);
    warn("failed to open %s", path);
    return -1;
  }

  int i = 0;


  struct timeval start_time;
  struct timeval end_time;

  double latency = 0.0;
  unsigned long print_count = 0;
  char c;
  while (fread(&c, sizeof(c), 1, f) == 1) {
    i++;
    update_histogram(local_histogram, c);
    int locked;
    if (((i % 100000) == 0 && (locked = pthread_mutex_trylock(&hist_mutex)) == 0) || (i % 1000000) == 0) {
      gettimeofday(&start_time, NULL);
      if (locked != 0) {
        pthread_mutex_lock(&hist_mutex);
      }
      gettimeofday(&end_time, NULL);

      latency += ((end_time.tv_sec * 1000000 + end_time.tv_usec) - (start_time.tv_sec * 1000000 + start_time.tv_usec))/1000000.0;
      print_count++;

      merge_histogram(local_histogram, global_histogram);
      print_histogram(global_histogram);
      pthread_mutex_unlock(&hist_mutex);
    }
  }

  latency_glob += latency;
  print_count_glob += print_count;
  jobcount_glob++;
  byte_counter += ftell(f);
  file_counter++;
  
  fclose(f);

  pthread_mutex_lock(&hist_mutex);
  merge_histogram(local_histogram, global_histogram);
  print_histogram(global_histogram);
  pthread_mutex_unlock(&hist_mutex);

  return 0;
}

void* worker(void *arg) {
  struct job_queue *jq = arg;

  while (1) {
    char* path;
    if (job_queue_pop(jq, (void**)&path) == 0) {
      fhistogram(path);
      free(path);
    } else {
      break;
    }
  }
  return NULL;
}

int main(int argc, char * const *argv) {
  if (argc < 2) {
    err(1, "usage: paths...");
    exit(1);
  }

  int num_threads = 1;
  char * const *paths = &argv[1];

  if (argc > 3 && strcmp(argv[1], "-n") == 0) {
    // Since atoi() simply returns zero on syntax errors, we cannot
    // distinguish between the user entering a zero, or some
    // non-numeric garbage.  In fact, we cannot even tell whether the
    // given option is suffixed by garbage, i.e. '123foo' returns
    // '123'.  A more robust solution would use strtol(), but its
    // interface is more complicated, so here we are.
    num_threads = atoi(argv[2]);

    if (num_threads < 1) {
      err(1, "invalid thread count: %s", argv[2]);
    }

    paths = &argv[3];
  } else {
    paths = &argv[1];
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
    err(1, "fts_open() failed");
    return -1;
  }

  FTSENT *p;
  while ((p = fts_read(ftsp)) != NULL) {
    switch (p->fts_info) {
    case FTS_D:
      break;
    case FTS_F:
      //assert(0); // Process the file p->fts_path, somehow.
      job_queue_push(&jq, (void*)strdup(p->fts_path));
      break;
    default:
      break;
    }
  }

  fts_close(ftsp);

  //assert(0); // Shut down the job queue and the worker threads here.

  job_queue_destroy(&jq);

  for (int i=0; i < num_threads; i++) {
    if (pthread_join(threads[i], NULL) != 0) {
      err(1, "pthread_join() failed\n");
    }
  }

  free(threads);

  move_lines(9);

  struct timeval end_time;
  gettimeofday(&end_time, NULL);

  double total_time = ((end_time.tv_sec * 1000000 + end_time.tv_usec) - (start_time.tv_sec * 1000000 + start_time.tv_usec)) / 1000000.0;

  printf("\nTotal time: %f\n", total_time); 
  printf("Files per second: %f\n", (file_counter / total_time));
  printf("Bytes per second: %f\n", (byte_counter / total_time)); 

  double total_latency;

  if (print_count_glob > 0) {

    if (num_threads < jobcount_glob) {
      total_latency = (latency_glob / num_threads);
    }
    else {
      total_latency = (latency_glob / jobcount_glob);
    }

    printf("Update mean latency per histogram update: %f\n", (total_latency/print_count_glob));
    printf("Total time spent updating histogram: %f\n", total_latency);
    printf("Spent %f%% of the time printing.", (total_latency / total_time) * 100 );
  }

  return 0;
}
