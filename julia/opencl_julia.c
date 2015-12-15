/* 
To compile:
make in the same repository as mykernel.cl
To run:
./opencl_julia <c(cpu) or g(gpu)><size of work group> <output.ppm>
*/


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


void compute_color(int iteration, int max_iterations){
  if(iteration == max_iterations){
    color[0]=0;
    color[1]=0;
    color[2]=0;
  }
      //Color white when outside the set
  else if (iteration > max_iterations/1.5){
    color[0]=255;
    color[1]=255;
    color[2]=255;
  }

  else if (iteration >max_iterations/2){
    color[0]=100;
    color[1]=100;
    color[2]=255;
  }
  else if (iteration >max_iterations/4){
    color[0]=50;
    color[1]=50;
    color[2]=255;
  }
  else if (iteration >max_iterations/6){
    color[0]=0;
    color[1]=0;
    color[2]=200;
  }
  else if (iteration >max_iterations/8){
    color[0]=0;
    color[1]=0;
    color[2]=150;
  }
  else{
    color[0]=0;
    color[1]=0;
    color[2]=75;
  }
}

int parallelJulia(int width, int height, double cx_min, double cx_max, double cy_min, double cy_max, int max_iterations, char* filename, int size_work_group, char* type){
  char name[128];

  int i;      

  //Create a dispatch queue to run on the GPU or CPU 
  dispatch_queue_t queue = NULL;
  if(strcmp(type,"g")==0){
    queue = gcl_create_dispatch_queue(CL_DEVICE_TYPE_GPU, NULL);
  }
  else{
    queue = gcl_create_dispatch_queue(CL_DEVICE_TYPE_CPU, NULL);
  }
  
    //Print the name of the device
  cl_device_id gpu = gcl_get_device_id_with_dispatch_queue(queue);
  clGetDeviceInfo(gpu, CL_DEVICE_NAME, 128, name, NULL);
  fprintf(stdout, "Created a dispatch queue using the %s\n", name);
  
  
    // Once the computation using CL is done, will have to read the results
    // back into our application's memory space.  Allocate some space for that.
  int* output = (int*)malloc(sizeof(cl_int) * width*height);
  if(output == NULL){
    printf("Not enough memory");
    return 1;
  }

   //Allocate OpenCL memory for the output
  void* mem_out = gcl_malloc(sizeof(cl_int)*width*height, NULL, CL_MEM_WRITE_ONLY);
  if(mem_out == NULL){
    printf("Not enough memory");
    return 1;
  }

  //Compute pixel_width and pixel_height before to reduce kernel work
  double pixel_width = (cx_max - cx_min)/width;
  double pixel_height = (cy_max - cy_min)/height;

  //struct timeval start,end;
  //gettimeofday(&start,NULL);
  dispatch_sync(queue, ^{
    
        // The N-Dimensional Range over which we'd like to execute our
        // kernel.  In this case, we're operating on a 1D buffer, so
        // it makes sense that the range is 1D.
    cl_ndrange range = {                                           
            1,                     // Dimensions to use
            
            {0,0,0},             //No offset in any dimension
            
            {width,0,0},    // The global range—this is how many items
            // IN TOTAL in each dimension you want to
            // process.
            
            {size_work_group,0,0}            // The local size of each workgroup.  This
            // determines the number of work items per
            // workgroup.  It indirectly affects the
            // number of workgroups, since the global
            // size / local size yields the number of
            // workgroups.  In this test case, there are
            // width/size_work_group groups
          };
          
        //Kernel call, we pass all required information
        //Note we use float instead of double because it is better implemented
          julia_kernel(&range,(cl_int)width, (cl_int)height, (cl_int)max_iterations, (cl_float)cx_min,(cl_float)cy_min,
            (cl_float)pixel_width,(cl_float)pixel_height,(cl_int*)mem_out);
          
        //Get the data back from the kernel memory space to application memory space
          gcl_memcpy(output, mem_out, sizeof(cl_int) * width*height);
          
        });
  //gettimeofday(&end, NULL);
  //printf("\n\nAlgorithm's computational part duration : %ld\n", \
              // ((end.tv_sec * 1000000 + end.tv_usec) - (start.tv_sec * 1000000 + start.tv_usec)));

  //Open and write ppm image

FILE *fp;
char *comment="# ";
fp = fopen(filename, "wb");

fprintf(fp,"P6\n %s\n %d\n %d\n %d\n",comment,width,height,MaxColorComponentValue);

  //Compute color and write it for each pixel
for(i= 0; i< width*height; i++){
  compute_color(output[i], max_iterations);
  fwrite(color,1,3,fp);
}
  //Free memory
gcl_free(mem_out);
free(output);
fclose(fp);

return 0; 


}




int main(int argc, char *argv[]){
  // Make sure that the input has the proper format
  if (argc != 4) {
    fprintf(stderr, "The format should be ./opencl_julia <c(cpu) or g(gpu)><size of work group> <output.ppm>");
    return 1;
  }

  //Harcoded values, otherwise too many values to enter at the command prompt
  int width = 2048;
  int height = 2048;
  int max_iterations = 300;
  double cx_min = -2.0;
  double cx_max = 2.0;
  double cy_min = -2.0;
  double cy_max = 2.0;
  char* type = argv[1];
  int size_work_group = atoi(argv[2]);
  char* filename = argv[3];
  int i;
  
  struct timeval start,end;
  //start timer
  gettimeofday(&start,NULL);
  for(i=0; i<10; i++){
    parallelJulia(width, height, cx_min, cx_max, cy_min, cy_max, max_iterations, filename, size_work_group,type);
    printf("iteration: %d\n",i);
  }
  gettimeofday(&end, NULL);
  //end timer
  printf("\n\ntotal duration : %ld s\n", (end.tv_sec - start.tv_sec));

  //open image
  char command[50];
  strcpy(command, "open ");
  strcat(command,filename);
  printf("%s\n",command);
  system(command);

}


