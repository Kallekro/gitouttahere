#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

int InitializeGame();

int PrintArray(int** arr, int dim, int max_x, int max_y);

int randInt(int lo, int hi) {
  return ((rand() % hi) + lo);
}

int main() {

  time_t t;
  srand((unsigned) time(&t));

  InitializeGame();

  int max_y, max_x; 
  getmaxyx(stdscr, max_y, max_x);

  int ch;

  ch = getch();
  if (ch == KEY_F(1)) {
    printw("F1 Key pressed\n");
  } 
  else {
    printw("Pressed key: %c", ch);
  }

  int dim = 4;
  
  int** arr;
  arr = malloc(sizeof(*arr) * dim);

  int numbers[6] = {0, 2, 4, 8, 16, 32};

  for (int i=0; i<dim; i++) {
    arr[i] = malloc(sizeof(int) * dim);

    for (int j=0; j<dim; j++) {
      arr[i][j] = numbers[randInt(0, 6)];
    }
  }

  PrintArray(arr, dim, max_y, max_x);

  for (int i=0; i<dim; i++) {
    free(arr[i]);
  }
  free(arr);
  refresh();
  getch();
  endwin();

  return 0;
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

  move(y_pos-2, start_x);
  for (int i=0; i < dim * cellSize + 1; i++) {
    addch('_'); 
  }

  for (int i=0; i < dim; i++) {
    move(y_pos, start_x);
    y_pos += 2;
    for (int j=0; j < dim; j++) {
      //printw("%d", arr[j][i]);
      sprintf(buffer, "%d", arr[j][i]);
      //printw("%d",strlen(buffer));
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
