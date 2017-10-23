#include<stdio.h>
#include<stdlib.h>

int quicksort (int* arr, int lo, int hi);
int partition (int* arr, int lo, int hi);

int main(int argc, char* argv[]) {

  if (argc != 2) {
    puts("Usage: dim");
    return 1337;
  }
  
  int arrLOL[9] = {1,56,65,77,34,87,432,7,32};

  quicksort(arrLOL, 0, 8);

  int i;
  for (i=0; i<9; i++) {
    printf ("%d,", arrLOL[i]);
  }
  
  exit(EXIT_SUCCESS);
}

int quicksort (int arr[] ,int lo, int hi) {
  if (lo < hi) {
    int p =  partition(arr, lo, hi);
    quicksort(arr, lo, p-1);
    quicksort(arr, p+1, hi);
  }
  return 0;
}


int partition (int arr[], int lo, int hi) {
  int pivot = arr[hi];
  int i = lo - 1;
  for (int j=lo; j<hi; j++) {
    if (arr[j] < pivot) {
      i++;
      // swap 
      int tmp = arr[j];
      arr[j] = arr[i];
      arr[i] = tmp;
    }
  }
  if (pivot < arr[i+1]) {
    int tmp = arr[hi];
    arr[hi] = arr[i+1];
    arr[i+1] = tmp;
  }
  return i+1;
}
