#!/usr/bin/env bash

# Exit immediately if any command below fails.
set -e

make

echo "Generating a test_files directory.."
mkdir -p test_files
rm -f test_files/*

echo "Generating test files.."
# Tests for ascii files
echo "Generating test files.."
printf "Hello, World!\n" > test_files/ascii.input
printf "Hello, World!" > test_files/ascii1.input

# various escape sequences
printf "\n\t\b\f\v\n" > test_files/ascii3.input

# very small ascii file (1 char)
printf "h" > test_files/ascii4.input
#very small non-ascii file
printf "\x01" > test_files/data1.input
# very long file
# big file no line endings
printf "%999999s" "longfileman" > test_files/ascii5.input

# big file with line endings
printf "\n%999999s\n" "longfileman" > test_files/ascii6.input

# Tests data file cases.
printf "Hello,\x00World!\n" > test_files/data.input

for testfile_i in {2..127}
do
    hex=$(printf "%x" $testfile_i)
    if [ $testfile_i -lt 7 ] || ([ $testfile_i -gt 13 ] && [ $testfile_i -lt 27 ]) ||
	                        ([ $testfile_i -gt 27 ] && [ $testfile_i -lt 32 ]) ||
	                        [ $testfile_i -gt 126 ];
    then
	printf "\x${hex}\n" > test_files/data${testfile_i}.input
    else
	printf "\x${hex}\n" > test_files/ascii${testfile_i}.input           
    fi
done

# Tests for empty files
printf "" > test_files/empty.input
# alternate creation of empty file
touch test_files/empty2.input
# another one using -n to omit the echo newline
echo -n > test_files/empty3.input

# Testing the file binary
cp ./file test_files/sourcebin.input

## Testing for bytes in the ISO-8859 range. No unicode.
for testfile_i in {160..255}
do
    hex=$(printf "%x" $testfile_i)
    printf "\x${hex}\n" > test_files/iso${testfile_i}.input           
done

## Testing for unicode.
## Testing subrange for 2 bytes-encoded unicode characters. Should not be ISO.
for testfile_i in {128..300}
do
    hex=$(printf "%x" $testfile_i)
    printf "\u${hex}\n" > test_files/utf8_2b_${testfile_i}.input
done

## Testing subrange for 3 bytes-encoded unicode characters.
for testfile_i in {2048..2200}
do
    hex=$(printf "%x" $testfile_i)
    printf "\u${hex}\n" > test_files/utf8_3b_${testfile_i}.input
done

## Testing subrange for 4 bytes-encoded unicode characters.
for testfile_i in {65536..65700}
do
    hex=$(printf "%x" $testfile_i)
    printf "\u${hex}\n" > test_files/utf8_4b_${testfile_i}.input
done

## Little endian and big endian tests
# some unicode tests.
test_cases=(300 500  2500 66000)
for test_case in "${test_cases[@]}"
do
    hex=$(printf "%x" $test_case)
    echo -e "\xFF\xFE\u${hex}\n" > test_files/little_endian${test_case}_unicode.input    
    echo -e "\xFE\xFF\u\{hex}\n" > test_files/big_endian${test_case}_unicode.input 
done

## som ascii, data and iso tests
for test_case in {1..255}
do
    hex=$(printf "%x" $test_case)
    echo -e "\xFF\xFE\u${hex}\n" > test_files/little_endian${test_case}_asc_dat_iso.input
    echo -e "\xFE\xFF\u\{hex}\n" > test_files/big_endian${test_case}_asc_dat_iso.input   
done


echo "Running the tests.."
exitcode=0
f=test_files/*.input
echo ">>> Testing ${f}.."
file    ${f} | sed 's/ASCII text.*/ASCII text/' > test_files/expected
./file  ${f} > test_files/actual

if ! diff -u test_files/expected test_files/actual
then
  echo ">>> Failed :-("
  exitcode=1
else
  echo ">>> Success :-)"
fi
printf "Total number of files tested: " & ls test_files/ | egrep input$ | wc -l


printf "Testing the the programs API.\n"
## Program should handle multiple arguments, with just empty files

touch test_files/argfile1 test_files/argfile2 test_files/argfile3
./file test_files/argfile1 test_files/argfile2 test_files/argfile3 
res=$(echo $?)
printf "Multiple arguments: ${res}\n"

## Program should exit with exitcode 1 if no arguments where passed, and print usage message.
res=$(./file || echo $?) 
printf "No arguments exitcode: ${res}\n"

## Program should exit with code 0 if the files dont exists.
res=$(./file idontexists || echo $?) 
printf "non existent files exitcode: ${res}\n"

## If the file is a directory(also a file) it should exit code 0
res=$(./file test_files/ || echo $?) 
printf "Open a directory arguments exitcode: ${res}\n"

## If some of the argument is valid, it should print the filetype
## And display the error message for the invalid arguments, and exit succesfull
res=$(./file dontexists test_files/ test_files/argfile1)
printf "Some valid and invalid arguments exitcode: ${res}\n"

exit $exitcode
