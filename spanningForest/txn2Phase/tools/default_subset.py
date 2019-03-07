options = {
        'SEQUENTIAL':['SQ', True, False],
        'SHOW_HOLES':['SH', False],
        'USE_REDUCER':['R', False],
        'EARLY_STOP':['ES', True],
        'PHASE3': ['P3', True],
        'PACKSKIP':['PS', True],
        'GRAIN_SIZE':['GS', False],
        'VERBOSE':['V', False],
        'REPEATEXEC':['RE', True, False],
        'MAINTAIN_WS':['MW', True, False],
	'DISJOINED_WS':['DW', False],
	'STEP':['ST', 1],
        # 'PURESEQUENTIAL':['PUS', False] # usually we set it to false
}

# TODO: add overriding rules, for example, when PURESEQUENTIAL is true, then all the others do not make differences.

runtime = {
        '-b':['batch', 100, 200, 300, 500, 700, 1000, 2000, 3000, 5000, 10000, 20000, 50000, 75000, 100000, 200000],
        'FILENAME':['_', 'randL']
}

basename = 'newmis'
