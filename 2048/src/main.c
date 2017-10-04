#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "graphics.h"

int randInt(int lo, int hi);

int gameLoop(int**arr);

int main (int argc, char* argv[]) {
  time_t t;
  srand((unsigned) time(&t));

  int dim;
  if (argc < 2) {
    dim = 4;
  } else {
    char * pEnd;
    dim = strtol(argv[1], &pEnd, 10);
  }

  int** arr;

  arr = malloc(sizeof(*arr) * dim);

  for (int i=0; i<dim; i++) {
    arr[i] = malloc(sizeof(int) * dim);
  }

  int numbers[15] = {0, 0, 0, 0, 2, 4, 8, 16, 32, 64,
                     128, 256, 512, 1024, 2048};
  for (int i=0; i<dim; i++) {
    for (int j=0; j<dim; j++) {
      arr[i][j] = numbers[randInt(0, 15)];
    }
  }

  InitializeGame(dim);
  gameLoop(arr);
  endwin();

  for (int i=0; i<dim; i++) {
    free(arr[i]);
  }
  free(arr);

  return 0;
}

int gameLoop (int** arr) {
  int ch;
  bool done = false;
  while (!done) {
    // Array, score, highscore
    PrintGame(arr, 200, 2000);

    ch = getch();
    if (ch == KEY_F(1)) {
      done = true;
    } 
    else {
      switch (ch) {
        case 'q':
          done = true;
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
    
  }
  return 0;
}

int randInt(int lo, int hi) {
  return ((rand() % hi) + lo);
}
