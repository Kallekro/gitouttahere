#include<stdlib.h>
#include<stdio.h>
#include<stdbool.h>

int randInt(int, int);
void printArray(int**, int);
int** initialize(int);
void free_memory(int** arr,int dim);
void fill_with_val(int**, int, int);
int** copy_array(int** arr, int);
bool is_filled(int**arr, int dim);
 
// generate a randval between lo and hi+1
int randInt(int lo, int hi) {  
  return ((rand() % hi) + lo);
}

// initialize 2d array of dimension, dim.
int** initialize(int dim) {
  // allocate memory for array
  int** arr;
  arr = malloc(dim * sizeof(*arr));
  for (int i = 0;i<dim;i++){
    arr[i] = malloc(dim * sizeof(*arr));
  }
  return arr;
}

/// free memory holding array
void free_memory(int** arr, int dim) {
  // Free memory for array.
  for (int i=0; i<dim; i++) {
    free(arr[i]);
  }
  free(arr);
}

/// A simple function to print an array to stdout
void printArray(int** arr, int dim) {
  int col, row;
  for (col=0; col<dim; col++){
    for (row=0; row<dim; row++) {
      if (row == 0) {printf ("| ");}
      
      printf("%d ", arr[col][row]);
      
      if (row == dim-1) {printf (" |");}
    }
    printf("\n");
  }
}

void fill_with_val(int** arr, int dim, int val) {  
  int col, row;
  for (col=0; col<dim; col++){
    for (row=0; row<dim; row++) {
      arr[col][row] = val;
    }
  }
}

int** copy_array(int** arr, int dim) {
  int** cp_arr = initialize(dim);
  int i,j;
  for (i=0;i<dim;i++){
    for (j=0;j<dim;j++){
      cp_arr[i][j] = arr[i][j];
    }
  }
  return cp_arr;
}

bool is_filled(int** arr, int dim) {
  bool full = true;
  int i,j;

  for(i=0; i<dim; i++) {
    for(j=0; j<dim; j++){
      if(arr[i][j] == 0) {
     	  full = false;
      }
    }
    if(!full) {
      break;
    }
  }
  return full;
}

int max_uint(int** arr, int dim) {
  int max = -1;
  for (int i = 0; i<dim; i++) {
    for (int j = 0; j<dim; j++) {
      if (arr[i][j] > max) {
        max = arr[i][j];
      }
    }
  }
  return max;
}

