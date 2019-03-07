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
#include "MIS.h"

using namespace std;

// **************************************************************
//    MAXIMAL INDEPENDENT SET
// **************************************************************
struct MISstep {
  char flag;
  DVector<char> &Flags;
  vertex<intT>*G;
  MISstep(DVector<char> _F, vertex<intT>* _G) : Flags(_F), G(_G) {}
  void run(intT i) {
    intT d = G[i].degree;
  	for(intT j=0; j<d; j++) {
      intT ngh = G[i].Neighbors[j];
      if (Flags[ngh] == 1)
        return;
	  }
    Flags[i] = 1;
  }
};

char* maximalIndependentSet(graph<intT> GS) {
  int step= 1;
  intT n = GS.n;
  vertex<intT>* G = GS.V;
  global_man = new GlobalMan(n);
  //global_man = new GlobalMan(65536 * 16);
  local_txn_man = new TxnMan2Phase * [getWorkers()];
  for (int i=0; i<getWorkers();i++) { 
    local_txn_man[i] = (TxnMan2Phase *) _mm_malloc(sizeof(TxnMan2Phase), 64); 
    new(local_txn_man[i]) TxnMan2Phase(i);
  }
  DVector<char> Flags(n, 0);
  MISstep mis(Flags, G);
  speculative_for(mis, 0, n/step, batchSize/step); // passing granularity
  char * result = newA(char,n);
  parallel_for(intT i=0;i<n;i++)
 	result[i] = Flags._array[i].get_val();
  return result; //Flags.array();

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
