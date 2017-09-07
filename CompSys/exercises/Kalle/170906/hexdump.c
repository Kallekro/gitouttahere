#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

int read16bytes(FILE* file, unsigned int* buffer, int size);

int flushArr(unsigned int* arr, int size);

int printBytes(unsigned int* arr, int size);

int printChars(unsigned int* arr, int size);

int main (int argc, char* argv[]) {
  // Check correct number of arguments      
  if (argc <= 1 || argc >= 3) {
    fprintf(stderr, "usage: %s path\n", argv[0]);
    exit(EXIT_FAILURE);
  }
  // Open file
  FILE* somefile = fopen(argv[1], "r");
  if (!somefile) {
    // If file opening failed      
    fprintf(stderr, "%s\n", strerror(errno));
    exit(EXIT_FAILURE);
  }
  // If file opening was succesful
  // Initialize bytes buffer
  unsigned int bytes[17];
  int size = sizeof(bytes)/sizeof(bytes[0]);
  // While not reached end of file
  while (!feof(somefile)) {
    // Flush the buffer effectively setting each byte to 0      
    flushArr(&bytes[0], size);
    // Print offset
    printf("%08lx  ", ftell(somefile)); 
    // Read 16 bytes into the buffer
    read16bytes(somefile, &bytes[0], size);
    // Print the buffer as bytes
    printBytes(&bytes[0], size);
    // Print the buffer as chars
    printChars(&bytes[0], size); 
  }
  fclose(somefile);
  exit(EXIT_SUCCESS); 
}

int flushArr(unsigned int* arr, int size) {
  for (int i=0; i<size; i++) {
    arr[i] = 0;
  }
  return 0;
}

int read16bytes(FILE* file, unsigned int* buffer, int size) {
  for (int i=0; i<size; i++) {
    fread(&buffer[i], 1, 1, file);      
    if (feof(file)) break;
  }  
  return 0;
}

int printBytes(unsigned int* arr, int size) {
  for (int i=0; i<size; i++) {
    if (arr[i] == 0) printf("   ");        
    else printf("%02x ", arr[i]);
    if (i%8==0 && i!=0) {
      printf(" ");
    }
  }  
  return 0;
} 

int printChars(unsigned int* arr, int size) {
  printf("|");
  for (int i=0; i<size; i++) {
    char c = (char) arr[i];      
    switch (c) {
      case '\n':
        printf(".");        
        break;
      case '\t':
        printf("_");  
        break;
      case 0:
        break;  
      default:
        printf("%c", arr[i]);   
    }
  }  
  printf("|\n");
  return 0;
}
