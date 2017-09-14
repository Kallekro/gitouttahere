#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main (int argc, char* argv[]) {
  if (argc != 3) {
    fprintf(stderr, "usage: %s A B", argv[0]);
    exit(EXIT_FAILURE);
  }
  char* e;
  unsigned long long int a = strtoull(argv[1], &e, 0);
  unsigned long long int b = strtoull(argv[2], &e, 0);
  unsigned long long int result = 0LL;
  int carry = 0;
  for (int i=0; i<64; i++) {
    if (a & 1 && b & 1) {
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
    a >>= 1;
    b >>= 1;
    if (!a && !b && !carry) break;
  }
  printf("Sum: %llu\n", result);
  if (carry) {
    printf("Integer overflow detected!\n");
  }
  exit(EXIT_SUCCESS);
}
