from utils import *
runtime = {
        '-b':['batch', 500, 700, 1000, 2000, 3000, 5000, 10000, 20000, 50000, 75000, 100000, 200000],
        'FILENAME':['_', 'randTI']
}
# TODO: migrate runtime_config inside configs
#runtimePQ = {
#	'-b':['batch', 500]
#	'FILENAME':['_', 'randL']
#}
previousBestSeq = {
        '_legend':'Previous Best Ordered',
        'SEQUENTIAL':['SQ', True],
        'SHOW_HOLES':['SH', False],
        'USE_REDUCER':['R', False],
        'EARLY_STOP':['ES', True],
        'PHASE3': ['P3', True],
        'PACKSKIP':['PS', True],
        'GRAIN_SIZE':['GS', False],
        'VERBOSE':['V', False],
        'REPEATEXEC':['RE', False],
        'MAINTAIN_WS':['MW', True],
        'DISJOINED_WS':['DS', False],
        'VERBOSE':['VB', False],
	'PURESEQUENTIAL':['PQ', False],
        'STEP':['ST', 1],
        # 'PURESEQUENTIAL':['PUS', False] # usually we set it to false
}

previousBestDet = cloneDict(previousBestSeq)
previousBestDet['SEQUENTIAL'] = ['SQ', False]
previousBestDet['_legend'] = 'Previous Best Deterministic'

pureSeq = cloneDict(previousBestSeq)
pureSeq['PURESEQUENTIAL'] = ['PS', True]
pureSeq['_legend'] = 'Pure Sequential'

challenger = cloneDict(previousBestDet)
challenger['REPEATEXEC'] = ['RE', True]
challenger['MAINTAIN_WS'] = ['MW', False]
challenger['_legend'] = 'With Application Specific Optimization 1'

# TODO: add overriding rules, for example, when PURESEQUENTIAL is true, then all the others do not make differences.

basename = 'newmis'

optionList = [previousBestSeq, previousBestDet, pureSeq, challenger]
