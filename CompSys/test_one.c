#include <stdio.h>
#include <stdlib.h>
#include <string.h>

short test_one(unsigned short x) {
  short val = 1;
  while (x) {
    val ^= x;
    x >>= 1;
  }
  return val & 1;
}

int main (int argc, char* argv[]) {
  printf("%d\n", test_one(atoi(argv[1])));
  return 0;
}

