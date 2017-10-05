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
int** initialize(int);
void move_board(int**,int,int);
void move_cell(int**,int,  int, int, enum moveType);
void feed_board(int**, int);
void free_memory(int**, int);
void free_all(int** arr,int dim);
void fill_with_val(int**, int, int);

int** mask;

int** initialize_logic(int dim);


int** initialize_logic (int dim) {
  time_t t;
  srand((unsigned) time(&t));
  
  int** arr = initialize(dim);
  fill_with_val(arr, dim,0);
  feed_board(arr, dim);
  feed_board(arr,dim);
  
  mask = initialize(dim);
  return arr;
}


void move_board(int** arr, int dim ,int move) {
  int start_col = 0;
  int start_row = 0;

  fill_with_val(mask, dim, 0);
  
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
    for(row=start_row;row>=0;row--){
      for(col=start_col; col<dim;col++){
	//printf("%d%d ", row, col);
	if (arr[row][col]!=0){
	  move_cell(arr,dim, row, col, Down);
	}
      }
    }    
  } else if (move==2) { // Right
    for (col=start_col; col>= 0; col--){
      for(row=start_row;row<dim;row++){
	if (arr[row][col]!=0){
	  //printf ("%d", arr[row][col]);
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
  
  if (move == Up) {
    i = row;
  }
  if (move == Down ) {
    i = row; 
  }

  if (move == Right) {
    i = col;
  }
  if (move == Left) {
    i = col;
  }
  while (!done_moving) {

    if (move == Up) {
      
      if (arr[i-1][col] == 0){
	arr[i-1][col] = curr;
	arr[i][col] = 0;
      }
      else if (arr[i-1][col] == curr && mask[i-1][col] != 1) {
	arr[i-1][col] += curr;
	arr[i][col] = 0;
	done_moving = true;
	mask[i-1][col] = 1;
      }
      else {
	done_moving = true;
      }
      
      i--;
      if (i < 1) {
	done_moving = true;
      }      
    }
    
    else if (move == Down) {
      if(arr[i+1][col] == 0 ) {
	arr[i+1][col] = curr;
	arr[i][col] = 0;
      }
      else if (arr[i+1][col] == curr && mask[i+1][col] != 1) {
	arr[i+1][col] += curr;
	arr[i][col] = 0;
	done_moving = true;
	mask[i+1][col] =1;
      }
      else {
	done_moving = true;
      }
      
      i++;
      if (i > dim-2) {
	done_moving = true;
      }
      
    }

    else if (move == Right) {
      if(arr[row][i+1] == 0) {
	arr[row][i+1] = curr;
	arr[row][i] = 0;
      }
      else if (arr[row][i+1] == curr && mask[row][i+1] != 1) {
	arr[row][i+1] += curr;
	arr[row][i] = 0;
	done_moving = true;
	mask[row][i+1] = 1;
      }
      else {
	done_moving =true;
      }
      i++;      
      if (i > dim-2) {
	done_moving = true;
      }      
    }

    else if (move == Left) {
      if(arr[row][i-1] == 0) {
	arr[row][i-1] = curr;
	arr[row][i] = 0;
      }
      else if (arr[row][i-1] == curr && mask[row][i-1] != 1) {
	arr[row][i-1] += curr;
	arr[row][i] = 0;
	done_moving = true;
	mask[row][i-1] = 1;
      }
      else {
	done_moving = true;
      }
      i--;
      if(i < 1) {
	done_moving = true;
      }
    }
  }
}



void feed_board(int** arr, int dim) {
  bool inserted = false;

  int feeds[2] = {2,4};
  int col_guess, row_guess;
  int rndIndex = randInt(0,2);

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

int** initialize(int dim) {
  // allocate memory for array
  int** arr;
  arr = malloc(dim * sizeof(*arr));
  for (int i = 0;i<dim;i++){
    arr[i] = malloc(dim * sizeof(*arr));
  }
  return arr;
}
void fill_with_val(int** arr, int dim, int val) {  
  int col, row;
  for (col=0; col<dim; col++){
    for (row=0; row<dim; row++) {
      arr[col][row] = val;
    }
  }
}

void free_all(int** arr, int dim) {
  free_memory(arr, dim);
  free_memory(mask, dim);
}

void free_memory(int** arr, int dim) {
  // Free memory for array.
  for (int i=0; i<dim; i++) {
    free(arr[i]);
  }
  free(arr);
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

