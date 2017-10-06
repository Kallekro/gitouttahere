#include<stdbool.h>

#ifndef CORE_H
#define CORE_H
int** initialize_logic(int dim);
void feed_board(int** arr, int dim);
int move_board(int** arr, int dim, int move, bool feed);
bool playerIsDead(int** arr, int dim);
#endif
