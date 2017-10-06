#include<time.h>
#include<stdlib.h>
#include<stdbool.h>
#include"support.h"

enum moveType {Up, Down, Left, Right};
int move_board(int**,int,int, bool feed);
int move_cell(int**,int,  int, int, enum moveType);
void feed_board(int**, int);
void free_all(int** arr,int dim);

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


int move_board(int** arr, int dim ,int move, bool feed) {
  int start_col = 0;
  int start_row = 0;

  int moveScore = -1;
  
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
	        int res = move_cell(arr,dim, row, col, Up);
	        if(res == -1 && moveScore == -1) {	    
	        }  else if (res != -1 && moveScore == -1) {
	          moveScore = res;
	        } else if (res != -1 && moveScore != -1) {
	          moveScore += res;
	        }

	      }	
      }
    }
    
  } else if(move==1) { // Down
    for(row=start_row;row>=0;row--){
      for(col=start_col; col<dim;col++){
        if (arr[row][col]!=0) {
	        int res = move_cell(arr,dim, row, col, Down);
	        if(res == -1 && moveScore == -1) {	  
	        } else if (res != -1 && moveScore == -1) {
	          moveScore = res;
	        } else if (res != -1 && moveScore != -1) {
	          moveScore += res;
	        }

        }
      }
    }    
  } else if (move==2) { // Right
    for (col=start_col; col>= 0; col--){
      for(row=start_row;row<dim;row++){
        if (arr[row][col]!=0){
          int res = move_cell(arr,dim, row, col, Right);
          if(res == -1 && moveScore == -1) {
          
          } else if (res != -1 && moveScore == -1) {
            moveScore = res;
          } else if (res != -1 && moveScore != -1){
            moveScore += res;
          }
        
        }
      }
    }
  } else { // Left
    for (col=start_col; col<dim; col++){
      for(row=start_row;row<dim;row++){
        if (arr[row][col]!=0){
          
          int res = move_cell(arr,dim, row, col, Left);
          if(res == -1 && moveScore == -1) {
          
          } else if (res != -1 && moveScore == -1) {
            moveScore = res;
          } else if (res != -1 && moveScore != -1) {
            moveScore += res;
          }
        
        }
      }
    }    
  }

  if (moveScore == -1) {

    return 0;
  }

  if (feed) 
    feed_board(arr, dim);

  if (moveScore != 0)
    return moveScore;

  return 0;
}

int move_cell(int** arr,int dim, int row, int col, enum moveType move) {
  int curr = arr[row][col];

  int score = 0;
  bool changed = false;
  
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
        changed = true;
	
      }
      else if (arr[i-1][col] == curr && mask[i-1][col] != 1) {
        arr[i-1][col] += curr;
        arr[i][col] = 0;
        done_moving = true;
        mask[i-1][col] = 1;
        
        changed = true;
        score = arr[i-1][col];
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
        changed = true;
	
      }
      else if (arr[i+1][col] == curr && mask[i+1][col] != 1) {
        arr[i+1][col] += curr;
        arr[i][col] = 0;
        done_moving = true;
        mask[i+1][col] =1;

        changed = true;
        score = arr[i+1][col];
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
        changed = true;
	
      }
      else if (arr[row][i+1] == curr && mask[row][i+1] != 1) {
        arr[row][i+1] += curr;
        arr[row][i] = 0;
        done_moving = true;
        mask[row][i+1] = 1;
        
        changed = true;
        score = arr[row][i+1];
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
        changed = true;
	
      }
      else if (arr[row][i-1] == curr && mask[row][i-1] != 1) {
        arr[row][i-1] += curr;
        arr[row][i] = 0;
        done_moving = true;
        mask[row][i-1] = 1;
        
        changed =true;
        score = arr[row][i-1];
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
  
  if (!changed)
    return -1;

  return score;
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

void free_all(int** arr, int dim) {
  free_memory(arr, dim);
  free_memory(mask, dim);
}


