#include "txn.h"
#include "txn2phase.h"
#include "parallel.h"
#include "utils.h"
#include <algorithm>
#include <iterator>
#include <iostream>
#include <stdio.h>
#include <mm_malloc.h>
#include <math.h>
using namespace std;

#if TXN_ALG == TXN_2PL
__thread TxnManLock * local_txn_man;
#elif TXN_ALG == TXN_2PHASE

TxnMan2Phase ** local_txn_man;
uint32_t max_num_threads = 64;
#endif
GlobalMan * global_man;

uint32_t * rs_records_base;
char * ws_records_base;

uint32_t lock_table_size;
uint32_t lock_table_mask;
uint32_t * lock_table;
uint32_t base_pri;
uint32_t upper_pri;
char * ready_table; 

uint32_t lock_table_size_mod = 1;

#define RSRB_SIZE (1ll << 33) // 1 << 30
#define WSRB_SIZE (1ll << 33) // 1 << 30

GlobalMan::GlobalMan(int size)
{
	_nworkers = getWorkers();
	rs_records_base = (uint32_t *) malloc(RSRB_SIZE);
	ws_records_base = (char *) malloc(WSRB_SIZE);
	assert(rs_records_base);
	assert(ws_records_base);

	//_managers = new TxnMan2Phase * [_nworkers];
	stats1 = new uint64_t * [_nworkers];
	stats2 = new uint64_t * [_nworkers];
	stats3 = new uint64_t * [_nworkers];
	parallel_for (int i = 0; i < _nworkers; i ++) {
		stats1[i] = (uint64_t *) _mm_malloc(sizeof(uint64_t), 64);
		stats2[i] = (uint64_t *) _mm_malloc(sizeof(uint64_t), 64);
		stats3[i] = (uint64_t *) _mm_malloc(sizeof(uint64_t), 64);
		*stats1[i] = 0;
		*stats2[i] = 0;
		*stats3[i] = 0;
	}
	lock_table_size = size;
	_lock_table_size_log2 = (uint32_t)log2(lock_table_size);
	lock_table_size = 2 << _lock_table_size_log2;
	/*if(lock_table_size_mod<0) // less than 0 means 'divided by'
		lock_table_size /= (uint32_t)(-lock_table_size_mod);// 8;
	else
		lock_table_size *= (uint32_t)lock_table_size_mod;*/
	lock_table_size /= lock_table_size_mod;
	lock_table_mask = lock_table_size - 1;
	//lock_table_size = _lock_table_size;
cout << size << "   " << lock_table_size << endl;
	lock_table = newA(uint32_t, lock_table_size);
  	parallel_for(intT i=0;i<lock_table_size;i++) 
		lock_table[i] = -1;
	_phase = 0;
	_sync = false;
	//pthread_barrier_init(&_barrier, NULL, _nworkers);
}
