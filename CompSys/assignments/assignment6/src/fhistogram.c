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

// err.h contains various nonstandard BSD extensions, but they are
// very handy.
#include <err.h>

#include "histogram.h"

unsigned long long file_counter = 0;
unsigned long long byte_counter = 0;
double total_latency = 0;

int global_histogram[8] = { 0 };

int fhistogram(char const *path) {
  FILE *f = fopen(path, "r");

  int local_histogram[8] = { 0 };

  if (f == NULL) {
    fflush(stdout);
    warn("failed to open %s", path);
    return -1;
  }

  double latency = 0.0;

  struct timeval start_time;
  struct timeval end_time;

  int i = 0;
  char c;

  while (fread(&c, sizeof(c), 1, f) == 1) {
    i++;
    update_histogram(local_histogram, c);
    if ((i % 100000) == 0) {
      gettimeofday(&start_time, NULL);

      merge_histogram(local_histogram, global_histogram);
      print_histogram(global_histogram);

      gettimeofday(&end_time, NULL);
      latency += ((end_time.tv_sec * 1000000 + end_time.tv_usec) - (start_time.tv_sec * 1000000 + start_time.tv_usec))/1000000.0;
    }
  }

  total_latency += latency;

  file_counter++;
  byte_counter += ftell(f);

  fclose(f);

  merge_histogram(local_histogram, global_histogram);
  print_histogram(global_histogram);

  return 0;
}

int main(int argc, char * const *argv) {
  if (argc < 2) {
    err(1, "usage: paths...");
    exit(1);
  }

  char * const *paths = &argv[1];

  struct timeval start_time;
  gettimeofday(&start_time, NULL);

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
      fhistogram(p->fts_path);
      break;
    default:
      break;
    }
  }

  fts_close(ftsp);

  move_lines(9);

  struct timeval end_time;

  gettimeofday(&end_time, NULL);
  double total_time = ((end_time.tv_sec * 1000000 + end_time.tv_usec) - (start_time.tv_sec * 1000000 + start_time.tv_usec)) / 1000000.0;


  printf("\nTotal time: %f\n", total_time);
  printf("Files per second: %f\n", (file_counter / total_time));
  printf("Bytes per second: %f\n", (byte_counter / total_time)); 

  printf("Total time spent printing: %f\n", total_latency);
  printf("Spent %f%% of the time printing.", (total_latency / total_time) * 100 );
  return 0;
}
