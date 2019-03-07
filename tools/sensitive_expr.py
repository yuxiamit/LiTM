import os
import math
import sys
import subprocess
import datetime
import re
import matplotlib as mpl
mpl.rcParams['hatch.linewidth'] = 0.05
font = {'family' : 'serif',
        'weight' : 'normal',
        'size'   : 22}
mpl.rc('font', **font)
mpl.use('Agg')
import matplotlib.pyplot as plt
resRE = re.compile(r'total keep (?P<keep>\d+)\s+Time (?P<time1>\d+),(?P<time2>\d+),(?P<time3>\d+)[\s\S]+PBBS-time: (?P<sec>[\d\.]+)')
FIGURE_DIR = 'figures/'
NAME = 'LiTM' # 'detReserve'
MARKER_SIZE = 10
trials = 5
colors = ['#136e89', '#52a1c7', '#f9f6bc', '#dfca99', '#6d2b2b']
serialPerf = {'MIS_serial_randLocalGraph_J_5_100000_max': [446.42857142857144, 442.47787610619474, 442.47787610619474, 446.42857142857144, 442.47787610619474, 448.43049327354254], 'MIS_serial_randLocalGraph_J_5_1000000_max': [19.49317738791423, 19.53125, 19.53125, 19.68503937007874, 19.76284584980237, 19.646365422396855], 'MIS_serial_randLocalGraph_J_5_100000000': [0.08771929824561403, 0.08771929824561403, 0.08771929824561403, 0.08771929824561403, 0.08771929824561403, 0.08771929824561403], 'MIS_serial_randLocalGraph_J_5_10000000': [1.9267822736030829, 1.923076923076923, 1.9342359767891681, 1.9305019305019304, 1.923076923076923, 1.9267822736030829], 'MIS_serial_randLocalGraph_J_5_10000000_max': [1.9267822736030829, 1.923076923076923, 1.9342359767891681, 1.9305019305019304, 1.923076923076923, 1.9267822736030829], 'MIS_serial_randLocalGraph_J_5_100000000_max': [0.08771929824561403, 0.08771929824561403, 0.08771929824561403, 0.08771929824561403, 0.08771929824561403, 0.08771929824561403], 'MIS_serial_randLocalGraph_J_5_1000000': [19.49317738791423, 19.53125, 19.53125, 19.68503937007874, 19.76284584980237, 19.646365422396855], 'MIS_serial_randLocalGraph_J_5_100000': [446.42857142857144, 442.47787610619474, 442.47787610619474, 446.42857142857144, 442.47787610619474, 448.43049327354254]}

def addPrefix(l, prefix):
    return [prefix + s for s in l]
def addSuffix(l, suffix):
    return [s + suffix for s in l]
resDir = 'result/stm_sensitive/'
if not os.path.exists(resDir):
    os.makedirs(resDir)

style = {
    'stm': '-o',
    'serial': '-x',
    'stmRE': '-<',
    'inc': '-*',
    'parallel': '-*',
    'detBase': ':>',
    'pull': ':>',
    'blockedasync': ':>',
}

colormap = {
    'stm': '#F56E48',
    'serial': '#00798C',
    'stmRE': '#6F7570',
    'inc': '#F7CB15',
    'parallel': '#F7CB15',
    'detBase': '#79BD8F',
    'pull': '#79BD8F',
    'blockedasync': '#79BD8F',
}
xlabels = {
  'ths': 'Number of Threads',
  'is': 'Number of Vertices',
  'bs': 'Batch Size',
  'lt': 'Input Size / Lock Table Size'
}

ylabels = {
  'bd': 'Normalized Time Breakdown',
  'thr': 'Speedup over Serial', # 'Normalized Throughput',
  'keep': 'Total Abort Rate',
}

appConfig = [
    {
        'name':'MIS',
        'directory':'maximalIndependentSet',
        'versions':['txn2Phase', 'serialMIS', 'incrementalMIS'], # ['serialMIS', 'txn2PhaseRE', 'txn2Phase', 'incrementalMIS', 'slowerMIS'], # ['serialMIS','txn2PhaseRE', 'txn2Phase-stm2v-opt','txn2Phase', 'incrementalMIS', 'slowerMIS'],
        'alias': ['stm', 'serial', 'inc'], # ['serial','stmRE', 'stm', 'inc', 'slow'],
        'data':["randLocalGraph_J_5_100000000"], # , "rMatGraph_J_5_100000000", "3Dgrid_J_100000000"], # ["randLocalGraph_J_5_100000000", "rMatGraph_J_5_100000000", "3Dgrid_J_100000000"],
	'batchSize':200000,
    },
    {
        'name':'matching',
        'directory':'maximalMatching',
        'versions':['txn2Phase', 'serialMatching', 'incrementalMatching'], # ['serialMatching', 'txn2PhaseRE', 'txn2Phase', 'incrementalMatching'], # ['serialMatching', 'txn2PhaseRE', 'txn2Phase-stm2v-opt','txn2Phase', 'incrementalMatching'],
        'alias':['stm', 'serial', 'inc'], # , 'stmRE', 'stm', 'inc'],
        'data': ['3Dgrid_E_100000000'], # ["rMatGraph_E_5_100000000", "2Dgrid_E_100000000"],# ["randLocalGraph_E_5_100000000", "rMatGraph_E_5_100000000", "2Dgrid_E_100000000", "expGraph_E_5000000", "starGraph_E_50000000"],
        'batchSize':200000,
    },
    {
        'name':'ST',
        'directory':'spanningForest',
        'versions':['txn2Phase', 'serialST', 'incrementalST'], # ['serialST', 'txn2PhaseRE','txn2Phase', 'incrementalST'],
        'alias':['stm', 'serial', 'inc'], # ['serial', 'stmRE', 'stm', 'inc'],
        'data':["rMatGraph_E_5_100000000", "3Dgrid_E_100000000"],  #["randLocalGraph_E_5_100000000", "rMatGraph_E_5_100000000", "3Dgrid_E_100000000"],
        'batchSize':200000,
    },
    {
	'name':'pagerank',
	'directory':'pagerank',
	'versions':['txn2Phase', 'txnSerial'], # ['txnSerial', 'txn2Phase'],
	'alias':['stm', 'serial'], # ['serial', 'stm'],
	'data':["rMatGraph_J_5_100000000", "3Dgrid_J_100000000"], # ["randLocalGraph_J_5_100000000"],
	'batchSize':50000,

    },
    {
        'name':'randPerm',
        'directory':'randPerm',
        'versions': ['serialRandPerm2', 'parallelRandPerm2','txn2Phase'],
        'alias': ['serial', 'parallel', 'stm'],
        'data':['1000000000'],# ["randLocalGraph_E_5_10000000", "rMatGraph_E_5_10000000", "3Dgrid_E_10000000"]
        'batchSize': 200000,
    },
    {
        'name':'listRanking',
        'directory':'listRanking',
        'versions':['serialListRanking2', 'parallelListRanking2','txn2Phase'], # , 'txn2PhaseRE'],
        'alias':['serial', 'parallel', 'stm'], # , 'stmRE'],
        'data':['1000000000'],# ["randLocalGraph_E_5_10000000", "rMatGraph_E_5_10000000", "3Dgrid_E_10000000"]
        'batchSize': 200000,
    },
]

ESTIMATE_RUNNINGTIME = 60 # second

def estimateTime(iterRange, localAppConfig):
    configCollector = 0
    for app in localAppConfig:
        for inputF in app['data']: # inputFiles:
            for t in iterRange:
                configCollector += trials * ESTIMATE_RUNNINGTIME
    return configCollector


def expr(prefix, iterRange, commandlineGenerator, localAppConfig):
  print "Estimate time", estimateTime(iterRange, localAppConfig) / 60 # minutes
  totalRes = {}
  for app in localAppConfig: # appConfig[-2:]: # [appConfig[3]]:
    appName = app['name']
    appDir = app['directory']
    # only test 2phase-stm for sensitive study
    if prefix == '_is':
      appVers = app['versions']
    else:
      appVers = ['txn2Phase']      # app['versions']
    
    appAlias = app['alias']
    appData = app['data']
    bs = app['batchSize']
    executables = addPrefix(addSuffix(appVers, '/' + appName), appDir + '/')

    if appName in ['randPerm', 'listRanking']:
       inputFiles = appData
    else:
       inputFiles = addPrefix(appData, appDir + '/graphData/data/')
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
            bd1 = []
            bd2 = []
            bd3 = []
            keepList = []
            #for batchSize in batchRanges:
            for batchSize in iterRange:
                    currentL = []
                    keepL = []
                    breakdown1 = []
                    breakdown2 = []
                    breakdown3 = []
                    for t in range(trials):
                     noError = True
                     bst = bs
                     #cmdLine = "numactl -i 0,1,2,3 -N 0,1,2,3 -- ./" + executable + " -b " + str(bs) + " -c " + str(batchSize) + " " + inputFile
                     while noError:
                      try:
                        # print '!!', "numactl -i 0,1,2,3 -N 0,1,2,3 -- ./" + executable + " -b 200000 -c " + str(batchSize) + " " + inputFile
                        cmdLine = commandlineGenerator(batchSize, locals())
                        ret = subprocess.check_output(cmdLine, shell=True)
                        noError = False
                      except subprocess.CalledProcessError:
                        print cmdLine
                        bs /= 2
                        continue
                      #print ret, ret.split('PBBS-time:')[1].split('\n')[0].strip()
                      if currentAlias in ['inc', 'serial']:
                       time = float(ret.split('PBBS-time:')[1].split('\n')[0].strip())
                       keepNum = 0
                       timeBreakdown = [0,0,0]
                      else:
                       reDict = [m.groupdict() for m in resRE.finditer(ret)][0]
                       time = float(reDict['sec'])
                       keepNum = int(reDict['keep'])
                       timeBreakdown = [int(reDict['time1']), int(reDict['time2']), int(reDict['time3'])]
                      #time = float(ret.split('PBBS-time:')[1].split('\n')[0].strip())
                      keepL.append(keepNum)
                      breakdown1.append(timeBreakdown[0])
                      breakdown2.append(timeBreakdown[1])
                      breakdown3.append(timeBreakdown[2])
                      currentL.append(1. / time)
                      bs = bst  # recover the bs
                    timeList.append(avg(currentL))
                    maxList.append(max(currentL))
                    bd1.append(avg(breakdown1))
                    bd2.append(avg(breakdown2))
                    bd3.append(avg(breakdown3))
                    keepList.append(avg(keepL))
                    print batchSize, avg(currentL), max(currentL), currentL, breakdown1, breakdown2, breakdown3
            
            ansAlias = "%s_%s_%s" % (currentApp, currentAlias, currentData)
            ansAlias.replace('-','_') # to adapt MATLAB naming conventions
            res[ansAlias + "_max"] = maxList
            res[ansAlias + "_struct"] = {
                    'thr':maxList,
                    'bd':[bd1, bd2, bd3],
                    'keep':keepList,
                    }
            print executable, inputFile, timeList, maxList

            output = []
            for i in range(len(bd1)):
                    output.append("%d, %d, %d" % (bd1[i], bd2[i], bd3[i]))
            print executable + "_" + inputFile, "["+ "; ".join(output) + "]"
    open(resDir + appName + '_' + datetime.datetime.today().strftime('%Y%m%d-%H%M') + prefix, 'w').write(repr(res))
    open(resDir + appName + '_latest' + prefix, 'w').write(repr(res))
    totalRes.update(res)
  print totalRes
  open(resDir + datetime.datetime.today().strftime('%Y%m%d-%H%M') + prefix, 'w').write(repr(totalRes))

  open(resDir + '_latest' + prefix, 'w').write(repr(totalRes))
  return totalRes

def plotStruct(prefix, itrange, appConfig, res):
    code = '''
%########################
%# Plot Structure $PREFIX$
%########################
'''.replace('$PREFIX$', prefix)
    assert len(appConfig) == 1
    app = appConfig[0]
    appName = app['name']
    appDir = app['directory']
    if prefix == '_is':
      appVers = app['versions']
    else:
      appVers = ['txn2Phase']
    appAlias = app['alias']
    appData = app['data']
    executables = addPrefix(addSuffix(appVers, '/' + appName), appDir + '/')
    bs = app['batchSize']
    if appName in ['randPerm', 'listRanking']:
        inputFiles = appData
    else:
        inputFiles = addPrefix(appData, appDir + '/graphData/data/')
    inputFile = inputFiles[0]
    currentData = inputFile.split('/')[-1]
    def drawLineMatlab(x, y, style, filename, xlab, ylab, xdesc='', ydesc=''):
        localCode = "fig = figure()\nhold on\n"	
        localCode += "x = %s;\n" % x
        if style != "":
            style = ', ' + style
        if isinstance(y[0], list):
            for yi in range(len(y)):
                localCode += "y%d = %s;\n" % (yi, y[yi])
                localCode += "plot(x, y%d %s);\n" % (yi, style)
        else: 
            localCode += "y = %s;\n" % y
            localCode += "plot(x, y %s);\n" % style
        localCode += "saveas(fig, '%s.pdf')\n" % filename
        return localCode
    def sciConv(x, base=10):
        t = str(x)
        if base==2:
            # assuming x is in 2^k
            s = int(math.log(x, 2))
            return r'$2^{%d}$' % s
        ik = len(t) - len(t.rstrip('0'))
        if t.rstrip('0') == '1':
            return '$10^{%d}$' % ik
        return '$' + t.rstrip('0') + r'\times{}10^{%d}' % ik + '$'
    def drawLine(x, xlab, ylab, xdesc='', ydesc=''):
      plt.figure(figsize=(8, 6))
      if not(xdesc == 'is' and ydesc == 'thr'):
          executables_t = [executables[0]]
      else:
          executables_t = executables
      for i in range(len(executables_t)):
        currentAlias = appAlias[i]
        executable = executables[i]
        currentDir, currentVer, currentApp = tuple(executable.split('/'))
        ansAlias = "%s_%s_%s" % (currentApp, currentAlias, currentData)
        ansAlias.replace('-','_') # to adapt MATLAB naming conventions
        currentStruct = res[ansAlias + "_struct"]
        filename = ansAlias + prefix + '_' + ydesc
        #with currentStruct[ydesc] as y:
        y = currentStruct[ydesc]
        print(ydesc, y)
        if True:
          ax = plt.gca()
          currentLabel = currentAlias
          if ydesc in ['thr']:
            sp = serialPerf['MIS_serial_randLocalGraph_J_5_100000000_max']
            spavg = float(sum(sp)) / len(sp)
            if xdesc in ['is']:
              if currentAlias == 'stm':
                  currentLabel = NAME 
              elif currentAlias in ['inc', 'parallel']:
                  currentLabel = 'PBBS'
              elif currentAlias == 'serial':
                  currentLabel = 'Serial'
              y = [float(y[i])/(float(sum(serialPerf['MIS_serial_randLocalGraph_J_5_%d_max' % x[i]])) / len(sp))   for i in range(len(x))]
            else:
              y = [float(y[i]) / spavg for i in range(len(x))]
          if ydesc in ['keep']:
            fn = float(10**8)  # default input size
            if xdesc in ['is']: # input size sweep
              y = [float(y[i])/x[i] for i in range(len(x))]
            else:
              y = [float(y[i])/fn for i in range(len(x))]
          ax.plot(x, y, style[currentAlias], linewidth=2, label=currentLabel, color=colormap[currentAlias], markersize = MARKER_SIZE)
      if ydesc == 'keep':
          plt.subplots_adjust(left=0.15)
      if ydesc == 'thr' and xdesc == 'is':
          # add galois
          y = res['galois_detBase']
          y = [float(y[i])/(float(sum(serialPerf['MIS_serial_randLocalGraph_J_5_%d_max' % x[i]])) / len(sp))   for i in range(len(x))]
          ax.plot(x, y, style['detBase'], linewidth=2, label='Galois', color=colormap['detBase'], markersize = MARKER_SIZE)
          filename = filename.replace('inc', 'stm')
      if xdesc in ['lt', 'is', 'bs']:
          ax = plt.gca()
          ax.set_xscale('log')
          ax.get_xaxis().set_major_formatter(mpl.ticker.ScalarFormatter())
          if xdesc in ['bs', 'is']:
            if xdesc == 'bs':
              tickx = [10**tk for tk in range(4, 8)]
            else:
              tickx = [10**tk for tk in range(5, 9)] # x # [10**tk for tk in range(4,8)]
            sciX = [sciConv(xi)  for xi in tickx]
            print(sciX)
            #ax.set_xticks(x, sciX)
            plt.xticks(tickx, sciX)
          else:
            tickx = [2**tk for tk in range(0, 16, 3)]
            sciX = [sciConv(xi, 2)  for xi in tickx]
            print(sciX)
            plt.xticks(tickx, sciX)
            #ax.set_xticks(x, sciX)
      plt.xlabel(xlab)
      plt.ylabel(ylab)
      if xdesc == 'is' and ydesc == 'thr':
        ax = plt.gca()
        handles,labels = ax.get_legend_handles_labels()
        handles = [handles[2], handles[0], handles[1], handles[3]]
        labels = [labels[2], labels[0], labels[1], labels[3]]
        plt.legend(handles, labels, loc='upper left')
        plt.subplots_adjust(left=0.145)
        # plt.legend(loc=2)
      plt.subplots_adjust(bottom=0.125)
      # plt.xlim(xmin=0.0)
      plt.ylim(ymin=0.0)
      plt.savefig(FIGURE_DIR + filename + '.pdf')
      return ''
    def findcorrespond(y, x, xo):
      for xi in x:
        if not xi in xo:
          print('bad xi %d in %s' % (xi, str(xo)))
      return [y[xo.index(xi)] for xi in x]

    def drawBar(x, xlab, ylab, xdesc='', ydesc=''):
      plt.figure(figsize=(8, 6))
      oldx = x
      if xdesc == 'lt':
        x = [2**tk for tk in range(0, 16, 3)]  # to make it uniform
      for i in range(len(executables)):
       currentAlias = appAlias[i]
       executable = executables[i]
       currentDir, currentVer, currentApp = tuple(executable.split('/'))
       ansAlias = "%s_%s_%s" % (currentApp, currentAlias, currentData)
       ansAlias.replace('-','_') # to adapt MATLAB naming conventions
       currentStruct = res[ansAlias + "_struct"]
       filename = ansAlias + prefix + '_' + ydesc
       #with currentStruct[ydesc] as y:
       y = [findcorrespond(yi, x, oldx) for yi in currentStruct[ydesc]]
       if True:
        y = [elementwiseDiv(l, 10**6) for l in y]  # convert time to ms
        assert isinstance(y[0], list)
        def pairwiseAdd(l1, l2):
          assert len(l1)==len(l2)
          return [l1[i] + l2[i] for i in range(len(l1))]
        t = pairwiseAdd(range(len(x)), [-0.3]*len(x))
        ysum = [y[0][i] + y[1][i] + y[2][i] for i in range(len(y[0]))]
        for i in range(len(y[0])):
          y[0][i] = y[0][i] / ysum[i]
          y[1][i] = y[1][i] / ysum[i]
          # just in case rounding error happens here
          y[2][i] = 1. - y[0][i] - y[1][i]
        # y = [elementwiseDivList(y[0], ysum), elementwiseDivList(y[1], ysum), elementwiseDivList(y[2], ysum)]
        plt.bar(t, y[0], color=colors[0], hatch='xx', label='Reserve Phase', edgecolor='#000000')
        plt.bar(t, y[1], bottom=y[0], color=colors[1], hatch='//', label='Commit Phase', edgecolor='#000000') 
        plt.bar(t, y[2], bottom=pairwiseAdd(y[1], y[0]), color=colors[2], label='Cleanup Phase', edgecolor='#000000')
      plt.xlabel(xlab)
      plt.ylabel(ylab)
      plt.legend(loc='lower right') # ['Phase 1', 'Phase 2', 'Phase 3'])
      if xdesc == 'bs':
          plt.xticks(pairwiseAdd(t, [0.0]*len(x)), [sciConv(xi) for xi in x], fontsize=16)
      elif xdesc == 'lt':
          #tickx = [2**tk for tk in range(0, 16, 3)]
          #sciX = [sciConv(xi, 2) for xi in tickx]
          #plt.xticks(tickx, sciX)
          plt.xticks(pairwiseAdd(t, [0.0]*len(x)), [sciConv(xi, 2) for xi in x],  fontsize=16)
      else:
          plt.xticks(pairwiseAdd(t, [0.0]*len(x)), x, fontsize=16)
      plt.subplots_adjust(bottom=0.125)
      plt.savefig(FIGURE_DIR + filename + '.pdf')
      return ''

    def elementwiseDiv(li, d):
        return [l/d for l in li]
    def elementwiseDivList(li, di):
        return [li[i] / di[i] for i in range(len(li))]

    code += drawLine(itrange, xlabels[prefix[1:]], ylabels['thr'], xdesc=prefix[1:], ydesc='thr')
    code += drawLine(itrange, xlabels[prefix[1:]], ylabels['keep'], prefix[1:], 'keep')
    #code += drawLine(itrange, currentStruct['bd'], '', ansAlias + prefix + '_bd')
    if prefix != '_is':
      code += drawBar(itrange, xlabels[prefix[1:]], ylabels['bd'], prefix[1:], 'bd')
    print code
    return code

def threadSweep(it, local):
      return "numactl -i 0,1,2,3 -N 0,1,2,3 -- ./" + local['executable'] + " -b " + str(local['bs']) + " -c " + str(it) + " -l 1  " + local['inputFile']

def batchsizeSweep(it, local):
      return "numactl -i 0,1,2,3 -N 0,1,2,3 -- ./" + local['executable'] + " -b " + str(it) + " -c 40 -l 1 " + local['inputFile']

def inputsizeSweep(it, local):
    return "numactl -i 0,1,2,3 -N 0,1,2,3 -- ./" + local['executable'] + " -b " + str(local['bs']) + " -c 40 -l 1 " + local['inputFile'].replace('100000000', str(it))

def locktableSizeSweep(it, local):
     return "numactl -i 0,1,2,3 -N 0,1,2,3 -- ./" + local['executable'] + " -b " + str(local['bs']) + " -c 40 -l " + str(it) + " " + local['inputFile']


thrRange = [1, 5, 10, 20, 30, 40]
bsRange = [10000, 50000, 100000, 200000, 500000, 1000000, 10000000]
isRange = [10**5, 10**6, 10**7, 10**8]
ltsRange = [1, 8, 64, 256, 512, 1024, 4096, 32768]

def main(args):
    resThr = {}
    resBS = {}
    resIS = {}
    resLTS = {}
  ############################
  # Thread Sweep
  ############################

    resThr = expr('_ths', thrRange, threadSweep, appConfig[:1])

############################
# Batch Size Sweep
############################

    resBS = expr('_bs', bsRange, batchsizeSweep, appConfig[:1])

############################
# Input Size Sweep
############################

    resIS = expr('_is', isRange, inputsizeSweep, appConfig[:1])
############################
# Lock Table Size Sweep
############################
    resLTS = expr('_lt', ltsRange, locktableSizeSweep, appConfig[:1])
    return resThr, resBS, resIS, resLTS

def generatePlot(**kwargs):
  if kwargs['resThr'] != {}:
    plotStruct('_ths', thrRange, appConfig[:1], kwargs['resThr'])
  if kwargs['resBS'] != {}:
    plotStruct('_bs', bsRange, appConfig[:1], kwargs['resBS'])
  if kwargs['resIS'] != {}:
    plotStruct('_is', isRange, appConfig[:1], kwargs['resIS'])
  if kwargs['resLTS'] != {}:
    plotStruct('_lt', ltsRange, appConfig[:1], kwargs['resLTS'])


def loadLatest(prefix):
  return eval(open(resDir + '_latest' + prefix, 'r').read())  

if __name__=="__main__":
  import argparse
  parser = argparse.ArgumentParser(description='Sensitive Study')
  parser.add_argument('--data', action='store')
  parser.add_argument('--run', action='store')
  parser.add_argument('--bs', action='store')
  parser.add_argument('--lt', action='store')
  args = parser.parse_args()
  resThr, resBS, resIS, resLTS = loadLatest('_ths'), loadLatest('_bs'), loadLatest('_is'), loadLatest('_lt') 
  # resIS.update({'galois_detBase': [35.714285714285715, 8.264462809917356, 1.1695906432748537, 0.07693491306354824]})
  resIS.update({'galois_detBase': [25.42412460885807, 7.904083342287388, 1.1077881944887233, 0.13155912737944758]})
  if args.data != None:
    dat = __import__(args.data)
    resThr, resBS, resIS, resLTS = dat.resThr, dat.resBS, dat.resIS, dat.resLTS
  if args.run != None:
    resThr, resBS, resIS, resLTS = main(args)
  if args.bs != None:
    resBS = eval(open(resDir + args.bs + '_bs', 'r').read())
  if args.lt != None:
    resLTS = eval(open(resDir + args.lt + '_lt', 'r').read())
  generatePlot(resThr=resThr, resBS=resBS, resIS=resIS, resLTS=resLTS)

