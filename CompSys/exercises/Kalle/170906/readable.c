#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

int main (int argc, char* argv[]) {
  if (argc <= 1 || argc >= 3) {
    printf("usage: %s path\n", argv[0]);
    exit(EXIT_FAILURE);
  }
  FILE* somefile = fopen(argv[1], "r");
  if (!somefile) {
    fprintf(stderr, "%s\n", strerror(errno));
    exit(EXIT_FAILURE);
  }
  fprintf(stdout, "Yes");
  exit(EXIT_SUCCESS); 
}
