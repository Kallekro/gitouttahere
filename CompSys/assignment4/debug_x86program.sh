#!/usr/bin/env bash

if [ "$#" -ne 1 ]; then
  echo "usage: path (without extension)" 
  exit 1
fi

filename=$1
printf "set args $filename.o $filename.trc\nb error" > $filename.gdb

gdb -q -x $filename.gdb ./src/sim
