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

// **************************************************************
// PAGE RANK
// **************************************************************
struct PRStep1 {
  float * acc;
  float * val;
  vertex<intT>*G;
  PRStep1(float* _acc, float * _val, vertex<intT>* _G) : acc(_acc), val(_val), G(_G) {}
  /*void run(intT i) {
    intT d = G[i].degree;
    float sourceVal = val[i] / d;
  	for(intT j=0; j<d; j++) {
		  intT ngh = G[i].Neighbors[j];
      acc[ngh] = acc[ngh] + sourceVal;  // the same as Galois
	  }
  }*/
  void run(intT i) {
  	intT d = G[i].degree;
	float accTmp = acc[i];
	for(intT j=0; j<d; j++) {
		intT ngh = G[i].Neighbors[j];
		accTmp += val[ngh] / G[ngh].degree;
	}
	acc[i] = accTmp;
  }
};

template<class T> T myabs(T a){
  return a<0?-a:a;
}

struct PRStep2 {
  intT &smallDelta;
  float &maxDelta;
  float * acc;
  float * val;
  vertex<intT>*G;
  PRStep2(float* _acc, float * _val, intT &sd, float &md, vertex<intT>* _G) : acc(_acc), val(_val), smallDelta(sd), maxDelta(md), G(_G) {}
  void run(intT i) {
    float accCache = acc[i];
    float sourceVal = (1.0 - alpha) * accCache + alpha;
    float valDiff = myabs<float>(sourceVal - val[i]);
  	if(valDiff <= tolerance)
      __sync_fetch_and_add(&smallDelta, 1);
    if(valDiff > maxDelta)
    {
      // atomically set maxDelta = valDiff, 
      // or someone else has increased 
      // maxDelta to an even larger one.
      maxDelta = valDiff;
      // while(__sync_val_compare_and_swap(maxDelta, maxDelta, valDiff) < valDiff);
    }
    val[i] = sourceVal;
    acc[i] = 0;
  }
};

float* pagerank(graph<intT> GS) {
  intT maxIterations = 100;
  intT n = GS.n;
  vertex<intT>* G = GS.V;
  global_man = new GlobalMan(n);
  //global_man = new GlobalMan(65536 * 16);
  local_txn_man = new TxnMan2Phase * [getWorkers()];
  for (int i=0; i<getWorkers();i++) { 
    local_txn_man[i] = (TxnMan2Phase *) _mm_malloc(sizeof(TxnMan2Phase), 64); 
    new(local_txn_man[i]) TxnMan2Phase(i);
  }
  //char *Flags = (char*) _mm_malloc(n, 64);
  float *acc = (float*) _mm_malloc(n * sizeof(float), 64);
  float *val = (float*) _mm_malloc(n * sizeof(float), 64);
  memset(acc, 0, sizeof(float)*n);
  for(intT i=0; i<n; i++)
	  val[i] = 1.0;
  //memset(val, 22, sizeof(float)*n);
  // it will be OK if we set all the elements to the same initial value

  unsigned int iteration = 0;
  PRStep1 pr(acc, val, G);
  for(;;){
    intT smallDelta = 0;
    float maxDelta = std::numeric_limits<float>::min();
    PRStep2 pr2(acc, val, smallDelta, maxDelta, G);
    for(int i=0; i<n; i++)
      pr.run(i);
    
    
    for(int i=0; i<n; i++)
      pr2.run(i);
    iteration += 1;

    std::cout << "iteration: " << iteration
                 << " max delta: " << maxDelta
                 << " small delta: " << smallDelta
                 << " (" << smallDelta / (float) n << ")"
                 << "\n";
 
    if (maxDelta <= tolerance || iteration >= maxIterations)
      break;
  }
  if (iteration >= maxIterations) {
       std::cout << "Failed to converge\n";
  }

  return val;
/*
  uint64_t total1 = 0;
  uint64_t total2 = 0;
  for (int i =0; i < global_man->_nworkers;i++) {
	  total1 += *global_man->stats1[i];
	  total2 += *global_man->stats2[i];
  }
  printf("stats1 = %f, %f\n", total1 / 1000000000.0, total1 / 1000000000.0 / global_man->_nworkers);
  printf("stats2 = %f, %f\n", total2 / 1000000000.0, total2 / 1000000000.0 / global_man->_nworkers);
 */
}
