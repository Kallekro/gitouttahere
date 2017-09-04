#include <stdio.h>

int main (int argc, char* argv[]) {
  if (argc != 2) {
    printf("Wrong number of arguments");
  }
  if (*argv[1] == 'A' || *argv[1] == 'a') {
    printf("No beginning A's allowed");
  }
  else {
    printf(argv[1]);
  }
  return 0;
}
