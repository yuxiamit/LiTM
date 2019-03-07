from utils import *

commit = '''
Experiment on 01/18. 

Improved the performance for [deterministic + heal] configuration.

Compare current ordered code with deterministic, as well as Julian's code.
'''

from multiheal import *

runtime = {
        #'-b':['batch', 500, 700, 1000, 2000, 3000, 5000, 10000, 20000, 50000, 75000, 100000, 200000],
        '-b':['batch', 7000, 50000, 200000, 500000, 2000000, 5000000],
        'FILENAME':['_', 
#            'randLocalGraph_J_5_10000000',
#            'rMatGraph_J_5_10000000',
            '3Dgrid_J_10000000'
            ]
}
# TODO: migrate runtime_config inside configs
#runtimePQ = {
#	'-b':['batch', 500]
#	'FILENAME':['_', 'randL']
#}

#from multiheal import *
challenger['INITIALIZE_POOL'] = ['IP', True]
challenger['_legend'] = 'multiheal_with_reducer'
healNoReducer = cloneDict(challenger)
healNoReducer['_commit'] = 'master'
healNoReducer['USE_REDUCER'] = ['R', False]
healNoReducer['_legend'] = 'With_Multi_Healing_no_reducer'
previousHealDet = cloneDict(previousHeal)
previousHealDet['SEQUENTIAL'] = ['SQ', False]
previousHealDet['_legend'] = 'PreviousHealDet'
challengerDet = cloneDict(challenger)
challengerDet['SEQUENTIAL'] = ['SQ', False]
challengerDet['_legend'] = 'challengerDet'
healNoReducerDet = cloneDict(healNoReducer)
healNoReducerDet['SEQUENTIAL'] = ['SQ', False]
healNoReducerDet['_legend'] = 'healNoReducerDet'

healNoReducer16 = cloneDict(healNoReducer)
healNoReducer16['ALLOCATE_SIZE'] = ['AS', 16]
healNoReducer16['_legend'] = 'healNoReducer_with_allocate_16'

healNoReducer256 = cloneDict(healNoReducer16)
healNoReducer256['ALLOCATE_SIZE'] = ['AS', 256]
healNoReducer256['_legend'] = 'healNoReducer_with_allocate_256'
healNoReducer256Det = cloneDict(healNoReducer256)
healNoReducer256Det['SEQUENTIAL'] = ['SQ', False]
healNoReducer256Det['_legend'] = 'det_heal_allocate_256'
# TODO: add overriding rules, for example, when PURESEQUENTIAL is true, then all the others do not make differences.
originalDet = {
        '_legend':'Original_Det',
        'SEQUENTIAL':['SQ', False],
        'SHOW_HOLES':['SH', False],
        'USE_REDUCER':['R', False],
        'MISHEAL':['MH', False],
        'EARLY_STOP':['ES', False],
        'PHASE3': ['P3', True],
        'PACKSKIP':['PS', False],
        'GRAIN_SIZE':['GS', False],
        'VERBOSE':['V', False],
        'REPEATEXEC':['RE', False],
        'MAINTAIN_WS':['MW', True],
        'DISJOINED_WS':['DS', False],
'PURESEQUENTIAL':['PQ', False],
'INITIALIZE_POOL':['IP', True],
'STEP':['ST', 1],
'HEAL_NO_WAIT':['HNW', True],
'ALLOCATE_SIZE':['AS', 1],
        }

basename = 'newmis'

optionList = [
pureSeq,
healNoReducer256,
healNoReducer256Det,
challengerDet,
healNoReducerDet,
#originalDet
#previousHeal, pureSeq, challenger, 
#healNoReducer, healNoReducer16, healNoReducer256,
#previousHealDet, challengerDet, healNoReducerDet,
] # [previousBestSeq, previousBestDet, pureSeq, challenger]
