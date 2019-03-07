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

from multiheal import *

healNoReducer = cloneDict(challenger)
healNoReducer['_commit'] = 'master'
healNoReducer['USE_REDUCER'] = ['R', False]
healNoReducer['_legend'] = 'With_Multi_Healing_no_reducer'



# TODO: add overriding rules, for example, when PURESEQUENTIAL is true, then all the others do not make differences.

basename = 'newmis'

optionList = [previousHeal, pureSeq, challenger, healNoReducer] # [previousBestSeq, previousBestDet, pureSeq, challenger]
