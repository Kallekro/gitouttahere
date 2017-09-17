#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include<stdbool.h>
#include<limits.h>

// Macros to check how many bytes the utf8 symbols contains.
#define UTF8_CONT(byte) (((byte) & (1u << 7)) && !((byte) & (1u << 6)))
#define UTF8_2B(byte) (((byte) & (3u << 6)) == (3u<<6) && !((byte) & (1u << 5)))
#define UTF8_3B(byte) (((byte) & (7u << 5)) == (7u<<5) && !((byte) & (1u << 4)))
#define UTF8_4B(byte) (((byte) & (15u << 4)) == (15u<<4) && !((byte) & (1u << 3)))

enum file_type {
  ASCII,  
  ISO8859,
  UTF8,
  DATA,
  BIGUTF16,
  LITTLEUTF16,
  EMPTY,
};

const char* const file_type_strings[] = {
  "ASCII text",    
  "ISO-8859 text",
  "UTF-8 Unicode text",
  "data",
  "Big-endian UTF-16 Unicode text",
  "Little-endian UTF-16 Unicode text",   
  "empty",
};  

unsigned int max_path_length = 0;

int findFileType(char*);

enum file_type changeFileType(enum file_type, enum file_type);

int isStartOfUnicode(unsigned char);

int isASCII(unsigned char);

int getFileSize(FILE*, long*);

int printError(char*);

int main (int argc, char* argv[]) {    
  // Check correct number of arguments      
  if (argc < 2) {
    fprintf(stderr, "usage: %s path\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  // Get the longest path length to use when printing
  for (int i=1; i<argc;i++) {
    if (strlen(argv[i]) > max_path_length)
      max_path_length = strlen(argv[i]); 
  }
  
  // Print the type of each file argument
  for (int i=1; i<argc;i++) {
    findFileType(argv[i]);     
  }
  
  exit(EXIT_SUCCESS);
}

int findFileType(char* file_name) {
  // Finds the type of file at path file_name and prints it to stdout  
  
  FILE* somefile = fopen(file_name, "r");
  if (!somefile) {
    // If file opening failed      
    printError(file_name);
    return 1;
  }

  // Get the file size
  long filesize;
  if (getFileSize(somefile, &filesize) != 0){
    printError(file_name);
    return 1;
  }  

  // We start by assuming the file is ASCII
  enum file_type filetype = ASCII;
  enum file_type alt_filetype = ASCII;
  
  int check_utf8_bytes = 0;

  // if filesize is 0, file is empty 
  if (!filesize) {
    filetype = EMPTY;
  }
  else { 
    // declaring char to hold each byte read
    unsigned char byte;
    unsigned char first_two_bytes[2];
    int byte_counter = 0;
    // Read to end of file.
    while(1) {          
      // read 1 byte
      fread(&byte, 1, 1, somefile);
      // Handle any error fread might have thrown
      if (ferror(somefile) != 0) {
	      printError(file_name);
        return 1;
      }
      // break loop if stream reached end of file 
      if (feof(somefile)) break;
      
      if (byte_counter < 2) {
	first_two_bytes[byte_counter++] = byte;	
      }

      if (byte_counter == 2) {
	if (first_two_bytes[0] == 255 && first_two_bytes[1] == 254) { 
	  filetype = LITTLEUTF16;
	  break;
	}
	else if (first_two_bytes[0] == 254 && first_two_bytes[1] == 255) {
	  filetype = BIGUTF16;
	  break;
	}
	byte_counter++;
      }
      
      
      // If not currently checking for unicode continue bytes, check if start byte
      if (check_utf8_bytes == 0) {
        check_utf8_bytes = isStartOfUnicode(byte);
      }
      else {
        // Check if byte is continued unicode byte
        if (UTF8_CONT(byte)) {
          // Decrement amount of bytes to check
          check_utf8_bytes--;
          // If reached zero bytes to check, we read a unicode character and we set the filetype to unicode
          if (check_utf8_bytes == 0) {
            filetype = changeFileType(filetype, UTF8);
            // Since we know the identity of the current byte we continue to next loop iteration
            continue;
          }
        }
        else {
          // If the byte was not a continue byte check if start byte
          check_utf8_bytes = isStartOfUnicode(byte);
          // Since the byte was not unicode, some of the previous read bytes might have been exotic bytes
          // Therefore set the filetype to be alt_filetype which will have been set if we reached an exotic byte while searching for unicode continue bytes
          // However don't overwrite UTF8 with ISO8859 since it is contained
          // alt_filetype will also never be ASCII	  
	        filetype = changeFileType(filetype, alt_filetype);
        }
      }
      
      if (!isASCII (byte)) {
        // Check if in ISO8859 range
        if (byte >= 160) {
          // If checking for continue bytes only set alt_filetype, not overwriting filetype
          if (check_utf8_bytes != 0) {
            alt_filetype = ISO8859;
          }
          else { 
            // Don't overwrite UTF8 with ISO8859 because it is contained	    
	    filetype = changeFileType(filetype, ISO8859);	             
	  }
        }
        else {
          // Same as before, only set alt_filetype if checking for continue bytes
          if (check_utf8_bytes != 0) alt_filetype = DATA;
          else {
            filetype = DATA;
            // We can safely break here, because the data byte cannot be part of a unicode character at this point
            break;
          }
        }			
      }
    }
    // if we're checking for cont bytes and the loop breaks, we use the alt-filetype as the filetype.
    if (check_utf8_bytes != 0) {
      filetype = changeFileType(filetype, alt_filetype);
    }
  }
  // close the file and handle possible errors
  if(fclose(somefile) != 0) {
    printError(file_name);
  }
  
  printf("%s:%*s%s\n", file_name, (int)(max_path_length - strlen(file_name)) + 1, " ", file_type_strings[filetype]);
  return 0;
}

enum file_type changeFileType (enum file_type ft_curr, enum file_type ft_new) {
  return (ft_curr > ft_new) ? ft_curr : ft_new;
}


int isStartOfUnicode (unsigned char byte) {
  if (UTF8_2B(byte)) {
    return 1;
  }
  else if (UTF8_3B(byte)) {
    return 2;
  }
  else if (UTF8_4B(byte)) {
    return 3;
  }
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

int printError(char* file_name) {
  fprintf(stderr, "%s:%*scannot determine (%s)\n", file_name, (int) (max_path_length - strlen(file_name)) + 1, " ", strerror(errno));
  return 0;
}


