#include<stdio.h>
#include<stdlib.h>
#include<errno.h>
#include<string.h>

FILE* file_exists(char*);


int main (int argc, char* argv[])
{
  
  FILE* file;  
  
  if (argc != 2) {
    fprintf(stderr, "Usage: the function takes exactly one arguments\n");
    return EXIT_FAILURE;
  }

  file = file_exists(argv[1]);
  
  if (file != NULL)
    {
      printf("The file exists. Printing each byte:\n");          
      unsigned int curr_byteread;
      
      while (!feof(file)) {
	fread(&curr_byteread, 1, 1, file);	
	printf("%02x\n", curr_byteread);		
      }
            
      fclose(file);      
      return EXIT_SUCCESS;
      
    }
  else
    {
      fprintf(stderr, "ERROR: %s\n", strerror(errno));
      return EXIT_FAILURE;
    }  
}

FILE* file_exists (char* file_name) {
  FILE* f = fopen(file_name, "r");
  return f;
}


