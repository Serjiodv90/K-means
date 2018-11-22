#include "parallel_Manager.h"


Parallel_Manager::Parallel_Manager(string inputFileName) : successQM(DBL_MAX), successTime(DBL_MAX)
{
	this->inputFileName = inputFileName;
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

void Parallel_Manager::createArrayOfStructs()
{
	this->pointsForProc = new Point::PointAsStruct[this->numberOfPoints];
	this->clustersForProc = new Cluster::ClusterAsStruct[this->numberOfClusters];
	int i;

	for (i = 0; i < this->numberOfPoints; i++)
		this->pointsForProc[i] = this->points[i].getPointAsStruct();

	for (i = 0; i < this->numberOfClusters; i++)
		this->clustersForProc[i] = this->clusters[i].getClusterAsStruct();
}

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

void Parallel_Manager::masterBroadcastToSlavesFirstTime(int numOfProcs, int myId)
{
	if (myId == MASTER && numOfProcs > 1)
	{
		this->numOfProcesses = numOfProcs;
		createArrayOfStructs();
		this->numOfPointsForProc = this->numberOfPoints / numOfProcs;
		
		//broadcast to all of the slaves the number of points they'll get from the master.		#bcast1
		MPI_Bcast(&this->numOfPointsForProc, 1, MPI_INT, MASTER, MPI_COMM_WORLD);

		//broadcast to all of the slaves the number of clusters.	#bcast2
		MPI_Bcast(&this->numberOfClusters, 1, MPI_INT, MASTER, MPI_COMM_WORLD);

		//broadcast to all of the slaves the same cluster struct array.		#bcast3
		MPI_Bcast(this->clustersForProc, this->numberOfClusters, this->clusterStructMPIType, MASTER, MPI_COMM_WORLD);

		scatterPointsToSlavesFirstTime();

		/*from this point the master will work on the */
	}

}

void Parallel_Manager::createPointArrayFromStruct()
{
	//parallel with OMP!!!!!!!
}

void Parallel_Manager::slavesRecieveStartInformation(int myId)
{
	//recieve first the number of points for each proc, and create array of point structs.	#bcast1
	MPI_Bcast(&this->numOfPointsForProc, 1, MPI_INT, MASTER, MPI_COMM_WORLD);
	this->pointsToHandle = new Point::PointAsStruct[this->numOfPointsForProc];

	//recieve the number of clusters , and create array of cluster structs.		#bcast2
	MPI_Bcast(&this->numberOfClusters, 1, MPI_INT, MASTER, MPI_COMM_WORLD);
	this->clustersForProc = new Cluster::ClusterAsStruct[this->numberOfClusters];

	//recieve the clusters.		#bcast3
	MPI_Bcast(this->clustersForProc, this->numberOfClusters, this->clusterStructMPIType, MASTER, MPI_COMM_WORLD);

	//#scatter 1- get the points for each slave
	int startIndexForBroadcast = 0;	//irrelevant index for the slaves
	MPI_Scatter(this->pointsForProc + startIndexForBroadcast, this->numOfPointsForProc, this->pointStructMPIType,
		this->pointsToHandle, this->numOfPointsForProc, this->pointStructMPIType, MASTER, MPI_COMM_WORLD);

	
	



}


void Parallel_Manager::calcPointsNewPosition(double time)
{
	if (time > 0.0)
		for (int i = 0; i < this->numberOfPoints; i++)
			points[i].calcNewPositionViaTime(time);

}


bool Parallel_Manager::runSequencialAlgorithm()
{
	double currentQM = DBL_MAX;
	double currentTime = 0.0;
	SequencialKmeans* sKmeansAlgo = new SequencialKmeans();


	for (int i = 0; (currentTime < totalRunTime) && (currentQM >= qualityMessure); i++)
	{
		currentTime = i * velocityCalcTimeInterval;
		calcPointsNewPosition(currentTime);

		//step 2 - 4
		currentQM = sKmeansAlgo->runKmeansSequencial(numberOfClusters, numberOfPoints, points, clusters, maxIterations);
		//print resultsof each iteration for checking
		cout << "t = " << currentTime << "\tQM = " << currentQM << "\tDesired QM = " << qualityMessure << endl;

		this->successQM = currentQM;
		this->successTime = currentTime;
	}


	if (currentQM < this->qualityMessure)
		return true;
	else	// no clusters combination to obtain QM less then desired
		return false;
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
