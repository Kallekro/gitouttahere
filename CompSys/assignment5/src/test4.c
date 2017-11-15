#include "transducers.h"

#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

void int_stream(const void *arg, FILE *out) {
  int len = ((int*) arg)[0];
  for (int i=0; i < len; i++) {
    fwrite(&((int*)arg)[i+1], sizeof(int), 1, out);
  }
  sleep(1);
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

void save_stream(void *arg, FILE *in) {
  int *d = arg;

  while (fread(d, sizeof(int), 1, in) == 1) {
    d++; 
  }
}

void string_stream(const void *arg, FILE *out) {
  fputs((const char*) arg, out);
  exit(0);
}

void save_string_stream(void *arg, FILE *in) {
  /* We will be writing bytes to this location. */
  unsigned char *d = arg;

  while (fread(d, sizeof(unsigned char), 1, in) == 1) {
    d++; /* Move location ahead by one byte. */
  }
}

int check_equal (int* a, int* b, int n) {
  for (int i = 0; i < n; i++) {
    if (a[i] != b[i]) {
      return 1;
    }
  }
  return 0;
}

int main() {
  stream* s[7];

  const int len = 8;
  const int input1[9] = {/*length first ele*/len, 4, 1, 8, 2, 10, -1, 0, 100};
  const int input2[9] = {/*length first ele*/len, 1, 1, 1, 1, 1, 1, 1, 1};

  char *input_str = "Intermediate sink says hello!";
  char *output_str = malloc(strlen(input_str)+1);
  output_str[strlen(input_str)] = '\0'; /* Ensure terminating NULL. */

  int *output = malloc(sizeof(int) * len);
  //output[strlen(input)] = '\0'; /* Ensure terminating NULL. */
 
  // "main" program - takes at least 2 seconds
  assert(transducers_link_source(&s[0], int_stream, input1) == 0);
  assert(transducers_link_source(&s[1], int_stream, input2) == 0);

  assert(transducers_dup(&s[2], &s[3], s[1]) == 0);

  assert(transducers_link_2(&s[4], add_stream, 0, s[2], s[3]) == 0);

  // "intermediate" program
  // should finish before main
  assert(transducers_link_source(&s[6], string_stream, input_str) == 0);
  assert(transducers_link_sink(save_string_stream, output_str, s[6]) == 0);
  printf("%s\n", output_str); 
  assert(strcmp(input_str, output_str) == 0);

  // "main" continues
  assert(transducers_link_2(&s[5], add_stream, 0, s[4], s[0]) == 0);
  assert(transducers_link_sink(save_stream, output, s[5]) == 0);

  int expectedOutput[8] = {6, 3, 10, 4, 12, 1, 2, 102};
  assert(check_equal(expectedOutput, output, len) == 0);

  free(output);

  /* Note the sizeof()-trick to determine the number of elements in
     the array.  This *only* works for statically allocated arrays,
     *not* ones created by malloc(). */
  for (int i = 0; i < (int)(sizeof(s)/sizeof(s[0])); i++) {
    transducers_free_stream(s[i]);
  }

  return 0;
}
