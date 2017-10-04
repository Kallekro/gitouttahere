#include<string.h>
#include<stdlib.h>
#include<stdio.h>
#include<time.h>
#include<stdbool.h>
#include<ncurses.h>
#include<curses.h>
#include<stdbool.h>


enum moveType {Up, Down, Left, Right};

int randInt(int, int);
void printArray(int**, int);
void initialize_with_val(int**, int, int);
void move_board(int**,int,int);
void move_cell(int**,int,  int, int, enum moveType);
void feed_board(int**, int);

int main (int argc, char* argv[]) {

  if (argc != 2) {
    printf("usage: dim");
    exit(EXIT_SUCCESS);
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
  
  int movArr[1] = {0};
  
  feed_board(arr, dim);feed_board(arr, dim);
  printArray(arr, dim);

  printf ("\n\n");
  
  move_board(arr, dim, movArr[0]);

  printArray(arr, dim);
  
  // Free memory for array.
  for (i=0; i<dim; i++) {
    free(arr[i]);
  }
  free(arr);

  exit(EXIT_SUCCESS);
}

void move_board(int** arr, int dim ,int move) {
  int start_col = 0;
  int start_row = 0;
  
  switch(move){
  case 0:
    start_row = 1; // up
    break;
  case 1:
    start_row = dim-2;// down
    break;
  case 2:
    start_col = dim-2; // right 
    break;
  case 3:
    start_col = 1; // left
    break;
  }
  
  int col, row;
  
  if (move==0) { // UP 
    for (row=start_row; row < dim;row++) { 
      for (col=start_col; col < dim;col++) {	
	if (arr[row][col]!=0){
	  move_cell(arr,dim, row, col, Up);
	}	
      }
    }  
  } else if(move==1) { // Down
    for(row=start_row;row>-1;row--){
      for(col=start_col; col<dim;col++){
	if (arr[row][col]!=0){
	  move_cell(arr,dim, row, col, Down);
	}
      }
    }    
  } else if (move==2) { // Right
    for (col=start_col; col>-1; col--){
      for(row=start_row;row<dim;row++){
	if (arr[row][col]!=0){
	  move_cell(arr,dim, row, col, Right);
	}
      }
    }
  } else { // Left
    for (col=start_col; col<dim; col++){
      for(row=start_row;row<dim;row++){
	if (arr[row][col]!=0){
	  move_cell(arr,dim, row, col, Left);      
	}
      }
    }    
  }
}



void move_cell(int** arr,int dim, int row, int col, enum moveType move) {

  int curr = arr[row][col];
  bool done_moving = false;

  int i;
  
  if (move==Up || move==Left) {
    i = 0;
  } else {
    if (move == Right) {
      i = col-1;
    } else { i = row-1;}
  }
  
  while (!done_moving || i < dim-1) {

    if (move == Up) {
      printf("LOLOL");
      if (arr[row+i-1][col] == 0){	
	arr[row+i-1][col] = curr;
	arr[row+i][col] = 0;
      }

      else if (arr[row-1][col] == curr) {
	arr[row+i-1][col] += curr;
	arr[row+i][col] = 0;
	done_moving = true;
      }
      
      i++;
    }

    else if (move == Down) {
      if(arr[row+i+1][col] == 0) {
	arr[row+i+1][col] = curr;
	arr[row+i][col] = 0;
      }
      else if (arr[row+1+i][col] == curr) {
	arr[row+1+i][col] += curr;
	arr[row+i][col] = 0;
	done_moving = true;
      }
      i--;
    }

    else if (move == Right) {
      if(arr[row][col+1] == 0) {
	arr[row][col+1] = curr;
	arr[row][col] = 0;
      }
      else if (arr[row][col+1] == curr) {
	arr[row][col+1] += curr;
	arr[row][col] = 0;
      }
    }

    else if (move == Left) {
      if(arr[row][col-1] == 0) {
	arr[row][col-1] = curr;
	arr[row][col] = 0;
      }
      else if (arr[row][col-1] == curr) {
	arr[row][col-1] += curr;
	arr[row][col-1] = 0;
      }
    }
    
  }
}



void feed_board(int** arr, int dim) {
  bool inserted = false;

  int feeds[2] = {2,4};
  int col_guess, row_guess;
  int rndIndex = randInt(0,1);

  while (!inserted) {
    col_guess = randInt(0,dim);row_guess = randInt(0,dim);
    
    if (arr[row_guess][col_guess] == 0) {
      arr[row_guess][col_guess] = feeds[rndIndex];
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

      printf("%d ", arr[col][row]);

      if (row == dim-1) {printf (" |");}
    }
    printf("\n");
  }
}

