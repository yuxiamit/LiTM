#pragma once
#include <stdint.h>
#include <string.h>
#include <vector>
#include <list>
#include "sequence.h"
#include <setjmp.h> 
#include "graph.h"
#include "parallel.h"
#include <pthread.h>
// we assume we are using cilk
#include <cilk/cilk.h>
#include <cilk/cilk_api.h>
#include <cilk/reducer_min.h>
#include <cilk/reducer_opadd.h>
#include <cilk/reducer_list.h>

#include <setjmp.h>

#include "txn.h"
#include "config.h"

#define PRINT_TIME true 


#define TXN_START(priority, id) \
	txn_man->_priority = priority; \
	txn_man->_id = id; \
	txn_man->readonly = true; \
	txn_man->_ws_size = txn_man->_rs_size = 0; 

#define TXN_END \
	txn_man->txnEnd();

extern uint32_t totalHoles;
extern int batchSize;

#if DEBUG
extern intT readCounter;
extern intT writeCounter;
#endif 

// TODO. we can use uint32_t for these two structures.
extern uint32_t * rs_ptrs;
extern uint32_t * ws_ptrs;
extern __thread int thread_id;
extern __thread bool initialized;
extern __thread TxnMan2Phase * txn_man;

extern uint32_t * rs_records_base;
extern char * ws_records_base;
extern uint32_t maxRS;
// Per-thread local structure
class TxnMan2Phase {
public:
	jmp_buf env;
	TxnMan2Phase(int thread_id);

	inline bool txnEnd() {
                //if(_rs_size > maxRS) maxRS = _rs_size; // do not care about thread safe
		// for read only txn, simply return. 
		if (_ws_size == 0) {
			rs_ptrs[ _id ] = (uint32_t)((uint64_t)next_rs_record - (uint64_t)rs_records_base);
			ws_ptrs[ _id ] = (uint32_t)((uint64_t)next_ws_record - (uint64_t)ws_records_base);
			*(uint32_t*)next_rs_record = _rs_size;
			*(uint32_t*)next_ws_record = _ws_size;
                        next_rs_record += _rs_size + 1;
			next_ws_record = curr_ws_end; 
			curr_ws_end += 4;
			secondRound = false;
			_rs_size = 0;
			_ws_size = 0;
			return false;
		}
		// TODO TODO
		// rs_ptrs should store the offset. Not the addresses.
		// This way, we can use uint32_t
		rs_ptrs[ _id ] = (uint32_t)((uint64_t)next_rs_record - (uint64_t)rs_records_base);
		ws_ptrs[ _id ] = (uint32_t)((uint64_t)next_ws_record - (uint64_t)ws_records_base);
		*(uint32_t*)next_rs_record = _rs_size;
		*(uint32_t*)next_ws_record = _ws_size;
		next_rs_record += _rs_size + 1;
		next_ws_record = curr_ws_end; 
		curr_ws_end += 4;
		secondRound = false;	
		_rs_size = 0;
		_ws_size = 0;
		return true;
	}
	inline void add_to_ws(uint64_t key, char * val, uint32_t size) {
		// TODO. since we use DVectors, no need to keep key here. Just keep the DVectorID and index.
#if REPEATEXEC
		if(secondRound) {
			*(uint64_t*)curr_ws_end = key;
			*(uint32_t*)(curr_ws_end + 8) = size;
			memcpy(curr_ws_end + 12, val, size);
			curr_ws_end += 12 + size;
			_ws_size ++;
			//*/ 
			// Then we don't have to log write set during second phase as we already have it in the first phase.
			// TODO: record the write set somewhere.
		}
		else{
#endif
			*(uint64_t*)curr_ws_end = key;
			*(uint32_t*)(curr_ws_end + 8) = size;
			memcpy(curr_ws_end + 12, val, size);
			curr_ws_end += 12 + size;
			_ws_size ++;

			bool r = false;
			uint32_t pri;
			uint32_t * en;
			en = &lock_table[mhash(key)];
			pri = *en;
			do pri = *en;
			while (_priority < pri && !(r=__sync_bool_compare_and_swap(en, pri, _priority)));
		
#if REPEATEXEC
		}
#endif
	}
	inline void* add_to_rs(uint64_t key, char * val, uint64_t size) {
		// now we do read set check inside speculative_for assuming those code was provided by the programmer or the compiler.
#if REPEATEXEC
		// we don't need to do anything here.
		if(secondRound && !checkFailure)
		{  // Or we need to see if we have already seen this read soemwhere else
			// TODO: check dependency, and somehow halt execution if it is true by raising an exception. Then let the caller to catch it.
			uint32_t id = mhash(key);
			uint32_t en = lock_table[id];
			if (_priority > en){	
#if !HALT_REPEAT_EXEC
				checkFailure = true; 
#else
				longjmp(env, 1);
#endif
			}//throw id;
		}
#else
		next_rs_record[++_rs_size] = mhash(key); // TODO: need to check if this already exists
#endif
		char * ws = next_ws_record;  
		ws += sizeof(uint32_t);
		char * end = ws;
		char * res = NULL;
		while(end < curr_ws_end){
			uint64_t wkey = *(uint64_t*)end;
			// check the validity of reads
			end += 8;
			uint32_t size = *(uint32_t*)end;
			if (wkey == key){
				res = end+4;
			}
			end += size + 4; 
		}
		return res;
	}

	inline void* add_to_rs(uint64_t key) {
		return add_to_rs(key, NULL, 0); // hopefully compiler can do the shortcut for us.
	}
	bool process_phase2( uint32_t id );
	bool process_phase3( uint32_t id );

	uint32_t _priority;
	uint32_t _id;
	uint32_t _ws_size;
	uint32_t _rs_size;
	bool readonly;
	bool secondRound;
	bool checkFailure;
#define MAX_NUM_LOCAL_TXNS 655360
	void cleanup() {
		secondRound = false;
		_ws_size = 0;
		next_rs_record = rs_records;
		next_ws_record = ws_records;
		curr_ws_end = next_ws_record + 4;
	}
	uint32_t * rs_records;
	char * ws_records;
	uint32_t * next_rs_record;
	char * next_ws_record;
	char * curr_ws_end;
};


#ifndef UINT64_MAX
#define UINT64_MAX      18446744073709551615UL
#endif


	template <class S>
intT speculative_for(S & step, intT s, intT e, int roundsize, 
		bool hasState=0, int maxTries=-1) 
{
	int granularity = (e - s) / roundsize;
	if (maxTries < 0) maxTries = 100 + 200*granularity;
	intT maxRoundSize = roundsize;

	intT *I = newA(intT,maxRoundSize);
	intT *Ihold = newA(intT,maxRoundSize);
	rs_ptrs = newA(uint32_t, maxRoundSize);
	ws_ptrs = newA(uint32_t, maxRoundSize);

	bool *keep = newA(bool,maxRoundSize);
	parallel_for(intT i=0;i<maxRoundSize;i++)
		keep[i]=false;
	S *state;
	base_pri = UINT_T_MAX / 2;
	upper_pri = UINT_T_MAX / 2;

	int round = 0; 
	intT numberDone = s; // number of iterations done
	intT numberKeep = 0; // number of iterations to carry to next round
	intT totalProcessed = 0;
	intT curRoundSize = maxRoundSize; //40000; //maxRoundSize; //10000;
        intT totalKeep = 0;

	uint64_t allmem = 0;
	while (numberDone < e) {
		if (round++ > maxTries) {
			cout << "speculativeLoop: too many iterations, increase maxTries parameter" << endl;
			abort();
		}
		intT size = min(curRoundSize, e - numberDone);
		assert(base_pri > size);
		base_pri -= size;
		totalProcessed += size;
		uint64_t t1 = get_sys_clock();  // TODO: change to INTEL on-chip clock
		global_man->_phase = 0;
		
		parallel_for (intT i1 =0; i1 < size; i1++) {
			if (__builtin_expect(!initialized, 0)) {
				initialized = true;
				thread_id = __cilkrts_get_worker_number();
			}
			txn_man = local_txn_man[thread_id];
			if (i1 >= numberKeep)
				I[i1] = numberDone + i1;
			TXN_START(i1 + base_pri, i1);
			step.run(I[i1]);
			keep[i1] = TXN_END
		}
		global_man->_phase = 1;
		uint64_t tt = get_sys_clock();
		*global_man->stats1[0] += tt - t1;  // execution phase
		parallel_for(intT i2 = 0; i2<size; i2++) {

		bool isHole = false;
		txn_man = local_txn_man[thread_id];
		if (keep[i2]) {
#if REPEATEXEC
			intT index;
			index = I[i2];						
			// BEGIN user-provided code
			// automatic re-execute.
			txn_man->secondRound = true;
			
			if (i2 >= numberKeep)
				I[i2] = numberDone + i2;
				TXN_START(i2 + base_pri, i2);
			#if HALT_REPEAT_EXEC	
				int val;
				val = setjmp(txn_man -> env);
				if(!val){
					step.run(I[i2]); TXN_END;
					isHole = false; keep[i2] = false;
				}
				else {isHole = keep[i2] = true;}
			#else
				txn_man->checkFailure = false;
				step.run(I[i2]); TXN_END;
				isHole = keep[i2] = txn_man->checkFailure;
			#endif	
#else
			// didn't finish in phase 1. should check dependency in phase 2.
			keep[i2] = txn_man->process_phase2( i2 );
			isHole = keep[i2];
#endif
		}
	}

	uint64_t t3 = get_sys_clock();
	*global_man->stats2[0] += t3 - tt;

	// apply writes for deterministic without healing
	#if REPEATEXEC
	parallel_for(intT i=0; i<size; i++) {
		txn_man = local_txn_man[thread_id];
		if(!keep[i])
			txn_man ->process_phase3(i); 
	}
	#endif
	uint64_t currentMem = 0;

        for(int i=0; i<size; i++)
	{
		//int i = I[ki];
		uint32_t *rs = (uint32_t *)((uint64_t)rs_ptrs[i] + (uint64_t)rs_records_base);
		currentMem += 4 + rs[0] * sizeof(uint32_t); // rs_size
		//cout << "rs size " << rs[0] * sizeof(uint32_t) << endl;
	        char * ws = (char *)((uint64_t)ws_ptrs[i] + (uint64_t)ws_records_base);
	        currentMem += 4 + *(uint32_t*)ws;	
		//cout << "ws size" << *(uint32_t*)ws << endl;
	}

	if(currentMem > allmem) allmem = currentMem;

	numberKeep = sequence::pack(I, Ihold, (bool *)keep, size);
	swap(I, Ihold);

	totalKeep += numberKeep;
	numberDone += size - numberKeep;

	parallel_for (intT i=0; i<getWorkers(); i++)
	local_txn_man[i]->cleanup();
	*global_man->stats3[0] += get_sys_clock() - t3;

	assert(numberKeep < size);

	}

	cout << endl << "allmem " << allmem << endl << "maxRS " << maxRS << endl;
	cout << endl << totalHoles << endl;
	cout << "total keep " << totalKeep << endl;

	free(I); free(Ihold); 

	#if PRINT_TIME
	cout << "Time " << *(global_man->stats1[0]) << "," << *(global_man->stats2[0]) << "," << *(global_man->stats3[0]) << endl;
	#endif

	free(keep); 
	if(hasState) free(state);

	return totalProcessed;
}

