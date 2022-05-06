#include "solver_seq.h"
#include <algorithm>
#include <string>
#define _USE_MATH_DEFINES
#include <math.h>
#include <stdio.h>
#include <vector>

#include <cuda.h>
#include <cuda_runtime.h>
#include <driver_functions.h>

int old_dim_x;
int old_dim_y;

static int _argc;
static const char **_argv;
#define thread_size 32

__global__ const char *get_option_string(const char *option_name, const char *default_value) {
    for (int i = _argc - 2; i >= 0; i -= 2)
        if (strcmp(_argv[i], option_name) == 0)
            return _argv[i + 1];
    return default_value;
}

int get_option_int(const char *option_name, int default_value) {
    for (int i = _argc - 2; i >= 0; i -= 2)
        if (strcmp(_argv[i], option_name) == 0)
            return atoi(_argv[i + 1]);
    return default_value;
}

__global__ float get_option_float(const char *option_name, float default_value) {
    for (int i = _argc - 2; i >= 0; i -= 2)
        if (strcmp(_argv[i], option_name) == 0)
            return (float)atof(_argv[i + 1]);
    return default_value;
}

__global__ void  print_hints(thrust::device_vector<thrust::device_vector<int>> hints) {
    for (int j = 0; j < hints.size(); j++) {
        for (int k = 0; k < hints[j].size(); k++) {
            printf("%d ", hints[j][k]);
        }
        printf("\n");
    }    
}
void print_puzzle(pic_cross_t pic_cross) {  
    int* puzzle = pic_cross.puzzle;
    int dim_x = pic_cross.dim_x;
    int dim_y = pic_cross.dim_y;
    for (int i = 0; i < dim_x; i++) {
        std::bitset<64> x(puzzle[i]);
            for (int k = dim_y - 1; k != -1; k --) {
                std::cout << x[k];
            }
            std::cout << "\n";
        }
}
__global__ void  print_2d(int** col) { 
    for (int i = 0; i < old_dim_x; i++) {
        for (int j = 0; j < old_dim_y; j++) {
            std::cout << col[i][j];
        }
        std::cout << "\n";
    }
}
__global__ void  print_row_perm(thrust::device_vector<thrust::device_vector<int>> Row_perm, int dim_y) {
    for (int i = 0; i < Row_perm.size(); i ++) {
        for (int j = 0; j < Row_perm[i].size(); j ++) {
            std::bitset<64> x(Row_perm[i][j]);
            for (int k = dim_y - 1; k != -1; k --) {
                std::cout << x[k];
            }
            std::cout << " ";
        }
            std::cout << "\n";
        }
}
__global__ void  write_output(int argc, const char *argv[], pic_cross_t pic_cross) {
    _argc = argc - 1;
    _argv = argv + 1;
    const char *input_filename = get_option_string("-f", NULL);
    int* puzzle = pic_cross.puzzle;
    int dim_x = pic_cross.dim_x;
    int dim_y = pic_cross.dim_y;

    std::string filename_long = ((std::string) input_filename);
    int index =filename_long.find_last_of("/");
    std::string filename = filename_long.substr(index + 1, (filename_long.size() - 4 - index - 1));
    std::ofstream output1;
    output1.open((std::string)"outputs/output_" + filename + ".txt");
    output1 << dim_x << " " << dim_y << "\n";
    for (int i = 0; i < dim_x; i++) {
        std::bitset<64> x(puzzle[i]);
            for (int k = dim_y - 1; k != -1; k --) {
                output1 << x[k];
            }
            output1 << "\n";
        }
    output1.close();
}
__global__ pic_cross_t read_input(int argc, const char *argv[]) {
    int dim_x, dim_y;
    pic_cross_t pic_cross;
    int temp = 1;
    
    _argc = argc - 1;
    _argv = argv + 1;
    const char *input_filename = get_option_string("-f", NULL);
    FILE *input = fopen(input_filename, "r");
    if (!input) {
        printf("Unable to open file: %s.\n", input_filename);
        return pic_cross;
    }
    fscanf(input, "%d %d\n", &dim_x, &dim_y);
    pic_cross.puzzle = (int*)calloc(dim_x, sizeof(int));
    pic_cross.dim_x = dim_x;
    pic_cross.dim_y = dim_y;
    thrust::device_vector<thrust::device_vector<int>> hints;
    hints.resize(dim_x + dim_y);
    std::ifstream file(input_filename);
    std::string line;
    int i = 0;
    while(getline(file, line)) {
        std::istringstream ss(line);
        int num;
        if (temp == 1) {
            temp = 0;
            continue;
        }
        while (ss >> num) {
            hints[i].push_back(num);
        }
        i++;
    }
    pic_cross.hints = hints;
    return pic_cross;
}

// void updatedim_xols(int row, int* grid, int* colVal, int* colIdx, int* cols){
//     float ixc = 1;
//     for(int c = 0; c < dim_x; c++){
//         // copy from previous
//         colVal[(row * dim_x) + c] = row==0 ? 0 : colVal[(row-1) * dim_x + c];
//         colIdx[row * dim_x + c] = row==0 ? 0 : colIdx[(row-1) * dim_x + c];
//         if((grid[row] & ixc)==0){
//             if(row > 0 && colVal[(row-1) * dim_x + c] > 0){ 
//                 // bit not set and col is not empty at previous row => close blocksize
//                 colVal[row * dim_x + c]=0;
//                 colIdx[row * dim_x + c]++;
//             }
//         }
//         else{
//             colVal[row * dim_x + c]++; // increase value for set bit
//         }
//         ixc <<= 1;
//     }
// }


__global__ float bits(int b){
    return (1 << b) - 1; // 1 => 1, 2 => 11, 3 => 111, ...
}

__global__ void  calcPerms(int r, int cur, int spaces, std::size_t perm, int shift, pic_cross_t pic_cross, thrust::device_vector<int> &res){
    // int dim_x = pic_cross.dim_x;
    // int dim_y = pic_cross.dim_y;
    // int* puzzle = pic_cross.puzzle;
    int index = blockIdx.x * blockDim.x + threadIdx.x;
    int thread_index = threadIdx.y * blockDim.x + threadIdx.x; 

    thrust::device_vector<thrust::device_vector<int>> hints = pic_cross.hints;

    if(cur == hints[r].size()){
        // if((puzzle[r] & perm) == puzzle[r]){
        //     res.add(perm);				
        // }
        res.push_back(perm);
        return;
    }
    while(spaces >= 0){
        int b = bits(hints[r][cur]);
        calcPerms(r, cur+1, spaces, perm|(b<<shift), shift+hints[r][cur]+1, pic_cross, res);
        shift++;
        spaces--;
    }
}

// at every row and column (every box in the grid)
// colVal[r][c]: current position within the current blocksize
// colIx[r][c]: current block index
// The value increased by 1 if the column is painted in the current row.
// The value reset to 0 and index increased by 1 if the column was painted in the previous row and is not in the current row.
__global__ void  updateCols(int row, int numCol, int* puzzle, int** colVal, int** colIx) {
    int index = blockIdx.x * blockDim.x + threadIdx.x;
    int thread_index = threadIdx.y * blockDim.x + threadIdx.x;
    int ixc = (int)pow(2, numCol-1);
    for (int c = 0; c < numCol; c++) {

        colVal[row][c] = (row == 0) ? 0 : colVal[row-1][c];
        colIx[row][c] = (row == 0) ? 0 : colIx[row-1][c];
        if ((puzzle[row] & ixc) == 0) {

            if ((row > 0) && (colVal[row-1][c] > 0)) {

                colVal[row][c] = 0;
                colIx[row][c]++; 
            }
        }
        else {
            
            colVal[row][c]++;
        }
        ixc >>= 1;
    }
}

__global__ void  rowMask(int row, int numCol, long* mask, long* val, 
             int** colVal,
             int** colIx, pic_cross_t* pic_cross) {

    int index = blockIdx.x * blockDim.x + threadIdx.x;
    int thread_index = threadIdx.y * blockDim.x + threadIdx.x;
    int dim_x = pic_cross->dim_x;
    int dim_y = pic_cross->dim_y;
    thrust::device_vector<thrust::device_vector<int>> hints = pic_cross->hints;
    mask[row] = 0;
    val[row] = 0;
    if (row == 0) {
        return;
    }
    int ixc = (int)pow(2, numCol-1);
    for (int c = 0; c < dim_y; c++) {
        if (colVal[row-1][c] > 0) {
            mask[row] |= ixc;
            int index = colIx[row-1][c];
            // printf("c: %d row: %d hints: %d colVal: %d \n", c, row, hints[c+dim_x][index], colVal[row-1][c]);
            if (hints[c + dim_x][index] > colVal[row-1][c]) {
                val[row] |= ixc;
            }
        }
        else if (colVal[row-1][c] == 0 && colIx[row-1][c] == hints[c+dim_x].size()) {
            mask[row] |= ixc;
        }
        ixc >>= 1;
    }
}



__global__ bool dfs(int row, thrust::device_vector<thrust::device_vector<int>>& Row_perm, 
        long* mask,
        long* val,
        int** colVal,
        int** colIx,
        pic_cross_t * pic_cross){
    int dim_x = pic_cross->dim_x;
    int dim_y = pic_cross->dim_y;
    int* puzzle = pic_cross->puzzle;
    int check = std::rand() %  Row_perm[row].size();
    int index = blockIdx.x * blockDim.x + threadIdx.x;
    int thread_index = threadIdx.y * blockDim.x + threadIdx.x;

    if (row == dim_x) {
        return true;
    }

    rowMask(row, dim_y, mask, val, colVal, colIx, pic_cross);

    for (int i = 0; i < Row_perm[row].size();i++) {
        if ((Row_perm[row][i] & mask[row]) != val[row] && i != check) {
            continue;
        }
        __syncthreads();
        puzzle[row] = Row_perm[row][i];

        updateCols(row, dim_y, puzzle, colVal, colIx);
        if (dfs(row+1, Row_perm, mask, val, colVal, colIx, pic_cross)) {   
            return true;
        }
    }
    return false;
}

int main(int argc, const char *argv[]) {

   const int bytes = sizeof(float) * N;
    cudaMalloc(&device_x, bytes);  // allocate array in device memory
    cudaMalloc(&device_y, bytes);      // allocate array in device memory
    cudaMalloc(&device_result, bytes);      // allocate array in device memory
    double startTime = CycleTimer::currentSeconds();


    auto init_start = Clock::now();
    double t_time = 0;
    pic_cross_t pic_cross = read_input(argc, argv);
    if (pic_cross.dim_x == 0) {
        return 1;
    }
    // print_hints(pic_cross.hints);
    int dim_x = pic_cross.dim_x;
    int dim_y = pic_cross.dim_y;
    old_dim_x = dim_x;
    old_dim_y = dim_y;
    thrust::device_vector<thrust::device_vector<int>> hints = pic_cross.hints;
    thrust::device_vector<thrust::device_vector<int>> Row_perm;
    //Precal stuff
    thrust::device_vector<int> res;
    for (int r = 0; r < dim_x; r++) {
        res.clear();
        int space = dim_y - (hints[r].size() - 1);
        for (int i = 0; i < hints[r].size(); i ++) {
            space -= hints[r][i];
        }
        calcPerms(r, 0, space, 0, 0, pic_cross, res);
        Row_perm.push_back(res);
    }
    cudaDeviceSynchronize();
    cudaMemcpy(resultarray, device_result, bytes, cudaMemcpyDeviceToHost);


    int** colVal = (int**) calloc(dim_x , sizeof(int*)) ;
    int** colIx = (int**) calloc(dim_x, sizeof(int*));
    for (int i = 0; i < dim_y; i++) {
        colVal[i] = (int*) calloc(dim_y, sizeof(int));
        colIx[i] = (int*) calloc(dim_y, sizeof(int));
    }
    long* mask = (long*) calloc(dim_x, sizeof(long));
    long* val = (long*) calloc(dim_x, sizeof(long));

    if (dfs(0, Row_perm, mask, val, colVal, colIx, &pic_cross)) {
    };
    double endTime = CycleTimer::currentSeconds();
    double overallDuration = endTime - startTime;
    printf("Overall: %.3f ms\t\t[%.3f GB/s]\n", 1000.f * overallDuration, toBW(totalBytes, overallDuration));
    cudaError_t errCode = cudaPeekAtLastError();

    if (errCode != cudaSuccess) {
        fprintf(stderr, "WARNING: A CUDA error occured: code=%d, %s\n", errCode, cudaGetErrorString(errCode));
    }

    write_output(argc, argv, pic_cross);
    
}
void printCudaInfo() {

    int deviceCount = 0;
    cudaError_t err = cudaGetDeviceCount(&deviceCount);

    printf("---------------------------------------------------------\n");
    printf("Found %d CUDA devices\n", deviceCount);

    for (int i = 0; i < deviceCount; i++) {
        cudaDeviceProp deviceProps;
        cudaGetDeviceProperties(&deviceProps, i);
        printf("Device %d: %s\n", i, deviceProps.name);
        printf("   SMs:        %d\n", deviceProps.multiProcessorCount);
        printf("   Global mem: %.0f MB\n", static_cast<float>(deviceProps.totalGlobalMem) / (1024 * 1024));
        printf("   CUDA Cap:   %d.%d\n", deviceProps.major, deviceProps.minor);
    }
    printf("---------------------------------------------------------\n");
}