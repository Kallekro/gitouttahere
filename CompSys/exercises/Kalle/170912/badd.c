#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main (int argc, char* argv[]) {
  if (argc != 3) {
    fprintf(stderr, "usage: %s A B", argv[0]);
    exit(EXIT_FAILURE);
  }
  char* e;
  unsigned long long int a = strtoll(argv[1], &e, 0);
  unsigned long long int b = strtoll(argv[2], &e, 0);
  unsigned long long int result = 0;
  int carry = 0;
  int i = 0; 
  while (a || b) {
    if (a & b & 1) {
      if (carry == 1) {
        result ^= (1llu << i);
      }
      else {
        carry = 1;
      }
    }
    else if (a & 1 || b & 1) {
      if (!carry) result ^= (1llu << i);
    }
    else {
      if (carry == 1) {
        result ^= (1llu<< i);
        carry = 0;
      }
    }
    i++;
    a >>= 1;
    b >>= 1;
  }
  printf("Sum: %llu\n", result);
  if (carry) {
    printf("Integer overflow detected!\n");
    printf("Sum with extra bit (if under ULL_MAX): %llu\n", (1llu << i) | result); 
  }
  exit(EXIT_SUCCESS);
}
