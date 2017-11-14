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

int check_equal (int* a, int* b, int n) {
  for (int i = 0; i < n; i++) {
    if (a[i] != b[i]) {
      return 1;
    }
  }
  return 0;
}

int main() {
  stream* s[3];

  const int len = 8;
  const int input1[9] = {/*length first ele*/len, 4, 1, 8, 2, 10, -1, 0, 100};
  const int input2[9] = {/*length first ele*/len, 1, 1, 1, 1, 1, 1, 1, 1};

  int *output = malloc(len);
  //output[strlen(input)] = '\0'; /* Ensure terminating NULL. */
 
  assert(transducers_link_source(&s[0], int_stream, input1) == 0);
  assert(transducers_link_source(&s[1], int_stream, input2) == 0);
  assert(transducers_link_2(&s[2], add_stream, 0, s[0], s[1]) == 0);
  assert(transducers_link_sink(save_stream, output, s[2]) == 0);
  
  for (int i=0; i < len; i++) {
    printf("out: %d\n", output[i]);
  }

  /* We cannot use the '==' operator for comparing strings, as strings
     in C are just pointers.  Using '==' would compare the _addresses_
     of the two strings, which is not what we want. */
  int expectedOutput[8] = {5, 2, 9, 3, 11, 0, 1, 101};
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
