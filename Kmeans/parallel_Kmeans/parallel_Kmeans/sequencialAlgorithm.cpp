#include "sequencialAlgorithm.h"



void SequencialKmeans::calcNewDistancesForPoints()
{
	double calculatedCenter;
	Cluster* containingCluster;

	for (int i = 0; i < numberOfPoints; i++)
	{
		//calculate the distance of the point - i from the center of its containing cluster (after the cluster has new center).
		//if the point has no containing cluster then its distance from center will be default value, in class Point.
		containingCluster = points[i].getContainingCluster();
		if (containingCluster)
			this->points[i].setMinDistanceFromCenterPoint(points[i].calculateDistanceFromPoints(points[i].getContainingCluster()->getClusterCenterPoint()));

		for (int j = 0; j < numberOfClusters; j++)
		{
			calculatedCenter = points[i].calculateDistanceFromPoints(clusters[j].getClusterCenterPoint());

			/* if there is distance from points[i] to cluster center that is less then its current distance
			*  save that distance to the point, and add the containing cluster, clusters[j]
			*/
			if (points[i].getMinDistanceFromCenterPoint() > calculatedCenter)
			{
				this->points[i].setMinDistanceFromCenterPoint(calculatedCenter);
				if (containingCluster != this->clusters + j)	//check if the point isn't already in that cluster
				{
				//	cout << "point[i]: " << points + i << "\tprev containing cluster: " << containingCluster << "\new cluster: " << clusters + j << endl;
					isContainingClusterChanged = true;
					this->points[i].addContainingCluster(this->clusters + j);
				}
			}
		}
	}
}

//step 2 of the simple kmeans algo
void SequencialKmeans::sequentialPointsToCluster()
{
	// erase all the previous points from the clusters, before adding the new points
	for (int i = 0; i < numberOfClusters; i++)
		clusters[i].deleteAllPointsFromCluster();

	//after calculation of the distances for each point, add the points to the correct clusters
	for (int i = 0; i < numberOfPoints; i++)
		points[i].getContainingCluster()->addPoint(points + i);
}

double SequencialKmeans::calcQualityMessure()
{
	double currentQM = 0;
	Point* firstClusterCenterPoint, *secondClusterCenterPoint;
	double firstClusterDiameter;
	double distanceBetweenClustersCenterPoints;

	for (int j = 0; j < this->numberOfClusters; j++)
	{
		firstClusterDiameter = this->clusters[j].culculateDiameter();
		firstClusterCenterPoint = this->clusters[j].getClusterCenterPoint();

		for (int k = 0; k < this->numberOfClusters; k++)
		{
			if (k != j)
			{
				secondClusterCenterPoint = this->clusters[k].getClusterCenterPoint();
				distanceBetweenClustersCenterPoints = (firstClusterCenterPoint->calculateDistanceFromPoints(secondClusterCenterPoint));
				currentQM += (firstClusterDiameter / distanceBetweenClustersCenterPoints);
			}

		}
	}

	currentQM = currentQM / (numberOfClusters * (numberOfClusters - 1));
	return currentQM;
}

double SequencialKmeans::runKmeansSequencial(int numOfClusters, int numOfPoints, Point * pointsArr, Cluster * clustersArr, double limitOfIterations)
{
	numberOfPoints = numOfPoints;
	numberOfClusters = numOfClusters;
	points = pointsArr;
	clusters = clustersArr;
	limit = limitOfIterations;

	calcNewDistancesForPoints();	//assign the containing clusters to points in the beginning (good for case that the points don't have containing cluster yet)

	for (int i = 0; i < limit; i++)
	{
		sequentialPointsToCluster();
		//step 3 - recalculate cluster new center
		for (int c = 0; c < numberOfClusters; c++)
			clusters[c].culculateNewCenterPositionSequencial();

		calcNewDistancesForPoints();	//assign the containing clusters to points after the new centers were calculated

		if (!isContainingClusterChanged)
			break;
		isContainingClusterChanged = false;
	}

	sequentialPointsToCluster();	//assign the containing clusters to points vectors at the end of the 'limit' iterations (is needed for after the last iteration)
	return calcQualityMessure();	
}
