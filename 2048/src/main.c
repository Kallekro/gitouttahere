#include "graphics.h"


#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


int main () {
  time_t t;
  srand((unsigned) time(&t));

  InitializeGame();

  int max_y, max_x; 
  getmaxyx(stdscr, max_y, max_x);

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
  

  int dim = 4;
  
  int** arr;
  arr = malloc(sizeof(*arr) * dim);

  int numbers[6] = {0, 2, 4, 8, 16, 32};

  for (int i=0; i<dim; i++) {
    arr[i] = malloc(sizeof(int) * dim);

    for (int j=0; j<dim; j++) {
      arr[i][j] = numbers[randInt(0, 6)];
    }
  }

  PrintArray(arr, dim, max_y, max_x);

  for (int i=0; i<dim; i++) {
    free(arr[i]);
  }
  free(arr);
  refresh();
  getch();
  endwin();

  return 0;
}
