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
      printf("Could not open and read specified file");      
      exit(EXIT_FAILURE);
    }
    else {
      printf("Succesfully opened and read specified file");      
      exit(EXIT_FAILURE);
    }
    fclose(somefile);
  }
  exit(EXIT_SUCCESS);
  }
