#include "transducers.h"

#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>


void string_stream(const void *arg, FILE *out) {
  fputs((const char*) arg, out);
  sleep(2);
  kill(getpid(), SIGKILL);
}

void save_stream(void *arg, FILE *in) {
  /* We will be writing bytes to this location. */
  unsigned char *d = arg;
  while (fread(d, sizeof(unsigned char), 1, in) == 1) {
    d++; /* Move location ahead by one byte. */
  }
}

int main() {
  stream* s[1];

  char *input = "Hello, World!";
  char *output = malloc(strlen(input)+1);
  output[strlen(input)] = '\0'; /* Ensure terminating NULL. */

  assert(transducers_link_source(&s[0], string_stream, input) == 0);
  assert(transducers_link_sink(save_stream, output, s[0]) == -1);

  for (int i = 0; i < (int)(sizeof(s)/sizeof(s[i])); i++) {
    transducers_free_stream(s[i]);
  }

  return 0;
}
