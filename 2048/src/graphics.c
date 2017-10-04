#include <ncurses.h>
#include <stdlib.h>
#include <string.h>

int InitializeGame();

int PrintArray(int** arr);

WINDOW *create_new_window(int height, int width, int starty, int startx);

WINDOW *main_win;
  
int dim;

int max_y, max_x; 

int cellSize;
  
int start_x;  
int y_pos; 

int InitializeGame(int _dim) {
  dim = _dim;
  initscr();
  raw();
  keypad(stdscr, TRUE);
  noecho();
  curs_set(0);

  getmaxyx(stdscr, max_y, max_x);
  cellSize = 9;
  start_x = max_x / 2 - dim * cellSize / 2;
  y_pos = max_y/2 - dim*2/2;

  if (start_x <= 0 || y_pos <= 0) {
    printw("Game does not fit on this screen. Please resize and restart");
    return 1;
  }

  main_win = create_new_window(10, 10, 10, 10);
  return 0;
}

WINDOW *create_new_window(int height, int width, int starty, int startx) {
  WINDOW *new_win;

  new_win = newwin(height, width, starty, startx);
  box(new_win, 0, 0);
  wrefresh(new_win);

  return new_win;    
}

int PrintArray(int** arr) {
  char buffer[cellSize];

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
