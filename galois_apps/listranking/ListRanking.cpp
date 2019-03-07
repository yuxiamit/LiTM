/** Maximal independent set application -*- C++ -*-
 * @file
 *
 * A simple spanning tree algorithm to demostrate the Galois system.
 *
 * @section License
 *
 * Galois, a framework to exploit amorphous data-parallelism in irregular
 * programs.
 *
 * Copyright (C) 2012, The University of Texas at Austin. All rights reserved.
 * UNIVERSITY EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES CONCERNING THIS
 * SOFTWARE AND DOCUMENTATION, INCLUDING ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR ANY PARTICULAR PURPOSE, NON-INFRINGEMENT AND WARRANTIES OF
 * PERFORMANCE, AND ANY WARRANTY THAT MIGHT OTHERWISE ARISE FROM COURSE OF
 * DEALING OR USAGE OF TRADE.  NO WARRANTY IS EITHER EXPRESS OR IMPLIED WITH
 * RESPECT TO THE USE OF THE SOFTWARE OR DOCUMENTATION. Under no circumstances
 * shall University be liable for incidental, special, indirect, direct or
 * consequential damages or loss of profits, interruption of business, or
 * related expenses which may arise from use of Software or Documentation,
 * including but not limited to those resulting from defects in Software and/or
 * Documentation, or loss or inaccuracy of data of any kind.
 *
 * @author Donald Nguyen <ddn@cs.utexas.edu>
 */
#include "Galois/Galois.h"
#include "Galois/Bag.h"
#include "Galois/Statistic.h"
#include "Galois/Graph/LCGraph.h"
#include "Galois/ParallelSTL/ParallelSTL.h"
#ifdef GALOIS_USE_EXP
#include "Galois/Runtime/ParallelWorkInline.h"
#endif
#include "llvm/Support/CommandLine.h"

#include "Lonestar/BoilerPlate.h"

#include <utility>
#include <vector>
#include <algorithm>
#include <iostream>
#define newA(__E,__n) (__E*) malloc((__n)*sizeof(__E))

const char* name = "List Ranking";
const char* desc = "Perform list ranking on an array.";
const char* url = "list_ranking";
const int N = 1000000000;
typedef int intT;
enum Algo {
	serial,
	pull,
	newdet,
	nondet,
	detBase,
	detPrefix,
	detDisjoint,
	orderedBase,
};

namespace cll = llvm::cl;
static cll::opt<std::string> filename(cll::Positional, cll::desc("<input file>"), cll::Required);
static cll::opt<Algo> algo(cll::desc("Choose an algorithm:"),
		cll::values(
			clEnumVal(serial, "Serial"),
			clEnumVal(pull, "Pull-based (deterministic)"),
			clEnumVal(newdet, "Deterministic List Ranking"),
			clEnumVal(nondet, "Non-deterministic"),
			clEnumVal(detBase, "Base deterministic execution"),
			clEnumVal(detPrefix, "Prefix deterministic execution"),
			clEnumVal(detDisjoint, "Disjoint deterministic execution"),
			clEnumVal(orderedBase, "Base ordered execution"),
			clEnumValEnd), cll::init(nondet));
struct Node {
	int prev;
	int next;
	Node() {}
};


struct SerialAlgo {
	typedef Galois::Graph::LC_CSR_Graph<Node,void>
		::with_numa_alloc<true>::type
		::with_no_lockable<true>::type
		//::with_compressed_node_ptr<true>::type 
		Graph;
	typedef Graph::GraphNode GNode;

	Graph & graph;
	SerialAlgo(Graph & g): graph(g) {}

	void operator()(){
		for (Graph:: iterator ii = graph.begin(), ei = graph.end(); ii != ei; ii++) {
			run(graph, *ii);
		}
	}

	void operator()(Graph& graph) {
		for (Graph::iterator ii = graph.begin(), ei = graph.end(); ii != ei; ++ii) {
			run(graph, *ii);
		}
	}

	void run(Graph &graph, GNode src){
		Node & data = graph.getData(src);
		int next = data.next, prev = data.prev;
		if (prev != N)
		{
			Node & prevData = graph.getData(prev);
			prevData.next = next;
		}
		if (next != N)
		{
			Node & nextData = graph.getData(next);
			nextData.prev = prev;
		}
	}

};

struct Det {
	typedef Galois::Graph::LC_CSR_Graph<Node,void>
		::with_numa_alloc<true>::type
		::with_no_lockable<true>::type
		//::with_compressed_node_ptr<true>::type
		Graph;
	typedef Graph::GraphNode GNode;
	Graph & graph;
	Det(Graph &g): graph(g) {}

	void operator()(GNode src, Galois::UserContext<GNode>& ctx){
		run(graph, src);
	}
	void operator()(){
		Galois::for_each_det(graph.begin(), graph.end(), *this);
	}
	void operator()(Graph & graph) {
		Galois::for_each_det(graph.begin(), graph.end(), *this);
	}
	void run(Graph &graph, GNode src){
		Node & data = graph.getData(src);
		int next = data.next, prev = data.prev;
		if (prev != N)
		{
			Node & prevData = graph.getData(prev);
			prevData.next = next;
		}
		if (next != N)
		{
			Node & nextData = graph.getData(next);
			nextData.prev = prev;
		}
	}


};

struct NonDet {
	typedef Galois::Graph::LC_CSR_Graph<Node,void>
		::with_numa_alloc<true>::type
		::with_no_lockable<true>::type
		//::with_compressed_node_ptr<true>::type
		Graph;
	typedef Graph::GraphNode GNode;

	Graph & graph;
	NonDet(Graph & g) : graph(g) {}

	void operator()(GNode src, Galois::UserContext<GNode>& ctx)
	{
		run(graph, src);
	}
	void operator()(){
		Galois::for_each(graph.begin(), graph.end(), *this);
	}
	void operator()(Graph & graph) {
		Galois::for_each(graph.begin(), graph.end(), *this); 
	}
	void run(Graph &graph, GNode src){
		Node & data = graph.getData(src);
		int next = data.next, prev = data.prev;
		if (prev != N)
		{
			Node & prevData = graph.getData(prev);
			prevData.next = next;
		}
		if (next != N)
		{
			Node & nextData = graph.getData(next);
			nextData.prev = prev;
		}
	}



};

inline unsigned int myhash(unsigned int a)
{
	a = (a+0x7ed55d16) + (a<<12);
	a = (a^0xc761c23c) ^ (a>>19);
	a = (a+0x165667b1) + (a<<5);
	a = (a+0xd3a2646c) ^ (a<<9);
	a = (a+0xfd7046c5) + (a<<3);
	a = (a^0xb55a4f09) ^ (a>>16);
	return a;
}

inline void swap(int &a, int &b){
	int t = a;
	a = b;
	b = t;
}

template <class E>
void brokenCompiler__(intT n, E* x, E v) {
	  for(intT i=0; i<n; i++) x[i] = v;
}

template <class E>
static E* newArray(intT n, E v) {
	E* x = (E*) malloc(n*sizeof(E));
	brokenCompiler__(n, x, v);
	return x;
}

template<typename Algo>
void run() {
	typedef typename Algo::Graph Graph;

	uint32_t n = N;
	Graph graph;
	//Galois::Graph::readGraph(graph, filename);
	std::cout << "Starting to build random graph" << std::endl;
	Galois::Graph::FileGraphWriter writer;
	writer.setNumNodes(n);
	writer.setNumEdges(n);
	writer.phase1();
	for(uint32_t i=0; i<n; i++) writer.incrementDegree(i, 0);  // this part cannot be skipped
	writer.phase2();
	for(uint32_t i=0; i<n; i++)
	{
		uint32_t dest = i;
		writer.addNeighbor(i, dest);
	}
	writer.finish<void>();
	Galois::Graph::readGraphDispatch(graph, Galois::Graph::read_default_graph_tag(), writer);

	std::cout << "Construction Over" << std::endl;
	// XXX Test if this matters
	Galois::preAlloc(numThreads + (graph.size() * sizeof(Node) * numThreads / 8) / Galois::Runtime::MM::pageSize);
	intT* A = newA(intT,n);
	for(intT i=0;i<n;i++) A[i] = i;
	for(intT i=0;i<n;i++)
		swap(A[i],A[i+myhash(i)%(n-i)]);
	bool* processed = newArray(n,(bool)0);

	//get rid of cycles
	for(intT i=0;i<n;i++) {
		if(!processed[i]) {
			intT j = i;
			while(A[j] != j) {
				processed[j] = 1;
				j = A[j];
				if(j == i) { A[i] = i; break; }
			}
		}
	}
	free(processed);

	for(intT i=0; i<n; i++)
	{
		Node & gnData = graph.getData(i);
		gnData.next = gnData.prev = n;
	}
	for(intT i=0; i<n; i++)
	{
		if(A[i] != i) {
			graph.getData(i).next = A[i];
			graph.getData(A[i]).prev = i;
		}
	}
	Galois::reportPageAlloc("MeminfoPre");
	Algo algo(graph);
	Galois::StatTimer T;
	T.start();
	algo();
	T.stop();
	Galois::reportPageAlloc("MeminfoPost");

}

int main(int argc, char** argv) {
	Galois::StatManager statManager;
	LonestarStart(argc, argv, name, desc, url);

	switch (algo) {
		case serial: run<SerialAlgo>(); break;
		case newdet: run<Det>(); break;
		case nondet: run<NonDet>(); break; 
		default: std::cerr << "Unknown algorithm" << algo << "\n"; abort();
	}
	return 0;
}
