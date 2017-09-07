#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>

int main (int argc, char* argv[]) {
  if (argc != 2) {
    printf("Wrong number of arguments");
  }
  else {
    FILE* somefile = fopen(argv[1], "r");
    if (!somefile) {
      exit(EXIT_FAILURE);
    }
    else {
      while (!feof(somefile)) {
        char c = fgetc(somefile);      
        if (c == 'A') { 
          printf("No A's allowed");
          exit(EXIT_FAILURE);
        }
        //printf("%c", c); 
      }
      printf("OK");
    }
    fclose(somefile);
  }
  exit(EXIT_SUCCESS);
}
