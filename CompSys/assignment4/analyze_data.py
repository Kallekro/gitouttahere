#!/usr/bin/env python
from __future__ import division
import math, sys
from matplotlib import pyplot as plt
from scipy.optimize import curve_fit

def main():
    if len(sys.argv) != 3:
        print "usage: filepath program_type" 
        return 0

    filename = sys.argv[1].split('/')[1]

    prg_type = sys.argv[2]

    machinetype = filename.split('.')[0][-1]
    print machinetype

    program_name = "".join(filename.split('_')[0:2])

    if prg_type == "1":
        program_name = program_name[0:6]

    n = []
    cycles = []

    with open(sys.argv[1], 'r') as f:
        for line in f:
            data_row = line.split(' ')
            n.append(int(data_row[0]))
            cycles.append(int(data_row[1]))


    if prg_type == "1":
        fun = lambda x: x**3
    elif prg_type == "2":
        fun = lambda x: x * math.log(x, 2)

    # Determine constant factor
    k = 0
    for i in range(len(n)):
        k_n = cycles[i] / fun(n[i])
        if i == len(n)-1:
            k = k_n

        print "k_n =",k_n
    print "k=",k


    # Plot
    plt.figure(1)

    if prg_type=="1":
        func="O(n) = n^3*k"
    else:
        func="O(n) = n*log(n)*k"

    #plt.subplot(211)
    p1 = plt.plot(n,cycles, 'o', label="Actual measurements")
    p2 = plt.plot([x for x in n], [fun(x)*k for x in n], label=func)
    plt.plot([],[],'wo',label="k = %.4f" %k)
    plt.axis([0, n[-1]+5, 0, cycles[-1]+250000])

    plt.title("Program: " + program_name + " - Machinetype: " + machinetype)
    plt.xlabel("n")
    plt.ylabel("Cycles")

    plt.legend()

    plt.show()

if __name__== "__main__":
    main()
