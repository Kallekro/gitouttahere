#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

bool isEqual (char* str1, char* str2) {
  while (*str1 != '\0') {
    if (*str1 != *str2) return false;
    str1++;
    str2++;
  }
  return true;
}

int main (int argc, char* argv[]) {
  int n;      
  if (argc < 2 || argc > 4) {
    printf("Wrong number of arguments");
    exit(EXIT_FAILURE);
  }
  else if (argc == 3) {
    n = atoi(argv[2]);      
  }
  else {
    n = 1;
  }
  if (argc == 4 && isEqual(argv[3], "-dec")) {
    n *= -1;      
  }
  for (; *argv[1] != '\0'; argv[1]++) {
    char c = *argv[1];      
    printf("%c", c + n); 
  }

}
