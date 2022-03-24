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
The challenge of creating a parallel picture cross solver exists partially in the lack of many resources dedicated to this endeavor. Other logic puzzles and games such as chess and sudoko have been parallized to death, with a huge variety of different resources available for inspection. When solving a picture cross puzzle using a parallel implementation it is important for us to make sure we are making proper use of spatial locality in our memory accesses. Depending on how we go about solving the logic puzzle it is possible for us to take drastically longer than if we were to use a different approach. It is up to us to try out multiple different approaches and see how well that generalizes to the average puzzle, and choose the one that gives us the best results. This sort of trial and error approach may prove to be time consuming but it is somethng we should attempt in order to optimize our solution.

## Resources
There are a few resources that uses Python (https://github.com/mprat/nonogram-solver) and JSON (https://github.com/ThomasR/nonogram-solver) to solve the picture cross puzzles.

## Goals and Deliverables
### Plan to Achieve
* Implement a full functional parallel picture cross solver with non-trivial speedup
* Provide speedup graphs of based on number of cores
* Have our solver working on line solvable puzzles, puzzles that do not require backtracking

### Hope to Achieve
* Have our solver working on Symmetrical Puzzles, puzzles with symmetry are not line solvable and will require the use of a backtracking algorithm
* Show images of different stages of our puzzle as it is being solved


## Platform Choice
### Languages
We are using CUDA because the resources we are using to run parallelism is on GPU. We are also using C++ because it work well with CUDA, it is fast, and has many defined libraries for us to utilize.

### Machine
We are using GPUs to run the parallelism. We would want to use GPUs because there the problem can be broken up into multiple independent parts, like cells, rows, and columns. There are also many different stages of parallelism for the solver. At each stage, we have similar execution instructions, so we can utilize the SIMD parallelism in the GPUs. 

## Schedule
3/28: Implement sequential version of Picture Cross Solver

4/4: Implement parallel version of Picture Cross solver using CUDA

4/11: Milestone Report due, Evaluate effectiveness of Picture Cross Solver

4/18: Attempt to refine our parallel algorithm to seek better speedup

4/25: Implement backtracking so that we can solve non-line solvable puzzles

5/2: Handin and Demo
