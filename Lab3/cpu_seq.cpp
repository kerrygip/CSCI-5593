#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <cfloat>
#include <sys/time.h>

using namespace std; 

float seq_sum(const float array[], int array_size){
  /*
  Sequential version of reduction sum. 
  */
  float sum = 0; 
  for(int i = 0; i < array_size; i++ ) {
    sum += array[i];
  }  

  return sum ; 
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

  //seed for randomization
  unsigned int seed = atoi( argv[2] );
  srand( seed );

  // start timer
  struct timeval start, end;
  gettimeofday( &start, NULL ); 
  
  // allocate array 
  unsigned int array_size = atoi(argv[1]);
  float *array = new float[array_size] ;
  for(int i = 0; i < array_size; i++ )
    array[i] = -1.0f + (float)random()/((float)RAND_MAX/2.0f);

  //calculate sequential sum
  float sum =  seq_sum(array, array_size);

  //calculate run time
  gettimeofday( &end, NULL );
  float runtime = ( ( end.tv_sec  - start.tv_sec ) * 1000.0 ) + ( ( end.tv_usec - start.tv_usec ) / 1000.0 );    


  // print out
  cout << "Statistics for array size:" << array_size << " seed:"<<  seed << endl ; 
  cout << "    Sum:" << sum << endl ;
  cout << "Processing Time:" << runtime << "(ms)" << endl ;

  delete array ;
  return 0;
}
