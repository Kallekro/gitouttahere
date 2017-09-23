#!/usr/bin/env bash

if [ "$#" -ne 1 ]; then
  echo "usage: path (without extension)" 
  exit 1
fi
cd src/ && make && cd ..
filename=$1
echo $filename 
echo "Assembling program.."
./architecture-tools/asm $filename.x86 $filename.o
echo "Simulating program with reference simulator.."
./architecture-tools/sim $filename.o $filename.trc
echo "Simulating program with generated trace file.."
./src/sim $filename.o $filename.trc

