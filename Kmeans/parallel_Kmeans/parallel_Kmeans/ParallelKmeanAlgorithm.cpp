#include "ParallelKmeanAlgorithm.h"



ParallelKmeanAlgorithm::ParallelKmeanAlgorithm(int numOfPoints, int numOfClusters, Point * pointsArr, Cluster * clustersArr, double limitOfIterations)
{
	numberOfPoints = numOfPoints;
	numberOfClusters = numOfClusters;
	points = pointsArr;
	clusters = clustersArr;
	limit = limitOfIterations;
	this->clustersCentersSum = new double[numOfClusters * POINT_DIMENSION];	//zero init

}

void ParallelKmeanAlgorithm::calcNewDistancesForPoints()
{
	double calculatedCenter;
	Cluster* containingCluster;

#pragma omp parallel for private(containingCluster, calculatedCenter)
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
					this->points[i].setContainingClusterIndex(j);
				}
			}
		}
	}
}

//step 2 of the simple kmeans algo
void ParallelKmeanAlgorithm::groupPointsToCluster()
{
	// erase all the previous points from the clusters, before adding the new points
#pragma omp parallel for
	for (int i = 0; i < numberOfClusters; i++)
		clusters[i].deleteAllPointsFromCluster();

	//after calculation of the distances for each point, add the points to the correct clusters
	for (int i = 0; i < numberOfPoints; i++)
		points[i].getContainingCluster()->addPoint(points + i);
}

//double ParallelKmeanAlgorithm::calcQualityMessure()
//{
//	double currentQM = 0;
//	Point* firstClusterCenterPoint, *secondClusterCenterPoint;
//	double firstClusterDiameter;
//	double distanceBetweenClustersCenterPoints;
//
//	for (int j = 0; j < this->numberOfClusters; j++)
//	{
//		firstClusterDiameter = this->clusters[j].culculateDiameter();
//		firstClusterCenterPoint = this->clusters[j].getClusterCenterPoint();
//
//		for (int k = 0; k < this->numberOfClusters; k++)
//		{
//			if (k != j)
//			{
//				secondClusterCenterPoint = this->clusters[k].getClusterCenterPoint();
//				distanceBetweenClustersCenterPoints = (firstClusterCenterPoint->calculateDistanceFromPoints(secondClusterCenterPoint));
//				currentQM += (firstClusterDiameter / distanceBetweenClustersCenterPoints);
//			}
//
//		}
//	}
//
//	currentQM = currentQM / (numberOfClusters * (numberOfClusters - 1));
//	return currentQM;
//}


void ParallelKmeanAlgorithm::runKmeansParallel_Master(int numOfProcs)
{
	int c, j;
	double* newClusterCenters = new double[numberOfClusters * POINT_DIMENSION];
	int* localClusterNumOfPoints = new int[numberOfClusters];
	int* totalClusterNumOfPoints = new int[numberOfClusters];
	int clusterChangedFlag = 0;		//1 - cluster changed, 0 cluster didn't changed
	int countClusterChanges = 0;	//count how many procs. haven't changed the containing clusters

	calcNewDistancesForPoints();	//assign the containing clusters to points in the beginning (good for case that the points don't have containing cluster yet)

	//cout << "in KMEANS - MASTER, number of points: " << numberOfPoints<<"\tnumber of clusters: " << numberOfClusters << endl;
	//fflush(stdout);

	for (int i = 0; i < limit && (countClusterChanges != numOfProcs); i++)
	{
		cout << "in KMEANS - MASTER, iteration #: " << i << endl;
		fflush(stdout);

		clusterChangedFlag = 0;
		countClusterChanges = 0;

		//cout << "MASTER going to: sequentialPointsToCluster \n" << endl;
		//fflush(stdout);
		groupPointsToCluster();
		//cout << "MASTER going to: calcSumOfPointVectors \n" << endl;
		//fflush(stdout);
		//step 3 - calculate the sum of point vectors
#pragma omp parallel for
		for ( c = 0; c < numberOfClusters; c++)
		{
			clusters[c].calcSumOfPointVectors(clustersCentersSum[(c*POINT_DIMENSION)], clustersCentersSum[(c*POINT_DIMENSION) + 1], clustersCentersSum[(c*POINT_DIMENSION) + 2]);
			localClusterNumOfPoints[c] = clusters[c].getNumOfPoints();
		}

		//cout << "MASTER reducing #1\n" << endl;
		//fflush(stdout);

		//collect from all the slaves and the master the sums of each cluster center, and calculate the new centers for ALL the clusters.
		//#Reduce 1
		MPI_Reduce(clustersCentersSum, newClusterCenters, numberOfClusters * POINT_DIMENSION, MPI_DOUBLE, MPI_SUM, MASTER, MPI_COMM_WORLD);
		MPI_Reduce(localClusterNumOfPoints, totalClusterNumOfPoints, numberOfClusters, MPI_INT, MPI_SUM, MASTER, MPI_COMM_WORLD);

		//divide each coordinate by the number of clusters, to get the new centers 
#pragma parallel omp for private(j,c)
		for (j = 0, c = 0; j < numberOfClusters * POINT_DIMENSION, c < numberOfClusters; j += POINT_DIMENSION, c++)
		{
			newClusterCenters[j] /= totalClusterNumOfPoints[c];		//X coordinate
			newClusterCenters[j + 1] /= totalClusterNumOfPoints[c];	//Y coordinate
			newClusterCenters[j + 2] /= totalClusterNumOfPoints[c];	//Z coordinate
		}

		/*cout << "MASTER broadcasting new centers #1\n" << endl;
		fflush(stdout);*/

		//send the new centers to all the slaves.	#bcast1
		MPI_Bcast(newClusterCenters, numberOfClusters * POINT_DIMENSION, MPI_DOUBLE, MASTER, MPI_COMM_WORLD);

		/*cout << "MASTER updates its own new centers #1\n" << endl;
		fflush(stdout);*/

		//update the new centers to clusters
#pragma omp parallel for
		for (j = 0; j < numberOfClusters; j++)
			clusters[j].updateCenterPointCordinates(newClusterCenters[(j * POINT_DIMENSION)], newClusterCenters[(j * POINT_DIMENSION) + 1], newClusterCenters[(j * POINT_DIMENSION) + 2]);
		

		calcNewDistancesForPoints();	//assign the containing clusters to points after the new centers were calculated

		if (!isContainingClusterChanged)
			clusterChangedFlag = 1;
		isContainingClusterChanged = false;

		//collect how many clusters reported that the containing clusters didn't changed.	#Reduce 2
		MPI_Reduce(&clusterChangedFlag, &countClusterChanges, 1, MPI_INT, MPI_SUM, MASTER, MPI_COMM_WORLD);

		/*cout << "MASTER reducing #2 - num of found: " <<countClusterChanges << "\n" << endl;
		fflush(stdout);*/

		int tmpTag;
		if (countClusterChanges == numOfProcs)
			tmpTag = STOP_KMEANS;
		else
			tmpTag = WORKING_TAG;
		//in that case there were no changes in the containing cluster - kmeans done, annonce to all the slaves.
#pragma omp parallel for
		for (int p = 1; p < numOfProcs; p++)
			//#Send 1
			MPI_Send(&countClusterChanges, 1, MPI_INT, p, tmpTag, MPI_COMM_WORLD);


	}

	groupPointsToCluster();	//assign the containing clusters to points vectors at the end of the 'limit' iterations (is needed for after the last iteration)
	
//	delete[]newClusterCenters;

//	return calcQualityMessure(); 
}

void ParallelKmeanAlgorithm::runKmeansParallel_Slave()
{
	MPI_Status status;
	status.MPI_TAG = WORKING_TAG;
	int c, j;

	localClusterNumOfPoints = new int[numberOfClusters];

	int clusterChangedFlag = 0;		//1 - cluster changed, 0 cluster didn't changed
	calcNewDistancesForPoints();	//assign the containing clusters to points in the beginning (good for case that the points don't have containing cluster yet)

	//cout << "in KMEANS - SLAVE, number of points: " << numberOfPoints << "\tnumber of clusters: " << numberOfClusters << endl;
	//fflush(stdout);

	//cout << "The last point: " << points[numberOfPoints - 1] << endl;
	/*cout << "The last cluster: " << clusters[numberOfClusters - 1] << endl;
	fflush(stdout);*/

	while(status.MPI_TAG == WORKING_TAG)
	{
		/*cout << "in KMEANS - SLAVE" << endl;
		fflush(stdout);*/

		clusterChangedFlag = 0;

		/*cout << "SLAVE going to: sequentialPointsToCluster \n" << endl;
		fflush(stdout);
*/
		groupPointsToCluster();

		/*cout << "SLAVE going to: calcSumOfPointVectors \n" << endl;
		fflush(stdout);*/

		//step 3 - calculate the sum of point vectors
#pragma omp parallel for
		for (c = 0; c < numberOfClusters; c++)
		{
			clusters[c].calcSumOfPointVectors(clustersCentersSum[(c*POINT_DIMENSION)], clustersCentersSum[(c*POINT_DIMENSION) + 1], clustersCentersSum[(c*POINT_DIMENSION) + 2]);
			localClusterNumOfPoints[c] = clusters[c].getNumOfPoints();
		}
		
		/*cout << "SLAVE reducing #1 \n" << endl;
		fflush(stdout);*/

		//send to master the sums of the point vectors for each cluster
		//#Reduce 1
		MPI_Reduce(clustersCentersSum, clustersCentersSum, numberOfClusters * POINT_DIMENSION, MPI_DOUBLE, MPI_SUM, MASTER, MPI_COMM_WORLD);
		MPI_Reduce(localClusterNumOfPoints, localClusterNumOfPoints, numberOfClusters, MPI_INT, MPI_SUM, MASTER, MPI_COMM_WORLD);

		/*cout << "SLAVE get bcast #1 \n" << endl;
		fflush(stdout);*/

		//receive from the master the new centers of clusters.	#bcast 1
		MPI_Bcast(clustersCentersSum, numberOfClusters * POINT_DIMENSION, MPI_DOUBLE, MASTER, MPI_COMM_WORLD);

		/*cout << "SLAVE going to: updateCenterPointCordinates \n" << endl;
		fflush(stdout);*/

		//update the new centers to clusters
#pragma omp parallel for
		for (j = 0; j < numberOfClusters; j++)
			clusters[j].updateCenterPointCordinates(clustersCentersSum[(j * POINT_DIMENSION)], clustersCentersSum[(j * POINT_DIMENSION) + 1], clustersCentersSum[(j * POINT_DIMENSION) + 2]);

		/*cout << "The last cluster: " << clusters[numberOfClusters - 1] << endl;
		fflush(stdout);*/

		calcNewDistancesForPoints();	//assign the containing clusters to points after the new centers were calculated

		if (!isContainingClusterChanged)
			clusterChangedFlag = 1;
		isContainingClusterChanged = false;

		/*cout << "SLAVE reducing #2 \n" << endl;
		fflush(stdout);*/

		//#Reduce 2
		MPI_Reduce(&clusterChangedFlag, &clusterChangedFlag, 1, MPI_INT, MPI_SUM, MASTER, MPI_COMM_WORLD);
		/*cout << "SLAVE receiving #1 \n" << endl;
		fflush(stdout);*/

		//#Recv 1
		MPI_Recv(&clusterChangedFlag, 1, MPI_INT, MASTER, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

		/*cout << "SLAVE recevied status: " << status.MPI_TAG << " and clusterChangedFlag: " << clusterChangedFlag << endl;
		fflush(stdout);*/
	}
	
	groupPointsToCluster();	//assign the containing clusters to points vectors at the end of the 'limit' iterations (is needed for after the last iteration)
	
}
