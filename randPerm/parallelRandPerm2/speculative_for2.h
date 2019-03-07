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
#include "sequence.h"

struct reservation {
  intT r;
  reservation() : r(INT_T_MAX) {}
  void reserve(intT i) { utils::writeMin(&r, i); }
  bool reserved() { return (r < INT_T_MAX);}
  void reset() {r = INT_T_MAX;}
  bool check(intT i) { return (r == i);}
  bool checkReset(intT i) {
    if (r==i) { r = INT_T_MAX; return 1;}
    else return 0;
  }
};

inline void reserveLoc(intT& x, intT i) {utils::writeMin(&x,i);}

template <class S>
intT speculative_for(S step, intT s, intT e, int rsize, 
		     bool hasState=1, int maxTries=-1) {
  intT granularity = (e - s)/rsize + 1;
  if (maxTries < 0) maxTries = 100 + 200*granularity;
  intT maxRoundSize = max((intT)1,(e-s)/granularity);
  intT *I = newA(intT,maxRoundSize);
  intT *Ihold = newA(intT,maxRoundSize);
  bool *keep = newA(bool,maxRoundSize);
  S *state;
  if (hasState) {
    state = newA(S, maxRoundSize);
    for (intT i=0; i < maxRoundSize; i++) state[i] = step;
  }

  int round = 0; 
  intT numberDone = s; // number of iterations done
  intT numberKeep = 0; // number of iterations to carry to next round
  intT totalProcessed = 0;
  uint64_t reserveTime = 0, commitTime = 0, cleanupTime =0;

  while (numberDone < e) {
    //cout << "numberDone=" << numberDone << endl;
    if (round++ > maxTries && 0) {
      cout << "speculativeLoop: too many iterations, increase maxTries parameter" << endl;
      abort();
    }
    //intT size = min(maxRoundSize, e - numberDone);
    intT size = max((intT)1,(e-numberDone)/granularity);

    totalProcessed += size;
    uint64_t t0 = get_sys_clock();
    if (hasState) {
      parallel_for (intT i =0; i < size; i++) {
	if (i >= numberKeep) I[i] = numberDone + i;
	keep[i] = state[i].reserve(I[i]);
      } 
    } else {
      parallel_for (intT i =0; i < size; i++) {
	if (i >= numberKeep) I[i] = numberDone + i;
	keep[i] = step.reserve(I[i]);
      } 
    }
    uint64_t t1 = get_sys_clock();
    reserveTime += t1 - t0;

    if (hasState) {
      parallel_for (intT i =0; i < size; i++) 
	if (keep[i]) keep[i] = !state[i].commit(I[i]);
    } else {
      parallel_for (intT i =0; i < size; i++) 
	if (keep[i]) keep[i] = !step.commit(I[i]);
    }
    uint64_t t2 = get_sys_clock();
    commitTime += t2 - t1;
    // keep edges that failed to hook for next round
    numberKeep = sequence::pack(I, Ihold, keep, size);
    swap(I, Ihold);
    numberDone += size - numberKeep;
    uint64_t t3 = get_sys_clock();
    cleanupTime += t3 - t2;
  }
  free(I); free(Ihold); free(keep); 
  if(hasState) free(state);
  cout << "number of rounds = " << round << endl;
  cout << "max round size = " << maxRoundSize << endl;
  cout << "total work = " << totalProcessed << endl;

  cout << "time " << reserveTime << "," << commitTime << "," << cleanupTime << endl;
  return totalProcessed;
}
