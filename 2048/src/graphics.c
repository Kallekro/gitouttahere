#include <ncurses.h>
#include <stdlib.h>
#include <string.h>

int InitializeGame();

int PrintArray(int** arr, int dim, int max_x, int max_y);

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

  int start_x = max_x / 2 - dim * cellSize / 2; 
  int y_pos = max_y/2 - dim*2/2;

  if (start_x <= 0 || y_pos <= 0) {
    printw("Game does not fit on this screen. Please resize and restart");
    return 1;
  }

  for (int i=0; i < dim; i++) {
    y_pos += 2;
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
  }

  return 0;
}
