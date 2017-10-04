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
int start_y;

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
  start_y = max_y/2 - dim;

  if (start_x <= 0 || start_y <= 0) {
    printw("Game does not fit on this screen. Please resize and restart");
    return 1;
  }

  main_win = create_new_window(dim * 3-1, dim * cellSize + 4, start_y, start_x-2);
  return 0;
}

WINDOW *create_new_window(int height, int width, int starty, int startx) {
  WINDOW *new_win;

  new_win = newwin(height, width, starty, startx);
  wrefresh(new_win);

  return new_win;    
}

int PrintArray(int** arr) {
  refresh();
  char buffer[cellSize];
  int local_x = 2;
  int y_pos = 2;

  for (int i=0; i < dim; i++) {
    wmove(main_win, y_pos, local_x);
    for (int j=0; j < dim; j++) {
      if (arr[j][i] == 0) { 
        wprintw(main_win, "%*s", cellSize, " ");
        continue;
      }
      sprintf(buffer, "%d", arr[j][i]);
      if (strlen(buffer) % 2 == 0) {
        wprintw(main_win,
               "%*s%s%*s", (cellSize - strlen(buffer))/2, " ",
                           buffer,
                           (cellSize - strlen(buffer))/2 + 1, " ");
      }
      else {
        wprintw(main_win, 
                "%*s%s%*s", (cellSize - strlen(buffer))/2, " ",
                           buffer,
                           (cellSize - strlen(buffer))/2, " ");
      }
    }
    y_pos += 2;
  }

  box(main_win, 0, 0);
  wrefresh(main_win);

  return 0;
}
