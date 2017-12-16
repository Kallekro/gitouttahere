#!/usr/bin/env bash

if [ "$#" -ne 2 ]; then
  echo "usage: ./run_sim.sh path architecture_tools/" 
  exit 1
fi
make
filename=$1
tools=$2
echo $filename 
echo "Assembling program.."
$tools/asm $filename.x86 $filename.o
echo "Simulating program with reference simulator.."
$tools/sim $filename.o $filename.trc
echo "Simulating program with generated trace file.."
./sim $filename.o $filename.trc

