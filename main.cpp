//----------------------------------------------------------------------------------------------------
//Author: Arun Sangar
//Course: CPSC474
//Project 2
//Problem: Find largest AxB matrix sum in a NxM matrix
//Command line arugments: <matrix row> <matrix col> <submatrix row> <submatrix col> <filename>
//----------------------------------------------------------------------------------------------------

#include <iostream>
#include <fstream>
#include <string>
#include <iomanip>
#include <mpi.h>

bool verifyArgs(int, char**, int, int);
void readFile(int**, int, int, std::string);
void printMatrix(int**, int, int);
void printResults(int*, int**, int, int, int ,int);
void getSubMatrix(int**, int*, int, int, int, int);
int calculateSubMatrix(int*, int);

int main(int argc, char** argv) {
	int rank, size;

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	//verify command line arguments
	if (!verifyArgs(argc, argv, rank, size)) return 0;

	//get matrix dimensions and submatrix subdimensions
	int dimensionRow = std::stoi(argv[1]);
	int dimensionCol = std::stoi(argv[2]);
	int subDimensionRow = std::stoi(argv[3]);
	int subDimensionCol = std::stoi(argv[4]);

	int subMatrixLength = subDimensionRow * subDimensionCol;
	int* subMatrix = new int[subMatrixLength];
	int** matrix = NULL;

	//used for storing results of each process
	int localResults[3] = { 0, rank, 0 };
	int globalResults[3] = { 0, 0, 0 };

	//master process
	if (rank == 0) {
		std::string filename = argv[5];

		//dynamic memory for nxn matrix
		matrix = new int* [dimensionRow];
		for (int i = 0; i < dimensionRow; i++)
			matrix[i] = new int[dimensionCol];

		//read, store and print input file
		readFile(matrix, dimensionRow, dimensionCol, filename);
		printMatrix(matrix, dimensionRow, dimensionCol);

		//intialize helper variable
		int workerRank = 0;

		//loop for every submatrix
		for (int i = 0; i < dimensionRow - subDimensionRow + 1; i++)
			for (int j = 0; j < dimensionCol - subDimensionCol + 1; j++) {
				//calculate worker rank - decides which worker process get the submatrix
				workerRank = (workerRank % (size - 1)) + 1;

				//get the submatrix and send it to the worker
				getSubMatrix(matrix, subMatrix, i, j, subDimensionRow, subDimensionCol);
				MPI_Send(&subMatrix[0], subMatrixLength, MPI_INT, workerRank, workerRank, MPI_COMM_WORLD);
			}
	}
	else {
		//initialize helper variables
		int numSubMatrices = (dimensionRow - subDimensionRow + 1) * (dimensionCol - subDimensionCol + 1);
		int currentResult = 0;

		//loop for every submatrix the process should receieve
		for (int i = rank; i < numSubMatrices + 1; i += (size - 1)) {
			//receive submatrix from master process
			MPI_Recv(&subMatrix[0], subMatrixLength, MPI_INT, 0, rank, MPI_COMM_WORLD, MPI_STATUSES_IGNORE);

			//get the sum of the submatrix and check if it is the largest so far
			//save the sum and the sub matrix number
			currentResult = calculateSubMatrix(subMatrix, subMatrixLength);
			if (currentResult > localResults[0]) {
				localResults[0] = currentResult;
				localResults[2] = i;
			}
		}
	}

	//clear dyanmic memory for sub matrix
	delete[] subMatrix;

	//mpi reduce for finding the maximum sum from all the process
	//(localResults[0] stored local max sum and globalResults[0] receives global max sum)
	//mpi_maxloc allows us to identify the rank which found the largest value
	//(localResults[1] stored local rank and globalResults[1] receives rank of process with max sum)
	MPI_Reduce(localResults, globalResults, 1, MPI_2INT, MPI_MAXLOC, 0, MPI_COMM_WORLD);

	//broadcast the winner to all processes (used in next if statement)
	MPI_Bcast(&globalResults, 2, MPI_INT, 0, MPI_COMM_WORLD);

	//if current process is the winner send its results to the master process
	if (globalResults[1] == rank)
		MPI_Send(localResults, 3, MPI_INT, 0, 0, MPI_COMM_WORLD);
	//if current process is master process receive results from the winner
	else if (rank == 0) {
		//receive and print the results
		MPI_Recv(globalResults, 3, MPI_INT, globalResults[1], 0, MPI_COMM_WORLD, MPI_STATUSES_IGNORE);
		printResults(globalResults, matrix, dimensionRow, dimensionCol, subDimensionRow, subDimensionCol);

		//clear dynamic memory for matrix
		for (int i = 0; i < dimensionRow; i++) 
			delete[] matrix[i];
		delete[] matrix;
	}
	
	MPI_Finalize();
	return 0;
}

bool verifyArgs(int argc, char** argv, int rank, int size) {
	//check number of processes
	if (size < 2) {
		if (rank == 0) std::cerr << "Error: Incorrect number of processes";
		return false;
	}
	//check number of command line arguments
	if (argc < 5) {
		if (rank == 0) std::cerr << "Error: Incorrect command line arguments";
		return false;
	}
	//check matrix dimension
	if (std::stoi(argv[1]) < 10 || std::stoi(argv[2]) < 10) {
		if (rank == 0) std::cerr << "Error: Matrix must be at least 10x10";
		return false;
	}
	//check if subdimension is less than dimension
	if ((std::stoi(argv[1]) < std::stoi(argv[3])) || (std::stoi(argv[2]) < std::stoi(argv[4]))) {
		if (rank == 0) std::cerr << "Error: Sub matrix dimension is larger than matrix dimension";
		return false;
	}
	return true;
}

void readFile(int** matrix, int dimensionRow, int dimensionCol, std::string filename) {
	std::ifstream file;
	file.open(filename);

	for (int i = 0; i < dimensionRow; i++)
		for (int j = 0; j < dimensionCol; j++)
			file >> matrix[i][j];

	file.close();
}

void printMatrix(int** matrix, int dimensionRow, int dimensionCol) {
	std::cout << dimensionRow << "x" << dimensionCol << " matrix: " << std::endl;
	for (int i = 0; i < dimensionRow; i++) {
		for (int j = 0; j < dimensionCol; j++)
			std::cout << std::setw(3) << matrix[i][j] << " ";
		std::cout << std::endl;
	}	
}

void printResults(int* results, int** matrix, int dimensionRow, int dimensionCol, int subDimensionRow, int subDimensionCol) {
	//get x and y coordinates of the sub matrix using the submatrix number
	int locX = (results[2] - 1) / (dimensionRow - subDimensionRow + 1);
	int locY = (results[2] - 1) % (dimensionCol - subDimensionCol + 1);

	std::cout << std::endl << "Process " << results[1] << " found the " << subDimensionRow 
		<< "*" << subDimensionCol << " matrix with the largest sum." << std::endl;
	std::cout << "Sub matrix sum: " << results[0] << std::endl;
	std::cout << "Sub matrix number: " << results[2] << std::endl;
	std::cout << "Location: (" << locX << ", " << locY << ")" << std::endl;
	std::cout << "Sub matrix:" << std::endl;
	for (int i = locX; i < locX + subDimensionRow; i++) {
		for (int j = locY; j < locY + subDimensionCol; j++)
			std::cout << std::setw(3) << matrix[i][j] << " ";
		std::cout << std::endl;
	}
}

void getSubMatrix(int** matrix, int* subMatrix, int x, int y, int subDimensionRow, int subDimensionCol) {
	int counter = 0;
	for (int i = x; i < x + subDimensionRow; i++)
		for (int j = y; j < y + subDimensionCol; j++)
			subMatrix[counter++] = matrix[i][j];
}

int calculateSubMatrix(int* subMatrix, int subMatrixLength) {
	int sum = 0;
	for (int i = 0; i < subMatrixLength; i++) 
		sum += subMatrix[i];

	return sum;
}
