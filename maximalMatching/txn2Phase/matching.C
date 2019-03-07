// This code is part of the Problem Based Benchmark Suite (PBBS)
// Copyright (c) 2011 Guy Blelloch and the PBBS team
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

#include <iostream>
#include "parallel.h"
#include "sequence.h"
#include "graph.h"
#include "utils.h"
#include "txn.h"
#include "dsm.h"
using namespace std;
int batchSize;
// <code-length>
struct matchStep {
  edge<intT> *E;  
  DVector<intT> &R;  
  DVector<bool> &matched;
  matchStep(edge<intT>* _E, DVector<intT> &_R, DVector<bool> &m) : E(_E), R(_R), matched(m) {}
  void run(intT i) {
	intT v = E[i].v;
    intT u = E[i].u;
	if (u == v || matched[v] || matched[u])	return;
	matched[v] = 1;
	matched[u] = 1;
	R[u] = i;
  }
};
// </code-length>
struct nonNegative { bool operator() (intT i) {return i >= 0;}};

// Finds a maximal matching of the graph
// Returns cross pointers between vertices, or -1 if unmatched
pair<intT*,intT> maximalMatching(edgeArray<intT> EA) {
  intT m = EA.nonZeros;
  intT n = EA.numRows;

  global_man = new GlobalMan(n);

#if TXN_ALG == TXN_2PHASE
  local_txn_man = new TxnMan2Phase * [getWorkers()];
  for (int i=0; i<getWorkers();i++) { 
    local_txn_man[i] = (TxnMan2Phase *) _mm_malloc(sizeof(TxnMan2Phase), 64); 
    new(local_txn_man[i]) TxnMan2Phase(i);
  }
#endif

  DVector<bool> matched(n, 0);
  DVector<intT> R(n, -1);

  matchStep mStep(EA.E, R, matched);
  speculative_for(mStep, 0, m, batchSize);
  
  intT* Out = newA(intT,n);
  intT* RR = newA(intT,n);
  parallel_for(intT i=0;i<n;i++)
 	RR[i] = R._array[i].get_val();
  intT nMatches = sequence::filter(RR, Out, n, nonNegative());
  free(RR);

  //free((void *)lock_table);

  cout << "number of matches = " << nMatches << endl;
  return pair<intT*,intT>(Out,nMatches);
}  

