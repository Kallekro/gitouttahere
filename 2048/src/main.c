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
  getch();


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
  int c = 0;
  int prevMove = -1;
  while (!done) {
    mvprintw(0, c, "%d", prevMove);
    c++;
    //if (c > 100000) {
    //  done = true;
    //}

    // Array, score, highscore
    PrintGame(arr, score, highscore);

    if (playerIsDead(arr, dim)) {
      done = true;
    }
    
    int res;

    if (useAI) {
      //if (c % 10 == 0) {
      //if (getch() == 'q') {
      //  done=true;
      //}
      //}
      int move = findBestMove(arr, dim);
      prevMove = move;
      res = move_board(arr, dim, move, true);
      if (res == -1) { res = 0; }
      score += res;
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
            res = move_board(arr, dim, 3, true);
            if (res == -1) { res = 0; }
            score += res;
            break;
          case KEY_RIGHT :
            res = move_board(arr, dim, 2, true);
            if (res == -1) { res = 0; }
            score += res;
            break;
          case KEY_UP :
            res = move_board(arr, dim, 0, true);
            if (res == -1) { res = 0; }
            score += res;
            break;
          case KEY_DOWN:
            res = move_board(arr, dim, 1, true);
            if (res == -1) { res = 0; }
            score += res;
            break;
          default:  
            break;
        }
      }
    }

  }
  return 0;
}
