#include <stdio.h>

int main (int argc, char* argv[]) {
  if (argc != 2) {
    printf("Wrong number of arguments");      
    return 1;      
  }  
  for (int i=0; i<2; i++) {
    printf("%s\n", argv[1]);
  }
  return 0;
}
