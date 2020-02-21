#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "jpeglib.h"
#include "cs338jpeg.h"


/* Provide two different multi-threaded approaches for blurring
/ a JPEG image by making each pixel a weighed average of its neighbors,
/ part of a larger project which provided a framework for reading from
/ and writing to JPEGS.
*/

int r; // blur radius
frame_ptr from, to; // pointers to source and destination images

// Blur image, parallelizing using row blocking
void *row_block(void *rank){
  int i,j,k;
  long my_rank = (long)rank;

  int weight[r][r]; // 2 dimensional array stores every possible weight
  long denom = 0;

  // Pre-calculate weights and final division factor
  // Build a 2 dimensional array containing every possible weight
  for (i = r-1; i > -r; i--)
    for (j = r-1; j > -r; j--){
      // for positive values of i and j, update table
      if (i >= 0 && j >= 0)
	weight[i][j] = (r - i) * (r - j);	    

      denom += weight[abs(i)][abs(j)];
    }

  // iterate through rows
  int block_size = from->image_height / num_procs;
  int row_block_start = my_rank * block_size;

  // iterate through rows within this thread's assigned block
  for (i = row_block_start; i < row_block_start + block_size; i++)
    // iterate through columns
    for (j=0; j < from->image_width; j++){
      int istart = -(r - 1);
      int jstart = -(r - 1);
      int ifinish = r - 1;
      int jfinish = r - 1;

      // Check if pixel i,j is within r pixels of edge of image
      if (i + istart >= 0 && i + ifinish < from->image_height
	  && j + jstart >= 0 && j +jfinish < from->image_width){
	    
	//iterate through rgb values
	for (k=0; k < from->num_components; k++){
	  long num = 0; // the numerator for our final fraction

	  // sum over all pixels within r distance of pixel i,j,
	  // adding value of each step in summation to our numerator
	  for(int x = istart; x <= ifinish; x++){
	    for (int y = jstart; y <= jfinish; y++){
	      int input = from->row_pointers[x+i][(from->num_components)*(j+y)+k];
	      num += input * weight[abs(x)][abs(y)];
	    }
	    // set output pixel equal to result of division
	    to->row_pointers[i][(from->num_components)*j+k] = num / denom;
	  }
	}
      }
      // code for pixels within r pixels of the edge of the image
      else{
	// Change start and end for summation, so that we only
	// are summing over pixels actually in the image
	while (i + istart < 0)
	  istart ++;
	while (i + ifinish >= from->image_height)
	  ifinish --;
	while (j + jstart < 0)
	  jstart ++;
	while (j + jfinish >= from->image_width)
	  jfinish --;
	      	      	      
	//iterate through rgb values
	for (k=0; k < from->num_components; k++){
	  long num = 0; // numerator
	  long d = 0; // denominator

	  // Special loops for pixels within r of height,
	  // calculate sum only over pixels that actually exist
	  for(int x = istart; x <= ifinish; x++){
	    for (int y = jstart; y <= jfinish; y++){		
	      int input = from->row_pointers[x+i][(from->num_components)*(j+y)+k];
	      int w = weight[abs(x)][abs(y)];
	      d += w; // add weight to denominator
	      num += input * w; // update our numeraator
	    }
	    // set output pixel equal to result of division
	    to->row_pointers[i][(from->num_components)*j+k] = num / d;
	  }
	}
      }	 	  
    }
  return NULL;
}

// Blur image, parallelizing using row interleaving
void *row_interleave(void *rank){
  int i,j,k;
  long my_rank = (long)rank;

  int weight[r][r]; // 2 dimensional array stores every possible weight
  long denom = 0;

  // Pre-calculate weights and final division factor
  // Build a 2 dimensional array containing every possible weight
  for (i = r-1; i > -r; i--)
    for (j = r-1; j > -r; j--){
      // for positive values of i and j, update table
      if (i >= 0 && j >= 0)
	weight[i][j] = (r - i) * (r - j);	    

      denom += weight[abs(i)][abs(j)];
    }
  // iterate through rows, each thread interleaving
  for (i = my_rank; i < from->image_height; i+= num_procs)
    // iterate through columns
    for (j=0; j < from->image_width; j++){
      int istart = -(r - 1);
      int jstart = -(r - 1);
      int ifinish = r - 1;
      int jfinish = r - 1;

      // Check if pixel i,j is within r pixels of edge of image
      if (i + istart >= 0 && i + ifinish < from->image_height
	  && j + jstart >= 0 && j +jfinish < from->image_width){
	    
	//iterate through rgb values
	for (k=0; k < from->num_components; k++){
	  long num = 0; // the numerator for our final fraction

	  // sum over all pixels within r distance of pixel i,j,
	  // adding value of each step in summation to our numerator
	  for(int x = istart; x <= ifinish; x++){
	    for (int y = jstart; y <= jfinish; y++){
	      int input = from->row_pointers[x+i][(from->num_components)*(j+y)+k];
	      num += input * weight[abs(x)][abs(y)];
	    }
	    // set output pixel equal to result of division
	    to->row_pointers[i][(from->num_components)*j+k] = num / denom;
	  }
	}
      }
      // code for pixels within r pixels of the edge of the image
      else{
	// Change start and end for summation, so that we only
	// are summing over pixels actually in the image
	while (i + istart < 0)
	  istart ++;
	while (i + ifinish >= from->image_height)
	  ifinish --;
	while (j + jstart < 0)
	  jstart ++;
	while (j + jfinish >= from->image_width)
	  jfinish --;
	      	      	      
	//iterate through rgb values
	for (k=0; k < from->num_components; k++){
	  long num = 0; // numerator
	  long d = 0; // denominator

	  // Special loops for pixels within r of height,
	  // calculate sum only over pixels that actually exist
	  for(int x = istart; x <= ifinish; x++){
	    for (int y = jstart; y <= jfinish; y++){		
	      int input = from->row_pointers[x+i][(from->num_components)*(j+y)+k];
	      int w = weight[abs(x)][abs(y)];
	      d += w; // add weight to denominator
	      num += input * w; // update our numeraator
	    }
	    // set output pixel equal to result of division
	    to->row_pointers[i][(from->num_components)*j+k] = num / d;
	  }
	}
      }	 	  
    }
  return NULL;
}

/* 
Blur Image by making each pixel a weighted average of 
itself and its neighbors, providing implementations for 2 
different parallel appraoches
*/
void blur_image()
{
  int i, j, k;
  float percent_blur = 0.05;
	
  from = input_frames[0];
  to = output_frames[0] = allocate_frame(from->image_height,
					 from->image_width, from->num_components);

  // Set blur radius equal to max out of image height and width
  if (from->image_height >= from->image_width)
    r = percent_blur * from->image_height;
  else
    r = percent_blur * from->image_width;

  pthread_t ids[num_procs]; // the number of threads

  // ONLY ONE OF THE FOLLOWING SEGMENTS OF CODE WILL BE EXECUTED
#ifdef BLOCK	
  // Create threads and call row blocking function,
  // passing each thread its rank  
  for(long p = 0; p < num_procs; p++){
    pthread_create(&ids[p], NULL, row_block, (void*)p);
  }
  for(int p = 0; p < num_procs; p++){
    pthread_join(ids[p], NULL);
  }
  return;
#endif
  
#ifdef INTERLEAVE 
  // Create threads and call row interleaving function
  // passing each thread its rank	
  for(long p = 0; p < num_procs; p++){
    pthread_create(&ids[p], NULL, row_interleave, (void*)p);
  }
  for(int p = 0; p < num_procs; p++){
    pthread_join(ids[p], NULL);
  }
  return;
#endif
}
