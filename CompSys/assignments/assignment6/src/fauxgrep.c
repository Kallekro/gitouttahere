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

unsigned long long file_counter = 0;
unsigned long long byte_counter = 0;
double total_latency = 0;
unsigned long linecount = 0;

int fauxgrep_file(char const *needle, char const *path) {
  FILE *f = fopen(path, "r");

  if (f == NULL) {
    warn("failed to open %s", path);
    return -1;
  }

  char *line = NULL;
  size_t linelen = 0;
  int lineno = 1;

  double latency = 0.0;

  struct timeval start_time;
  struct timeval end_time;

  while (getline(&line, &linelen, f) != -1) {
    if (strstr(line, needle) != NULL) {
      gettimeofday(&start_time, NULL);
      printf("%s:%d: %s", path, lineno, line);
      gettimeofday(&end_time, NULL);
      latency += ((end_time.tv_sec * 1000000 + end_time.tv_usec) - (start_time.tv_sec * 1000000 + start_time.tv_usec))/1000000.0;
      linecount++;
    }
    lineno++;
  }

  total_latency += latency;

  file_counter++;
  byte_counter += ftell(f);

  free(line);
  fclose(f);

  return 0;
}

int main(int argc, char * const *argv) {
  if (argc < 2) {
    err(1, "usage: STRING paths...");
    exit(1);
  }

  char const *needle = argv[1];
  char * const *paths = &argv[2];

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
      fauxgrep_file(needle, p->fts_path);
      break;
    default:
      break;
    }
  }
  fts_close(ftsp);
  struct timeval end_time;

  gettimeofday(&end_time, NULL);
  double total_time = ((end_time.tv_sec * 1000000 + end_time.tv_usec) - (start_time.tv_sec * 1000000 + start_time.tv_usec)) / 1000000.0;

  printf("\nTotal time: %f\n", total_time);
  printf("Files per second: %f\n", (file_counter / total_time));
  printf("Bytes per second: %f\n", (byte_counter / total_time)); 

  printf("Print mean latency per line: %f\n", total_latency/linecount);
  printf("Total time spent printing: %f\n", total_latency);
  printf("Spent %f%% of the time printing.", (total_latency / total_time) * 100 );

  return 0;
}
