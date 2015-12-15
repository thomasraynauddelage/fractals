// Kernel block.
kernel void mandelbrot(int width,int height,int max_iterations,float cx_min, float cy_min, float pixel_width, float pixel_height, global int* output)
{
    
    float zx,zy,zx_squared,zy_squared;
    int iteration;
    int x,y;
    float cx, cy;
    for(y = 0; y<height; y++){
        
        
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