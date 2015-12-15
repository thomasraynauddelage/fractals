/*
  Collection of useful information I found online,
  based on http://www.caam.rice.edu/~timwar/HPC12/OpenCL/cl_stuff.c
  to find the available OpenCL devices and print their characteristics
  To compile on MAC OS:
  gcc -framework opencl -o info device_info.c
  To run:
  ./info
*/

#include <stdio.h>
#include <math.h>
#include <string.h>
#include <sys/time.h>
#include <OpenCL/opencl.h>


int main(int argc, char *argv[]){

	char name[128];

  int i, j;        //iterator variables for loops

  
  
  cl_platform_id platforms[32];     //an array to hold the IDs of all the platforms, hopefuly there won't be more than 32

  cl_uint num_platforms;        //this number will hold the number of platforms on this machine

  char vendor[1024];        //this strirng will hold a platforms vendor

  

  cl_device_id devices[32];     //this variable holds the number of devices for each platform, hopefully it won't be more than 32 per platform

  cl_uint num_devices;        //this number will hold the number of devices on this machine

  char deviceName[1024];        //this string will hold the devices name

  cl_uint numberOfCores;        //this variable holds the number of cores of on a device

  cl_long amountOfMemory;       //this variable holds the amount of memory on a device

  cl_uint clockFreq;        //this variable holds the clock frequency of a device

  cl_ulong maxAlocatableMem;      //this variable holds the maximum allocatable memory

  cl_ulong localMem;        //this variable holds local memory for a device

  cl_bool available;        //this variable holds if the device is available

  cl_ulong buf_ulong;

  size_t workitem_dims;

  size_t workitem_size[3];

  

  //get the number of platforms

  clGetPlatformIDs (32, platforms, &num_platforms);

  

  printf("\nNumber of platforms:\t%u\n\n", num_platforms);

  

  //this is a loop for platforms

  for(i = 0; i < num_platforms; i++)

  {

    printf("Platform:\t\t%u\n\n", i);

    clGetPlatformInfo (platforms[i], CL_PLATFORM_VENDOR, sizeof(vendor), vendor, NULL);

    printf("\tPlatform Vendor:\t%s\n", vendor);

    

    clGetDeviceIDs (platforms[i], CL_DEVICE_TYPE_ALL, sizeof(devices), devices, &num_devices);

    printf("\tNumber of devices:\t%u\n\n", num_devices);

    //this is a loop for devices

    for(j = 0; j < num_devices; j++)

    {

      //scan in device information

      clGetDeviceInfo(devices[j], CL_DEVICE_NAME, sizeof(deviceName), deviceName, NULL);

      clGetDeviceInfo(devices[j], CL_DEVICE_VENDOR, sizeof(vendor), vendor, NULL);

      clGetDeviceInfo(devices[j], CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(numberOfCores), &numberOfCores, NULL);

      clGetDeviceInfo(devices[j], CL_DEVICE_GLOBAL_MEM_SIZE, sizeof(amountOfMemory), &amountOfMemory, NULL);

      clGetDeviceInfo(devices[j], CL_DEVICE_MAX_CLOCK_FREQUENCY, sizeof(clockFreq), &clockFreq, NULL);

      clGetDeviceInfo(devices[j], CL_DEVICE_MAX_MEM_ALLOC_SIZE, sizeof(maxAlocatableMem), &maxAlocatableMem, NULL);

      clGetDeviceInfo(devices[j], CL_DEVICE_LOCAL_MEM_SIZE, sizeof(localMem), &localMem, NULL);

      clGetDeviceInfo(devices[j], CL_DEVICE_AVAILABLE, sizeof(available), &available, NULL);

      clGetDeviceInfo(devices[j], CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(buf_ulong), &buf_ulong, NULL);

      clGetDeviceInfo(devices[i], CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS, sizeof(workitem_dims), &workitem_dims, NULL);

      clGetDeviceInfo(devices[i], CL_DEVICE_MAX_WORK_ITEM_SIZES, sizeof(workitem_size), &workitem_size, NULL);



      //print out device information

      printf("\tDevice: %u\n", j);

      printf("\t\tName:\t\t\t\t%s\n", deviceName);

      printf("\t\tVendor:\t\t\t\t%s\n", vendor);

      printf("\t\tAvailable:\t\t\t%s\n", available ? "Yes" : "No");

      printf("\t\tCompute Units:\t\t\t%u\n", numberOfCores);

      printf("\t\tClock Frequency:\t\t%u mHz\n", clockFreq);

      printf("\t\tGlobal Memory:\t\t\t%0.00f mb\n", (double)amountOfMemory/1048576);

      printf("\t\tMax Allocatable Memory:\t\t%0.00f mb\n", (double)maxAlocatableMem/1048576);

      printf("\t\tLocal Memory:\t\t\t%u kb\n", (unsigned int)localMem);

      printf("\t\tdevice max work group size : \t%llu\n", (unsigned long long)buf_ulong);

      printf("\t\tdevice max work item dimensions:%u\n", (unsigned int) workitem_dims);

      printf("\t\tdevice max work item sizes:\t%u / %u / %u \n\n", workitem_size[0], workitem_size[1], workitem_size[2]);


    }

  }

}