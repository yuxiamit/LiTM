# run.pt
import sys

sys.path.append('~/pip/')

from options import *
import subprocess32
import os
import json

trials = 20 # 10
timeout = 20 # seconds
basePath = '/'.join(os.path.dirname(os.path.realpath(__file__)).split('/')[:-1])
def avg(l):
    return float(sum(l))/len(l)

def serializeParameters(k, v):
    if k[0] == '-':
        if isinstance(v, bool):
            if v:
                return k
            else:
                return ''
        return k + ' ' + str(v)
    else:
        return str(v)

def run(compileOption):
    name = getNameByOption(compileOption)
    nameL = [name] # pass argument to inner function
    def runInner(runoption):
        command = 'numactl -i all -- ./' + nameL[0]
        for k, v in sorted(runoption.items()):
            command += ' ' + serializeParameters(k, v)
        print command
        currentL = []
        retL = []
        for t in range(trials):
            hasResult = False
            while not hasResult:
                try:
                    ret = subprocess32.check_output('cd %s; %s' % (basePath, command), shell=True, timeout=timeout)
                    hasResult = True
                except subprocess32.CalledProcessError as exc:
                    print "Status : FAIL", exc.returncode, exc.output
                    # retL.append(str(exc.output))
                    continue # no data points here
                except subprocess32.TimeoutExpired as te:
                    print "Timeout Expired"
                    continue
            retL.append(ret)
            time = float(ret.split('PBBS-time:')[1].strip())
            currentL.append(time)
        outputFileName = nameL[0] + getNameByOption(runoption, '')
        open('%s/results/%s.txt' % (basePath, outputFileName), 'w').write(json.dumps({
            'currentL':currentL,
            'retL':retL
            }))
    enum(runInner, runtime)

for opt in optionList:
    enum(run, opt)
