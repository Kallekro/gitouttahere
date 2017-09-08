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
# newline is ascii
printf "\n\n\n\n\n" > test_files/ascii3.input
# it doesnt work for a very small file (1 char)
printf "h" > test_files/ascii4.input
# should work with a big file
printf "%999999s" "longfileman" > test_files/ascii5.input


# Tests data file cases.
printf "Hello,\x00World!\n" > test_files/data.input

for data_i in {1..6} 
do
    printf "\x0${data_i} \"magic\"" > test_files/data${data_i}.input
done

# given unicode characters, file doesnt return data 
printf "\u0129\u0023\u2391" > test_files/data_uni.input
# no error with small file, because unicode is 3 bytes.
for non_ascii in Æ Ø Å Ô È
do
    printf "${non_ascii}" > test_files/data${non_ascii}.input
done

# Tests for empty files
printf "" > test_files/empty.input
# alternate creation of empty file
touch test_files/empty2.input





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
exit $exitcode
