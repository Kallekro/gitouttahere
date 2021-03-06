#include<stdio.h>
#include<stdlib.h>
#include<errno.h>
#include<string.h>

FILE* file_exists(char*);
int get_sizeof(FILE*);

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
      int size_of_file = get_sizeof(file);
      fprintf(stdout, "The file exists and consist of %d bytes\n", size_of_file);     
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

int get_sizeof (FILE* fp) {
  fseek(fp, 0L, SEEK_END);
  int size = ftell(fp);
  rewind(fp);
  return size;
}

