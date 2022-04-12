#include <assert.h>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <omp.h>
#include <iostream>
#include <algorithm>
#include <fstream>
#include <sstream>
typedef struct { /* Define the data structure for wire here */
   std::vector<std::vector<int>> hints;
   int* puzzle;
   int dim_x;
   int dim_y;
} pic_cross_t;