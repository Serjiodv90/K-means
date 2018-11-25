#pragma once
#ifndef __KERNEL_H
#define __KERNEL_H
#include "parallel_Manager.h"
#include "cuda_runtime.h"
#include "device_launch_parameters.h"
#include <stdio.h>

cudaError_t calculateNewPointPositionViaTime_Cuda(Point::PointAsStruct* pointsArr, int numberOfPoints, double currentTime);

__global__ void calcNewPosition(Point::PointAsStruct* pointsArr, int numberOfPoints, double currentTime);




#endif // !__KERNEL_H

