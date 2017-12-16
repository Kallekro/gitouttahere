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

const char* file_name; 

int isASCII(char byte);

int getFileSize(FILE* file, long* size);

int printError();

int main (int argc, char* argv[]) {    
  // Check correct number of arguments      
  if (argc != 2) {
    fprintf(stderr, "usage: %s path\n", argv[0]);
    exit(EXIT_FAILURE);
  }
  
  file_name = argv[1];
  
  // Open file
  FILE* somefile = fopen(file_name, "r");
  if (!somefile) {
    // If file opening failed      
    printError();
    exit(EXIT_FAILURE);
  }

  // We start by assuming the file is ASCII
  enum file_type filetype = ASCII;

  // Get the file size
  long filesize;
  if (getFileSize(somefile, &filesize) != 0){
    printError();
    exit(EXIT_FAILURE);
  }  

  // if filesize is 0, file is empty 
  if (!filesize) {
    filetype = EMPTY;
  }
  else { 
    // declaring char to hold each byte read
    char byte;
    // Read to end of file.
    while(!feof(somefile)) {    

      // read 1 byte
      fread(&byte, 1, 1, somefile);
      
      if (ferror(somefile) != 0) {
	printError();
	exit(EXIT_FAILURE);
      }
      
      // Break while loop and set filetype to data if the byte is not ASCII
      if (!isASCII (byte)) {
        filetype = DATA;
        break;        
      }    
    }
  }
  // print the appropriate message
  printf("%s: %s\n", file_name, file_type_strings[filetype]);    
  exit(EXIT_SUCCESS);
}

int isASCII (char byte) {
  // returns 1 if byte is in ascii range, 0 if not
  return (    (7 <= byte && byte <= 13)
          ||  (27 == byte)
          ||  (32 <= byte && byte <= 126)); 
}

int getFileSize (FILE* file, long* size) {
  // Get size of file and store in passed by reference size variable
  // setting the cursor to the last byte in the file
  if (fseek(file, 0L, SEEK_END) != 0) return 1;

  // reading the cursors byte offset from beginning of file, ie. file size
  *size = ftell(file);
  if (*size == -1) return 1;
    
  // setting cursor to the first byte of the file
  if (fseek(file, 0L, SEEK_SET) != 0) return 1;

  return 0;
}

int printError() {
  fprintf(stderr, "%s: could not determine (%s)\n", file_name, strerror(errno));
  return 0;
}
