#include "parallel_Manager.h"


Parallel_Manager::Parallel_Manager(string inputFileName, int procId) : successQM(DBL_MAX), successTime(DBL_MAX)
{
	this->inputFileName = inputFileName;
	this->myId = procId;
	readInputFromFile();
}

Parallel_Manager::~Parallel_Manager()
{
	if (this->points)
		delete[]this->points;
	if (this->clusters)
		delete[]this->clusters;
	if(this->pointsForProc)
		delete[]this->pointsForProc;
	if (this->pointsToHandle)
		delete[]this->pointsToHandle;
	if (this->clustersForProc)
		delete[]this->clustersForProc;
	if (this->clusterToHandle)
		delete[]this->clusterToHandle;
}

void Parallel_Manager::readInputFromFile(string inputFileName)
{
	if (!inputFileName.empty())
		this->inputFileName = inputFileName;

	ifstream inputFile(this->inputFileName);
	if (inputFile.is_open())
	{
		initKmeansParamsFromFile(inputFile);
		initPointsFromFile(inputFile);
		inputFile.close();
	}
	else {
		cout << "Couldn't open the input file! ABORT" << endl;
		MPI_Abort(MPI_COMM_WORLD, 1);
	}

	initClustersFirstTime();
}


void Parallel_Manager::initKmeansParamsFromFile(ifstream& inputFile)
{
	inputFile >> this->numberOfPoints >> this->numberOfClusters >> this->totalRunTime
		>> this->velocityCalcTimeInterval >> this->maxIterations >> this->qualityMessure;
}

void Parallel_Manager::initPointsFromFile(ifstream& inputFile)
{
	this->points = new Point[this->numberOfPoints];
	if (!this->points)
	{
		cout << "Couldn't allocate points array in kmeansManager.cpp" << endl;
		return;
	}

	double pointXPos;
	double pointYPos;
	double pointZPos;
	double pointXVelocity;
	double pointYVelocity;
	double pointZVelocity;
	int pointsCounter = 0;

	while (inputFile >> pointXPos >> pointYPos >> pointZPos >> pointXVelocity >> pointYVelocity >> pointZVelocity)
	{
		this->points[pointsCounter].setStartPosition(Point::Position{ pointXPos, pointYPos, pointZPos });
		this->points[pointsCounter].setVelocity(Point::Velocity{ pointXVelocity, pointYVelocity, pointZVelocity });
		pointsCounter++;
	}

}

void Parallel_Manager::initClustersFirstTime()
{

	this->clusters = new Cluster[this->numberOfClusters];
	//check allocation
	if (!this->clusters)
	{
		cout << "Couldn't allocate clusters array in kmeansManager.cpp" << endl;
		return;
	}

	//insert OMP
	for (int i = 0; i < numberOfClusters; i++)
	{
		this->clusters[i].setCenterPoint(this->points + i);
		this->clusters[i].addPoint(this->points + i);

		this->points[i].addContainingCluster(this->clusters + i);
		this->points[i].setContainingClusterIndex(i);
	}

}

/**************************************************************************************************************/
/*											  END OF INIT FROM FILE											  */
/**************************************************************************************************************/


void Parallel_Manager::createPointsMPIDataType()
{

	Point::PointAsStruct point;
	MPI_Datatype pointStructTypes[NUM_OF_ELEMENTS_IN_POINT_STRUCT] =
	{
		MPI_DOUBLE, MPI_DOUBLE, MPI_DOUBLE, MPI_DOUBLE, MPI_DOUBLE, MPI_DOUBLE, MPI_DOUBLE, MPI_DOUBLE, MPI_DOUBLE, MPI_INT
	};

	int pointBlockLength[NUM_OF_ELEMENTS_IN_POINT_STRUCT] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };
	MPI_Aint pointDisplacement[NUM_OF_ELEMENTS_IN_POINT_STRUCT];
	pointDisplacement[0] = (char*)&point.X0 - (char*)&point;
	pointDisplacement[1] = (char*)&point.Y0 - (char*)&point;
	pointDisplacement[2] = (char*)&point.Z0 - (char*)&point;
	pointDisplacement[3] = (char*)&point.current_x - (char*)&point;
	pointDisplacement[4] = (char*)&point.current_y - (char*)&point;
	pointDisplacement[5] = (char*)&point.current_z - (char*)&point;
	pointDisplacement[6] = (char*)&point.velocity_x - (char*)&point;
	pointDisplacement[7] = (char*)&point.velocity_y - (char*)&point;
	pointDisplacement[8] = (char*)&point.velocity_z - (char*)&point;
	pointDisplacement[9] = (char*)&point.containingClusterIndex - (char*)&point;

	MPI_Type_create_struct(NUM_OF_ELEMENTS_IN_POINT_STRUCT, pointBlockLength, pointDisplacement, pointStructTypes, &this->pointStructMPIType);
	MPI_Type_commit(&this->pointStructMPIType);
}


void Parallel_Manager::createClustersMPIDataType()
{

	Cluster::ClusterAsStruct cluster;
	MPI_Datatype clusterStructTypes[NUM_OF_ELEMENTS_IN_CLUSTER_STRUCT] =
				{ MPI_DOUBLE, MPI_DOUBLE, MPI_DOUBLE, MPI_DOUBLE };

	int clusterBlockLength[NUM_OF_ELEMENTS_IN_CLUSTER_STRUCT] = { 1, 1, 1, 1 };
	MPI_Aint clusterDisplacement[NUM_OF_ELEMENTS_IN_CLUSTER_STRUCT];
	clusterDisplacement[0] = (char*)&cluster.center_x - (char*)&cluster;
	clusterDisplacement[1] = (char*)&cluster.center_y - (char*)&cluster;
	clusterDisplacement[2] = (char*)&cluster.center_z - (char*)&cluster;
	clusterDisplacement[3] = (char*)&cluster.diameter - (char*)&cluster;
	
	MPI_Type_create_struct(NUM_OF_ELEMENTS_IN_CLUSTER_STRUCT, clusterBlockLength, clusterDisplacement, clusterStructTypes, &this->clusterStructMPIType);
	MPI_Type_commit(&this->clusterStructMPIType);
}

void Parallel_Manager::createPointArrayOfStructs()
{
	this->pointsForProc = new Point::PointAsStruct[this->numberOfPoints];
	for (int i = 0; i < this->numberOfPoints; i++)
		this->pointsForProc[i] = this->points[i].getPointAsStruct();

}

void Parallel_Manager::createClusterArrayOfStructs()
{
	this->clustersForProc = new Cluster::ClusterAsStruct[this->numberOfClusters];
	for (int i = 0; i < this->numberOfClusters; i++)
		this->clustersForProc[i] = this->clusters[i].getClusterAsStruct();

}

//
//void Parallel_Manager::createArrayOfStructs()
//{
//	this->pointsForProc = new Point::PointAsStruct[this->numberOfPoints];
//	this->clustersForProc = new Cluster::ClusterAsStruct[this->numberOfClusters];
//	int i;
//
//	for (i = 0; i < this->numberOfPoints; i++)
//		this->pointsForProc[i] = this->points[i].getPointAsStruct();
//
//	for (i = 0; i < this->numberOfClusters; i++)
//		this->clustersForProc[i] = this->clusters[i].getClusterAsStruct();
//}

void Parallel_Manager::scatterPointsToSlavesFirstTime()
{
	//if the number of points divided by number of processes without a reminder, than spread the array of point structs even for slaves.
	//otherwise master proc. will handle numOfPointsForProc + reminder, points.
	int startIndexForBroadcast = (this->numberOfPoints % this->numOfProcesses) == 0 ? 0 : (this->numberOfPoints % this->numOfProcesses);

	this->pointsToHandle = new Point::PointAsStruct[numOfPointsForProc + startIndexForBroadcast];

	//copy the points for master to handle, according to the reminder of (numberOfPoints % numOfProcs)
	for (int i = 0; i < startIndexForBroadcast; i++)
		pointsToHandle[i] = pointsForProc[i];

	//scatter points to all processes.	#scatter1
	MPI_Scatter(this->pointsForProc + startIndexForBroadcast, this->numOfPointsForProc, this->pointStructMPIType,
		this->pointsToHandle, this->numOfPointsForProc, this->pointStructMPIType, MASTER, MPI_COMM_WORLD);

	//if there is a reminder, move each element in the array: pointsToHandle, by reminder value's indeces forward
	if (startIndexForBroadcast != 0)
	{
		for (int i = numOfPointsForProc - 1; i >= 0; i--)
			this->pointsToHandle[i + startIndexForBroadcast] = this->pointsToHandle[i];

		//copy the points from the original point array to the new one
		for (int i = 0; i < startIndexForBroadcast; i++)
			this->pointsToHandle[i] = this->pointsForProc[i];
	}
}

void Parallel_Manager::master_broadcastToSlavesFirstTime(int numOfProcs, int myId)
{
	if (myId == MASTER && numOfProcs > 1)
	{
		this->myId = myId;
		this->numOfProcesses = numOfProcs;
		createPointArrayOfStructs();
		createClusterArrayOfStructs();
		//createArrayOfStructs();
		this->numOfPointsForProc = this->numberOfPoints / numOfProcs;
		
		////broadcast to all of the slaves the number of points they'll get from the master.		#bcast1
		//MPI_Bcast(&this->numOfPointsForProc, 1, MPI_INT, MASTER, MPI_COMM_WORLD);

		////broadcast to all of the slaves the number of clusters.	#bcast2
		//MPI_Bcast(&this->numberOfClusters, 1, MPI_INT, MASTER, MPI_COMM_WORLD);

		////broadcast to all of the slaves the same cluster struct array.		#bcast3
		//MPI_Bcast(this->clustersForProc, this->numberOfClusters, this->clusterStructMPIType, MASTER, MPI_COMM_WORLD);

		for (int p = 1; p < numOfProcesses; p++)
		{
			MPI_Send(&this->numOfPointsForProc, 1, MPI_INT, p, INIT_INFO_TAG, MPI_COMM_WORLD);
			MPI_Send(&this->numberOfClusters, 1, MPI_INT, p, INIT_INFO_TAG, MPI_COMM_WORLD);
			MPI_Send(this->clustersForProc, this->numberOfClusters, this->clusterStructMPIType, p, INIT_INFO_TAG, MPI_COMM_WORLD);
		}

		scatterPointsToSlavesFirstTime();

		/*from this point the master will work on the original clusters array and points array*/
	}

}

void Parallel_Manager::createPointArrayFromStruct()
{
	this->points = new Point[this->numOfPointsForProc];

	//parallel with OMP!!!!!!!
	for (int i = 0; i < this->numOfPointsForProc; i++)
	{
		this->points[i].setStartPosition(Point::Position{pointsToHandle[i].X0, pointsToHandle[i].Y0, pointsToHandle[i].Z0 });
		this->points[i].setPosition(Point::Position{ pointsToHandle[i].current_x, pointsToHandle[i].current_y, pointsToHandle[i].current_z });
		this->points[i].setVelocity(Point::Velocity{ pointsToHandle[i].velocity_x, pointsToHandle[i].velocity_y, pointsToHandle[i].velocity_z });
	}
}

void Parallel_Manager::createClusterArrayFromStruct()
{
	this->clusters = new Cluster[this->numberOfClusters];

	//parallel with OMP
	for (int i = 0; i < this->numberOfClusters; i++)
	{
		this->clusters[i].setCenterPoint(new Point(Point::Position{ clustersForProc[i].center_x, clustersForProc[i].center_y, clustersForProc[i].center_z }));
		this->clusters[i].setDiameter(clustersForProc[i].diameter);
	}
}

void Parallel_Manager::slaves_recieveStartInformation(int myId)
{
	this->myId = myId;
	MPI_Status status;

	//recieve first the number of points for each proc, and create array of point structs.	#bcast1
	//MPI_Bcast(&this->numOfPointsForProc, 1, MPI_INT, MASTER, MPI_COMM_WORLD);
	MPI_Recv(&this->numOfPointsForProc, 1, MPI_INT, MASTER, INIT_INFO_TAG, MPI_COMM_WORLD, &status);
	this->pointsToHandle = new Point::PointAsStruct[this->numOfPointsForProc];

	//recieve the number of clusters , and create array of cluster structs.		#bcast2
	//MPI_Bcast(&this->numberOfClusters, 1, MPI_INT, MASTER, MPI_COMM_WORLD);
	MPI_Recv(&this->numberOfClusters, 1, MPI_INT, MASTER, INIT_INFO_TAG, MPI_COMM_WORLD, &status);
	this->clustersForProc = new Cluster::ClusterAsStruct[this->numberOfClusters];

	//recieve the clusters.		#bcast3
	//MPI_Bcast(this->clustersForProc, this->numberOfClusters, this->clusterStructMPIType, MASTER, MPI_COMM_WORLD);
	MPI_Recv(this->clustersForProc, this->numberOfClusters, this->clusterStructMPIType, MASTER, INIT_INFO_TAG, MPI_COMM_WORLD, &status);

	//#scatter 1- get the points for each slave
	int startIndexForBroadcast = 0;	//irrelevant index for the slaves
	MPI_Scatter(this->pointsForProc + startIndexForBroadcast, this->numOfPointsForProc, this->pointStructMPIType,
		this->pointsToHandle, this->numOfPointsForProc, this->pointStructMPIType, MASTER, MPI_COMM_WORLD);

	//create for each slave the array of points and clusters (objects)
	createPointArrayFromStruct();
	createClusterArrayFromStruct();
	
}

void Parallel_Manager::collectUpdatedPointsFromSlaves()
{
	MPI_Status status;
	int masterStartIndex = (numberOfPoints % numOfProcesses);
	for (int i = 1; i < numOfProcesses; i++)
		MPI_Recv(pointsForProc + masterStartIndex + (numOfPointsForProc * i), numOfPointsForProc, pointStructMPIType,
			i, COMPUTE_QM_TAG, MPI_COMM_WORLD, &status);
	
	for (int i = (masterStartIndex + numOfPointsForProc); i < this->numberOfPoints; i++)
	{
		this->points[i].setPosition(Point::Position{ pointsForProc[i].current_x, pointsForProc[i].current_y, pointsForProc[i].current_z });
		this->points[i].addContainingCluster(&this->clusters[pointsForProc[i].containingClusterIndex]);
	}

}

double Parallel_Manager::calcQualityMessure()
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


void Parallel_Manager::calcPointsNewPosition(double time)
{
	if (time > 0.0)
	{
		int numOfPointToIterate = this->myId == MASTER ? (numOfPointsForProc + (this->numberOfPoints % numOfProcesses)) : this->numOfPointsForProc;
		for (int i = 0; i < numOfPointToIterate; i++)
			points[i].calcNewPositionViaTime(time);
	}

}


bool Parallel_Manager::runParallelAlgorithm_Master()
{
	double currentQM = DBL_MAX;
	double currentTime = 0.0;
	int masterNumOfPoints = numOfPointsForProc + (numberOfPoints % numOfProcesses);
	ParallelKmeanAlgorithm* parKmeansAlgo = new ParallelKmeanAlgorithm(masterNumOfPoints, numberOfClusters, points, clusters, maxIterations);


	for (int i = 0; (currentTime < totalRunTime) && (currentQM >= qualityMessure); i++)
	{
		currentTime = i * velocityCalcTimeInterval;

		/*cout << "in main loop, current time: " << currentTime << "\tQM: " << currentQM << endl;
		fflush(stdout);*/

		//send all the slaves the time to update point location. use send and not broadcast for tags.	#send 1
		for (int p = 1; p < this->numOfProcesses; p++)
			MPI_Send(&currentTime, 1, MPI_DOUBLE, p, WORKING_TAG, MPI_COMM_WORLD);

		calcPointsNewPosition(currentTime);

		//step 2 - 4
	/*	currentQM =*/ parKmeansAlgo->runKmeansParallel_Master(numOfProcesses);
	//	updateStructPointPosition();

		collectUpdatedPointsFromSlaves();
		currentQM = calcQualityMessure();

		//print resultsof each iteration for checking
		cout << "\nt = " << currentTime << "\tQM = " << currentQM << "\tDesired QM = " << qualityMessure << "\n" << endl;

		this->successQM = currentQM;
		this->successTime = currentTime;
	}

	//finished the running, announce to all slave about termination
	for (int p = 1; p < numOfProcesses; p++)
		MPI_Send(&currentTime, 1, MPI_DOUBLE, p, TERMINATION_TAG, MPI_COMM_WORLD);

	cout << "\nTime of termination: " << successTime << "\tQM: " << successQM << "\n" << endl;
	fflush(stdout);

	if (currentQM < this->qualityMessure)
		return true;	
	else	// no clusters combination to obtain QM less then desired
		return false;
}

void Parallel_Manager::updateStructPointPosition()
{
	int numOfPointToIterate = this->myId == MASTER ? (numOfPointsForProc + (this->numberOfPoints % numOfProcesses)) : this->numOfPointsForProc;
	
	for (int i = 0; i < numOfPointToIterate; i++)
	{
		Point::Position p = points[i].getPointPosition();
		this->pointsToHandle[i].current_x = p.x;
		this->pointsToHandle[i].current_y = p.y;
		this->pointsToHandle[i].current_z = p.z;
	}
}

void Parallel_Manager::runParallelAlgorithm_Slave()
{
	double currentTime;
	MPI_Status status;
	ParallelKmeanAlgorithm* parKmeansAlgo = new ParallelKmeanAlgorithm(numOfPointsForProc, numberOfClusters, points, clusters, maxIterations);


	//recieve the time for calculation for the first time.	#recv1
	MPI_Recv(&currentTime, 1, MPI_DOUBLE, MASTER, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
	
	while (status.MPI_TAG == WORKING_TAG)
	{
		calcPointsNewPosition(currentTime);
		parKmeansAlgo->runKmeansParallel_Slave();


		//when finished the kmeans, send to master the points of the slave for QM calculation

		/*cout << "SLAVE #" << myId << " is updating points\n" << endl;
		fflush(stdout);*/

		updateStructPointPosition();

		/*cout << "SLAVE #" << myId << " is sending points to master\n" << endl;
		fflush(stdout);*/

		MPI_Send(this->pointsToHandle, this->numOfPointsForProc, pointStructMPIType, MASTER, COMPUTE_QM_TAG, MPI_COMM_WORLD);

		//#Recv 1
		MPI_Recv(&currentTime, 1, MPI_DOUBLE, MASTER, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

	}



}



ostream & operator<<(ostream & out, const Parallel_Manager & man)
{
	out.precision(6);	//6 digit after decimal point, in 'double' values
	out << "First occurance: t = " << man.successTime << "\twith q = " << man.successQM << endl;
	out << "Centers of clusters: " << endl;
	for (int i = 0; i < man.numberOfClusters; i++)
		out << man.clusters[i];

	return out;

}
