// This code is part of the "A Simple and Practical Linear-Work
// Parallel Algorithm for Connectivity" in Proceedings of the ACM
// Symposium on Parallelism in Algorithms and Architectures (SPAA),
// 2014.  Copyright (c) 2014 Julian Shun, Laxman Dhulipala and Guy
// Blelloch
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights (to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//#if APP == APP_PERM
#include "parallel.h"
#include "utils.h"
#include "randPerm.h"
#include <iostream>
#include "sequence.h"
//#include "speculative_for2.h"
//#include "speculative_for.h"
#include "dsm.h"
#include "txn2phase.h"
using namespace std;
int batchSize;
// <code-length>
template <class E>
struct randPermStep {
  typedef pair<E,intT> pairInt;
  DVector <intT> &dataCheckF, &dataCheckS;
  intT* H;
  randPermStep(intT* _H, DVector<intT> &_dataCheckS, DVector<intT> &_dataCheckF) : 
    H(_H), dataCheckS(_dataCheckS), dataCheckF(_dataCheckF) {}
  inline bool run (intT i) {
    intT h = H[i];
        intT t = dataCheckF[i];
        dataCheckF[i] = dataCheckF[h];
        dataCheckF[h] = t;
	dataCheckS[h] = INT_T_MAX;
	return 1;
  }
};
// </code-length>
template <class E>
void randPerm(E *A, intT n, intT r) {
  
  //typedef pair<E,intT> pairInt;

 global_man = new GlobalMan(n);

#if TXN_ALG == TXN_2PHASE
  local_txn_man = new TxnMan2Phase * [getWorkers()];
  for (int i=0; i<getWorkers();i++) { 
    local_txn_man[i] = (TxnMan2Phase *) _mm_malloc(sizeof(TxnMan2Phase), 64); 
    new(local_txn_man[i]) TxnMan2Phase(i);
  }
#endif

  intT *H = newA(intT,n);
  //intT *check = newA(intT,n);
  //pairInt* dataCheck = newA(pairInt,n);
  DVector<intT> dataCheckS(n), dataCheckF(n);
  // <code-length>
  parallel_for (intT i=0; i < n; i++) {
    H[i] = i+utils::hash(i)%(n-i);
    dataCheckF._array[i]._val = A[i]; dataCheckS._array[i]._val = i;  // won't be a problem since these txns are disjoint
  }
  // </code-length>

  randPermStep<E> rStep(H, dataCheckS, dataCheckF);
  speculative_for(rStep, 0, n, batchSize);

  {parallel_for (intT i=0;i<n;i++) A[i] = dataCheckF[i].get_val();}

  free(H); //free(dataCheck);
}

template void randPerm<intT>(intT*,intT,intT);
template void randPerm<double>(double*,intT,intT);
//#endif
