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
  int highscore;

  InitializeGraphics(dim);
  gameLoop(arr, highscore);
  endwin();

  return 0;

}

int gameLoop (int** arr, int _highscore) {
  int ch;
  bool done = false;
  int highscore = _highscore;
  int score = 0;
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
