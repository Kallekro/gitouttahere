#!/usr/bin/env python
from __future__ import division
import math, sys
from matplotlib import pyplot as plt
from scipy.optimize import curve_fit

def main():
    if len(sys.argv) < 3:
        print "usage: program_type filepath(s)" 
        return 0

    prg_type = sys.argv[1]

    if prg_type == "1":
        fun = lambda x: x**3
        fun_name="n^3*k"
        plt_name="matmul"
        title="Matmul - Maskintype "
    elif prg_type == "2":
        fun = lambda x: x * math.log(x, 2)
        fun_name="n*log(n)*k"
        plt_name="sort"
        title="Sorting - Maskintype "
    else:
        print "first argument should be 1 for matmul and 2 for sort"
        return 0

    # Start plot figure
    plt.figure(1)

    for i in range(2, len(sys.argv)):
        path = sys.argv[i]
        filename = path.split('/')[-1]

        machinetype = filename.split('.')[0][-1]

        program_name = "".join(filename.split('_')[0:2])

        if prg_type == "1":
            program_name = program_name[0:6]

        n = []
        cycles = []

        with open(path, 'r') as f:
            for line in f:
                data_row = line.split(' ')
                n.append(int(data_row[0]))
                cycles.append(int(data_row[1]))

        # Determine constant factor
        k = 0
        for i in range(len(n)):
            k_n = cycles[i] / fun(n[i])
            k += k_n

            print "k_{0} = {1}".format(i+1, k_n)
        k /= len(n)    
        print "mean k = {0}\n".format(k)

        plt.plot(n,cycles, '-', label=program_name)
        plt.plot([x for x in n], [fun(x)*k for x in n], 'o',label=fun_name)

    plt.title(title+machinetype)
    plt.xlabel("n")
    plt.ylabel("Cycles")

    plt.legend()

    plt.savefig("plots/"+plt_name+machinetype+".png")
    plt.show()
    

if __name__== "__main__":
    main()
