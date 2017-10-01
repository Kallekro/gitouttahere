#!/usr/bin/env bash

num_test=$(ls programs/*/*x86 | wc -l)

test_i=1

echo "Running ${num_test} number of test."
for test_dir in $(ls programs/) 
do
    echo "Testing \"${test_dir}\" commands:"
    printf "\n"
    for test_program in $(ls programs/$test_dir/*x86)
        do				    	    	    
	    raw=$(cut -d'.' -f1 <<< $test_program)
	    echo "Test number: ${test_i} - Name: ${raw}.x86"
	    printf "Result:  "
	    ./run_sim.sh $raw | egrep "Done" | wc -l
	    printf "\n"
	    let test_i++
	done    
done


