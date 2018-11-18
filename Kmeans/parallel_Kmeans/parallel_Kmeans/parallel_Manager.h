#pragma once
#ifndef __PARALLELMANAGER_H
#define __PARALLELMANAGER_H

#include "Cluster.h"
#include "sequencialAlgorithm.h"
#include <fstream>
#include <stdio.h>
#include <math.h>
#include <omp.h>



class parallel_Manager
{

private:
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


public:
	parallel_Manager(string inputFileName);
	~parallel_Manager();


	/*
	*reading the whole file, and setting the parameters, also the points and initiallize the clusters, the function can get
	*nothing, then it will use the input file that was passed in the c'tor.
	*/
	void readInputFromFile(string inputFileName = "");


	//this method goes through all the points and calculate their new position, according to the time
	void calcPointsNewPosition(double time);

	bool runSequencialAlgorithm();

	friend ostream& operator<< (ostream& out, const parallel_Manager& man);
};

#endif // __PARALLELMANAGER_H