#include <algorithm>
#include <string>
#define _USE_MATH_DEFINES
#include <math.h>
#include <stdio.h>
#include <vector>

#include <cuda.h>
#include <cuda_runtime.h>
#include <driver_functions.h>
#include <thrust/device_vector.h>

typedef struct { /* Define the data structure for wire here */
   thrust::device_vector<thrust::device_vector<int>>hints;
   int* puzzle;
   int dim_x;
   int dim_y;
} pic_cross_t;