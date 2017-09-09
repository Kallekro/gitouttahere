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

# Test the first bytes (after 0 and up to the first ascii byte)
for data_i in {1..6} 
do
    printf "\x0${data_i} datamagic\n" > test_files/data${data_i}.input
done
# Test the first ascii bytes
for ascii_i in {7..9} 
do
    printf "\x0${ascii_i} asciimagic\n" > test_files/ascii${ascii_i}.input       
done    
# need new loop for hexes with 2 digits
for ascii_i in {10..13} 
do
    printf "\x${ascii_i} asciimagic\n" > test_files/ascii${ascii_i}.input       
done    
# Test the second group of data bytes
for data_i in {14..26} 
do
    printf "\x${data_i} datamagic\n" > test_files/data${data_i}.input
done
# Test the second group of ascii bytes (just one byte)
printf "\x1B someasciibyte\n" > test_files/ascii27.input
# Test the third group of data bytes
for data_i in {28..31}
do
    printf "\x${data_i} datamagic\n" > test_files/data${data_i}.input
done
# Test the last group of ascii bytes
for ascii_i in {31..126} 
do
    printf "\x${ascii_i} asciimagic\n" > test_files/ascii${ascii_i}.input       
done    
# Test the byte after the last ascii byte
printf "\x7F datamagic\n" > test_files/data127.input

# some unicode bytes
for non_ascii in Æ Ø Å Ô È
do
    printf "${non_ascii}" > test_files/data${non_ascii}.input
done

# Tests for empty files
printf "" > test_files/empty.input
# alternate creation of empty file
touch test_files/empty2.input
# another one using -n to omit the echo newline
echo -n > test_files/empty3.input


# Testing the file binary
cp file.exe test_files/sourcebin.input

echo "Running the tests.."
exitcode=0
for f in test_files/*.input
do
  echo ">>> Testing ${f}.."
  file    "${f}" | sed 's/ASCII text.*/ASCII text/' > "${f}.expected"
  file.exe  "${f}" > "${f}.actual"

  if ! diff -u "${f}.expected" "${f}.actual"
  then
    echo ">>> Failed :-("
    exitcode=1
  else
    echo ">>> Success :-)"
  fi
done
exit $exitcode
