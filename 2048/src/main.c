#include "graphics.h"

#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

int randInt(int lo, int hi) {
  return ((rand() % hi) + lo);
}

int main () {
  time_t t;
  srand((unsigned) time(&t));

  int dim = 4;
  InitializeGame(dim);

  int ch;

  ch = getch();
  if (ch == KEY_F(1)) {
    printw("F1 Key pressed\n");
  } 
  else {
    switch (ch) {
      case 10: // Enter key
        break;
      case KEY_LEFT :
        break;
      case KEY_RIGHT :  
        break;
      case KEY_UP :
        break;
      case KEY_DOWN:
        break;
      default:  
        break;
    }
  }
  

  
  int** arr;
  arr = malloc(sizeof(*arr) * dim);

  int numbers[6] = {0, 2, 4, 8, 16, 32};

  for (int i=0; i<dim; i++) {
    arr[i] = malloc(sizeof(int) * dim);

    for (int j=0; j<dim; j++) {
      arr[i][j] = numbers[randInt(0, 6)];
    }
  }

  //create_new_window(10, 10, 10, 10);
  PrintArray(arr);
  refresh();
  getch();

  for (int i=0; i<dim; i++) {
    free(arr[i]);
  }
  free(arr);
  endwin();

  return 0;
}
