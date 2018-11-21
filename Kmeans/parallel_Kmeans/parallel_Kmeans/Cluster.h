#ifndef __CLUSTER_H
#define __CLUSTER_H

#include "Point.h"
#include <vector>
#include <iomanip> 
#include <iostream>
#include <stdio.h>
#include <algorithm> // for copy(), remove, erase
#include <iterator> // for back_inserter

using namespace std;

class Point;

class Cluster
{

private:
	Point* centerPoint = nullptr;
	vector<const Point*> clusterPoints;
	double diameter = 0.0; 


public:
#define NUM_OF_ELEMENTS_IN_CLUSTER_STRUCT 4
	struct ClusterAsStruct
	{
		double center_x;
		double center_y;
		double center_z;
		double diameter;
	};

	Cluster() {};	//default c'tor for creation of dynamic array of clusters
	Cluster(int numOfPoints, Point* centerPoint);
	//~Cluster();


	void addPoint(const Point* point);
	void removePointFromCluster(const Point* point);
	void deleteAllPointsFromCluster();
	void setCenterPoint(Point* centerPoint);

	//sequencial calculation of new cluster center point
	void culculateNewCenterPositionSequencial();
	
	//parallel calculation of new cluster center point, via OMP
	void culculateNewCenterPositionParallel();

	Point* getClusterCenterPoint()	const;

	double culculateDiameter();

	//for MPI implementation
	ClusterAsStruct getClusterAsStruct()	const;
	int getNumOfElementsInClusterAsStruct()	const;

	friend ostream& operator<< (ostream& out, const Cluster& cluster);

	//const Cluster& operator=(Cluster& cluster);


};





#endif // !__CLUSTER_H
