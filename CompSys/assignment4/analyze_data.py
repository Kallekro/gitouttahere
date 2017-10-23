from __future__ import division
import math, sys
from matplotlib import pyplot as plt
from scipy.optimize import curve_fit

def main():
    if len(sys.argv) != 3:
        print "usage: filepath program_type" 
        return 0

    n = []
    cycles = []

    with open(sys.argv[1], 'r') as f:
        for line in f:
            data_row = line.split(' ')
            n.append(int(data_row[0]))
            cycles.append(int(data_row[1]))

    if sys.argv[2] == "matmul":
        fun = lambda x: x**3
    else:
        fun = lambda x: x * math.log(x, 2)

    def fit_func_mul (x, a, b, c, d):
        return a*x**3 + b*x**2 + c*x + d
        #return a * x  math.log(x, 2)
    
    
    # Determine constant factor
    k = 0
    for i in range(len(n)):
        if n[i] != 0:
            k = cycles[i] / fun(n[i])
            print k
    k = k/len(n)    

    params = curve_fit(fit_func_mul, n, cycles)
    a,b,c,d = params[0]
    
    
    # Plot
    plt.figure(1)
    
    #plt.subplot(211)
    plt.plot(n,cycles, 'o')
    #plt.subplot(212)
    plt.plot([x for x in n], [fit_func_mul(x, a,b,c,d) for x in n])
    plt.axis([0, n[-1], 0, cycles[-1]])
    plt.plot([math.log(x, 2) for x in n])
    plt.show()
    
if __name__== "__main__":
    main()
