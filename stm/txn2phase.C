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

#if DEBUG
intT readCounter = 0;
intT writeCounter = 0;
#endif

#define WAR_OPT true 
uint32_t * rs_ptrs;
uint32_t * ws_ptrs;
uint32_t maxRS = 0;
__thread int thread_id;
__thread bool initialized;
__thread TxnMan2Phase * txn_man;

uint32_t totalHoles = 0;

TxnMan2Phase::TxnMan2Phase(int thread_id)
{
	assert(global_man);
	//_max_buffer_size = 655360;
	secondRound = checkFailure = false;
	_rs_size = 0;
	_ws_size = 0;
	assert(thread_id < (1 << 8));
	rs_records = (uint32_t *)((uint64_t)rs_records_base +  thread_id * (1<<24)); //7200016); //(1<<23));
	ws_records = ws_records_base +  thread_id * (1<<24); //7200016; // (1<<23);
	next_rs_record = rs_records;
	next_ws_record = ws_records;
	curr_ws_end = next_ws_record + 4;
}

bool 
TxnMan2Phase::process_phase2( uint32_t id ) {
	_priority = id + base_pri;
  #if REPEATEXEC
	// we don't have read set here.
  #else
	// check read set.
	uint32_t * rs = (uint32_t*)((uint64_t)rs_ptrs[id] + (uint64_t)rs_records_base);
	_rs_size = rs[0]; 
	for (int i=0;i<_rs_size;i++) {
		uint32_t id = rs[i+1]; 
		uint32_t en = lock_table[id];
		if (_priority > en)	
			return true;
	}
  #endif

	// check write set.
	char * ws = (char *)((uint64_t)ws_ptrs[id] + (uint64_t)ws_records_base);
	_ws_size = *(uint32_t *)ws;
	ws += sizeof(uint32_t);
	char * end = ws;
	for (int i=0;i<_ws_size;i++) {
		uint64_t key = *(uint64_t*)end;
		// check the validity of reads
		uint32_t en = lock_table[mhash(key)];
		if (_priority > en)		
			return true;
		end += 8;
		uint32_t size = *(uint32_t*)end;
		end += size + 4; 
	}
	// copy write set. 
	end = ws;
	for (int i=0;i<_ws_size;i++) {
		uint64_t key = *(uint64_t*)end;
		end += 8;
		uint32_t size = *(uint32_t*)end;
//		assert(size == 1);
		end += 4;
		memcpy((char *)key, end, size); 
		end += size; 
	}
	return false;
}



#if REPEATEXEC
bool 
TxnMan2Phase::process_phase3( uint32_t id ) {
	char * ws = (char *)((uint64_t)ws_ptrs[id] + (uint64_t)ws_records_base);
	_ws_size = *(uint32_t *)ws;

	ws += sizeof(uint32_t);
	char * end = ws;
	for (int i=0;i<_ws_size;i++) {
		uint64_t key = *(uint64_t*)end;
		//std::cout << "key " << key << std::endl;
		end += 8;
		uint32_t size = *(uint32_t*)end;
//		assert(size == 1);
		end += 4;
		memcpy((char *)key, end, size); 
		end += size; 
	}
	return true;
}
#endif

