#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <ncurses.h>
#include "core.h"
#include "support.h"

int initAI (int dim);
int findBestMove(int** arr, int dim, int depth);
int fillWeightArray(float** arr, int dim);
int fillWeightArray_B(float** arr, int dim);
int weightedSum(int** arr, int dim);

float** weightArr;

int initAI (int dim) {
  weightArr = malloc(dim * sizeof(*weightArr));
  for (int i = 0; i < dim; i++) {
    weightArr[i] = malloc(dim * sizeof(*weightArr));
  }

  //fillWeightArray(weightArr, dim);
  fillWeightArray_B(weightArr, dim);
  
  return 0;
}

int freeWeightArr (int dim) {
  for (int i=0; i<dim; i++) {
    free(weightArr[i]);
  }
  free(weightArr);
  return 0;
}

int findBestMove(int** arr, int dim, int depth) {
  int greatestSum = 0;
  int bestMove = 0;
  for (int i=0; i < 4; i++) {
    int** arrCopy = copy_array(arr, dim);
    int res = move_board(arrCopy, dim, i, false);
    if (res != -1) {
      int newSum;
      if (depth == 0) {
        newSum = weightedSum(arrCopy, dim);
      } 
      else {
        newSum = weightedSum(arrCopy, dim) * depth + findBestMove(arrCopy, dim, depth-1);
      }
      if (newSum >= greatestSum) {
        greatestSum = newSum;
        bestMove = i;
      }
    }
    free_memory(arrCopy, dim);
  }
  return bestMove;
}

int weightedSum(int** arr, int dim) {
  float sum = 0.0;
  float numberCount = 0.0;
  for (int i=0; i < dim; i++) {
    for (int j=0; j < dim; j++) {
      sum += ((float) arr[i][j]) * weightArr[i][j];
      if (arr[i][j] != 0) {
        numberCount += 1.0;
      }
    }
  }
  int numberCount_coeff = (numberCount) ? 1.0/numberCount*numberCount : 0.0;
  return (int) (roundf(sum) * numberCount_coeff);
}

int fillWeightArray(float** arr, int dim) {
  float w = 1.0;
  for (int i=0; i < dim; i++) {
    for (int j=0; j < dim; j++) {
      arr[i][j] = w;
      w -= 1.0/(dim*dim);
      printw("%f", w);
    }
    printw("\n");
  }
  return 0;
}

int fillWeightArray_B (float** arr, int dim) {
  float w = 1.0;
  for (int i=0; i < dim; i++) {
    for (int j=0; j < dim; j++) {
      if (i + j > 0) {
        w = 1.0 / (((float) i) * 2.75 + (((float) j)/1.85) + 1.0);
      }
      arr[i][j] = w;
      printw("%f", w);
    }
    printw("\n");
  }
  return 0;
}

