//-----------------------------------------------------------------------
// Matrix Multiplication
//-----------------------------------------------------------------------
//  Program includes four versions of Matrix Multiplication
//  Two sequential versions and two parallel versions
//-----------------------------------------------------------------------
#include <iostream>
#include <iomanip>
#include <cmath>
#include <omp.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h> 
#include <unistd.h>
using namespace std;
//-----------------------------------------------------------------------
//   Get user input for matrix dimension or printing option
//-----------------------------------------------------------------------
bool GetUserInput(int argc, char *argv[],int& n,int& version,int& isPrint)
{
	bool isOK = true;

	if(argc < 3) 
	{
		cout << "Arguments:<X> <Y> [<Z>]" << endl;
		cout << "X : Matrix size [X x X]" << endl;
		cout << "Y : Version (Sequential Version for Y is 1 or 2; Parallel Version for Y is 3 or 4)" << endl;
		cout << "Z = 1: print the input/output matrix if X < 10" << endl;
		isOK = false;
	}
	else 
	{
		//get matrix size
		n = atoi(argv[1]);
		if (n <=0) 
		{
			cout << "Matrix size must be larger than 0" <<endl;
			isOK = false;
		}

		//get version
		version = atoi(argv[2]);
		if (version <= 0)
		{	cout << "Version must be 1, 2, 3, or 4" <<endl;
			isOK = false;
		}

		//is print the input/output matrix
		if (argc >=4)
			isPrint = (atoi(argv[3])==1 && n <=15)?1:0;
		else
			isPrint = 0;
	}
	return isOK;
}

//-----------------------------------------------------------------------
//Initialize the value of matrix x[n x n]
//-----------------------------------------------------------------------
void InitializeMatrix(double** &x,int n,double value)
{
	x = new double*[n];
	x[0] = new double[n*n];

	for (int i = 1; i < n; i++)
    x[i] = x[i-1] + n;

	for (int i = 0 ; i < n ; i++)
	{
		for (int j = 0 ; j < n ; j++)
		{
			if (value == 0) // initialize matrix C with zeros
         x[i][j] = value;
      if (value == 1) // initialize matrix aA or B
        x[i][j] = ((i+2) + (j+3)) + 5 ;
		}
	}
}
//------------------------------------------------------------------
//Delete matrix x[n x n]
//------------------------------------------------------------------
void DeleteMatrix(double **x,int n)
{
	delete[] x[0];
	delete[] x; 
}
//------------------------------------------------------------------
//Print matrix	
//------------------------------------------------------------------
void PrintMatrix(double **x, int n) 
{
	for (int i = 0 ; i < n ; i++)
	{
		cout<< "Row " << (i+1) << ":\t" ;
		for (int j = 0 ; j < n ; j++)
		{
			printf("%.2f\t", x[i][j]);
		}
		cout<<endl ;
	}
}
//------------------------------------------------------------------
//Compute Matrix Multiplication (Sequential Version 1)
//------------------------------------------------------------------
void SequentialMatrixMultiplication_Version1(double** a, double** b,double** c, int n)
{
	for (int i = 0 ; i < n ; i++)
		for (int j = 0 ; j < n ; j++)
			for (int k = 0 ; k < n ; k++)
				c[i][j] += a[i][k]*b[k][j];
}
//------------------------------------------------------------------
//Compute Matrix Multiplication (Sequential Version 2)
//------------------------------------------------------------------
void SequentialMatrixMultiplication_Version2(double** a, double** b,double** c, int n)
{
	for (int i = 0 ; i < n ; i++)
		for (int k = 0 ; k < n ; k++)
			for (int j = 0 ; j < n ; j++)
				c[i][j] += a[i][k]*b[k][j];
}
//------------------------------------------------------------------
//Compute Matrix Multiplication (Parallel Version 3)
//------------------------------------------------------------------
void ParallelMatrixMultiplication_Version3(double** a, double** b,double** c, int n)
{
    //printf("Version 3 - OpenMP with %d threads\n", omp_get_max_threads());
    #pragma omp parallel for
    for (int i = 0 ; i < n ; i++)
    {
        for (int j = 0 ; j < n ; j++)
            for (int k = 0 ; k < n ; k++)
                c[i][j] += a[i][k]*b[k][j];
    }
}
//------------------------------------------------------------------
//Compute Matrix Multiplication (Parallel Version 4)
//------------------------------------------------------------------
void ParallelMatrixMultiplication_Version4(double** a, double** b,double** c, int n)
{
    //printf("version 4 - OpenMP with %d threads\n", omp_get_max_threads());
    #pragma omp parallel for
    for (int i = 0 ; i < n ; i++)
    {
        for (int k = 0 ; k < n ; k++)
            for (int j = 0 ; j < n ; j++)
                c[i][j] += a[i][k]*b[k][j];
    }
}
//------------------------------------------------------------------
// Main Program
//------------------------------------------------------------------
int main(int argc, char *argv[])
{
	double **a,**b,**c;
	int	n,isPrint,version;
	double runtime;

	if (GetUserInput(argc,argv,n,version,isPrint)==false) return 1;

	//Initialize the value of matrix a, b, c
	InitializeMatrix(a,n,1.0);
	InitializeMatrix(b,n,1.0);
	InitializeMatrix(c,n,0.0);

	//Print the input maxtrices
	if (isPrint==1)
	{
		cout<< "Matrix a[n][n]:" << endl;
		PrintMatrix(a,n); 
		cout<< "Matrix b[n][n]:" << endl;
		PrintMatrix(b,n); 
	}

    
    
    
    //omp_set_dynamic(0);        // Explicitly disable dynamic teams
    //omp_set_num_threads(24);   // Use 24 threads for all consecutive parallel regions
    const int nt=omp_get_max_threads();
    
    printf("I have %ld logical cores.\n", sysconf(_SC_NPROCESSORS_ONLN ));
    printf("OpenMP with %d threads\n", nt);
    
	runtime = omp_get_wtime();

	//Do Matrix Multiplication
	switch (version)
	{
		case 1: SequentialMatrixMultiplication_Version1(a,b,c,n);
				break;
		case 2: SequentialMatrixMultiplication_Version2(a,b,c,n);
				break;
		case 3: ParallelMatrixMultiplication_Version3(a,b,c,n);
				break;
		case 4: ParallelMatrixMultiplication_Version4(a,b,c,n);
				break;
	}
	runtime = omp_get_wtime() - runtime;

	//The matrix is as below:
	if (isPrint==1)
	{
		cout<< "Output matrix:" << endl;
		PrintMatrix(c,n); 
	}
	cout<< "Program runs in "	<< setiosflags(ios::fixed) << setprecision(2)  << runtime << " seconds\n"; 
	
	DeleteMatrix(a,n);	
	DeleteMatrix(b,n);	
	DeleteMatrix(c,n);	
	return 0;
}
