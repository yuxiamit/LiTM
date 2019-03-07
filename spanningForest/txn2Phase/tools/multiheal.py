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
previousHeal = {
        '_legend':'Previous_Heal',
        # '_commit':'3eea74c',
        'SEQUENTIAL':['SQ', True],
        'SHOW_HOLES':['SH', False],
        'USE_REDUCER':['R', False],
	'MISHEAL':['MH', True],
        'EARLY_STOP':['ES', False],
        'PHASE3': ['P3', True],
        'PACKSKIP':['PS', True],
        'GRAIN_SIZE':['GS', False],
        'VERBOSE':['V', False],
        'REPEATEXEC':['RE', True],
        'MAINTAIN_WS':['MW', False],
        'DISJOINED_WS':['DS', False],
	'PURESEQUENTIAL':['PQ', False],
        'INITIALIZE_POOL':['IP', False],
        'STEP':['ST', 1],
        'HEAL_NO_WAIT':['HNW', False],
        'ALLOCATE_SIZE':['AS', 1],
        # 'PURESEQUENTIAL':['PUS', False] # usually we set it to false
}

pureSeq = cloneDict(previousHeal) # previousBestSeq)
pureSeq['PURESEQUENTIAL'] = ['PQ', True]
pureSeq['_legend'] = 'Pure_Sequential'

challenger = cloneDict(previousHeal) # previousBestDet)
challenger['HEAL_NO_WAIT'] = ['HNW', True]
challenger['USE_REDUCER'] = ['R', True]
challenger['_legend'] = 'With_Multi_Healing'

# TODO: add overriding rules, for example, when PURESEQUENTIAL is true, then all the others do not make differences.

basename = 'newmis'

optionList = [previousHeal, pureSeq, challenger] # [previousBestSeq, previousBestDet, pureSeq, challenger]
