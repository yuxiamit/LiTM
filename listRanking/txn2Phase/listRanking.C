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

#include "parallel.h"
#include "utils.h"
#include <iostream>
#include "sequence.h"
#include "dsm.h"
#include "txn2phase.h"
//#include "speculative_for.h"
#include "listRanking.h"
using namespace std;
int batchSizeP;
// <code-length>
struct listRankingStep {
	DVector<intT> &nodesP;
	DVector<intT> &nodesN;
	intT n;
	listRankingStep(DVector<intT> &_nodesP, DVector<intT> &_nodesN, intT _n) : nodesP(_nodesP), nodesN(_nodesN), n(_n) {}
	bool run (intT i) {
		intT next = nodesN[i], prev = nodesP[i];
		if(i<next && i<prev){ //local min 
			if(next != n) nodesP[next] = prev;
			if(prev != n) nodesN[prev] = next;
			return 1; } 
		else return 0; //lost 
	}};
// </code-length>

void listRanking(DVector<intT> &_nodesP, DVector<intT> &_nodesN, intT n, intT r) {
	listRankingStep lStep(_nodesP, _nodesN, n);
	global_man = new GlobalMan(n);
#if TXN_ALG == TXN_2PHASE
	local_txn_man = new TxnMan2Phase * [getWorkers()];
	for (int i=0; i<getWorkers();i++) { 
		local_txn_man[i] = (TxnMan2Phase *) _mm_malloc(sizeof(TxnMan2Phase), 64); 
		new(local_txn_man[i]) TxnMan2Phase(i);
	}
#endif
	speculative_for(lStep, 0, n, batchSizeP);
	//free(R);
}
