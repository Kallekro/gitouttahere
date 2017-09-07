#include<stdio.h>
#include<stdlib.h>


int main (int argc, char* argv[])
{
  
  if (argc != 2) {
    fprintf(stderr, "Usage: the function takes exactly two arguments");
  }
  else {
    fprintf(stdout, "Hello %s\n", argv[1]);
  }

}
  


