#include "transducers.h"

#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

void string_stream(const void *arg, FILE *out) {
  fputs((const char*) arg, out);
}

void add_stream(const void *arg, FILE *out, FILE *in1, FILE *in2) {
  arg=arg; // Unused
  int x, y;
  while ((fread(&x, sizeof(int), 1, in1) == 1) &&
         (fread(&y, sizeof(int), 1, in2) == 1)) {
    int sum = x + y;
    fwrite(&sum, sizeof(int), 1, out);
  }
}

void increment_stream(const void *arg, FILE *out, FILE *in) {
  int d = *(int*)arg;

  unsigned char c;
  while (fread(&c, sizeof(unsigned char), 1, in) == 1) {
    c += d;
    if (fwrite(&c, sizeof(unsigned char), 1, out) != 1) {
      break;
    }
  }
}

void save_stream(void *arg, FILE *in) {
  /* We will be writing bytes to this location. */
  unsigned char *d = arg;

  while (fread(d, sizeof(unsigned char), 1, in) == 1) {
    d++; /* Move location ahead by one byte. */
  }
}

int main() {
  stream* s[4];

  char *input = "Hello, World!";
  char *output = malloc(strlen(input)+1);
  output[strlen(input)] = '\0'; /* Ensure terminating NULL. */
  int inc = 1;

  assert(transducers_link_source(&s[0], string_stream, input) == 0);
  assert(transducers_link_1(&s[1], increment_stream, &inc, s[0]) == 0);
  assert(transducers_link_1(&s[1], increment_stream, &inc, s[0]) == 2);

  assert(transducers_dup(&s[2], &s[3], s[1]) == 0);
  assert(transducers_dup(&s[2], &s[3], s[1]) == 2);

  assert(transducers_link_2(&s[2], add_stream, 0, s[2], s[3]) == 0);
  assert(transducers_link_2(&s[2], add_stream, 0, s[2], s[3]) == 2);

  assert(transducers_link_sink(save_stream, output, s[2]) == 0);

  for (int i = 0; i < (int)(sizeof(s)/sizeof(s[0])); i++) {
    transducers_free_stream(s[i]);
  }

  return 0;
}
