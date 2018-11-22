#pragma once
#ifndef __PARALLELMANAGER_H
#define __PARALLELMANAGER_H

#include "Cluster.h"
#include "sequencialAlgorithm.h"
#include <fstream>
#include <stdio.h>
#include <math.h>
#include <omp.h>
#include <mpi.h>



class Parallel_Manager
{

private:
#define MASTER 0

	//arranged by the file input order
	int numberOfPoints;					// N
	int numberOfClusters;				// K
	double totalRunTime;				// T
	double velocityCalcTimeInterval;	// dT
	double maxIterations;				// LIMIT
	double qualityMessure;				// QM

	double successTime;
	double successQM;

	string inputFileName;
	Point *points = nullptr;
	Cluster *clusters = nullptr;

	

	//initiallizing the parameters: N, K, T. dT, LIMIT, QM
	void initKmeansParamsFromFile(ifstream& inputFile);
	//reading the points coordinates from the file in the format Xi Yi Zi Vxi Vyi Vzi
	void initPointsFromFile(ifstream& inputFile);
	//assign the first k points as the centers of the k clusters, for the first time the program runs
	void initClustersFirstTime();


	/***************************************************************************************************/
	/*											PARALLEL WORK										   */			
	/***************************************************************************************************/

	//MPI
	MPI_Datatype pointStructMPIType;
	MPI_Datatype clusterStructMPIType;
	//arrays for master proc. to obtain and send the data.
	Point::PointAsStruct* pointsForProc = NULL;
	Cluster::ClusterAsStruct* clustersForProc = NULL;
	//arrays for slaves (and master) to work on
	Point::PointAsStruct* pointsToHandle;
	Cluster::ClusterAsStruct* clusterToHandle;
	int numOfProcesses;
	int numOfPointsForProc;

	//create 2 arrays of structs for points and clusters, to be sent to the slaves from master. 
	//The creation is from the original points that been read from the file. (clusters, points)
	void createArrayOfStructs();

	//calculate the amount of the points to send to each slave, also if there is a reminder in dividing the numOfPoints by numOfProc, handle it.
	void scatterPointsToSlavesFirstTime();
	
	/*collect clusters and points from structs to objects*/
	void createPointArrayFromStruct();
	void createClusterArrayFromStruct();




public:
	Parallel_Manager() {};
	Parallel_Manager(string inputFileName);
	~Parallel_Manager();


	/*
	*reading the whole file, and setting the parameters, also the points and initiallize the clusters, the function can get
	*nothing, then it will use the input file that was passed in the c'tor.
	*/
	void readInputFromFile(string inputFileName = "");

	//mpi bcast - master broadcasts the starting information to the slaves
	void masterBroadcastToSlavesFirstTime(int numOfProcs, int myId);

	//slaves collent the starting information from the master
	void slavesRecieveStartInformation(int myId);

	//this method goes through all the points and calculate their new position, according to the time
	void calcPointsNewPosition(double time);

	bool runSequencialAlgorithm();

	friend ostream& operator<< (ostream& out, const Parallel_Manager& man);

	/***************************************************************************************************/
	/*											PARALLEL WORK										   */
	/***************************************************************************************************/

	//create MPIDataType of pointsStruct
	void createPointsMPIDataType();
	//create MPIDataType of pointsStruct
	void createClustersMPIDataType();

};

#endif // __PARALLELMANAGER_H