#!/usr/bin/env bash

num_test=$(ls programs/*/*x86 | wc -l)

echo "Running ${num_test} number of test."
for test_dir in $(ls test_programs/) 
do
    test_i=1
    echo "Testing \"${test_dir}\" commands:"
    for test_program in $(ls test_programs/$test_dir/*x86)
    do				    	    	    
	raw=$(cut -d'.' -f1 <<< $test_program)
	name=$(cut -d'/' -f3 <<< $raw)
	echo "Test number: ${test_i} - Name: ${name}.x86"
	printf "Result:  "
	./run_sim.sh $raw | egrep "Done" | wc -l
	let test_i++
    done
    printf "\n"
done


