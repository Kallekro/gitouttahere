#!/usr/bin/env bash

num_test=$(ls programs/*x86 | wc -l)

test_i=1

echo "Running ${num_test} number of test."
for test_program in $(ls programs/*.x86 | cut -d'.' -f1) 
do
    raw=$(cut -d'/' -f2 <<< $test_program)
    echo "Test number: ${test_i} - Name: ${raw}.x86"
    printf "Result: "
    ./run_sim.sh $test_program | egrep "Done" | wc -l
    let test_i++
done


