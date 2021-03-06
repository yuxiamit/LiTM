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
#include <algorithm>
#include <cstdlib>
#include <string.h>
#include "gettime.h"
#include "utils.h"
#include "graph.h"
#include "parallel.h"
#include "IO.h"
#include "graphIO.h"
#include "parseCommandLine.h"
#include "pagerank.h"
using namespace std;
using namespace benchIO;

int batchSize;

void timePR(graph<intT> G, int rounds, char* outFile) {
  graph<intT> H = G.copy(); //because MIS might modify graph
  float* flags = pagerank(H);
  for (int i=0; i < rounds; i++) {
    free(flags);
    H.del();
    H = G.copy();
    startTime();
    flags = pagerank(H);
    nextTimeN();
  }
  cout << endl;
  /*
  if (outFile != NULL) {
    int* F = newA(int, G.n);
    for (int i=0; i < G.n; i++) 
    {
      F[i] = (int)flags[i];  // we lose precision on this
    }
    writeIntArrayToFile(F, G.n, outFile);
    free(F);
  }
  */

  free(flags);
  G.del();
  H.del();
}

int parallel_main(int argc, char* argv[]) {
  commandLine P(argc, argv, "[-o <outFile>] [-r <rounds>] [-b <batchSize>] [-c <threadCount>] <inFile>");
  char* iFile = P.getArgument(0);
  char* oFile = P.getOptionValue("-o");
  int rounds = P.getOptionIntValue("-r",1);
  batchSize = P.getOptionIntValue("-b", 10000);

  graph<intT> G = readGraphFromFile<intT>(iFile);
  int cilkThreadCount = P.getOptionIntValue("-c", -1);
  if(cilkThreadCount > 0)
  {
      //std::string s = std::to_string(cilkThreadCount);
	  char num[3];
	  sprintf(num,"%d",cilkThreadCount);
      __cilkrts_end_cilk();
      __cilkrts_set_param("nworkers", num);
      __cilkrts_init();
      std::cout << "The number of threads " << cilkThreadCount << std::endl;
  }

  timePR(G, rounds, oFile);
}
