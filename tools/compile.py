# compile.py



from options import *
import os
import re
import subprocess 
from IPython import embed
from sys import exit

basePath = '/'.join(os.path.dirname(os.path.realpath(__file__)).split('/')[:-1])
templateFile = basePath + '/tools/configtemplate'
configTemplate = open(templateFile,'r').read()
configPattern = r'//\s*\{\{options\}\}'
disclaimer = '// This file is auto-generated from %s' % templateFile
print 'basePath', basePath
executable = 'MIS'
def serialize(v):
    if isinstance(v, bool):
        return str(v).lower()
    return str(v)

def compile(option):
    if '_commit' in option:
        commitVersion = option['_commit']
        ret = subprocess.check_output("cd '%s'; git checkout %s" % (basePath, commitVersion), shell=True)
        if 'error' in ret:
            print ret
            exit(0)
    code = ''
    # print option
    keys = removeMetaData(option.keys())
    for k in keys:
	# if k[0] != '_':
       		code += '#define %s %s\n' % (k, serialize(option[k]))
    configcode = re.sub(configPattern, code, disclaimer + '\n' + configTemplate)
    print configcode
    open(basePath + '/config.h', 'w').write(configcode)
    command = "cd '%s'; make clean; rm %s; make; mv %s %s" % (basePath, executable, executable, getNameByOption(option))
    proc = subprocess.Popen(command, shell=True, stdout=subprocess.PIPE) # , stderr=subprocess.STDOUT)
    while proc.poll() is None:
        commandResult = proc.wait() #catch return code
        if commandResult != 0:
            print  "Error in job. "  + repr(option), commandResult
            exit(0)
            # embed()  # deal with error and pause

for opt in optionList:
    enum(compile, opt)
subprocess.check_output("cd '%s'; git checkout master" % basePath, shell=True)  # restore the latest version.

