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

  file = fopen(argv[1], "r");

  if (file != NULL)
    {
      long offset = ftell(file);      
      int curr_16_byte[17];
      int curr_byte;      
      while (!feof(file)) {		
	printf("%08lx  ", offset);	
	for (int byte=0; byte < 16; byte++)
	  {
	    fread(&curr_byte, 1, 1, file);			
	    if (byte==8) {printf(" ");}
	    curr_16_byte[byte] = curr_byte;
	    printf("%02x ", curr_byte);	    
	  }				
	printf(" |");	
	for (int byte=0; byte < 16; byte++) {
	  int cb = curr_16_byte[byte];	  
	  switch (cb)
	    {
	    case '\n':
	      printf(".");
	      break;
	    case '\t':
	      printf("-");
	      break;	      
	    default:
	      printf("%c", cb);
	    }	  	  
	}
	printf("|");      
	offset = ftell(file);	
	printf("\n");	
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




