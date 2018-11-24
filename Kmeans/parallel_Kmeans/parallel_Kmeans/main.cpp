#include "seq_Kmeans_Manager.h"
#include "parallel_Manager.h"
#include <stdio.h>
#include <iostream>
#include <string>
#include <fstream>
#include <mpi.h>
#include <omp.h>

using namespace std;

#define MASTER 0 


const string INPUT_FILE_NAME = "D:\\parallel computing\\K-means\\Kmeans\\parallel_Kmeans\\parallel_Kmeans\\INPUT_FILE2.txt";
const string OUTPUT_FILE_NAME = "D:\\parallel computing\\K-means\\Kmeans\\parallel_Kmeans\\parallel_Kmeans\\output2.txt";

void createClusterAndPointMPITypes(Parallel_Manager&);


void main(int argc, char *argv[])
{

	int myId, numprocs, processorNameLen;
	char processor_name[MPI_MAX_PROCESSOR_NAME];
	MPI_Status status;
	Parallel_Manager* pm, *slave_pm;



	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &myId);
	MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
	MPI_Get_processor_name(processor_name, &processorNameLen);



	double startTime, endTime;
	startTime = MPI_Wtime();

	if (myId == MASTER)
		pm = new Parallel_Manager(INPUT_FILE_NAME, myId);
	else
		slave_pm = new Parallel_Manager(myId);

	MPI_Barrier(MPI_COMM_WORLD);

	if (myId == MASTER)
	{
		ofstream outputFile(OUTPUT_FILE_NAME);
		//Parallel_Manager* pm = new Parallel_Manager(INPUT_FILE_NAME);
		createClusterAndPointMPITypes(*pm);
		pm->master_broadcastToSlavesFirstTime(numprocs, myId);

		if (pm->runParallelAlgorithm_Master())
		{
			cout << "Found clusters position !\n" << endl;
			outputFile << *pm;

		}
		else
		{
			cout << "Clusters position with desired QM were not found!\n" << endl;
			outputFile << *pm;
		}
		endTime = MPI_Wtime();
		

	}
	else
	{
		createClusterAndPointMPITypes(*slave_pm);
		slave_pm->slaves_recieveStartInformation(myId);
		slave_pm->runParallelAlgorithm_Slave();
	}


	cout <<"pc: "<<processor_name <<" --> Process #: " << myId << " has finished\n" << endl;
	fflush(stdout);

	if (myId == MASTER)
	{
		cout << "The algorithm took: " << endTime - startTime << " seconds.\n" << endl;
		fflush(stdout);
	}


	MPI_Finalize();


	/************************************************************************************/
	/*									SEQUENCIAL ALGORITHM RUN						*/
	/************************************************************************************/

	/*Seq_Kmeans_Manager* seq_kmm = new Seq_Kmeans_Manager(INPUT_FILE_NAME);
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
	}*/
}

void createClusterAndPointMPITypes(Parallel_Manager& pm)
{
	pm.createPointsMPIDataType();
	pm.createClustersMPIDataType();
}