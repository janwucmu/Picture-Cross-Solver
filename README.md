# Picture-Cross-Solver
## Authors
Yunchieh Janabelle Wu (yunchiew) and Goran Goran (ggoran)

## Summary
Picture Cross is a puzzle game, and we are parallelizing the solver for the large puzzles with high ambiguity. We will be using data structures, like a matrix, to perform parallelism on GPUs.  

## Background
Picture Cross is a smartphone inspired game that colors in the grids depending on the information given at each row and column. The solver we are trying to build is to figure out which cells in the grid that should be shaded. The shaded grid has to match the requirements given at each row and column. The numbers given at each row and column represent the number of contiguous shaded cells. 

For example, when you have 8x8 grid, and the information given for a row is:

`2 2 2 |.|.|.|.|.|.|.|.|`

`|.|` represents an unshaded cell in the grid

`|x|` represents a shaded cell in the grid

`| |` represents a cell that is guaranteed unshaded in the grid

Each of the "2" in the row/column information represent 2 contiguous shaded cells with at least 1 cell between each set of contigous shaded cells. The only solution to fit the 3 different contiguous shaded cells, which are 2 cells each, is shown below:

`2 2 2 |x|x|.|x|x|.|x|x|`

On the other hand, when the requirements for the row is:

`1 3 1 |.|.|.|.|.|.|.|.|`

There are multiple solutions for this to work. In order to verify the correctness of it, the solver will have to also check the column requirement. Below are some  (but not all) possible solutions: 

`1 3 1 |x|.|x|x|x|.|x|.|`

`1 3 1 |x|.|x|x|x|.|.|x|`

`1 3 1 |.|x|.|x|x|x|.|x|`

With the `1 3 1` requirement, we know that all possible shadings will have these cells (below) shaded in for the 3 contiguous shaded cells. So we can shade these cells in and guarantee their correctness.

`1 3 1 |.|.|.|x|x|.|.|.|`

Furthermore, if we are sure that the cell below is shaded in from the column shading

`2 1 1 |.|.|x|.|.|.|.|.|`

We set the status of the first cell in this row to be guaranteed unshaded in the grid because no other contiguous shaded cells comes before `2`, and the 2 possible shadings are one to the left or one to the right of the shaded cell 

`2 1 1 | |.|x|.|.|.|.|.|`

The parallelism in the solver will find tasks at each stage and parallelize these tasks.

The first pass of the grid is to look through all the requirements at each row and column and figure out the guaranteed shaded cells based on intersections of all possible solution. This algorithm can be done in parallel.

The second pass will be to look over all the cells in the grid to figure out the next set of cells that can be guaranteed shaded or unshaded based on the new updates in the grid. This is looped over multiple times while checking what strategies it can use to guarantee shaded or unshaded cells.

## Challenge


## Resources
There are a few resources that uses Python (https://github.com/mprat/nonogram-solver) and JSON (https://github.com/ThomasR/nonogram-solver) to solve the picture cross puzzles.

## Goals and Deliverables


## Platform Choice
### Languages
We are using CUDA because the resources we are using to run parallelism is on GPU. We are also using C++ because it work well with CUDA, it is fast, and has many defined libraries for us to utilize.

### Machine
We are using GPUs to run the parallelism. We would want to use GPUs because there the problem can be broken up into multiple independent parts, like cells, rows, and columns. There are also many different stages of parallelism for the solver. At each stage, we have similar execution instructions, so we can utilize the SIMD parallelism in the GPUs. 

## Schedule
3/28: 

4/4:

4/11:

4/18:

4/25:

5/2:
