#include <stdio.h>
#include <stdlib.h>

int main (int argc, char* argv[]) {
  if (argc <= 1 || argc >= 3) {
    printf("usage: %s argument\n", argv[0]);
    exit(EXIT_FAILURE);
  }
  
  printf("Hello, %s!\n", argv[1]);

  exit(EXIT_SUCCESS); 
}
