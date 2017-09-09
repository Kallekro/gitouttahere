#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>

enum file_type {
  ASCII,
  DATA,  
  EMPTY,
};

const char* const file_type_strings[] = {
  "ASCII text",
  "data",
  "empty",
};  

int isASCII(char byte);

int getFileSize(FILE* file, unsigned long* size);
  
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

  // We start by assuming the file is ASCII
  enum file_type filetype = ASCII;

  // Get the file size
  unsigned long filesize;
  getFileSize(somefile, &filesize);

  // if filesize is 0, file is empty 
  if (!filesize) {
    filetype = EMPTY;
  }
  else { 
    // declaring char to hold each byte read
    char byte;
    // Read through file to end of file.
    while(!feof(somefile)) {    
      fread(&byte, 1, 1, somefile);
      // Break while loop and set filetype to data if the byte is not ASCII
      if (!isASCII (byte)) {
        filetype = DATA;
        break;        
      }    
    }
  }
  // print the appropriate message
  printf("%s: %s\n", argv[1], file_type_strings[filetype]);    
  exit(EXIT_SUCCESS);
}

int isASCII (char byte) {
  // returns 1 if byte is in ascii range, 0 if not
  return (    (7 <= byte && byte <= 13)
          ||  (27 == byte)
          ||  (32 <= byte && byte <= 126)); 
}

int getFileSize (FILE* file, unsigned long* size) {
  // Get size of file and store in passed by reference size variable
  // setting the cursor to the last byte in the file
  fseek(file, 0L, SEEK_END);
  // reading the cursors byte offset from beginning of file, ie. file size
  *size = ftell(file);  
  // setting cursor to the first byte of the file
  fseek(file, 0L, SEEK_SET);
  return 0;
}
