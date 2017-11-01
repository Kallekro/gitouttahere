/* Find, in parallel, those random integers that are divisible by two
   given numbers and print them to the screen. */

#include "transducers.h"

#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <stdlib.h>

void random_garbage(const void *arg, FILE *out) {
  int len = *(int*)arg;

  for (int i = 0; i < len; i++) {
    int x = rand();
    fwrite(&x, sizeof(int), 1, out);
  }
}

void divisible_by(const void *arg, FILE *out, FILE *in) {
  int d = *(int*)arg;

  int x;
  while (fread(&x, sizeof(int), 1, in) == 1) {
    int y = (x % d) == 0;
    fwrite(&y, sizeof(int), 1, out);
  }
}

void and_streams(const void *arg, FILE *out, FILE *in1, FILE *in2) {
  arg=arg; // Unused
  int x, y;
  while ((fread(&x, sizeof(int), 1, in1) == 1) &&
         (fread(&y, sizeof(int), 1, in2) == 1)) {
    int z = x && y;
    fwrite(&z, sizeof(int), 1, out);
  }
}

void filter_streams(const void *arg, FILE *out, FILE *in1, FILE *in2) {
  arg=arg; // Unused
  int x, y;
  while ((fread(&x, sizeof(int), 1, in1) == 1) &&
         (fread(&y, sizeof(int), 1, in2) == 1)) {
    if (x) {
      fwrite(&y, sizeof(int), 1, out);
    }
  }
}

void print_stream(void *arg, FILE *in) {
  arg=arg; // Unused
  int x;
  while (fread(&x, sizeof(int), 1, in) == 1) {
    printf("%d\n", x);
  }
}

int main(int argc, char** argv) {
  stream* s[9];

  if (argc != 4) {
    fprintf(stderr, "%s usage: <n> <first divisor> <second divisor>\n", argv[0]);
    return 1;
  }

  int len = atoi(argv[1]);
  int two = atoi(argv[2]);
  int three = atoi(argv[3]);

  assert(transducers_link_source(&s[0], random_garbage, &len) == 0);
  assert(transducers_dup(&s[1], &s[2], s[0]) == 0);
  assert(transducers_dup(&s[3], &s[4], s[1]) == 0);
  assert(transducers_link_1(&s[5], divisible_by, &two, s[2]) == 0);
  assert(transducers_link_1(&s[6], divisible_by, &three, s[3]) == 0);
  assert(transducers_link_2(&s[7], and_streams, NULL, s[5], s[6]) == 0);
  assert(transducers_link_2(&s[8], filter_streams, NULL, s[7], s[4]) == 0);
  assert(transducers_link_sink(print_stream, NULL, s[8]) == 0);

  // Note the sizeof()-trick to determine the number of elements in
  // the array.  This *only* works for statically allocated arrays,
  // *not* ones created by malloc().
  for (int i = 0; i < (int)(sizeof(s)/sizeof(s[0])); i++) {
    transducers_free_stream(s[i]);
  }

  return 0;
}
