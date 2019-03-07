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
#include "config.h"

#include <iostream>
#include "sequence.h"
#include "graph.h"
#include "parallel.h"
#include "utils.h"
//#include "speculative_for.h"
#include "gettime.h"
#include "dsm.h"
#include "pagerank.h"
#include <atomic>

using namespace std;
unsigned int iteration = 0;
// **************************************************************
// PAGE RANK
// **************************************************************

template<class T> T myabs(T a){
  return a<0?-a:a;
}
// <code-length>
struct PRElem {
	float val[3];
};
struct PRStep {
  intT &smallDelta;
  float &maxDelta;
  DVector<PRElem> & val;
  vertex<intT>*G;
  int iter;
  PRStep(DVector<PRElem> & _val, intT &sd, float &md, vertex<intT>* _G) : val(_val), maxDelta(md), smallDelta(sd), G(_G) { iter = 0; }
  void run(intT i) {
    intT d = G[i].degree;
    PRElem original = val[i];
    float originalVal = original.val[iter];  //val[i*3 + iter]; //(*val[iteration])[i];
    float accTmp = 0.;
    for(intT j=0; j<d; j++) {
      intT ngh = G[i].Neighbors[j];
      PRElem valStruct = val[ngh];
      accTmp += valStruct.val[iter] * valStruct.val[2];//val[ngh*3 + iter] * val[ngh*3 +2]; // G[ngh].degree;  //(*val[iteration])[ngh] / G[ngh].degree;
    }
    float sourceVal = (1.0 - alpha) * accTmp + alpha;
    float valDiff = myabs<float>(sourceVal - originalVal);
    original.val[iter^1] = sourceVal;
    val[i] = original;
    if(valDiff > maxDelta) maxDelta = valDiff;
  }
};
float* pagerank(graph<intT> GS) {
  intT maxIterations = 100;
  intT n = GS.n;
  vertex<intT>* G = GS.V;
  DVector<PRElem> val(n); //val0(n, 1.0), val1(n, 1.0);
  intT smallDelta = 0;
  float maxDelta;// = std::numeric_limits<float>::min();
  PRStep pr(val, smallDelta, maxDelta, G); //(&val0, &val1, smallDelta, maxDelta, G);
  parallel_for(intT i=0; i<n; i++)
  {
	  val._array[i]._val.val[0] = 1.0;
	  val._array[i]._val.val[2] = 1.0 / G[i].degree;
  }
  int iterCounter = 0;
  for(;;){
	  maxDelta = std::numeric_limits<float>::min();
    global_man = new GlobalMan(n);
    local_txn_man = new TxnMan2Phase * [getWorkers()];
    for (int i=0; i<getWorkers();i++) { 
      local_txn_man[i] = (TxnMan2Phase *) _mm_malloc(sizeof(TxnMan2Phase), 64); 
      new(local_txn_man[i]) TxnMan2Phase(i);
    }
    speculative_for(pr, 0, n, batchSize);
    pr.iter ^= 1;
    iterCounter += 1;
    std::cout << "iteration: " << iterCounter
                 << " max delta: " << maxDelta
                 << " small delta: " << smallDelta
                 << " (" << smallDelta / (float) n << ")"
                 << "\n";
    if (maxDelta <= tolerance || iterCounter >= maxIterations)
      break;
    delete global_man;
    for(int i=0; i<getWorkers(); i++) {
	    free(local_txn_man[i]);
    }
    delete local_txn_man;
  }
  if (iterCounter >= maxIterations) {
       std::cout << "Failed to converge\n";
  }
  float * res = newA(float, n);
  parallel_for(intT i=0; i<n; i++)
    res[i] = val._array[i].get_val().val[pr.iter]; //res[i] = val._array[i*3 + pr.iter].get_val();    //valptr->_array[i].get_val();
  return res;
}
// </code-length>