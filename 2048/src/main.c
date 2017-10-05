#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "core.h"
#include "graphics.h"

int gameLoop(int**arr, int highscore);

int dim;

int main (int argc, char* argv[]) {
  time_t t;
  srand((unsigned) time(&t));
  
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
	  move_board(arr, dim, 3);
          break;
        case KEY_RIGHT :
	  move_board(arr, dim, 2);
          break;
        case KEY_UP :
	  move_board(arr, dim, 0);
          break;
        case KEY_DOWN:
	  move_board(arr, dim, 1);
          break;
        default:  
          break;
      }
    }
    feed_board(arr, dim);
  }
  return 0;
}
