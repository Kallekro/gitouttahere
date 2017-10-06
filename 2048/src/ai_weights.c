#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "core.h"
#include "support.h"

int initAI (int dim);
int findBestMove(int** arr, int dim);
int fillWeightArray(float** arr, int dim);
int weightedSum(int** arr, int dim);

float** weightArr;

int initAI (int dim) {
  weightArr = malloc(dim * sizeof(*weightArr));
  for (int i = 0; i < dim; i++) {
    weightArr[i] = malloc(dim * sizeof(*weightArr));
  }

  fillWeightArray(weightArr, dim);
  
  return 0;
}

int freeWeightArr (int dim) {
  for (int i=0; i<dim; i++) {
    free(weightArr[i]);
  }
  free(weightArr);
  return 0;
}

int findBestMove(int** arr, int dim) {
  int greatestSum = 0;
  int bestMove = 0;
  for (int i=0; i < 4; i++) {
    int** arrCopy = copy_array(arr, dim);
    move_board(arrCopy, 4, i, false);
    int newSum = weightedSum(arrCopy, 4);
    if (newSum > greatestSum) {
      greatestSum = newSum;
      bestMove = i;
    }
  }
  return bestMove;
}

int weightedSum(int** arr, int dim) {
  float sum = 0.0;
  float numberCount = 0;
  for (int i=0; i < dim; i++) {
    for (int j=0; j < dim; j++) {
      sum += ((float) arr[i][j]) * weightArr[i][j];
      if (arr[i][j] != 0) {
        numberCount += 1.0;
      }
    }
  }
  int numberCount_coeff = (numberCount) ? 1.0/numberCount : 0.0;
  return (int) roundf(sum) * numberCount_coeff;
}

int fillWeightArray(float** arr, int dim) {
  float w = 1.0;
  for (int i=0; i < dim; i++) {
    for (int j=0; j < dim; j++) {
      arr[i][j] = w;
      w -= 1.0/(dim*dim);
    }
  }
  return 0;
}
