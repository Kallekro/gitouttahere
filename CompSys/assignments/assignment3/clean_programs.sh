#!/usr/bin/env bash

all=$(ls ./test_programs/*/* | egrep "*[^6]$") #| egrep "!(x86$)")

rm $all
