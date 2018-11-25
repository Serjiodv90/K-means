
#include "kernel.h"

//better to access from a single process to avoid unknown behavior. so access only from master

cudaError_t calculateNewPointPositionViaTime_Cuda(Point::PointAsStruct* pointsArr, int numberOfPoints, double currentTime);

__global__ void calcNewPosition(Point::PointAsStruct* pointsArr, int numberOfPoints, double currentTime)
{
	int threadId = threadIdx.x;
	int blockId = blockIdx.x;
	int index = threadId + blockId * MAX_THREADS_FOR_CUDA;
	if (index < numberOfPoints)
	{
		pointsArr[index].current_x = pointsArr[index].X0 + (currentTime * pointsArr[index].velocity_x);
		pointsArr[index].current_y = pointsArr[index].Y0 + (currentTime * pointsArr[index].velocity_y);
		pointsArr[index].current_z = pointsArr[index].Z0 + (currentTime * pointsArr[index].velocity_z);
	}

}


// Helper function for using CUDA to add vectors in parallel.
cudaError_t calculateNewPointPositionViaTime_Cuda(Point::PointAsStruct* pointsArr, int numberOfPoints, double currentTime)
{

	int numOfBlocksForCuda;
	Point::PointAsStruct* pointsArr_device;
    cudaError_t cudaStatus;

	numOfBlocksForCuda = 1 + ((numberOfPoints - 1) / MAX_THREADS_FOR_CUDA);

    // Choose which GPU to run on, change this on a multi-GPU system.
    cudaStatus = cudaSetDevice(0);
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "cudaSetDevice failed!  Do you have a CUDA-capable GPU installed?");
		cudaFree(pointsArr_device);
    }

    // Allocate GPU buffer for array of Points.
    cudaStatus = cudaMalloc((void**)&pointsArr_device, numberOfPoints * sizeof(Point::PointAsStruct));
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "cudaMalloc failed!");
		cudaFree(pointsArr_device);
    }

    // Copy array of Points from host memory to GPU buffers.
    cudaStatus = cudaMemcpy(pointsArr_device, pointsArr, numberOfPoints * sizeof(Point::PointAsStruct), cudaMemcpyHostToDevice);
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "cudaMemcpy failed!");
		cudaFree(pointsArr_device);
    }

    // Launch a kernel on the GPU with one thread for each element.
    calcNewPosition<<<numOfBlocksForCuda, MAX_THREADS_FOR_CUDA>>>(pointsArr_device, numberOfPoints, currentTime);

    // Check for any errors launching the kernel
    cudaStatus = cudaGetLastError();
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "addKernel launch failed: %s\n", cudaGetErrorString(cudaStatus));
		cudaFree(pointsArr_device);
    }
    
    // cudaDeviceSynchronize waits for the kernel to finish, and returns
    // any errors encountered during the launch.
    cudaStatus = cudaDeviceSynchronize();
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "cudaDeviceSynchronize returned error code %d after launching addKernel!\n", cudaStatus);
		cudaFree(pointsArr_device);
    }

    // Copy output vector from GPU buffer to host memory.
    cudaStatus = cudaMemcpy(pointsArr, pointsArr_device, numberOfPoints * sizeof(Point::PointAsStruct), cudaMemcpyDeviceToHost);
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "cudaMemcpy failed!");
		cudaFree(pointsArr_device);
    }
    
    return cudaStatus;
}

