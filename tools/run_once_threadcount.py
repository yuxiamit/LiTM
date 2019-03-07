batchRanges =[
#5, 10, 20, 50, 100, 200, 500, 700,]
        #500, 700, 1000, 2000, 3000, 5000, 
#10000, 20000, 50000, 75000, 100000, 200000]
100, 500, 1000, 5000, 10000, 20000, 50000, 75000, 100000, 200000]
threadCount = [1, 3, 5, 10, 20, 30, 40] #[1, 3, 5, 8, 10, 15, 20, 25, 30, 35, 40] # , 50, 60, 80] # , 100, 120, 160]
batchRanges = threadCount
import os
import sys
import subprocess
import datetime
trials = 3 # 10
def addPrefix(l, prefix):
    return [prefix + s for s in l]
def addSuffix(l, suffix):
    return [s + suffix for s in l]
resDir = 'result/stmDB/'
if not os.path.exists(resDir):
    os.makedirs(resDir)
appConfig = [
    {
        'name':'MIS',
        'directory':'maximalIndependentSet',
        'versions':['txn2Phase', 'txn2PhaseDB'], #['serialMIS','txn2PhaseRE', 'txn2Phase-stm2v-opt','txn2Phase', 'incrementalMIS', 'slowerMIS'],
        'alias': ['stm', 'stmDB'], # 'serial','stmRE', 'stm2v_opt', 'stm', 'inc', 'slow'],
        'data':["randLocalGraph_J_5_10000000", "rMatGraph_J_5_10000000", "3Dgrid_J_10000000"],
	'batchSize':200000,
    },
    {
        'name':'matching',
        'directory':'maximalMatching',
        'versions':['serialMatching', 'txn2PhaseRE', 'txn2Phase-stm2v-opt','txn2Phase', 'incrementalMatching'],
        'alias':['serial', 'stmRE', 'stm2v_opt', 'stm', 'inc'],
        'data':["randLocalGraph_E_5_10000000", "rMatGraph_E_5_10000000", "2Dgrid_E_10000000", "expGraph_E_5000000", "starGraph_E_50000000"],
        'batchSize':200000
    },
    {
        'name':'ST',
        'directory':'spanningForest',
        'versions':['serialST', 'txn2PhaseRE', 'txn2Phase-stm2v-opt','txn2Phase', 'incrementalST'],
        'alias':['serial', 'stmRE', 'stm2v_opt', 'stm', 'inc'],
        'data':["randLocalGraph_E_5_10000000", "rMatGraph_E_5_10000000", "3Dgrid_E_10000000"],
        'batchSize':200000,
    },
    {
        'name':'randPerm',
        'directory':'randPerm',
        'versions': ['serialRandPerm2', 'parallelRandPerm2','txn2Phase', 'txn2PhaseRE'],
        'alias': ['serial', 'parallel', 'stm', 'stmRE'],
        'data':['1000000000'],# ["randLocalGraph_E_5_10000000", "rMatGraph_E_5_10000000", "3Dgrid_E_10000000"]
        'batchSize': 2000000,
    },
    {
        'name':'listRanking',
        'directory':'listRanking',
        'versions':['serialListRanking2', 'parallelListRanking2','txn2Phase', 'txn2PhaseRE'],
        'alias':['serial', 'parallel', 'stm', 'stmRE'],
        'data':['10000000'],# ["randLocalGraph_E_5_10000000", "rMatGraph_E_5_10000000", "3Dgrid_E_10000000"]
        'batchSize': 200000,
    },
]
totalRes = {}
for app in appConfig[3:4]: # appConfig[-2:]: # [appConfig[3]]:
    appName = app['name']
    appDir = app['directory']
    appVers = app['versions']
    
    appAlias = app['alias']
    appData = app['data']
    bs = app['batchSize']
    executables = addPrefix(addSuffix(appVers, '/' + appName), appDir + '/')

    if appName in ['randPerm', 'listRanking']:
       inputFiles = appData
    else:
       inputFiles = addPrefix(appData, appDir + '/graphData/data/')
    #if len(sys.argv) > 1:
        # inputFile = sys.argv[1]
    #   executable = sys.argv[1]
    bd1 = []
    bd2 = []
    bd3 = []
    def avg(l):
        return float(sum(l))/len(l)
    res = {}
    for i in range(len(executables)): # executable in executables:
        executable = executables[i]
        currentAlias = appAlias[i]
        currentDir, currentVer, currentApp = tuple(executable.split('/'))
        print 'Compiling %s' % executable
        ret = subprocess.check_output("cd %s; make clean; make" % (currentDir + '/' + currentVer), shell=True)
        
        for inputFile in inputFiles:

            currentData = inputFile.split('/')[-1]
            timeList = []
            maxList = []
            for batchSize in batchRanges:
                    currentL = []
                    breakdown1 = []
                    breakdown2 = []
                    breakdown3 = []
                    for t in range(trials):
                     noError = True
                     while noError:
                      try:
                        #print '!!', "numactl -i 0,1,2,3 -N 0,1,2,3 -- ./" + executable + " -b 200000 -c " + str(batchSize) + " " + inputFile
                        ret = subprocess.check_output("numactl -i 0,1,2,3 -N 0,1,2,3 -- ./" + executable + " -b " + str(bs) + " -c " + str(batchSize) + " " + inputFile, shell=True)
                        noError = False
                      except subprocess.CalledProcessError:
                        continue
                      #print ret, ret.split('PBBS-time:')[1].split('\n')[0].strip()
                      time = float(ret.split('PBBS-time:')[1].split('\n')[0].strip())
                      #if time <=0:
                      #                               time = 1e-6
                      #print ret
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
            
            ansAlias = "%s_%s_%s" % (currentApp, currentAlias, currentData)
            ansAlias.replace('-','_') # to adapt MATLAB naming conventions
            res[ansAlias] = timeList
            res[ansAlias + "_max"] = maxList
            print executable, inputFile, timeList, maxList

            output = []
            for i in range(len(bd1)):
                    output.append("%d, %d, %d" % (bd1[i], bd2[i], bd3[i]))
            print executable + "_" + inputFile, "["+ "; ".join(output) + "]"
    open(resDir + appName + '_' + datetime.datetime.today().strftime('%Y%m%d-%H%M'), 'w').write(repr(res))
    open(resDir + appName + '_latest', 'w').write(repr(res))
    totalRes.update(res)
print '=' * 20
print 'Done'
print '=' * 20
print totalRes
    
open(resDir + datetime.datetime.today().strftime('%Y%m%d-%H%M'), 'w').write(repr(totalRes))
open(resDir + '_latest', 'w').write(repr(totalRes))


