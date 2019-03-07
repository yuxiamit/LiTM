// This code is part of the paper "A Simple and Practical Linear-Work
// Parallel Algorithm for Connectivity" in Proceedings of the ACM
// Symposium on Parallelism in Algorithms and Architectures (SPAA),
// 2014.  Copyright (c) 2014 Julian Shun, Laxman Dhulipala and Guy
// Blelloch
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
#include "randPerm.h"
#include <iostream>
#include "sequence.h"
#include "speculative_for2.h"
using namespace std;
int batchSize;
// <code-length>
template <class EltType> struct randPermStep {
  int* H, *R; EltType* A;
  randPermStep(int* _H, EltType* _A, int* _R) : H(_H), A(_A), R(_R) {}
  inline bool reserve(int i) {
    reserveLoc(R[i],i);
    reserveLoc(R[H[i]], i);
    return 1;
  }
  inline bool commit (int i) {
    int h = H[i];
    if(R[h] == i) {
      if(R[i] == i) {
        EltType t = A[i];
        A[i] = A[h];
        A[h] = t;
        R[h] = INT_MAX;
	      return 1;
      }
      R[h] = INT_MAX;
    }
    return 0;
  }
};
// </code-length>
template <class EltType> void randPerm(EltType *A, int n, int psize) {  
  int *H = (int*) malloc(sizeof(int)*n), *R = (int*) malloc(sizeof(int)*n);
  // <code-length>
  parallel_for (int i=0; i < n; i++) {
    H[i] = i+utils::hash(i)%(n-i); 
    R[i] = i;
  }
  // </code-length>
  speculative_for(randPermStep<EltType>(H, A, R), 0, n, batchSize, 0);
  free(H); free(R);
}

template void randPerm<int>(int*,int,int);
template void randPerm<double>(double*,int,int);
