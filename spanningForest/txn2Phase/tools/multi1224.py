from utils import *
runtime = {
        '-b':['batch', 500, 700, 1000, 2000, 3000, 5000, 10000, 20000, 50000, 75000, 100000, 200000],
        'FILENAME':['_', 'randmm']
}
# TODO: migrate runtime_config inside configs
#runtimePQ = {
#	'-b':['batch', 500]
#	'FILENAME':['_', 'randL']
#}

from multiheal import *
challenger['INITIALIZE_POOL'] = ['IP', True, False]

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

# TODO: add overriding rules, for example, when PURESEQUENTIAL is true, then all the others do not make differences.

basename = 'newmis'

optionList = [
#previousHeal, pureSeq, challenger, 
#healNoReducer, healNoReducer16, healNoReducer256,
previousHealDet, challengerDet, healNoReducerDet,
] # [previousBestSeq, previousBestDet, pureSeq, challenger]
