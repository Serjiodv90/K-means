#include "parallel_Manager.h"


parallel_Manager::parallel_Manager(string inputFileName) : successQM(DBL_MAX), successTime(DBL_MAX)
{
	this->inputFileName = inputFileName;
	readInputFromFile();
}

parallel_Manager::~parallel_Manager()
{
	if (this->points)
		delete[]this->points;
	if (this->clusters)
		delete[]this->clusters;
}

void parallel_Manager::readInputFromFile(string inputFileName)
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


void parallel_Manager::initKmeansParamsFromFile(ifstream& inputFile)
{
	inputFile >> this->numberOfPoints >> this->numberOfClusters >> this->totalRunTime
		>> this->velocityCalcTimeInterval >> this->maxIterations >> this->qualityMessure;
}

void parallel_Manager::initPointsFromFile(ifstream& inputFile)
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

void parallel_Manager::initClustersFirstTime()
{

	this->clusters = new Cluster[this->numberOfClusters];
	//vector<Cluster> vec(this->numberOfClusters);
	//check allocation
	if (!this->clusters)
	{
		cout << "Couldn't allocate clusters array in kmeansManager.cpp" << endl;
		return;
	}

	for (int i = 0; i < numberOfClusters; i++)
	{
		this->clusters[i].setCenterPoint(this->points + i);
		this->clusters[i].addPoint(this->points + i);

		this->points[i].addContainingCluster(this->clusters + i);
	}

}

/********************************************************************************/
/*									SEQUENCIAL									*/
/********************************************************************************/



void parallel_Manager::calcPointsNewPosition(double time)
{
	if (time > 0.0)
		for (int i = 0; i < this->numberOfPoints; i++)
			points[i].calcNewPositionViaTime(time);

}


bool parallel_Manager::runSequencialAlgorithm()
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


ostream & operator<<(ostream & out, const parallel_Manager & man)
{
	out.precision(6);	//6 digit after decimal point, in 'double' values
	out << "First occurance: t = " << man.successTime << "\twith q = " << man.successQM << endl;
	out << "Centers of clusters: " << endl;
	for (int i = 0; i < man.numberOfClusters; i++)
		out << man.clusters[i];

	return out;

}
