/*
To Compile:
gcc -o sequential_julia sequential_julia.c 
To run:
./sequential_julia <output.ppm>
*/
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>


const int MaxColorComponentValue=255; 
const double escape_radius = 2;
static unsigned char color[3];


void compute_color(int iteration, int max_iterations){
  //color black if in the set
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

int sequentialJulia(int width, int height, double cx_min, double cx_max, double cy_min, double cy_max, int max_iterations, char* filename){

	double pixel_width = (cx_max - cx_min)/width;
	double pixel_height = (cy_max - cy_min)/height;

	
	int* output = (int*)malloc(sizeof(int) * width*height);
	if(output == NULL){
		printf("Not enough memory");
		return 1;
	}


	double zx,zy,zx_squared,zy_squared;
	int iteration;

	int x,y;
	double cx, cy;

  //struct timeval start,end;
  //gettimeofday(&start,NULL);
	for(y = 0; y<height; y++){

		cy = cy_min + y*pixel_height;

		for(x=0; x< width; x ++){
			cx = cx_min + x*pixel_width;
			zx = cx;
			zy = cy;
			zx_squared =zx*zx;
			zy_squared = zy*zy;

			//Continue until max iterations is reached or divergence
			for(iteration = 0; iteration < max_iterations && ((zx_squared +zy_squared) < 4); iteration++){
				//here k= 0.285+i0.013
				zy=2*zx*zy+0.013;
				zx = zx_squared - zy_squared + 0.285;
				zx_squared = zx*zx;
				zy_squared = zy*zy;
				
			};
			output[x+width*y] = iteration;
		}
	}

  //gettimeofday(&end, NULL);
  //printf("\n\nparallelizable part duration : %ld s\n", (end.tv_sec - start.tv_sec));
	int i;
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
	if (argc != 2) {
		fprintf(stderr, "The format should be ./sequential_julia <output.ppm>");
		return 1;
	}

  //Harcoded values, otherwise too many values to enter at the command prompt
	int width = 2048;
	int height = 2048;
	int max_iterations = 300;
	double cx_min = -2;
	double cx_max = 2;
	double cy_min = -2.0;
	double cy_max = 2.0;
	char* filename = argv[1];
	int i;
	struct timeval start,end;

	//start timer
	gettimeofday(&start,NULL);
	for(i=0; i<10; i++){
		sequentialJulia(width, height, cx_min, cx_max, cy_min, cy_max, max_iterations, filename);
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








