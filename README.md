# Parallel-Maximum-Sum-Submatrix-Variation
**CPSC 474 - Parallel and Distributed Computing**\
**Project 2 - Parallel Maximum Sum Submatrix Variation**

**Problem description:**\
Develop a parallel programming solution using the MPI C++ library to find the submatrix (AxB) with the greatest sum within a larger matrix (NxM). This problem is a simplified variation on the [Maximum Sum Submatrix](https://www.geeksforgeeks.org/maximum-sum-submatrix/) problem.

**How it works:**\
This program will use command line arguments for the matrix and submatrix dimensions (N,M,A,B). The master process will read in integer values from the specified input file. The master process will then distribute the submatrices iteratively to the workers. The workers will calculate the sum for each submatrix it receives and send the maximum back to the master process. The master process will calculate which worker process found the maximum sum submatrix and produce the results.

**Running the program:**\
Compile:
```
mpic++ main.cpp main.o
```
Execute:
```
mpiexec -n <nprocs> ./main.o <mrows> <mcols> <srows> <scols> <filename>
```

**Command line arguments:**\
nprocs - number of processes (master + workers)\
mrows - number of rows for the matrix to be read from the file (N)\
mcols - number of columns for the matrix to be read from the file (M)\
srows - number of rows for the submatrix (A)\
scols - number of columns for the submatrix (B)\
filename - filename of the file with the matrix values (include .txt)

**Constraints:**
1) Submatrix rows must be less than or equal to matrix rows. (A < N)
2) Submatrix columns must be less than or equal to matrix columns. (B < M)
3) Matrix rows and matrix columns must be greater than or equal to 10. (N >= 10 and M >= 10)
4) Must have at least 2 processes. (1 master process and 1 worker process)
  
**Test files:**\
Test1.txt – 10 rows 10 columns\
Test2.txt – 30 rows 30 columns\
Test3.txt – 12 rows 18 columns\
Test4.txt – 100 rows 100 columns\
Test5.txt – 1000 rows 1000 columns

**Links:**\
[GeeksForGeeks - Maximum Sum Submatrix](https://www.geeksforgeeks.org/maximum-sum-submatrix/)
