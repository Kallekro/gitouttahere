#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "support.h"
#include "core.h"
#include "graphics.h"

int dim;

int main (int argc, char* argv[]) {

  if (argc < 2) {
    dim = 4;
  } else {
    char * pEnd;
    dim = strtol(argv[1], &pEnd, 10);
  }

  int** arr = initialize_logic(dim);
  int highscore;

  InitializeGraphics(dim);
  gameLoop(arr, highscore);
  endwin();
  
  free_all(arr, dim);
  return 0;
}

int gameLoop (int** arr, int _highscore) {
  int ch;
  bool done = false;
  int highscore = _highscore;
  int score = 0;
  while (!done) {
    // Array, score, highscore
    PrintGame(arr, score, highscore);
    
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
	  score += move_board(arr, dim, 3, true);
          break;
        case KEY_RIGHT :
	  score += move_board(arr, dim, 2, true);
          break;
        case KEY_UP :
	  score += move_board(arr, dim, 0, true);
          break;
        case KEY_DOWN:
	  score += move_board(arr, dim, 1, true);
          break;
        default:  
          break;
      }
    }
  }
  return 0;
}
