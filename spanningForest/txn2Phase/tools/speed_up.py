batchRanges =[
#500, 700, 1000, 2000, 3000, 5000, 
5000, 10000, 20000, 50000, 75000, 100000, 200000, 500000] # [100, 200, 300, 500, 700, 1000, 2000, 3000, 5000, 10000, 20000, 50000, 75000, 100000, 200000]
import os
import sys
import subprocess
trials = 3
executable = "MIS"
inputFile = "randTI"
if len(sys.argv) > 1:
    # inputFile = sys.argv[1]
    executable = sys.argv[1]
timeList = []

def avg(l):
    return float(sum(l))/len(l)

for batchSize in batchRanges:
    currentL = []
    for t in range(trials):
        ret = subprocess.check_output("numactl -i all -- ./" + executable + " -c 1 -b " + str(batchSize) + " " + inputFile, shell=True)
        time = float(ret.split('PBBS-time:')[1].strip())
        #ret2 = subprocess.check_output(["numactl -i all -- ./" + executable," -c 40 " , "-b", str(batchSize), inputFile], shell=True) 
        ret2 = subprocess.check_output("numactl -i all -- ./" + executable + " -c 32 -b " + str(batchSize) + " " + inputFile, shell=True)
        time2 = float(ret2.split('PBBS-time:')[1].strip())
        speedup = float(time)/time2
        currentL.append(speedup)
    timeList.append(avg(currentL))
    print batchSize, avg(currentL), currentL

print timeList
resultSEQ = '''100 0.548
200 0.281
300 0.214
500 0.146
700 0.11
1000 0.0914
2000 0.0869
3000 0.0636
5000 0.0585
10000 0.0647
20000 0.0723
50000 0.127
1000000 0.641'''

resultDET = '''100 0.0562
200 0.0494
300 0.0504
500 0.0495
700 0.0535
1000 0.0496
2000 0.0533
3000 0.0511
5000 0.0559
10000 0.0602
20000 0.075
50000 0.103
1000000 0.495'''
