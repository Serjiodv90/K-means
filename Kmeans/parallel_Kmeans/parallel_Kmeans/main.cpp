#include "seq_Kmeans_Manager.h"
#include <stdio.h>
#include <fstream>
#include <mpi.h>
#include <omp.h>

using namespace std;

#define MASTER 0 


const string INPUT_FILE_NAME = /*"input3.txt"*/"INPUT_FILE.txt";
const string OUTPUT_FILE_NAME = "output.txt";

void main(int argc, char *argv[])
{

	int myId, numprocs;
	MPI_Status status;

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &myId);
	MPI_Comm_size(MPI_COMM_WORLD, &numprocs);

	if (numprocs == 1)
	{
		/*
		implement a menu choosing of sequencial or parallel running for the kmean, when there is only 1 proc.
		*/
		//MPI_Abort(MPI_COMM_WORLD, 1);
	}
	else
	{
		if (myId == MASTER)
		{

		}
	}




	Seq_Kmeans_Manager* seq_kmm = new Seq_Kmeans_Manager(INPUT_FILE_NAME);
	ofstream outputFile(OUTPUT_FILE_NAME);

	if (seq_kmm->runSequencialAlgorithm())
	{
		cout << "found clusters position !" << endl;
		outputFile << *seq_kmm;

	}
	else
	{ 
		cout << "clusters position with desired QM were not found!" << endl;
		outputFile << *seq_kmm;
	}
	


	
	
}