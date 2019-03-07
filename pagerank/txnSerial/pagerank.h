#include "graph.h"

extern int batchSize;
float* pagerank(graph<intT> G);

static const float alpha = 1.0 - 0.85;
static const float tolerance = 0.01;

