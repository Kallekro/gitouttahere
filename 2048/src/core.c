#include<string.h>
#include<stdlib.h>
#include<stdio.h>
#include<time.h>
#include<bool.h>

enum moveType {Up, Down, Left, Right};

int randInt(int, int);
void printArray(int**, int);
void initialize_with_val(int**, int, int);


int main (int argc, char* argv[]) {

  if (argc != 2) {
    printf("usage: dim");
  }

  time_t t;
  srand((unsigned) time(&t));
  
  int dim = atoi(argv[1]);
  
  int i;
  int** arr;
  
  // allocate memory for array
  arr = malloc(dim * sizeof(*arr));
  for (i = 0;i<dim;i++){
    arr[i] = malloc(dim * sizeof(*arr));
  }

  // initialize game board with zeros.
  initialize_with_val(arr, dim, 0);

  
  printArray(arr, dim);
  
  // Free memory for array.
  for (i=0; i<dim; i++) {
    free(arr[i]);
  }
  free(arr);

  exit(EXIT_SUCCESS);
}

void feed_board(int** arr, int dim) {
  bool inserted = false;

  int feeds[2] = {2,4};
  int col_guess, row_guess;
  int rndIndex = randInt(0,1);

  while (!inserted) {
    col_guess = randInt(0,dim);row_guess = randInt(0,dim);
    
    if (arr[row_guess, col_guess] != 0) {
      arr[row_guess, col_guess] = feeds[rndIndex];
      inserted = true;
    }  
  }
  printf("Inserted the value %d, at index (%d, %d)", feeds[rndIndex], row_guess, col_guess);
}


int randInt(int lo, int hi) {
  return ((rand() % hi) + lo);
}

void initialize_with_val(int** arr, int dim, int val) {
  int col, row;
  for (col=0; col<dim; col++){
    for (row=0; row<dim; row++) {
      arr[col][row] = val;
    }
  }
}


void printArray(int** arr, int dim) {
  int col, row;
  for (col=0; col<dim; col++){
    for (row=0; row<dim; row++) {
      if (row == 0) {printf ("| ");}

      printf(" %d ", arr[col][row]);

      if (row == dim-1) {printf (" |");}
    }
    printf("\n");
  }
}

