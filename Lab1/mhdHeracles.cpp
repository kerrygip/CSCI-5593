	//-----------------------------------------------------------------------
// Understanding the Impact of Memory System Design Tradeoffs on Machine Performance
//-----------------------------------------------------------------------
//  The source code of this lab is copied and adapted from the Figure 2.29, page 133-134 of the text book
//  Hennessey and Patterson, "Computer Architecture: A Quantitative Approach", 5th Ed., 2011 
//  Updated in 01/22/2019 by Iris and Mahn
//-----------------------------------------------------------------------
#include <stdio.h>
#include <time.h>
#include <iostream>
#include <iomanip>
#include <fstream>
using namespace std;
//In other to run this code in your own computer, adjust the ARRAY_MIN and ARRAY_MAX 
//to the size that matchs the cache configurations of your processor.
//Below setup is for the Heracles cluster
//Resize with suitable information of cache if you want to test on your computer
#define ARRAY_MIN (4*1024)		    // 4KB
#define ARRAY_MAX (30*1024*1024)	// 30MB
int x[ARRAY_MAX];				//array going to stride through 


//-----------------------------------------------------------------------------
//routine to read time in seconds
//-----------------------------------------------------------------------------
#ifdef __linux__ //for Linux

#include <sys/time.h>
double get_seconds()
{
  static int sec = -1;
  struct timeval tv;
  gettimeofday(&tv, NULL);
  if (sec < 0) 	sec = tv.tv_sec;
  return (double)((tv.tv_sec - sec) + 1.0e-6*tv.tv_usec);
}

#else //for Windows

double get_seconds() { 
	__time64_t ltime;
	_time64(&ltime);
	return (double) ltime;
}
#endif 	

//-----------------------------------------------------------------------------
//generate text labels
//-----------------------------------------------------------------------------
int label ( int i) {
	if (i<1e3) cout << i << "B,"; 
	else if (i<1e6) cout << i/1024 << "K,";
	else if (i<1e9) cout << i/1048576 << "M,";
	else cout << i/1073741824 << "G,";
	return 0;
}

//-----------------------------------------------------------------------------
//Main functions
//-----------------------------------------------------------------------------
int main(int argc, char* argv[])
{
	int register nextstep,i, index, stride;
	int csize;
	double steps , tsteps;
	double loadtime, lastsec, sec0, sec1, sec; //timing variables


	//Initialize output
	cout <<" ,";
	for (stride = 1 ; stride <= ARRAY_MAX/2; stride=stride*2)
	{
		label (stride*sizeof(int));
	}
	cout <<endl;

	//Main loop for each configuration
	for (csize = ARRAY_MIN; csize < ARRAY_MAX; csize = csize*2)
	{
		label (csize*sizeof(int)); // print cache size this loop

		for (stride = 1 ; stride <= csize/2; stride=stride*2) 
		{
			//Lay out path of memory references in array 
			for(index=0; index < csize; index=index + stride)
				x[index] = index + stride;	//pointer to next

			x[index-stride] = 0;  //loop back to beginning

			//Wait for timer to roll over
			lastsec = get_seconds();
			do sec0 = get_seconds(); while (sec0 == lastsec);

			//Walk through path in array for twenty seconds
			//This gives 5% accuracy with second resolution
			steps = 0.0;	//Number of steps taken
			nextstep = 0;	//Start at beginning of path
			sec0 = get_seconds();	//Start timer

			do {//Repeat until collect 20 seconds
					for (i=stride; i!=0; i--){ //Keep samples same
						nextstep = 0;

					do nextstep = x[nextstep]; //Dependency
					while (nextstep != 0);
				}
				steps = steps + 1.0; //Count loop iterations
				sec1 = get_seconds(); //End timer
			}
			while ((sec1 - sec0) < 20.0); //Collect 20 seconds

			sec = sec1 - sec0;

			//Repeat empty loop to loop subtract overhead
			tsteps = 0.0; //used to match no. while iterations
			sec0 = get_seconds() ; //start timer
			
			do { //repeat until some no. iterations as above
				for (i = stride;i!=0;i=i-1) { //keep samples some
					index = 0;
					do index = index + stride;
					while (index < csize);
				}
				tsteps = tsteps + 1.0;
				sec1 = get_seconds(); //- overhead
			} 
			while (tsteps<steps); //until = no. iterations

			sec = sec - (sec1 - sec0);
			loadtime = (sec*1e9)/(steps*csize); 
			
			cout << ((loadtime<0.1)? 0.1 : loadtime) << setiosflags(ios::fixed) << setprecision(1) << ",";//write out the results in .csv format for Excel 
		};//end of inner for loop
		cout << endl;

	};//end of outer for loop

	return 0;
}
