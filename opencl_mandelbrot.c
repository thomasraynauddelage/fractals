/* 
Original code for drawing the Mandelbrot set  found at:
http://rosettacode.org/wiki/Mandelbrot_set#PPM_Interactive
OpenCL example from
https://developer.apple.com/library/mac/documentation/Performance/Conceptual/OpenCL_MacProgGuide/ExampleHelloWorld/Example_HelloWorld.html 

http://www.caam.rice.edu/~timwar/HPC12/OpenCL/cl_stuff.c for all the the opencl characteristics*/


/*All includes necessary*/
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <sys/time.h>
#include <OpenCL/opencl.h>
#include "mykernel.cl.h"


const int MaxColorComponentValue=255; 
const double escape_radius = 2;
static unsigned char color[3];


//Color findColor(iteration, max_iterations){}

void compute_color(int iteration, int max_iterations){
	if(iteration == max_iterations){
		color[0]=0;
		color[1]=0;
		color[2]=0;
	}
      //Color white when outside the set
	else if (iteration > max_iterations/20){
		color[0]=255;
		color[1]=255;
		color[2]=255;
	}

	else if (iteration >max_iterations/40){
		color[0]=255;
		color[1]=100;
		color[2]=100;
	}
	else if (iteration >max_iterations/60){
		color[0]=255;
		color[1]=50;
		color[2]=50;
	}
	else if (iteration >max_iterations/80){
		color[0]=200;
		color[1]=0;
		color[2]=0;
	}
	else if (iteration >max_iterations/100){
		color[0]=150;
		color[1]=0;
		color[2]=0;
	}
	else{
		color[0]=75;
		color[1]=0;
		color[2]=0;
	}
}

int parallelMandelbrot(int width, int height, double cx_min, double cx_max, double cy_min, double cy_max, int max_iterations, char* filename, int size_work_group, char* type){
	char name[128];

  int i;        //iterator variables for loops

  
  
	// First, try to obtain a dispatch queue that can send work to the
    // GPU in our system.  
	//dispatch_queue_t queue = gcl_create_dispatch_queue(CL_DEVICE_TYPE_GPU, NULL);
  dispatch_queue_t queue =
        //gcl_create_dispatch_queue(CL_DEVICE_TYPE_USE_ID, devices[0]);
        //gcl_create_dispatch_queue(CL_DEVICE_TYPE_USE_ID, devices[2]);
        //gcl_create_dispatch_queue(CL_DEVICE_TYPE_CPU, NULL);
        gcl_create_dispatch_queue(CL_DEVICE_TYPE_GPU, NULL);

    // In the event that our system does NOT have an OpenCL-compatible GPU,
    // we can use the OpenCL CPU compute device instead.
    if (queue == NULL) {
        queue = gcl_create_dispatch_queue(CL_DEVICE_TYPE_CPU, NULL);
    }

    // This is not required, but let's print out the name of the device
    // we are using to do work.  We could use the same function,
    // clGetDeviceInfo, to obtain all manner of information about the device.
    cl_device_id gpu = gcl_get_device_id_with_dispatch_queue(queue);
    clGetDeviceInfo(gpu, CL_DEVICE_NAME, 128, name, NULL);
    fprintf(stdout, "Created a dispatch queue using the %s\n", name);

    //printf("%d\n",sizeof(int)*height*width);
    
    // Here we hardcode some test data.
    // Normally, when this application is running for real, data would come from
    // some REAL source, such as a camera, a sensor, or some compiled collection
    // of statistics—it just depends on the problem you want to solve.
    //int* input = (int*)malloc(sizeof(cl_int) * height*width);
    //for (i = 0; i < height*width; i++) {
   //   input[i] = i;
   // }
    // Once the computation using CL is done, will have to read the results
    // back into our application's memory space.  Allocate some space for that.
    //int* output = (int*)malloc(sizeof(cl_int) * height*width);
    int* output = (int*)malloc(sizeof(cl_int) * width*height);
    //int output [width*height];
    if(output == NULL){
      printf("Not enough memory");
      return 1;
    }


    // The test kernel takes two parameters: an input float array and an
    // output float array.  We can't send the application's buffers above, since
    // our CL device operates on its own memory space.  Therefore, we allocate
    // OpenCL memory for doing the work.  Notice that for the input array,
    // we specify CL_MEM_COPY_HOST_PTR and provide the fake input data we
    // created above.  This tells OpenCL to copy the data into its memory
    // space before it executes the kernel.                               // 3
   // void* mem_in  = gcl_malloc(sizeof(cl_int) * height*width, input,
    //                           CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR);

    // The output array is not initalized; we're going to fill it up when
    // we execute our kernel.   
    //void* mem_out = gcl_malloc(sizeof(cl_int)*height*width, NULL, CL_MEM_WRITE_ONLY);
    void* mem_out = gcl_malloc(sizeof(cl_int)*width*height, NULL, CL_MEM_WRITE_ONLY);
    if(mem_out == NULL){
      printf("Not enough memory");
      return 1;
    }


  double pixel_width = (cx_max - cx_min)/width;
  double pixel_height = (cy_max - cy_min)/height;

  //struct timeval start,end;
  //gettimeofday(&start,NULL);
  dispatch_sync(queue, ^{
        // Although we could pass NULL as the workgroup size, which would tell
        // OpenCL to pick the one it thinks is best, we can also ask
        // OpenCL for the suggested size, and pass it ourselves.
        size_t wgs;
       gcl_get_kernel_block_workgroup_info(mandelbrot_kernel,
                                           CL_KERNEL_WORK_GROUP_SIZE,
                                            sizeof(wgs), &wgs, NULL);
        
        // The N-Dimensional Range over which we'd like to execute our
        // kernel.  In this case, we're operating on a 1D buffer, so
        // it makes sense that the range is 1D.
        cl_ndrange range = {                                              // 6
            1,                     // The number of dimensions to use.
            
            {0,0,0},             // The offset in each dimension.  To specify
            // that all the data is processed, this is 0
            // in the test case.                   // 7
            
            {width,0,0},    // The global range—this is how many items
            // IN TOTAL in each dimension you want to
            // process.
            
            {64,0,0}            // The local size of each workgroup.  This
            // determines the number of work items per
            // workgroup.  It indirectly affects the
            // number of workgroups, since the global
            // size / local size yields the number of
            // workgroups.  In this test case, there are
            // NUM_VALUE / wgs workgroups.
        };
        // Calling the kernel is easy; simply call it like a function,
        // passing the ndrange as the first parameter, followed by the expected
        // kernel parameters.  Note that we case the 'void*' here to the
        // expected OpenCL types.  Remember, a 'float' in the
        // kernel, is a 'cl_float' from the application's perspective.   // 8
        
        //mandelbrot_kernel(&range,(cl_int)width, (cl_int)height, (cl_int)max_iterations, 
          //(cl_float)cx_min,(cl_float) cx_max, (cl_float) cy_min, (cl_float) cy_max,(cl_int*)mem_out);
        mandelbrot_kernel(&range,(cl_int)width, (cl_int)height, (cl_int)max_iterations, (cl_float)cx_min,(cl_float)cy_min,
          (cl_float)pixel_width,(cl_float)pixel_height,(cl_int*)mem_out);
        
        // Getting data out of the device's memory space is also easy;
        // use gcl_memcpy.  In this case, gcl_memcpy takes the output
        // computed by the kernel and copies it over to the
        // application's memory space.                                   // 9
        
        gcl_memcpy(output, mem_out, sizeof(cl_int) * width*height);
        
    });
  //gettimeofday(&end, NULL);
  //printf("\n\nparallel part duration : %ld s\n", (end.tv_sec - start.tv_sec));
  FILE *fp;
  char *comment="# ";
  fp = fopen(filename, "wb");

  fprintf(fp,"P6\n %s\n %d\n %d\n %d\n",comment,width,height,MaxColorComponentValue);

  for(i= 0; i< width*height; i++){
    //printf(" output at %d: %d\n",i,output[i]);
      compute_color(output[i], max_iterations);
      //write color to file
      fwrite(color,1,3,fp);
  }
  gcl_free(mem_out);
  free(output);
  fclose(fp);
  return 0; 
  

}





int main(int argc, char *argv[]){
	// Make sure that the input has the proper format
	if (argc != 4) {
		fprintf(stderr, "The format should be ./openclf <c(cpu) or g(gpu)><size of work group> <output.ppm>");
		return 1;
	}

  //Harcoded values, otherwise too many values to enter at the command prompt
	int width = 4096;
	int height = 4096;
	int max_iterations = 300;
	double cx_min = -2.5;
	double cx_max = 1.5;
	double cy_min = -2.0;
	double cy_max = 2.0;
	char* type = argv[1];
	int size_work_group = atoi(argv[2]);
	char* filename = argv[3];
	int i;
	struct timeval start,end;

	gettimeofday(&start,NULL);
	for(i=0; i<1; i++){
		parallelMandelbrot(width, height, cx_min, cx_max, cy_min, cy_max, max_iterations, filename, size_work_group,type);
		printf("iteration: %d\n",i);
	}
	gettimeofday(&end, NULL);
	printf("\n\ntotal duration : %ld s\n", (end.tv_sec - start.tv_sec));


	char command[50];
	strcpy(command, "open ");
	strcat(command,filename);
	printf("%s\n",command);
	system(command);

}


