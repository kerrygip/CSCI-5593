#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <cfloat>
#include <sys/time.h>
#include <cuda.h>
#include <stdio.h>

using namespace std; 

#define HANDLE_ERROR( err )  ( HandleError( err, __FILE__, __LINE__ ) )
#define NUM_THREADS 1024  // maximum number of threads per blocks
#define NUM_TRIALS 100    // number of trials 


void HandleError( cudaError_t err, const char *file, int line ) {
  if ( err != cudaSuccess ) {
    printf( "%s in %s at line %d\n", cudaGetErrorString( err ), file, line );
    exit( EXIT_FAILURE );
  }
}


__global__ void shared_memory_sum(float * d_out, const float * d_in)
{
    // sdata is allocated in the kernel call: 3rd arg to <<<b, t, shmem>>>
    extern __shared__ float sdata[];

    int myId = threadIdx.x + blockDim.x * blockIdx.x;
    int tid  = threadIdx.x;

    // load shared mem from global mem
    sdata[tid] = d_in[myId];
    __syncthreads();            // make sure entire block is loaded!

    // do reduction in shared mem
    for (unsigned int s = blockDim.x / 2; s > 0; s >>= 1)
    {
        if (tid < s)
        {
            sdata[tid] += sdata[tid + s];
        }
        __syncthreads();        // make sure all adds at one stage are done!
    }

    // only thread 0 writes result for this block back to global mem
    if (tid == 0)
    {
        d_out[blockIdx.x] = sdata[0];
    }
}



int main( int argc, char* argv[] ) {
  /*
  main program 
  */

  if(argc < 3) {
    cout << "Format: stats_s <size of array> <random seed>" << endl  ;
    cout << "Arguments:" << endl;
    cout << "  size of array - This is the size of the array to be generated and processed\n"  << endl ;
    cout << "  random seed   - This integer will be used to seed the random number\n"  << endl ;        
    cout << "                  generator that will generate the contents of the array\n"  << endl ;     
    cout << "                  to be processed\n"  << endl ;   
    exit(1);
  }

  int deviceCount;
  cudaGetDeviceCount(&deviceCount);
  if (deviceCount == 0) {
      fprintf(stderr, "error: no devices supporting CUDA.\n");
      exit(EXIT_FAILURE);
  }
  int dev = 0;
  cudaSetDevice(dev);

  cudaDeviceProp devProps;
  if (cudaGetDeviceProperties(&devProps, dev) == 0)
  {
      printf("Using device %d:\n", dev);
      printf("%s; global mem: %dB; compute v%d.%d; clock: %d kHz\n",
             devProps.name, (int)devProps.totalGlobalMem, 
             (int)devProps.major, (int)devProps.minor, 
             (int)devProps.clockRate);
  }

  //seed for randomization
  unsigned int seed = atoi( argv[2] );
  srand( seed );

  // initialize input array on host 
  unsigned int array_size = atoi(argv[1]);
  float sum = 0 ;
  float *h_in= (float*)malloc(array_size*sizeof(float));
  for(int i = 0; i < array_size; i++ ){
    h_in[i] = -1.0f + (float)random()/((float)RAND_MAX/2.0f);
    sum += h_in[i] ;
  }

  // declare device pointers
  float *d_in, *d_intermediate, *d_out;

  // allocate memory on device
  HANDLE_ERROR(cudaMalloc((void **) &d_in, array_size*sizeof(float)));   
  HANDLE_ERROR(cudaMalloc((void **) &d_intermediate, array_size*sizeof(float)));   
  HANDLE_ERROR(cudaMalloc((void **) &d_out, sizeof(float)));   

  // copy array host -> device
  HANDLE_ERROR(cudaMemcpy(d_in, h_in, array_size*sizeof(float), cudaMemcpyHostToDevice));   

  cudaEvent_t start, stop;
  cudaEventCreate(&start);
  cudaEventCreate(&stop);

  cudaEventRecord(start, 0);

  // Set number of blocks and number of threads.
  int num_threads = NUM_THREADS  ;
  int num_blocks = array_size/num_threads ;   
  
  // Run kernels 100 times
  for(int i = 0 ; i < NUM_TRIALS; i++){
    // KERNEL CALL 1ST TIME
    shared_memory_sum<<<num_blocks, num_threads, num_threads*sizeof(float)>>>(d_intermediate, d_in);

    // KERNEL CALL 2ND TIME
    num_threads = num_blocks; // launch one thread for each block in prev step
    num_blocks = 1;
    shared_memory_sum<<<num_blocks, num_threads, num_threads*sizeof(float)>>>(d_out, d_intermediate);
  }

  cudaEventRecord(stop, 0);
  cudaEventSynchronize(stop);
  float runtime;
  cudaEventElapsedTime(&runtime, start, stop);    
  runtime /= NUM_TRIALS; // average runtime of 100 trials


  // copy data from device -> host
  float h_out;
  HANDLE_ERROR(cudaMemcpy(&h_out, d_out, sizeof(float), cudaMemcpyDeviceToHost));

  cout << "Statistics for array size:" << array_size << " seed:"<<  seed << endl ; 
  //cout << "sequential sum: " << sum << endl ;
  //cout << "parallel sum:" << h_out << endl ;
  cout << "runtime:" << runtime << " milliseconds" << endl ;


  free(h_in) ;
  cudaFree(d_in);
  cudaFree(d_intermediate);
  cudaFree(d_out);

  return 0;
}
