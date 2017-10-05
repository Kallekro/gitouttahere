#ifndef CORE_H
#define CORE_H

int** initialize_logic(int dim);
void feed_board(int** arr, int dim);
void fill_with_val(int**, int, int);
void move_board(int** arr, int dim, int move);
void free_all(int**, int);


#endif
