#!/usr/bin/env bash
mat_mul="examples/matrixmult-big.x64"
heap_1="examples/heapsort.x64"
heap_2="examples/heapsort-2.x64"
merge="examples/mergesort.x64"

## Our custom x64 programs
quicksort="asm_programs/quicksort.x64"
mat_mul2="asm_programs/matmul.x64"

## temporary copy of x64-source file. Used for changing n.
cp $mat_mul matmul_n
cp $mat_mul2 matmul2_n

## Create files to hold simulation data.
echo -n > data_files/matmul_n_data1.data
echo -n > data_files/matmul_n_data2.data
echo -n > data_files/matmul_n_data3.data

echo -n > data_files/matmul2_n_data1.data
echo -n > data_files/matmul2_n_data2.data
echo -n > data_files/matmul2_n_data3.data

## tmp files
touch out_tmp.o
touch data_tmp

## Function takes a x64-program, an output file and flags to run the simulation with.
## Update the ouput file to contain the simulation data 
function get_cycles {
    input_file=$1
    output_file=$2
    sim_flags=$3

    ./architecture-tools/asm $input_file out_tmp.o
    ./architecture-tools/sim out_tmp.o -m $sim_flags > data_tmp
    cycles=$(egrep 'cycles.*:\ (\d*)' data_tmp | cut -d ':' -f3)
    echo $n$cycles >> $output_file
}


## Testing the matrix multiplication programs
for file in matmul_n matmul2_n
do
    printf "Testing $file\n"
    for n in {10..50..5} # Testing for n=[10..50]
    do
        printf "n=$n\n"
        let n_squared=$n*$n
        new_dim="    .quad $n"
        new_rand="    .rand 1234 $n_squared"
        #Searching through the x64 program and replacing input dimension
        sed -i 's/^.*\.quad .*$/'"$new_dim"'/' $file 
        #Searcing through the x64 program and replacing the input .quads(matrices)
        sed -i 's/^.*\.rand 1234 .*$/'"$new_rand"'/' $file

        ### Get the total ammount of cycles for program $file with input n for all machines.
        ## Machine 1
	      output=$(printf "%s_data1.data" $file)
        get_cycles $file data_files/$output
        ## Machine 2
	      output=$(printf "%s_data2.data" $file)
        get_cycles $file data_files/$output "-pw=4"
        ## Machine 3
	      output=$(printf "%s_data3.data" $file)
        get_cycles $file data_files/$output "-pw=4 -ooo"
    done
done

rm matmul_n
rm matmul2_n


## temporary copy of x64-source file. Used for changing n.
cp $heap_1 heap_1_n
cp $heap_2 heap_2_n
cp $merge merge_n

# our quicksort
cp $quicksort quicksort_n

## Create or overwrite files to hold simulation data.
echo -n > data_files/heap_1_n_data1.data
echo -n > data_files/heap_1_n_data2.data
echo -n > data_files/heap_1_n_data3.data

echo -n > data_files/heap_2_n_data1.data
echo -n > data_files/heap_2_n_data2.data
echo -n > data_files/heap_2_n_data3.data

echo -n > data_files/merge_n_data1.data
echo -n > data_files/merge_n_data2.data
echo -n > data_files/merge_n_data3.data

echo -n > data_files/quicksort_n_data1.data
echo -n > data_files/quicksort_n_data2.data
echo -n > data_files/quicksort_n_data3.data

## Running the x64 sorting programs.
for file in heap_1_n heap_2_n merge_n quicksort_n
do
    printf "\nTesting file: $file.x64\n"
    for n in {1000..10000..1000}
    do
	    printf "n=$n\n"
	    new_dim="    .quad $n"
	    new_rand="    .rand 1234 $n"
      ## searching and replacing the input dimension.
	    sed -i 's/^.*\.quad .*$/'"$new_dim"'/' $file
	    sed -i 's/^.*\.rand .*$/'"$new_rand"'/' $file

	    # Machine 1
	    output=$(printf "%s_data1.data" $file)
      get_cycles $file data_files/$output 

	    # Machine 2
	    output=$(printf "%s_data2.data" $file)
	    get_cycles $file data_files/$output "-pw=4"

	    # Machine 3
      output=$(printf "%s_data3.data" $file)
      get_cycles $file data_files/$output "-pw=4 -ooo"	
    done
done

rm heap_1_n
rm heap_2_n
rm merge_n

rm quicksort_n

rm out_tmp.o
rm data_tmp 

## Move datafiles to data-dir
#mv *data data_files/


