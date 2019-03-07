#include <iostream>
#include <limits.h>
#include "sequence.h"
#include "gettime.h"
#include "txn2phase.h"
#include "graph.h"
#include "parallel.h"
#include "unionFind.h"
#include "dsm.h"

using namespace std;

GlobalMan * global_manager;
// <code-length>
template <class intT>
struct unionFindStep {
	intT u;  
	intT v;  
	edge<intT> *E;
	unionFindStep(edge<intT>* _E, DVector<intT> &_parents, DVector<intT> &_hooks)
		: E(_E), parents(_parents), hooks(_hooks) {} 
	DVector<intT> &parents;
	DVector<intT> &hooks;
	inline intT find(intT i, DVector<intT> &parent) {
		if (parent[i] < 0) return i;
		intT j = parent[i]; 
		if (parent[j] < 0) return j;
		do j = parent[j]; 
		while (parent[j] >= 0);
		intT tmp = parent[i];
		while((tmp=parent[i])<j) {
			parent[i]=j; i=tmp;
		}
		return j;
	}
	void run(intT i) {
		intT u = E[i].u;
		intT v = E[i].v;
		v = find(v,parents);
		u = find(u,parents);
		if (u == v)
			return; //{cout << u << " " << v << endl; return;}
		if (u > v)
			swap(u,v);
		parents[u] = v;
		hooks[u] = i;
	}
};
// </code-length>


struct notMax { bool operator() (intT i) {return i < INT_T_MAX;}};

pair<intT*, intT> st(edgeArray<intT> EA){
	edge<intT>* E = EA.E;
	intT m = EA.nonZeros;
	intT n = EA.numRows;
	std::cout << "n " << n << ", m " << m << std::endl;
	//intT volatile *parents = newA(intT,n);
	DVector<intT> parents(n, -1); 
	DVector<intT> hooks(n, INT_T_MAX);
	std::cout << "Parent size " << parents._size << " Hooks size " << hooks._size << std::endl;
	global_man = new GlobalMan(m);
	local_txn_man = new TxnMan2Phase * [getWorkers()];
	cout << "Workers = " << getWorkers() << endl;
	for (int i=0; i<getWorkers();i++) {
		//local_txn_man[i] = new TxnMan2Phase;
		local_txn_man[i] = (TxnMan2Phase *) _mm_malloc(sizeof(TxnMan2Phase), 64); 
		new(local_txn_man[i]) TxnMan2Phase(i);
		//local_txn_man[i]->_in_txn = true;
	}

	unionFindStep<intT> step(E, parents, hooks); 
	speculative_for(step, 0, m, batchSize);

	_seq<intT> stIdx = sequence::filter((intT*) hooks.array(), n, notMax());
	//free((void*)parents); //free((void*)hooks); 
	cout<<"m = "<< m <<endl;
	cout<<"n = "<< n <<endl;
	cout<<"nInSt = "<<stIdx.n<<endl;

	uint64_t total1 = 0;
	uint64_t total2 = 0;
	for (int i =0; i < global_man->_nworkers;i++) {
		total1 += *global_man->stats1[i];
		total2 += *global_man->stats2[i];
	}
	printf("stats1 = %f, %f\n", total1 / 1000000000.0, total1 / 1000000000.0 / global_man->_nworkers);
	printf("stats2 = %f, %f\n", total2 / 1000000000.0, total2 / 1000000000.0 / global_man->_nworkers);


	return pair<intT*,intT>(stIdx.A, stIdx.n);
}

