//To compile: g++ -O3 -w GaussianBlur.cpp -lpthread -o GaussianBlur
//To run: ./GaussianBlur InputFile outFile thread_count k

#include "stb_image.h"

#include "timer.h"

#include <ctype.h>
#include <cmath>
#include <stdio.h>      /* printf, fgets */
#include <stdlib.h>  /* atoi */
#include <iostream>
#include <fstream>
#include <pthread.h>
using namespace std;
const int MAX_THREADS = 1024;
int thread_count;


uint8* src;
uint8* dst;
int width=0;
int z = 1;
int height=0;
int actual_comps=0;
int YR = 19595, YG = 38470, YB = 7471;




void* windowAverage(int y, int x, int k) {
	int d = k/2;
	int sumR = 0, sumG = 0, sumB = 0, count = 0, x2 = 0, y2=0, index = 0;
	for( int i = -d; i <= d; i++ ) {
		for ( int j = -d; j <= d; j++ ) {
			y2 = y + i;
			x2 = x + j;

            if(y2 < 0 || y2 >= height) {
                continue;
            }
            if(x2 < 0 || x2 >= width) {
                continue;
            }
            
            index = ( y2 * width + x2 ) * actual_comps;
            
            sumR = sumR + src[ index ];
            sumG = sumG + src[ index + 1] ;
            sumB = sumB + src[ index + 2 ];
			count++;
		}
	}
    int newR = sumR/count;
    int newG = sumG/count;
    int newB = sumB/count;
    
    index = ( y * width + x ) * actual_comps;
    dst[ index ] = newR;
    dst[ index + 1 ] = newG;
    dst[ index + 2 ] = newB;
}
/*
void convertPixelToBlur(){
    for( int y = 0; y< height; y++ ){
      for( int x = 0; x < width; x++ ){
          windowAverage( y, x, z );
      }
    }
} */
void* Thread_convertToBlur(void* rank){
    //Each thread will do a chunck of the rows (number of rows is height).
    //height is the number of tasks, n
    //You only have to split the first foor loop.
    int local_n = height / thread_count;
    long my_rank = (long)rank;
    int my_first_row = my_rank * local_n;
    int my_last_row = my_first_row + local_n;
    
    if( my_rank == thread_count - 1) {
        my_last_row = my_last_row + height % thread_count;
    }
    
    for( int y = my_first_row; y < my_last_row; y++ ) {
        for( int x = 0; x < width; x++ ) {
            windowAverage( y, x, z);
        }
    }

}



int main(int argc, char** argv)
{
	long thread;
	double start, finish;


	pthread_t* thread_handles;
    thread_handles = (pthread_t*) malloc(thread_count*sizeof(pthread_t));

    if(argc<2){
      cout<<"To run: ./assign5 fileName outFileName"<<endl;
      cout<<"./assign5 stone.jpeg stoneOut.jpeg"<<endl;
      cout<<"For Multithreading::i \n To run: ./assign5 fileName outFileName thread_count blur_value"<<endl;
      cout<<"./assign5 stone.jpeg stoneOut.jpeg 3"<<endl;
      return 0;
    }
     
  const char* pSrc_filename = argv[1];
  const char* pDst_filename = argv[2];
  thread_count = atoi(argv[3]);
z = atoi(argv[4]);

  // Load the source image.
  const int req_comps = 3; // request RGB image
  src = stbi_load(pSrc_filename, &width, &height, &actual_comps, req_comps);
  if (!src)
  {
    cout<<"Failed loading file "<<endl;
    return 1;
  }

  cout<<"Source file: "<< pSrc_filename <<" "<< width <<" " <<height <<" "<< actual_comps<<endl;
  cout<<"RGB for pixel 0 (0,0) ::: "<<(int) src[0]<< " "<< (int) src[1]<< " "<< (int) src[2]<<endl;

  //Allocate memory for the output image. 
  dst = (uint8*) malloc(width* height*3);

  GET_TIME(start);
    

  for( thread = 0; thread < thread_count; thread++) {
	  pthread_create(&thread_handles[thread], NULL,
			  Thread_convertToBlur, (void*) thread);
  }

  for ( thread = 0; thread < thread_count; thread++ ) {
	  pthread_join(thread_handles[thread], NULL);
  }
  
  GET_TIME(finish);
  printf( "Serial time = %e seconds \n", finish - start );
  if (!stbi_write_tga(pDst_filename, width, height, 3, dst))
  {
    cout<<"Failed writing image to file "<< pDst_filename <<endl;
    free(src);
    free(dst);
    return EXIT_FAILURE;
  }

  free(src);
  free(dst);
  return 0;
}


