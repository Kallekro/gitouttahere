#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "support.h"
#include "core.h"
#include "graphics.h"
#include "ai_weights.h"

int dim;

int gameLoop (int** arr, int _highscore, bool useAI);

int main (int argc, char* argv[]) {

  if (argc < 2) {
    dim = 4;
  } else {
    char * pEnd;
    dim = strtol(argv[1], &pEnd, 10);
  }

  int** arr = initialize_logic(dim);
  InitializeGraphics(dim);
  initAI(dim);

  int highscore;
  gameLoop(arr, highscore, true);

  getch();

  endwin();

  free_all(arr, dim);
  freeWeightArr(dim);
  return 0;
}

int gameLoop (int** arr, int _highscore, bool useAI) {
  int ch;
  bool done = false;
  int highscore = _highscore;
  int score = 0;
  while (!done) {
    // Array, score, highscore
    PrintGame(arr, score, highscore);

    if (useAI) {
      int move = findBestMove(arr, dim);
      move_board(arr, dim, move, true);
    }
    else {
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

    if (is_filled(arr, dim)) {
      done = true;
    }
  }
  return 0;
}
