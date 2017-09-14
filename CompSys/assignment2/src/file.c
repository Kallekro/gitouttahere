#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include<stdbool.h>
#include<limits.h>
enum file_type {
  ASCII,
  DATA,  
  EMPTY,
  ISO8859,
  UTF8,
  BIGUTF16,
  LITTLEUTF16
};

const char* const file_type_strings[] = {
  "ASCII text",
  "data",
  "empty",
  "ISO-8859 text",
  "UTF-8 Unicode text",
  "Big-endian UTF-16 text",
  "Little-endian UTF-16 text"
};  

const char* file_name; 

int findFileType(char*, unsigned int);

int isASCII( unsigned char );

int getFileSize(FILE*, long*);

int printError();

int main (int argc, char* argv[]) {    
  // Check correct number of arguments      
  if (argc < 2) {
    fprintf(stderr, "usage: %s path\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  unsigned int max_path_len=0;  

  for (int i=1; i<argc;i++) {
    if (strlen(argv[i]) > max_path_len)
      max_path_len = strlen(argv[i]); 
  }

  
  for (int i=1; i<argc;i++) {
    findFileType(argv[i], max_path_len);     
  }
  
  
  exit(EXIT_SUCCESS);
}

int findFileType(char* file_name, unsigned int max_path_len) {

  enum file_type filetype = ASCII;
  
  FILE* somefile = fopen(file_name, "r");
  if (!somefile) {
    // If file opening failed      
    printError();
    exit(EXIT_FAILURE);
  }
  // We start by assuming the file is ASCII
  
  // Get the file size
  long filesize;
  if (getFileSize(somefile, &filesize) != 0){
    printError();
    exit(EXIT_FAILURE);
  }  

  enum file_type alt_filetype = ASCII;
  int check_utf8_bytes = 0;
  // if filesize is 0, file is empty 
  if (!filesize) {
    filetype = EMPTY;
  }
  else { 
    // declaring char to hold each byte read
    unsigned char cbyte;
    // Read to end of file.
    while(true) {          
      // read 1 byte
      fread(&cbyte, 1, 1, somefile);
      int byte = (int) cbyte;
      if (feof(somefile)) break;      

      if (ferror(somefile) != 0) {
	printError();
	exit(EXIT_FAILURE);
      }
	
      int byteIsUTF8 = false;
      if (check_utf8_bytes == 0) {
        if ((byte & 110u << 5)) {
          check_utf8_bytes = 1;
        }
        else if ((byte & 1110u << 4)) {
          check_utf8_bytes = 2;
        }
        else if ((byte & 11110u << 3)) {
          check_utf8_bytes = 3;
        }
	//else if ((byte & 1u << 7)) {
	//  filetype = DATA;
	//  break;
	//}
      }
      else {
        if ((byte & 10u << 6)) {
          check_utf8_bytes--;
          if (check_utf8_bytes == 0) {
            filetype = UTF8;
            byteIsUTF8 = true;
            //break;
          }
        }
        else {
          check_utf8_bytes = 0;
	  if ((filetype != UTF8 && alt_filetype == ISO8859) || alt_filetype == DATA)
            filetype = alt_filetype;
        }
      }
      
      if (!byteIsUTF8) {
        // Break while loop and set filetype to data if the byte is not ASCII
        if (!isASCII (byte)) {
          if ((byte >= 160 && byte <= 255)) { // || (int)byte == 255) {
            if (check_utf8_bytes != 0) alt_filetype = ISO8859;
            else { 
	      if (filetype != UTF8)
	      	filetype = ISO8859;	  
	    }
          }
          else {
            if (check_utf8_bytes != 0) alt_filetype = DATA;
            else {
              filetype = DATA;
              //break;
            }
          }			
        }
      }
    }
  }
  
  printf("%s:%*s%s\n", file_name, (int)(max_path_len - strlen(file_name)) + 1, " ", file_type_strings[filetype]);
  return 0;
}

int isASCII(unsigned char byte) {
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
