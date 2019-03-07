#pragma once 
#include <stdint.h>
#include <string.h>
#include <pthread.h>
#include <vector>
#include "txn2phase.h"
#include "utils.h"
using namespace std;

template <class T>
class DVector 
{
public:
	
	struct Element {
	public:
		T _val;
		#if TXN_ALG == TXN_LOGICAL
		uint64_t rts;
		uint64_t wts;
		pthread_mutex_t m;

		Element(){rts = 0; wts = 0; pthread_mutex_init(&m, NULL);};

		#endif
		inline T get_val() {
			return _val;	
		}

		// write
		inline T& operator=(const T& val) {
			#if DEBUG
			writeCounter ++;
			#endif
#if TXN_ALG == TXN_2PHASE
			txn_man->add_to_ws((uint64_t)this, (char *)&val, sizeof(T));
#elif TXN_ALG == TXN_TL2
			TM_SHARED_WRITE(_val, val);
#elif TXN_ALG == TXN_2PL
			if (local_txn_man && local_txn_man->_in_txn) {
	            if (local_txn_man->add_to_ws((uint64_t)this, (char*)&_val, sizeof(T))) {
	                _val = val;
    	        } else {
        	        TXN_RESTART
				}
			} else
	            _val = val;
#elif TXN_ALG == TXN_LOGICAL
	    pthread_mutex_lock((pthread_mutex_t*)&m);
	    txn_man->add_to_ws((uint64_t)this, (char *)&val, sizeof(T), (uint64_t *)&rts, (uint64_t *)&wts);
	    pthread_mutex_unlock((pthread_mutex_t*)&m);
#endif
		}
		// read
		inline operator T() const {
			#if DEBUG
			readCounter ++;
			#endif
#if TXN_ALG == TXN_2PHASE
			
			txn_man->add_to_rs((uint64_t)this);
			return _val;
#elif TXN_ALG == TXN_TL2
			TM_SHARED_READ(_val);
#elif TXN_ALG == TXN_2PL
			if (local_txn_man && local_txn_man->_in_txn) {
	            if (local_txn_man->add_to_rs((uint64_t)this)) {
	            	return _val;
    	        } else {
        	        TXN_RESTART
				}
			} else
	            return _val;
#elif TXN_ALG == TXN_LOGICAL
	        pthread_mutex_lock((pthread_mutex_t*)&m);
	        txn_man->add_to_rs((uint64_t)this, (uint64_t *)&rts, (uint64_t *)&wts);
	        pthread_mutex_unlock((pthread_mutex_t*)&m);
	        return _val;
#endif
		}
	};
	
public:
	Element * _array;
	intT _size;
	DVector(uint64_t size) {
		_array = newA(Element, size);
		_size = size;
	}
	DVector(uint64_t size, T value) {
		_array = newA(Element, size);
		_size = size;
		parallel_for(intT i = 0; i < size; i++)
			_array[i]._val = value;
	}
	
	inline Element& operator[] (uint32_t idx) {
		return _array[idx];
	}
	T * array() { return (T *)_array; }
};
