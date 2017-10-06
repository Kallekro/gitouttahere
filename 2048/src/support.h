#include<stdbool.h>

#ifndef SUPPORT_H
#define SUPPORT_H

int randInt(int lo, int hi);
void printArray(int** arr, int dim);
int** initialize(int dim);
void free_memory(int** arr, int dim);
void fill_with_val(int** arr, int dim, int val);

#endif
