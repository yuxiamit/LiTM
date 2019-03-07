import resource, sys
from utils import *
resource.setrlimit(resource.RLIMIT_STACK, (2**29,-1))
sys.setrecursionlimit(10**6)

# from default import *

#configname = 'default'
#configname = 'multiheal'
#configname = 'multiheal_noreducer'
#configname = 'multi1224'
configname = 'multi0118'
import importlib

config_module = importlib.import_module(configname)
optionList = config_module.optionList
runtime = config_module.runtime
basename = config_module.basename

translator = { k:optionList[0][k][0] for k in optionList[0]} # translator
translator.update({ k:runtime[k][0] for k in runtime})

def optionsSanityCheck(opt):
    if '_legend' in opt and '"' in opt['_legend']:
        print "Found illegal character ", '"', " in legends of", opt
        return False
    return True

def removeMetaData(keys):
	return [k for k in keys if k[0] != '_']  # we keep _xxxx for internal uses

def enum(func, baseOption): # func takes an input as a dictionary
    if not optionsSanityCheck(baseOption):
        raise 0
    optionName = 'unknown config'
    if '_legend' in baseOption:
        optionName = baseOption['_legend']
    print 'Working on', optionName
    currentOption = {}
    keys = baseOption.keys()
    total = 1
    currentOption = cloneDict(baseOption)  # to migrate those items starting with _
    keys = removeMetaData(keys) # [k for k in keys if k[0] != '_']  # we keep _xxxx for internal uses
    for k in keys:
        total *= len(baseOption[k]) - 1  # minus its abbrevation
    counter = [0]
    def rec(ind):
        if ind == len(keys):
            func(dict(currentOption))  # hard copy
            counter[0] += 1
            print counter[0] * 100 / total, '%'
            return
        for i in range(1, len(baseOption[keys[ind]])):
            currentOption[keys[ind]] = baseOption[keys[ind]][i]
            rec(ind+1)
            currentOption.pop(keys[ind])
    rec(0)

def serializeValues(val):
    if isinstance(val, bool):
        if val:
            return 't'
        return 'f'
    return str(val)

def getNameByOption(opt, baseN=basename):
    keys = removeMetaData(sorted(opt.keys()))
    res = baseN
    for k in keys:
        val = opt[k]
        res += translator[k] + '_' + serializeValues(val)
    return res

