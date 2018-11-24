#pragma once
#ifndef __PARALLELKMEANSALGORITHM_H
#define __PARALLELKMEANSALGORITHM_H

#include "Point.h"
#include "Cluster.h"
#include <mpi.h>




class ParallelKmeanAlgorithm
{

#define POINT_DIMENSION 3
#define MASTER 0

#define INIT_INFO_TAG	0
#define WORKING_TAG		1
#define STOP_KMEANS		2
#define COMPUTE_QM_TAG	3
#define TERMINATION_TAG 4

private:
	Point* points = nullptr;
	Cluster* clusters = nullptr;
	int numberOfPoints;
	int numberOfClusters;
	double limit;
	bool isContainingClusterChanged = false;	//flag that means, if even one point moved from cluster to cluster, then there were a change during the cluster's center, and the clusters aren't cmplete yet
	double* clustersCentersSum;
	int* localClusterNumOfPoints;

	void calculateSumOfClustersCenters(int indexOfCluster);


public:
	//calculating distance between to points 
	//double calcDistanceBetweenTwoPoints(Point * p1, Point * p2);
	//~SequencialKmeans();

	ParallelKmeanAlgorithm(int numOfPoints, int numOfClusters, Point * pointsArr, Cluster * clustersArr, double limitOfIterations);

	//adding the new center points and containing clusters to the points, within the new clusters' centers are calculated
	void calcNewDistancesForPoints();

	//sequential claculation and attaching points to clusters
	void groupPointsToCluster();

//	double calcQualityMessure();


	//run the kmeans algo, parallel
	void runKmeansParallel_Master(int numOfProcs);
	void runKmeansParallel_Slave();


};

#endif // !__PARALLELKMEANSALGORITHM_H
