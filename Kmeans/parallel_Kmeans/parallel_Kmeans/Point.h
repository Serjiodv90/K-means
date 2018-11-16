#ifndef  __POINT_H
#define __POINT_H

#include <iomanip> 
#include <iostream>
#include <stdio.h>

using namespace std;

class Cluster;

class Point
{

public:
	struct Position
	{
		double x;
		double y;
		double z;
	};

	struct Velocity
	{
		double x;
		double y;
		double z;
	};

	Point();
	Point(Position position, Velocity velocity);
	// constructor for cluster center point, without velocity
	Point(Position position);
	Point(double posX, double posY, double posZ, double veloX, double veloY, double veloZ);
	//~Point();

	void setStartPosition(Position position);
	void setPosition(Position position);
	void setVelocity(Velocity velocity);
	void setMinDistanceFromCenterPoint(double distance);
	double getMinDistanceFromCenterPoint()	const;

	//calculate the position of the point in the next time interval from the current time, and update the current point position
	void calcNewPositionViaTime(double nextIntervalTime);

	//return the x,y,z coordinates of the point, as Point::Position structure
	Position getPointPosition()		const;

	void addContainingCluster(Cluster* cluster);
	Cluster* getContainingCluster()	const;

	//calculate the distance between this point, and point p as input
	double calculateDistanceFromPoints(const Point* p)	const;

	friend ostream& operator<< (ostream& out, const Point& point);
	

private:

	Position pointPosition = {};
	Velocity pointVelocityposition = {};
	Position startingPointPosition = {};

	Cluster* containingCluster = nullptr;
	double minDistanceFromCenter;

	//initial the center point with a big double
	void setMinDistanceFromCenterToMax();

	////point ;
	//double xPosition;
	//double yPosition;
	//double zPosition;
	////point velocity
	//double xVelocity;
	//double yVelocity;
	//double zVelocity;



};






#endif // ! __POINT_H