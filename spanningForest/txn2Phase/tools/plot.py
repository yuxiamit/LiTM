from options import *
import os
import json
basePath = '/'.join(os.path.dirname(os.path.realpath(__file__)).split('/')[:-1])
outputFile = 'plot.m'

import sys
if len(sys.argv) > 1:
    outputFile = sys.argv[1]

x = '-b'
xvalues = runtime[x][1:]  # all the values
y = 'currentL'
totalResult = {}
def reducerY(l):
    if len(l) == 0:
        return 0
    return float(sum(l))/len(l)

def reducerX(x):
    return int(x)

def plot(config):
    name = getNameByOption(config)
    nameL = [name]
    resultLine = {}
    def plotInner(runoptions):
        outputFileName = nameL[0] + getNameByOption(runoptions, '')
        print outputFileName
        try:
            content = open('%s/results/%s.txt' % (basePath, outputFileName), 'r').read()
            print content
            obj = json.loads(content)
            yval = reducerY(obj[y])
        except:
            yval = 0
        resultLine[reducerX(runoptions[x])] = yval
    enum(plotInner, runtime)
    totalResult[config['_legend'] + '_' + name] = [resultLine[v] for v in xvalues]

for opt in optionList:
    enum(plot, opt)

matlabCode = 'x = %s;\nhold on\n' % repr(xvalues)
for name in totalResult:
    matlabCode += 'y%s = %s;\n' % (name.replace(' ','_'), repr(totalResult[name]))
    matlabCode += 'semilogx(x, y%s)\n' % name.replace(' ', '_')
matlabCode += 'legend("%s")\n' % '","'.join([s.replace('_','-') for s in totalResult.keys()])
matlabCode += "set(gca,'xscale','log')"
open(basePath + '/' + outputFile,'w').write(matlabCode)


