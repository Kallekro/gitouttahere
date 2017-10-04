#include <ncurses.h>
#include <stdlib.h>
#include <string.h>

int InitializeGame();

int PrintGame(int** arr, int score, int highscore);
int PrintArray(int** arr);
int PrintGUI(int score, int highscore);


WINDOW *create_new_window(int height, int width, int starty, int startx);

WINDOW *main_win;
int main_height;
int main_width;
int main_posy;
int main_posx;

WINDOW *score_win;
  
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
  cellSize = 7;
  start_x = max_x / 2 - dim * cellSize / 2;
  start_y = max_y/2 - dim;

  if (start_x <= 0 || start_y <= 0) {
    printw("Game does not fit on this screen. Please resize and restart");
    return 1;
  }

  main_height = dim * 2+3;
  main_width = dim * cellSize + 3;
  main_posy = start_y;
  main_posx = start_x-1;

  main_win = create_new_window(main_height, main_width, main_posy, main_posx);

  score_win = create_new_window(4, main_width, start_y - 4, main_posx);

  return 0;
}

WINDOW *create_new_window(int height, int width, int starty, int startx) {
  WINDOW *new_win;

  new_win = newwin(height, width, starty, startx);
  wrefresh(new_win);

  return new_win;    
}

int PrintGame(int** arr, int score, int highscore) {
  PrintArray(arr);
  PrintGUI(score, highscore);

  refresh();
  box(main_win, 0, 0);
  box(score_win, 0, 0);
  wrefresh(main_win);
  wrefresh(score_win);
  return 0;
}

int PrintArray(int** arr) {
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

  return 0;
}

int PrintGUI(int score, int highscore) {
  // Print score and highscore
  wmove(score_win, 1, 2);
  wprintw(score_win, "Score: %*s%d", 4, " ", score);
  wmove(score_win, 2, 2);
  wprintw(score_win, "Highscore: %d", highscore);

  // Print help message
  char* msg = "Press q or F1 to quit..";
  mvprintw(main_posy + main_height + 1, main_posx + (main_width - strlen(msg))/2, "%s", msg);
  return 0;
}

