#include<stdbool.h>

#ifndef SUPPORT_H
#define SUPPORT_H

int randInt(int lo, int hi);
void printArray(int** arr, int dim);
int** initialize(int dim);
void free_memory(int** arr, int dim);
void fill_with_val(int** arr, int dim, int val);
int** copy_array(int** arr, int dim);
bool is_filled(int**arr, int dim);
int max_uint(int** arr, int dim);

#endif
