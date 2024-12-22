#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <cmath>
#define IS_ROOT (rank == 0)

int TestForPrime(int val)
{
	int limit, factor = 3;

	limit = (long)(sqrtf((float)val)+0.5f);
	while( (factor <= limit) && (val % factor))
		factor ++;

	return (factor > limit);
}


#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))



int main(int argc, char *argv[]) {

	MPI_Init(NULL, NULL);
	int rank, world_size;
	MPI_Comm_size(MPI_COMM_WORLD, &world_size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	int n = (1<<11)+1;
	int start = 3;
	int end = n;
	int *Primes; 
	MPI_Status status;

	MPI_Barrier(MPI_COMM_WORLD); 

	if (world_size==1) {
		int PrimesFound = 0;
		Primes=(int*) malloc(n/4*sizeof(int));//sollte genug Platz für alle sein
		for( int i = start; i <= end; i += 2 )
		{
			if( TestForPrime(i) )
			{
				Primes[PrimesFound++] = i; 
			}
		}
		printf("Found %d Prime numbers \n", PrimesFound);
	}
	else{

	}
	// 

	MPI_Finalize();

	return 0;
}

