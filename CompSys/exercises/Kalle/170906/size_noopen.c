#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>

int main (int argc, char* argv[]) {
  if (argc <= 1 || argc >= 3) {
    printf("usage: %s path\n", argv[0]);
    exit(EXIT_FAILURE);
  }
  
  struct stat file_stat;

  if (stat(argv[1], &file_stat) == -1) {
    fprintf(stderr, "%s\n", strerror(errno));
  }
  
  long long unsigned size = file_stat.st_size;
  printf("%llu\n", size);

  exit(EXIT_SUCCESS); 
}
