#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

int InitializeGame();

int PrintArray(int** arr, int dim, int max_x, int max_y);

int randInt(int lo, int hi) {
  return ((rand() % hi) + lo);
}


int InitializeGame() {
  initscr();
  raw();
  keypad(stdscr, TRUE);
  noecho();
  curs_set(0);
  return 0;
}

int PrintArray(int** arr, int dim, int max_y, int max_x) {
  int cellSize = 9;
  char buffer[cellSize];

  int start_x = max_x / 2 - dim * cellSize / 2 - 2; 
  int y_pos = max_y/2 - dim*2/2;

  if (start_x <= 0 || y_pos <= 0) {
    printw("Game does not fit on this screen. Please resize and restart");
    return 1;
  }

  move(y_pos-2, start_x);
  for (int i=0; i < dim * cellSize + 1; i++) {
    addch('_'); 
  }

  for (int i=0; i < dim; i++) {
    y_pos += 2;
    mvprintw(y_pos-1, start_x, "|  ");
    mvprintw(y_pos, start_x, "|  ");
    move(y_pos, start_x);
    for (int j=0; j < dim; j++) {
      sprintf(buffer, "%d", arr[j][i]);
      if (strlen(buffer) % 2 == 0) {
        printw("%*s%s%*s", (cellSize - strlen(buffer))/2, " ",
                           buffer,
                           (cellSize - strlen(buffer))/2 + 1, " ");
      }
      else {
        printw("%*s%s%*s", (cellSize - strlen(buffer))/2, " ",
                           buffer,
                           (cellSize - strlen(buffer))/2, " ");
      }
    }
    printw("  |");
  }
  move(y_pos, start_x-2);
  for (int i=0; i < dim * cellSize + 1; i++) {
    addch('_'); 
  }

  return 0;
}
