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
#include "Galois/Graph/FileGraph.h"
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

const char* name = "Random Permutation";
const char* desc = "Perform random permutations on an array.";
const char* url = "random_permutation";

enum Algo {
  serial,
  pull,
  newdet,
  newnondet,
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
      clEnumVal(newdet, "Deterministic Random Permutation"),
      clEnumVal(newnondet, "Non-Deterministic Random Permutation"),
      clEnumVal(nondet, "Non-deterministic"),
      clEnumVal(detBase, "Base deterministic execution"),
      clEnumVal(detPrefix, "Prefix deterministic execution"),
      clEnumVal(detDisjoint, "Disjoint deterministic execution"),
      clEnumVal(orderedBase, "Base ordered execution"),
      clEnumValEnd), cll::init(nondet));

enum MatchFlag {
  UNMATCHED, OTHER_MATCHED, MATCHED
};

struct Node {
  int val;
  static int counter;
  Node() { val = counter++; }
};

int Node::counter = 0;

struct SerialAlgo {
  typedef Galois::Graph::LC_CSR_Graph<Node,void>
    ::with_numa_alloc<true>::type
    ::with_no_lockable<true>::type
    //::with_compressed_node_ptr<true>::type 
    Graph;
  typedef Graph::GraphNode GNode;
  Graph & graph;

  SerialAlgo(Graph & g): graph(g) {}

  void operator()() {
    for (Graph::iterator ii = graph.begin(), ei = graph.end(); ii != ei; ++ii) {
        run(graph, *ii);
    }
  }

  void run(Graph& graph, GNode src) {
    Node & srcData = graph.getData(src);
    for (Graph::edge_iterator ii = graph.edge_begin(src, Galois::MethodFlag::NONE),
	    ei = graph.edge_end(src, Galois::MethodFlag::NONE); ii != ei; ++ii)
    {
	   GNode dst = graph.getEdgeDst(ii);
	   Node & data = graph.getData(dst);
	   int t = data.val;
	   data.val = srcData.val;
	   srcData.val = t; 
	   break; // for each vertex we only have a single neighbor
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
  Det(Graph & g): graph(g) {}

  void operator()(GNode src, Galois::UserContext<GNode>& ctx) {
    run(graph, src);
  }
  void operator()() {
    //for (Graph::iterator ii = graph.begin(), ei = graph.end(); ii != ei; ++ii) {
    //    run(graph, *ii);
    //}
	  Galois::for_each_det(graph.begin(), graph.end(), *this);
  }

  void run(Graph& graph, GNode src) {
    Node & srcData = graph.getData(src);
    for (Graph::edge_iterator ii = graph.edge_begin(src, Galois::MethodFlag::NONE),
	    ei = graph.edge_end(src, Galois::MethodFlag::NONE); ii != ei; ++ii)
    {
	   GNode dst = graph.getEdgeDst(ii);
	   Node & data = graph.getData(dst);
	   int t = data.val;
	   data.val = srcData.val;
	   srcData.val = t; 
	   break; // for each vertex we only have a single neighbor
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
  NonDet(Graph & g): graph(g) {}

  void operator()(GNode src, Galois::UserContext<GNode>& ctx) {
    run(graph, src);
  }
  void operator()() {
    //for (Graph::iterator ii = graph.begin(), ei = graph.end(); ii != ei; ++ii) {
    //    run(graph, *ii);
    //}
	  Galois::for_each(graph.begin(), graph.end(), *this);
  }

  void run(Graph& graph, GNode src) {
    Node & srcData = graph.getData(src);
    for (Graph::edge_iterator ii = graph.edge_begin(src, Galois::MethodFlag::NONE),
	    ei = graph.edge_end(src, Galois::MethodFlag::NONE); ii != ei; ++ii)
    {
	   GNode dst = graph.getEdgeDst(ii);
	   Node & data = graph.getData(dst);
	   int t = data.val;
	   data.val = srcData.val;
	   srcData.val = t; 
	   break; // for each vertex we only have a single neighbor
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

template<typename Algo>
void run() {
  typedef typename Algo::Graph Graph;

  Graph graph;
  // Dynamically create a new graph
  std::cout << "Starting to build random graph" << std::endl;
  uint32_t n = 1000000000; 
  Galois::Graph::FileGraphWriter writer;
  writer.setNumNodes(n);
  writer.setNumEdges(n);
  writer.phase1();
  for(uint32_t i=0; i<n; i++) writer.incrementDegree(i, 1);
  writer.phase2();
  for(uint32_t i=0; i<n; i++)
  {
	  uint32_t dest = i+myhash(i)%(n-i);
	  writer.addNeighbor(i, dest);
  }
  writer.finish<void>();
  std::cout << "Finished builidng the graph, starting to construct" << std::endl;
  Galois::Graph::readGraphDispatch(graph, Galois::Graph::read_default_graph_tag(), writer);
  
  //graph.constructFrom(writer, 0, 1); // a single thread
  std::cout << "Construction over" << std::endl;
  //Galois::Graph::readGraph(graph, filename);

  // XXX Test if this matters
  Galois::preAlloc(numThreads + (graph.size() * sizeof(Node) * numThreads / 8) / Galois::Runtime::MM::pageSize);

  Algo algo(graph);
  Galois::reportPageAlloc("MeminfoPre");
  Galois::StatTimer T;
  T.start();
  // dummy initialization for fair comparison
  // int *H = (int*) malloc(sizeof(int)*n), *R = (int*) malloc(sizeof(int)*n);
  
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
    case newnondet: run<NonDet>(); break;
    default: std::cerr << "Unknown algorithm" << algo << "\n"; abort();
  }
  return 0;
}
