#!/usr/bin/env bash

./analyze_data.py 1 data_files/oldmatmul_n_data1.data data_files/newmatmul_n_data1.data
./analyze_data.py 1 data_files/oldmatmul_n_data2.data data_files/newmatmul_n_data2.data
./analyze_data.py 1 data_files/oldmatmul_n_data3.data data_files/newmatmul_n_data3.data

./analyze_data.py 2 data_files/heap_1_n_data1.data data_files/heap_2_n_data1.data data_files/merge_n_data1.data data_files/quicksort_n_data1.data
./analyze_data.py 2 data_files/heap_1_n_data2.data data_files/heap_2_n_data2.data data_files/merge_n_data2.data data_files/quicksort_n_data2.data
./analyze_data.py 2 data_files/heap_1_n_data3.data data_files/heap_2_n_data3.data data_files/merge_n_data3.data data_files/quicksort_n_data3.data
