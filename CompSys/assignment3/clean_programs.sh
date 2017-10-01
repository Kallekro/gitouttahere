#!/usr/bin/env bash

all=$(ls ./programs/*/* | egrep "*[^6]$") #| egrep "!(x86$)")

rm $all
