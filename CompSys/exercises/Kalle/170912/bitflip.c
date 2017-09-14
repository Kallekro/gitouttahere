#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

int parseIntegerString(char* string, unsigned long long int* val);

int main (int argc, char* argv[]) {
  if (argc != 3) {
    fprintf(stderr, "usage: %s integer index\n", argv[0]);
    exit(EXIT_FAILURE);
  }	

  unsigned long long int val;
  int parse = parseIntegerString(argv[1], &val); 
  if ( parse == 1 ) {
    fprintf(stderr, "Could not parse value argument (Argument is not a 64-bit integer)\n");
    exit(EXIT_FAILURE);
  }
  if ( parse == 2 ) {
    fprintf(stderr, "Error while parsing argument (%s)\n", strerror(errno));
    exit(EXIT_FAILURE);
  }
  
  unsigned long long int index;
  parse = parseIntegerString(argv[2], &index); 
  if ( parse == 1 ) {
    fprintf(stderr, "Could not parse index argument (Argument is not a 64-bit integer)\n"); 
    exit(EXIT_FAILURE);
  }
  if ( parse == 2 ) {
    fprintf(stderr, "Error while parsing argument (%s)\n", strerror(errno));
    exit(EXIT_FAILURE);
  }
  if (index > 63) {
    fprintf(stderr, "Index is not in valid range (0-63)\n");
    exit(EXIT_FAILURE);
  }

  char* num_end = "th";
  switch (index+1) {
    case 1:
      num_end = "st";
      break;
    case 2:
      num_end = "nd";
      break;
    case 3:
      num_end = "rd";
      break;
  }
  printf("Flipped %llu%s bit: %llu\n", index+1, num_end, val ^ (1llu << index));
  exit(EXIT_SUCCESS);
}

int parseIntegerString (char* string, unsigned long long int* val) {
  char* e;
  *val = strtoull(string, &e, 0);

  if (*e != 0) {
    return 1;
  }
  if (errno != 0) {
    return 2;
  }

  return 0;
}
