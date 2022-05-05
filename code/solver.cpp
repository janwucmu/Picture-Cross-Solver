#include "solver.h"

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
#include <bitset>

int old_dim_x;
int old_dim_y;

static int _argc;
static const char **_argv;

const char *get_option_string(const char *option_name, const char *default_value) {
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

float get_option_float(const char *option_name, float default_value) {
    for (int i = _argc - 2; i >= 0; i -= 2)
        if (strcmp(_argv[i], option_name) == 0)
            return (float)atof(_argv[i + 1]);
    return default_value;
}

void print_hints(std::vector<std::vector<int>> hints) {
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
            std::cout << " ";
        }
            std::cout << "\n";
}
void print_row_perm(std::vector<std::vector<int>> Row_perm, int dim_y) {
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
void write_output(int argc, const char *argv[], pic_cross_t pic_cross) {
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
            output1 << " ";
        }
            output1 << "\n";
    output1.close();
}
pic_cross_t read_input(int argc, const char *argv[]) {
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
    std::vector<std::vector<int> > hints;
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


// void rowMask(int row, int* mask, int* val, int* colVal, int* colIdx, int* cols, pic_cross_t pic_cross){
//     int dim_x = pic_cross.dim_x;
//     mask[row] = 0;
//     val[row] = 0;
//     if(row == 0){
//         return;
//     }
//     float ixc = 1;
//     for(int c = 0; c < dim_x; c++){
//         if(colVal[row-1][c] > 0){
//             // when column at previous row is set, we know for sure what has to be the next bit according to the current size and the expected size
//             mask[row] |= ixc; 
//             if(cols[c][colIdx[row-1][c]] > colVal[row-1][c]){
//                 val[row] |= ixc; // must set
//             }
//         }
//         else if(colVal[row-1][c] == 0 && colIdx[row-1][c] == row){ // last row has to be length of columns
//             // can not add anymore since out of indices
//             mask[row] |= ixc;
//         }
//         ixc <<= 1;
//     }
// }
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


float bits(int b){
    return (1 << b) - 1; // 1 => 1, 2 => 11, 3 => 111, ...
}

// void printBit(float n){
//     while(n > 0){
//         printf("%d", n&1);
//         n >>= 1;
//     }
// }
void calcPerms(int r, int cur, int spaces, std::size_t perm, int shift, pic_cross_t pic_cross, std::vector<int> &res){
    // int dim_x = pic_cross.dim_x;
    // int dim_y = pic_cross.dim_y;
    // int* puzzle = pic_cross.puzzle;
    std::vector<std::vector<int>> hints = pic_cross.hints;

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
void updateCols(int row, int numCol, int* puzzle, int** colVal, int** colIx) {
    int ixc = 1;
    for (int c = 0; c < numCol; c++) {
        // copy from previous row
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
        ixc <<= 1;
    }
}

void rowMask(int row, int numCol, std::vector<int>& mask, std::vector<int>& val, 
             int** colVal,
             std::vector<std::vector<int>>& colIx, pic_cross_t pic_cross) {
    
    int dim_x = pic_cross.dim_x;
    int dim_y = pic_cross.dim_y;
    std::vector<std::vector<int>> hints = pic_cross.hints;
    mask[row] = 0;
    val[row] = 0;
    if (row == 0) {
        return;
    }
    int ixc = 1;
    for (int c = dim_x; c < dim_x+dim_y; c++) {
        if (colVal[row-1][c] > 0) {
            mask[row] |= ixc;
            int index = colIx[row-1][c];
            if (hints[c][index] > colVal[row-1][c]) {
                val[row] |= ixc;
            }
        }
        else if (colVal[row-1][c] == 0 && colIx[row-1][c] == hints[c].size()) {
            mask[row] |= ixc;
        }
        ixc <<= 1;
    }
}
// boolean dfs(int row, int* rowPerms, int *rowPermsdim_xnt, 
//             int* mask, int* val, int* grid, int* colVal, 
//             int* colIdx, int* cols, pic_cross_t pic_cross){
//     if(row == dim_y){
//         return true;
//     }
//     rowMask(row, mask, val, colVal, colIdx, cols, pic_cross); // calculate mask to stay valid in the next row
//     for(int i = 0; i < rowPermsdim_xnt[row]; i++){
//         if((rowPerms[row][i] & mask[row]) != val[row]){
//             continue;
//         }
//         grid[row] = rowPerms[row][i];
//         updatedim_xols(row, grid, colVal, colIdx, cols);
//         if(dfs(row + 1, rowPerms, rowPermsdim_xnt, mask, val, grid, colVal, colIdx, cols)){
//             return true;
//         }
//     }
//     return false;
// }


bool dfs(int row, std::vector<std::vector<int>>& Row_perm, 
        long* mask,
        long* val,
        int** colVal,
        int** cols,
        int** colIx,
        pic_cross_t * pic_cross){

    int dim_x = pic_cross->dim_x;
    int dim_y = pic_cross->dim_y;
    int* puzzle = pic_cross->puzzle;
    rowMask(row, dim_y, &mask, coVal, colIx, pic_cross);
    for (int i = 0; i < Row_perm[row].size();i++) {
        if (Row_perm[row][i] & mask[row] != val[row]) {
            continue;
        }
        puzzle[row] = Row_perm[row][i];
        updateCols(row, dim_y, &puzzle, &colVal, &colIx);
        if (dfs(row+1, Row_perm, &mask, &val, &colVal, &colIx, &pic_cross)) {
            return true;
        }
    }
    return false;
}

int main(int argc, const char *argv[]) {
    pic_cross_t pic_cross = read_input(argc, argv);
    if (pic_cross.dim_x == 0) {
        return 1;
    }
    print_hints(pic_cross.hints);
    int dim_x = pic_cross.dim_x;
    int dim_y = pic_cross.dim_y;
    std::vector<std::vector<int>> hints = pic_cross.hints;
    std::vector<std::vector<int>> Row_perm;
    //Precal stuff
    std::vector<int> res;
    for (int r = 0; r < dim_x; r++) {
        res.clear();
        int space = dim_y - (hints[r].size() - 1);
        for (int i = 0; i < hints[r].size(); i ++) {
            space -= hints[r][i];
        }
        calcPerms(r, 0, space, 0, 0, pic_cross, res);
        // printf("%d HUUHH", res[0]);
        Row_perm.push_back(res);
    }
    print_row_perm(Row_perm, dim_y);
    int colVal[dim_x][dim_y] ;
    int colIx[dim_x][dim_y];
    long mask[dim_x];
    long val[dim_x];
    // if (dfs(0, Row_perm, &mask, &val, &colVal, &colIx, &pic_cross)) {
    //     print_puzzle(pic_cross);
    // }



    // for (int i = 0; i < Row_perm.size(); i ++) {
    //     for (int j = 0; j < Row_perm[i].size(); j ++) {
    //         std::bitset<64> x(Row_perm[i][j]);
    //         for (int k = dim_y - 1; k != -1; k --) {
    //             std::cout << x[k];
    //         }
    //         std::cout << " ";
    //     }
    //       std::cout << "\n";

    // }
    // print_hints(Row_perm);
    // print_puzzle(pic_cross);
    // write_output(argc, argv, pic_cross);
    return 0;
}


// void calcPerms(int r, int cur, int spaces, float perm, int shift, int* rows, int* grid, int* res, int length){
//     if(cur == length){
//         if((grid[r] & perm) == grid[r]){
//             res.add(perm);				
//         }
//         return;
//     }
//     while(spaces >= 0){
//         calcPerms(r, cur+1, spaces, perm|(bits(rows[r][cur])<<shift), shift+rows[r][cur]+1, rows, grid, res);
//         shift++;
//         spaces--;
//     }
// }
// int main(int argc, char **argv) {
//     // read input
//     // TODO: find row and col from txt
//     int dim_x, dim_y; // number of rows and cols

//     // split into blocks
//     int blocksize = 3;

//     pic_cross_t pic_cross = read_input(argc, argv);
//     int* grid = pic_cross_t pic_cross;
//     std::vector<std::vector<int>> hints = pic_cross.hints;

//     dim_x = pic_cross_t pic_cross.dim_x;
//     dim_y = pic_cross_t pic_cross.dim_y;
//     old_dim_x = dim_x;
//     old_dim_y = dim_y;

// 	int rows[dim_y][dim_x/blocksize];
//     int cols[dim_y/blocksize][dim_x];

//     for (int i = 0; i < dim_y; i++) {
//         for (int j = 0; j < dim_x/block_size; j++) {
//             rows[i][j] = hints[dim_x/block_size * i + j];
//         }
//     }
//     for (int i = 0; i < dim_y/block_size; i++) {
//         for (int j = 0; j < dim_x; j++) {
//             cols[i][j] = hints[dim_x * i + j];
//         }
//     }
//     // grid 
//     // TODO: parse
// 	// float grid[dim_y][dim_x];

//     // Precalc
//     std::vector<std::vector<float>> rowPerms; // bitwise possible permutations per row
//     rowPerms.resize(dim_y);
//     for(int r = 0; r < dim_y; r++){
//         std::vector<int> res;
//         int spaces = dim_x - (length);
//         for(int i = 0; i < length; i++){
//             spaces -= rows[r][i];
//         }
//         calcPerms(r, 0, spaces, 0, 0, res);
//         rowPerms[r] = [res.size()];
//         while(res.size() != 0){
//             rowPerms[r][res.size()-1] = res[res.size() - 1];
//             res.erase(res.size() - 1);
//         }
//     }

//     // dim_xalculate
//     int colVal[dim_y*dim_x];
//     int colIdx[dim_y*dim_x];
//     float mask[dim_y];
//     float val[dim_y];
//     if(dfs(0, rowPerms, rowPermsdim_xnt, mask, val, grid, colVal, colIdx, cols, pic_cross)){
//         // Print
//         for(int r = 0; r < dim_y; r++){
//             for(int c = 0; c < dim_x; c++){
//                 int result = (grid[r] & (1<<c)) == 0 ? EMPTY : FILLED;
//                 printf("%d", result);
//             }
//             printf("\n");
//         }
//     }
//     else{
//         printf("No solution was found");
//     }
//     return 0;
// }