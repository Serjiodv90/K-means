#include "Cluster.h"

Cluster::Cluster(int numOfPoints, Point* centerPoint) 
{
	this->setCenterPoint(centerPoint);
	this->addPoint(this->centerPoint);	// add the first random center point to the cluster
}

void Cluster::addPoint(const Point* point)
{
	//check for duplication in the vector - if point already exists in the vector don't add it again
//	if (find(this->clusterPoints.begin(), this->clusterPoints.end(), point) == this->clusterPoints.end())
		this->clusterPoints.push_back(point);
}

void Cluster::removePointFromCluster(const Point * point)
{
	if (this->clusterPoints.size() > 0)
		this->clusterPoints.erase(remove(this->clusterPoints.begin(), this->clusterPoints.end(), point), this->clusterPoints.end());
}

void Cluster::deleteAllPointsFromCluster()
{
	this->clusterPoints.clear();
}

void Cluster::setCenterPoint(Point * centerPoint)
{
	if (centerPoint != nullptr)
		this->centerPoint = centerPoint;
}


void Cluster::culculateNewCenterPositionSequencial()
{
	double newCenterX = 0.0;
	double newCenterY = 0.0;
	double newCenterZ = 0.0;
	Point::Position point;

	if (this->clusterPoints.size() > 1)
	{
		for (int i = 0; i < clusterPoints.size(); i++)
		{
			point = clusterPoints.at(i)->getPointPosition();
			newCenterX += point.x;
			newCenterY += point.y;
			newCenterZ += point.z;
		}

		newCenterX /= clusterPoints.size();
		newCenterY /= clusterPoints.size();
		newCenterZ /= clusterPoints.size();
		this->centerPoint = new Point(Point::Position{ newCenterX, newCenterY, newCenterZ });

	}
	
	
}

Point * Cluster::getClusterCenterPoint() const
{
	return this->centerPoint;
}

double Cluster::culculateDiameter()
{
	double tmpDiameter = 0.0;
	if (this->clusterPoints.size() > 1)		//cluster contains at least its center point
		for (unsigned int i = 0; i < this->clusterPoints.size() - 2; i++)
		{
			for (unsigned int j = i + 1; j < this->clusterPoints.size() - 1; j++)
			{
				tmpDiameter = clusterPoints.at(i)->calculateDistanceFromPoints(clusterPoints.at(j));
				this->diameter = fmax(tmpDiameter, this->diameter);
			}
		}
	else	//if there is 1 or none points in the cluster there is no diameter, as the center point not a cluster's point (not one of the calculated point for the algorithm)
		this->diameter = 0.0;

	return this->diameter;
}

Cluster::ClusterAsStruct Cluster::getClusterAsStruct() const
{
	Point::Position center = this->centerPoint->getPointPosition();
	return ClusterAsStruct{center.x, center.y, center.z, diameter};
}

int Cluster::getNumOfElementsInClusterAsStruct() const
{
	return NUM_OF_ELEMENTS_IN_CLUSTER_STRUCT;
}

ostream & operator<<(ostream & out, const Cluster & cluster)
{
	out << *cluster.getClusterCenterPoint();
	return out;
}
