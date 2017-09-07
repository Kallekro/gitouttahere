#include <stdio.h>
#include <stdlib.h>

int main (int argc, char* argv[]) {
  (void)argv;      
  if (argc == 0 || argc >= 2) exit(EXIT_FAILURE);
  exit(EXIT_SUCCESS); 
}
