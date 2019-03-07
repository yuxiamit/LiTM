#include <iostream>
#include <limits.h>
#include "sequence.h"
#include "gettime.h"
#include "graph.h"
#include "parallel.h"
#include "unionFind.h"
using namespace std;

struct notMax { bool operator() (intT i) {return i < INT_T_MAX;}};

// Assumes root is negative 
// Not making parent array volatile improves
// performance and doesn't affect correctness
inline intT find(intT i, intT * parent) {
  if (parent[i] < 0) return i;
  intT j = parent[i];     
  if (parent[j] < 0) return j;
  intT x = 0;
  do j = parent[j];
  while (parent[j] >= 0);
  //note: path compression can happen in parallel in the same tree, so
  //only link from smaller to larger to avoid cycles
  intT tmp;
  while((tmp=parent[i])<j){ parent[i]=j; i=tmp;} 
  return j;
}

pair<intT*, intT> st(edgeArray<intT> EA){
  edge<intT>* E = EA.E;
  intT m = EA.nonZeros;
  intT n = EA.numRows;
  intT *parents = newA(intT,n);
  parallel_for (intT i=0; i < n; i++) parents[i] = -1;
  intT volatile *hooks = newA(intT,n);
  parallel_for (intT i=0; i < n; i++) hooks[i] = INT_T_MAX;
  
  parallel_for (intT i = 0; i < m; i++) {
    //intT j = 0;
    intT u = E[i].u, v = E[i].v;
    while(1){
      //if(j++ > 1000) abort();
      u = find(u,parents);
      v = find(v,parents);
      if(u == v) break;
      if(u > v) swap(u,v);
      //if successful, store the ID of the edge used in hooks[u]
      if(hooks[u] == INT_T_MAX && __sync_bool_compare_and_swap(&hooks[u],(volatile intT)INT_T_MAX,(volatile intT)i)){
	//if(u == find(u,parents)) { //check that root didn't change (we don't need this check because there is no unlock; however the transaction code might need it)
	parents[u]=v;
	break;
	  //} else { hooks[u] = INT_T_MAX; } 
      }
    }
  }

  //get the IDs of the edges in the spanning forest
  _seq<intT> stIdx = sequence::filter((intT*) hooks, n, notMax());
  
  free((void*)parents); free((void*)hooks); 
  cout<<"nInSt = "<<stIdx.n<<endl;
  return pair<intT*,intT>(stIdx.A, stIdx.n);
}
