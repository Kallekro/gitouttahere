#include<string.h>
#include<stdlib.h>
#include<stdio.h>
#include<time.h>
#include<stdbool.h>
#include<ncurses.h>

enum moveType {Up, Down, Left, Right};

int randInt(int, int);
void printArray(int**, int);
void initialize_with_val(int**, int, int);
void feed_board(int**, int dim);

int main (int argc, char* argv[]) {

  if (argc != 2) {
    printf("usage: dim");
  }
    
  time_t t;
  srand((unsigned) time(&t));
  
  int dim = atoi(argv[1]);
  
  int i;
  int** arr;

  //enum moveType keyPress;
  
  // allocate memory for array
  arr = malloc(dim * sizeof(*arr));
  for (i = 0;i<dim;i++){
    arr[i] = malloc(dim * sizeof(*arr));
  }

  // initialize game board with zeros and feed it twice.
  initialize_with_val(arr, dim, 0);
  feed_board(arr, dim); feed_board(arr, dim);
  printArray(arr, dim);
  
  bool game_running = true;

  
  while (game_running) { 

    char c = 0 ;
    c = getch();
    printf("%c", c);
  }
  
  // Free memory for array.
  for (i=0; i<dim; i++) {
    free(arr[i]);
  }
  free(arr);

  exit(EXIT_SUCCESS);
}



void feed_board(int** arr, int dim) {
  int feed[2] = {2,4};
  bool inserted = false;
  int col_guess, row_guess;
  int rnd_feedval = randInt(0,2);

  while (!inserted) {
    col_guess = randInt(0,dim);
    row_guess = randInt(0,dim);
    
    if (arr[row_guess][col_guess] == 0) {
      arr[row_guess][col_guess] = feed[rnd_feedval];
      inserted = true;
    }  
  }
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

