#ifndef __SEQUENCIALALGORITHM_H
#define __SEQUENCIALALGORITHM_H
#include "Point.h"
#include "Cluster.h"

class SequencialKmeans
{

private:
	Point* points = nullptr;
	Cluster* clusters = nullptr;
	int numberOfPoints;
	int numberOfClusters;
	double limit;
	bool isContainingClusterChanged = false;	//flag that means, if even one point moved from cluster to cluster, then there were a change during the cluster's center, and the clusters aren't cmplete yet


public:
	//calculating distance between to points 
	//double calcDistanceBetweenTwoPoints(Point * p1, Point * p2);
	//~SequencialKmeans();

	//adding the new center points and containing clusters to the points, within the new clusters' centers are calculated
	void calcNewDistancesForPoints();

	//sequential claculation and attaching points to clusters
	void sequentialPointsToCluster();

	double calcQualityMessure();

	//run the kmeans algo, sequncial
	double runKmeansSequencial(int numOfClusters, int numOfPoints, Point * pointsArr, Cluster * clustersArr, double limitOfIterations);


};


#endif	//__SEQUENCIALALGORITHM_H
