batchRanges =[
#5, 10, 20, 50, 100, 200, 500, 700,]
        #500, 700, 1000, 2000, 3000, 5000, 
#10000, 20000, 50000, 75000, 100000, 200000]
100, 500, 1000, 5000, 10000, 20000, 50000, 75000, 100000, 200000]
threadCount = [1, 3, 5, 8, 10, 15, 20, 25]
threadCount = [30, 35, 40] # , 50, 60, 80] # , 100, 120, 160]
batchRanges = threadCount
import os
import sys
import subprocess
trials = 3
executables = ['STlocal'] # ["seqMIS", "incMIS", "txnMIS", "paraMIS", "slowMIS"] # "MIS"
inputFiles = ["randLocalGraph_E_5_10000000", "rMatGraph_E_5_10000000", "3Dgrid_E_10000000"]  # ["randLocalGraph_J_5_10000000", "rMatGraph_J_5_10000000", "3Dgrid_J_10000000"]
#if len(sys.argv) > 1:
    # inputFile = sys.argv[1]
 #   executable = sys.argv[1]
bd1 = []
bd2 = []
bd3 = []
def avg(l):
    return float(sum(l))/len(l)
res = {}
for executable in executables:
    for inputFile in inputFiles:

      timeList = []
      maxList = []
      for batchSize in batchRanges:
            currentL = []
            breakdown1 = []
            breakdown2 = []
            breakdown3 = []
            for t in range(trials):
                ret = subprocess.check_output("numactl -i 0,1,2,3 -N 0,1,2,3 -- ./" + executable + " -b 200000 -c " + str(batchSize) + " " + inputFile, shell=True)
                time = float(ret.split('PBBS-time:')[1].split('\n')[0].strip())
                currentL.append(1. / time)
                '''breakdown = ret.split('Time ')[1:]
                #print ret
                bd = breakdown[0].split('\n')[0].split(',')
                #breakdown2 = breakdown[1].split('\n')[0].split(',')
                breakdown1.append(int(bd[0]))
                breakdown2.append(int(bd[1]))
                breakdown3.append(int(bd[2]))'''
            timeList.append(avg(currentL))
            maxList.append(max(currentL))
            '''bd1.append(avg(breakdown1))
            bd2.append(avg(breakdown2))
            bd3.append(avg(breakdown3))
            '''
            print batchSize, avg(currentL), max(currentL), currentL, breakdown1, breakdown2, breakdown3
      res[executable + "_" + inputFile] = timeList
      res[executable + "_" + inputFile + "_max"] = maxList
      print executable, inputFile, timeList, maxList

      output = []
      for i in range(len(bd1)):
            output.append("%d, %d, %d" % (bd1[i], bd2[i], bd3[i]))
      print executable + "_" + inputFile, "["+ "; ".join(output) + "]"
print res




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
