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

  char * pEnd;
  if (argc < 2) {
    printf("usage: test amount");
    return 1;
  }
  if (argc < 3) {
    dim = 4;
  } else {
    dim = strtol(argv[2], &pEnd, 10);
  }
 
  int amount = strtol(argv[1], &pEnd, 10);
   
  int** arr = initialize_logic(dim);
  InitializeGraphics(dim);

  initAI(dim);

  int totalScore = 0;
  int max = 0;
  int** maxArr;
  int highscore;
  for (int i = 0; i < amount; i++) { 
    int** testArr = copy_array(arr, dim);
    int score = gameLoop(testArr, highscore, true);
    if (score > max) {
      max = score;
      maxArr = copy_array(testArr, dim);
    }
    totalScore += score;    
    free_memory(testArr, dim);
  }
  PrintGame(maxArr, max, 0);
  free_memory(maxArr, dim);
  mvprintw(50, 10, "Mean score: %d\nMax score: %d", totalScore / amount, max); 
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
    if (c > 100000) {
      done = true;
    }

    // Array, score, highscore
    //PrintGame(arr, score, highscore);

    if (playerIsDead(arr, dim)) {
      done = true;
    }
    
    int res;

    if (useAI) {
      //if (c % 10 == 0) {
      //  if (getch() == 'q') {
      //    done=true;
      //  }
      //}
      int move = findBestMove(arr, dim, 2, 1);
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
  //PrintGame(arr, score, highscore);

  return score;
}
