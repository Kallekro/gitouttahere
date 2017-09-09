#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>

int main (int argc, char* argv[]) {    
  // Check correct number of arguments      
  if (argc != 2) {
    fprintf(stderr, "usage: %s path\n", argv[0]);
    exit(EXIT_FAILURE);
  }
  // Open file
  FILE* somefile = fopen(argv[1], "r");
  if (!somefile) {
    // If file opening failed      
    fprintf(stderr, "%s: could not determine (%s)\n", argv[1], strerror(errno));
    exit(EXIT_FAILURE);
  }

  // declaring  to hold each byte read
  unsigned int byte;
  // setting the cursor to the last byte in the file
  fseek(somefile, 0L, SEEK_END);
  // reading the cursors offset, ie.
  unsigned long sizeof_file = ftell(somefile);  
  // setting cursor to the first byte of the file
  fseek(somefile, 0L, SEEK_SET);

  // if the file is 0 bytes big
  if (!sizeof_file) {
    printf("%s: empty\n", argv[1]);
    exit(EXIT_SUCCESS);
  }
  
  // Read through file to end of file.
  while(!feof(somefile)) {    
    fread(&byte, 1, 1, somefile);
    // Stop if the the current byte is not ascii
    if(  !((0x07 <= byte &&  byte <= 0x0D)
	  || (0x1B == byte)
	  || (0x20 <= byte && byte <= 0x7E))) {
      
      printf("%s: data\n", argv[1]);                  
      exit(EXIT_SUCCESS);
    }    
  }
  
  // If no, non-ascii value encounterd, the file is an ascii file
  printf("%s: ASCII text\n", argv[1]);    
  exit(EXIT_SUCCESS);
}



