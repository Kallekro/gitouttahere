#include<stdbool.h>

#ifndef CORE_H
#define CORE_H

int** initialize_logic(int dim);
void feed_board(int** arr, int dim);
void fill_with_val(int**, int, int);
int move_board(int** arr, int dim, int move, bool feed);
void free_all(int**, int);
int randInt(int lo, int hi); 

#endif
