#!/usr/bin/env bash

# Exit immediately if any command below fails.
set -e

make

echo "Generating a test_files directory.."
mkdir -p test_files
rm -f test_files/*

# Tests for ascii files
echo "Generating test files.."
printf "Hello, World!\n" > test_files/ascii.input
printf "Hello, World!" > test_files/ascii2.input

# various escape sequences
printf "\n\t\b\f\v\n" > test_files/ascii3.input

# very small file (1 char)
printf "h" > test_files/ascii4.input

# big file no line endings
printf "%999999s" "longfileman" > test_files/ascii5.input

# big file with line endings
printf "\n%999999s\n" "longfileman" > test_files/ascii6.input

# Tests data file cases.
printf "Hello,\x00World!\n" > test_files/data.input

# Tests with non-ascii bytes before, after and in between the ascii set.
for data_i in {1..6}
do
    hex=$(printf "%x" $data_i)
    printf "\x${hex}" > test_files/data${data_i}.input    
done

for data_i in {14..26}
do
    hex=$(printf "%x" $data_i)
    printf "\x${hex}" > test_files/data${data_i}.input    
done

for data_i in {28..31}
do
    hex=$(printf "%x" $data_i)
    printf "\x${hex}" > test_files/data${data_i}.input    
done

printf "\x7F" > test_files/data127.input


# Tests for all ascii-bytes, ie. in the ascii set. 

for ascii_i in {7..13}
do
    hex=$(printf "%x" $ascii_i)
    printf "\x${hex}\n" > test_files/ascii${ascii_i}.input
done

printf "\x1B\n" > test_files/ascii27.input

for ascii_i in {32..126}
do
    hex=$(printf "%x" $ascii_i)
    printf "\x${hex}\n" > test_files/ascii${ascii_i}.input
done

# some unicode bytes
for non_ascii in Æ Ø Å Ô È
do
    printf "${non_ascii}\n" > test_files/data${non_ascii}.input
done

# Tests for empty files
printf "" > test_files/empty.input
# alternate creation of empty file
touch test_files/empty2.input
# another one using -n to omit the echo newline
echo -n > test_files/empty3.input

# Testing the file binary
cp ./file test_files/sourcebin.input

echo "Running the tests.."
exitcode=0
for f in test_files/*.input
do
  echo ">>> Testing ${f}.."
  file    "${f}" | sed 's/ASCII text.*/ASCII text/' > "${f}.expected"
  ./file  "${f}" > "${f}.actual"

  if ! diff -u "${f}.expected" "${f}.actual"
  then
    echo ">>> Failed :-("
    exitcode=1
  else
    echo ">>> Success :-)"
  fi
done

printf "Total number of files tested: " & ls test_files/ | egrep input$ | wc -l



exit $exitcode
