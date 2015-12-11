// Simple OpenCL kernel that squares an input array.
// This code is stored in a file called mykernel.cl.
// You can name your kernel file as you would name any other
// file.  Use .cl as the file extension for all kernel
// source files.

/*
float zx,zy,zx_squared,zy_squared;
int iteration;
int x,y;
float cx, cy;
*/

// Kernel block.                                     
//kernel void mandelbrot(int width,int height,int max_iterations,float cx_min,float cx_max,float cy_min, float cy_max, global int* output)
kernel void mandelbrot(int width,int height,int max_iterations,float cx_min, float cy_min, float pixel_width, float pixel_height, global int* output)
{
	

   // const int MaxColorComponentValue=255; 
	//const float escape_radius = 2;
	//float pixel_width = (cx_max - cx_min)/(width);
	//float pixel_height = (cy_max - cy_min)/(height);
	//int width_portion=width/get_num_groups(0);
   	//int height_portion = height /get_num_groups(1);
	//int height_portion = height;

	//printf("index x_part %d\n",get_global_id(0));
	//printf("size of y_part %d\n",get_global_id(1) );

	
	//
	//printf("number of groups x %d\n", get_num_groups(0));
	//printf("number of groups y %d\n", get_num_groups(1));
	//printf("number of groups x %d\n", get_global_id(0));
	//printf("width portion is %d\n", width_portion);
	//int start_position_x = get_global_id(0);
	//int start_position_y = get_global_id(1);
	
	float zx,zy,zx_squared,zy_squared;
	int iteration;
	int x,y;
	float cx, cy;
	
	//int j = 0;
	for(y = get_global_id(1); y<get_global_id(1)+(height /get_num_groups(1)); y++){

		cy = cy_min + y*pixel_height;
		for(x=get_global_id(0); x<get_global_id(0)+(width/get_num_groups(0)); x++){
			cx = cx_min + x*pixel_width;
			zx = 0;
			zy = 0;
			zx_squared =0;
			zy_squared =0;

			//Continue until max iterations is reached or divergence
			for(iteration = 0; iteration < max_iterations && ((zx_squared +zy_squared) < 4); iteration++){
				zy = 2*zx*zy +cy;
				zx = zx_squared - zy_squared + cx;
				zx_squared = zx*zx;
				zy_squared = zy*zy;
				
			};
			output[x+width*y]=iteration;
		}

	}
	 barrier(CLK_LOCAL_MEM_FENCE);
}