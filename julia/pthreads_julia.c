/*To compile:
 gcc -o pthreads_julia pthreads_julia.c -lpthread  
 To run:
./pthreads_julia <number of threads> <output.ppm>
*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <sys/time.h>

const int MaxColorComponentValue=255; 
const double escape_radius = 2;
static unsigned char color[3];

//Data passed to each thread
typedef struct{
	int* output;
	int width;
	int height;
	int width_portion;
	int start_position;
	int max_iterations;
	double cx_min;
	double cy_min;
	double pixel_width;
	double pixel_height;
}thread_data;



void compute_color(int iteration, int max_iterations){
	//Color black if in the set
	if(iteration == max_iterations){
		color[0]=0;
		color[1]=0;
		color[2]=0;
	}
      //Color depending on speed of divergence
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

void *worker_thread(void *arg) {
	//Extract all info
	thread_data *info = (thread_data *) arg;
	int* output =info->output;
	int start_position = info->start_position;
	int height = info->height;
	int width_portion = info->width_portion;
	int width = info-> width;
	int max_iterations = info-> max_iterations;
	double cx_min =info->cx_min;
	double cy_min = info->cy_min;
	double pixel_width = info->pixel_width;
	double pixel_height = info->pixel_height;


	double zx,zy,zx_squared,zy_squared;
	int iteration;
	int x,y;
	double cx, cy;
	int j=0;
	for(y = 0; y<height; y++){

		cy = cy_min + y*pixel_height;
		for(x=start_position; x<start_position+width_portion; x++){//each thread computes a section of the width
			cx = cx_min + x*pixel_width;
			zx = cx;
			zy = cy;
			zx_squared =zx*zx;
			zy_squared = zy*zy;

			//Continue until max iterations is reached or divergence
			for(iteration = 0; iteration < max_iterations && ((zx_squared +zy_squared) < 4); iteration++){
				//here k= 0.285+i0.013
				zy = 2*zx*zy +0.013f;
				zx = zx_squared - zy_squared +0.285;
				zx_squared = zx*zx;
				zy_squared = zy*zy;
				
			};
			
			output[x+width*y]=iteration;
			
		}

	}
	pthread_exit(NULL);

}

//pthread version
int parallelJulia(int width, int height, double cx_min, double cx_max, double cy_min, double cy_max, int max_iterations, char* filename, int num_threads){

int thread_count = num_threads;
double pixel_width = (cx_max - cx_min)/width;
double pixel_height = (cy_max - cy_min)/height;


//output buffer that stores the number of iterations for each pixel
int* output = (int*)malloc(sizeof(int) * width*height);
if(output == NULL){
	printf("Not enough memory");
	return 1;
}

int s;
pthread_t *threads = malloc(thread_count * sizeof(pthread_t));
thread_data *info = malloc(thread_count * sizeof(thread_data));

int start_position = 0;
	int section_size = width/thread_count;//division of work over width


	//struct timeval start,end;
	//gettimeofday(&start,NULL);
	int i;
	for(i = 0;i < thread_count;i++){

		info[i].output= output;
		info[i].width = width;
		info[i].height = height;
		info[i].width_portion = section_size;
		info[i].start_position = start_position;
		info[i].max_iterations = max_iterations;
		info[i].cx_min = cx_min;
		info[i].cy_min = cy_min;
		info[i].pixel_width = pixel_width;
		info[i].pixel_height = pixel_height;

		start_position+= section_size;


		s = pthread_create(&threads[i], NULL, worker_thread,&info[i]);

		if (s != 0) {
			printf("Error creating threads\n");
			exit(1);
		}
	}

	for(i = 0; i<thread_count; i++){

		s = pthread_join(threads[i], NULL);

		if (s != 0) {
			printf("Error joining threads\n");
			exit(1);
		}

	}

//gettimeofday(&end, NULL);
	//printf("\n\nparallel part duration : %ld s\n", (end.tv_sec - start.tv_sec));

	FILE *fp;
	char *comment="# ";
	fp = fopen(filename, "wb");

	fprintf(fp,"P6\n %s\n %d\n %d\n %d\n",comment,width,height,MaxColorComponentValue);


	for(i= 0; i< width*height; i++){
		compute_color(output[i], max_iterations);
  //write color to file
		fwrite(color,1,3,fp);
	}
	free(output);
	fclose(fp);

	return 0;
}


int main(int argc, char *argv[]){
// Make sure that the input has the proper format
	if (argc != 3) {
		fprintf(stderr, "The format should be ./pthreadf <number of threads> <output.ppm>");
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
	int num_threads = atoi(argv[1]);
	char* filename = argv[2];
	int i;
	struct timeval start,end;

//start timer
	gettimeofday(&start,NULL);
	for(i=0; i<100; i++){
		parallelJulia(width, height, cx_min, cx_max, cy_min, cy_max, max_iterations, filename, num_threads);
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