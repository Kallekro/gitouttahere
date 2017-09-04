#include<stdio.h>
#include<string.h>
#include<stdbool.h>

int main(int argc, char* argv[]) {
  char *input_filename;     

  if (argc != 2) {puts("Takes exactly one argument");}

  else {
    input_filename = argv[1];
    
    FILE *file_obj = fopen(input_filename, "r");

    char buffer[256];    
    char c;
    int count = 0;

    while (!feof(file_obj))
    {      
      c = fgetc(file_obj);
      buffer[count] = c;
      count++;
    }    
    buffer[count] = '\0';
    fclose(file_obj);
    printf("%s\n", buffer);    
  }  
}

