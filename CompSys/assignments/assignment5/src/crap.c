#include "transducers.h"

#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

void int_stream(const void *arg, FILE *out) {
  //fputs((int) arg, out);
  int len = (int) arg[0];
  for (int i=0; i < len; i++) {
    //fprintf(out, "%d", arg[i+1]);
    fwrite(&arg[i+1], sizeof(int), 1, out);
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
  /* We will be writing bytes to this location. */
  int *d = arg;

  while (fread(d, sizeof(int), 1, in) == 1) {
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
  stream* s[3];

  int len = 8;
  int input1[len+1] = {/*length first ele*/len, 4, 1, 8, 2, 10, -1, 0, 100};
  int input2[len+1] = {/*length first ele*/len, 1, 1, 1, 1, 1, 1, 1, 1};

  int *output = malloc(len);
  //output[strlen(input)] = '\0'; /* Ensure terminating NULL. */
 
  assert(transducers_link_source(&s[0], int_stream, input1) == 0);
  assert(transducers_link_source(&s[1], int_stream, input2) == 0);
  assert(transducers_link_2(&s[2], add_stream, s[0], s[1]) == 0);
  assert(transducers_link_sink(save_stream, output, s[2]) == 0);

  /* We cannot use the '==' operator for comparing strings, as strings
     in C are just pointers.  Using '==' would compare the _addresses_
     of the two strings, which is not what we want. */
  int expectedOutput[len+1] = {5, 2, 9, 3, 11, 0, 1, 101};
  assert(check_equal(expectedOutput, output, len) == 0);

  /* Note the sizeof()-trick to determine the number of elements in
     the array.  This *only* works for statically allocated arrays,
     *not* ones created by malloc(). */
  for (int i = 0; i < (int)(sizeof(s)/sizeof(s[0])); i++) {
    transducers_free_stream(s[i]);
  }

  return 0;
}
