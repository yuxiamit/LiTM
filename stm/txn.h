#pragma once
#include <stdint.h>
#include <string.h>
#include <vector>
#include "sequence.h"
#include "graph.h"
#include "parallel.h"
#include <setjmp.h>

#define TXN_TL2 0
#define TXN_2PHASE 1
#define TXN_2PL 2
#define TXN_LOGICAL 3

#include "config.h"
//#define TXN_ALG TXN_2PL

#define LIKELY(condition) __builtin_expect((condition), 1)
#define UNLIKELY(condition) __builtin_expect((condition), 0)

class GlobalMan;

#define DETERM false

#if TXN_ALG == TXN_2PL
class TxnManLock;
extern __thread TxnManLock * local_txn_man;
//extern TxnManLock ** txn_mans;
extern uint32_t max_num_threads;
#elif TXN_ALG == TXN_2PHASE
class TxnMan2Phase;
extern TxnMan2Phase ** local_txn_man;
#elif TXN_ALG == TXN_LOGICAL
class TxnManLogical;
extern TxnManLogical ** local_txn_man;
#endif

extern __thread int _tid;
extern GlobalMan * global_man;
extern char * ready_table;
extern uint32_t * lock_table;
extern uint32_t lock_table_size;
extern uint32_t lock_table_size_mod;
extern uint32_t lock_table_mask;
inline uint32_t mhash(uint64_t key) {
	return key & lock_table_mask;
}

extern uint32_t base_pri;
extern uint32_t upper_pri;
#define EARLY_LOCK false

#define CPU_FREQ 1.995
inline uint64_t get_sys_clock() {
#if defined(__i386__)
    uint64_t ret;
    __asm__ __volatile__("rdtsc" : "=A" (ret));
#elif defined(__x86_64__)
    unsigned hi, lo;
    __asm__ __volatile__ ("rdtsc" : "=a"(lo), "=d"(hi));
    uint64_t ret = ( (uint64_t)lo)|( ((uint64_t)hi)<<32 );
    ret = (uint64_t) ((double)ret / CPU_FREQ);
#else
	assert(false);
#endif
    return ret;
}

class GlobalMan {
public:
	GlobalMan(int size);
	uint32_t hash(uint64_t key) {
		//return (key ^ (key / _lock_table_size)) % _lock_table_size;
		return (key) % _lock_table_size;
	}
	//TxnMan ** _managers;
	uint32_t _lock_table_size;
	uint32_t _lock_table_size_log2;
	uint64_t ** stats1;
	uint64_t ** stats2;
	uint64_t ** stats3;

	uint32_t _phase; // 0 or 1
	volatile bool _sync;
	//pthread_barrier_t _barrier;
	uint32_t _nworkers;
};

#ifndef UINT64_MAX
#define UINT64_MAX      18446744073709551615UL
#endif
