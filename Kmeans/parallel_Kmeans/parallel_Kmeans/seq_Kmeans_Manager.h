#ifndef  __SEQKMEANSMANAGER_H
#define __SEQKMEANSMANAGER_H
#include "Cluster.h"
#include "sequencialAlgorithm.h"
#include <fstream>
#include <stdio.h>
#include <math.h>

using namespace std;

class Seq_Kmeans_Manager
{

private :
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

	/********************************************************************************/
	/*									SEQUENCIAL									*/
	/********************************************************************************/

	//calculating distance between to points 
	//double calcDistanceBetweenTwoPoints(Point * p1, Point * p2);

	//adding the new center points and containing clusters to the points
	void calcNewDistancesForPoints();
	//sequential claculation and attaching points to clusters
	void sequentialPointsToCluster();

	/*********************************************************************************/

public:
	Seq_Kmeans_Manager(string inputFileName);
	~Seq_Kmeans_Manager();


	/*
	 *reading the whole file, and setting the parameters, also the points and initiallize the clusters, the function can get
	 *nothing, then it will use the input file that was passed in the c'tor.
	 */
	void readInputFromFile(string inputFileName = "");

	
	//this method goes through all the points and calculate their new position, according to the time
	 void calcPointsNewPosition(double time);

	 double calcQualityMessure();

	 bool runSequencialAlgorithm();

	 friend ostream& operator<< (ostream& out, const Seq_Kmeans_Manager& kmm);

	





};









#endif // ! __SEQKMEANSMANAGER_H
