#include "Point.h"


Point::Point()
{
	this->setMinDistanceFromCenterToMax();
}

Point::Point(Position position, Velocity velocity)
{
	setStartPosition(position);
	setVelocity(velocity);
	this->setMinDistanceFromCenterToMax();
}

Point::Point(Position position)
	: Point(position, Velocity{ 0.0, 0.0, 0.0 })
{}

Point::Point(double posX, double posY, double posZ, double veloX, double veloY, double veloZ)
	: Point(Position{ posX, posY, posZ }, Velocity{ veloX, veloY, veloZ })
{}

void Point::setStartPosition(Position position)
{
	this->startingPointPosition = position;
	this->pointPosition = position;
}

void Point::setPosition(Position position)
{
	this->pointPosition = position;
}

void Point::setVelocity(Velocity velocity)
{
	this->pointVelocityposition = velocity;
}

void Point::setMinDistanceFromCenterPoint(double distance)
{
		this->minDistanceFromCenter = distance;
}

double Point::getMinDistanceFromCenterPoint() const
{
	return this->minDistanceFromCenter;
}

void Point::calcNewPositionViaTime(double nextIntervalTime)
{
	double nextXPos = this->startingPointPosition.x + (nextIntervalTime * this->pointVelocityposition.x);
	double nextYPos = this->startingPointPosition.y + (nextIntervalTime * this->pointVelocityposition.y);
	double nextZPos = this->startingPointPosition.z + (nextIntervalTime * this->pointVelocityposition.z);

	setPosition(Position{ nextXPos, nextYPos, nextZPos });
}

Point::Position Point::getPointPosition() const
{
	return this->pointPosition;
}

Point::Position Point::getStartingPoint()
{
	return this->startingPointPosition;
}

void Point::addContainingCluster(Cluster * cluster)
{
	if(this->containingCluster != cluster)
		this->containingCluster = cluster;
}

Cluster * Point::getContainingCluster() const
{
	return this->containingCluster;
}

double Point::calculateDistanceFromPoints(const Point * p)	const
{
	Point::Position p1Pos = this->getPointPosition();
	Point::Position p2Pos = p->getPointPosition();
	double xDist = pow(p1Pos.x - p2Pos.x, 2);
	double yDist = pow(p1Pos.y - p2Pos.y, 2);
	double zDist = pow(p1Pos.z - p2Pos.z, 2);
	return sqrt(xDist + yDist + zDist);
}

void Point::setContainingClusterIndex(int index)
{
	this->containingClusterIndex = index;
}

int Point::getNumOfElementsInPointStruct() const
{
	return NUM_OF_ELEMENTS_IN_POINT_STRUCT;
}

Point::PointAsStruct Point::getPointAsStruct() const
{
	return PointAsStruct{startingPointPosition.x, startingPointPosition.y, startingPointPosition.z,
						 pointPosition.x, pointPosition .y, pointPosition.z,
						 pointVelocityposition.x, pointVelocityposition.y, pointVelocityposition.z,
						 containingClusterIndex};
}

Point Point::getPointFromStruct(Point::PointAsStruct& point)
{
	return Point(Point::Position{ point.current_x, point.current_y, point.current_z },
				 Point::Velocity{ point.velocity_x, point.velocity_y, point.velocity_z });
}

void Point::setMinDistanceFromCenterToMax()
{
	this->minDistanceFromCenter = DBL_MAX;
}

ostream & operator<<(ostream & out, const Point & point)
{
	int columnWidth = 15;
	out.precision(10);
	out << setw(columnWidth) << point.pointPosition.x << setw(columnWidth) << point.pointPosition.y
		<< setw(columnWidth) << point.pointPosition.z << endl;
	return out;

}
